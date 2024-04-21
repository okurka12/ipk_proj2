/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**     IPK2     **
**     2024     **
*****************/

#include <arpa/inet.h>  // inet_pton, htons
#include <netinet/in.h>  // struct sockaddr_in
#include <sys/epoll.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdlib.h>  // malloc
#include <unistd.h>  // close

#include "server.h"
#include "utils.h"
#include "client.h"
#include "clientlist.h"

static int epollfd = -1;
static int add_sock_to_epoll(int sockfd, int epollfd, void *p);


void server_broadcast_join(
    const char *dname, const char *channel, int whence
) {

    /* should we send join broadcast to the client who joined? */
    #ifdef BROAD
    int from_who = -1;  // no client has this sockfd - send to all
    (void)whence;
    #else  // ifdef BROAD
    int from_who = whence;
    #endif  // ifdef BROAD

    char content[100];
    snprintf(content, 100, "%s joined %s.", dname, channel);
    msg_t msg = { .type = MTYPE_MSG, .dname = "Server", .content = content };
    server_broadcast(&msg, channel, from_who);

}


void server_broadcast(const msg_t *msg, const char *channel, int whence) {

    unsigned int len = 0;
    struct sockdata **clients = clist_get_arr(&len);
    logf(DEBUG, "cleints %p", (void *)clients);
    logf(DEBUG, "whence %d", whence);
    for (unsigned int i = 0; i < len; i++) {
        if (clients[i] == NULL) continue;
        if (clients[i]->data == NULL) continue;  // it is a welcome socket

        logf(DEBUG, "cleints[%u]=%p", i, (void *)clients[i]);
        logf(DEBUG, "cleints[%u]->fd=%d", i, clients[i]->fd);
        logf(DEBUG, "cleints[%u]->data=%p", i, (void *)clients[i]->data);
        logf(DEBUG, "cleints[%u]->data->channel=%p", i, (void *)clients[i]->data->channel);

        bool channel_matches = are_equal(clients[i]->data->channel, channel);
        if (clients[i]->fd != whence and channel_matches){
            client_send(clients[i]->data, msg, true);
        }
    }
}

/**
 * always returns false until it's called with true,
 * then it always returns true
*/
static bool stopper(bool stop) {
    static bool output = false;
    if (stop) {
        output = true;
    }
    return output;
}

/**
 * SIGINT handler to register
*/
static void sigint_handler(int sig) {
    (void)sig;
    stopper(true);
}

/**
 * accepts the new client and adds it to epoll, (it will eventually add him
 * to some list of clients too)
 * @return 0 on success else 1
*/
static int handle_tcp_welcome_socket(int tcpsock) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);  // unsigned int
    int clientfd;
    clientfd = accept(tcpsock, (struct sockaddr *)&addr, &addrlen);
    if (clientfd == -1) {
        log(ERROR, "error with accept");
        perror("accept");
        return 1;
    }
    struct client *client_data = client_ctor(clientfd, T_TCP, &addr);
    if (client_data == NULL) {
        log(ERROR, MEMFAIL_MSG);
        return 1;
    }
    int rc = add_sock_to_epoll(clientfd, epollfd, (void *)client_data);
    return rc;
}

/**
 * @return socket file descriptor on success, -1 on error
*/
static int create_udp_welcome_socket() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        log(ERROR, "couldn't create socket");
        perror("socket");
    }
    return sockfd;
}

/**
 * @return socket file descriptor on success, -1 on error
*/
static int create_tcp_welcome_socket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        log(ERROR, "couldn't create socket");
        perror("socket");
    }
    return sockfd;
}

/**
 * binds `sockfd` to the address and port int `args`
 * @return 0 on success, 1 on error
*/
static int bind_socket(int sockfd, struct args *args) {

    struct sockaddr_in bind_addr = { .sin_family = AF_INET };
    bind_addr.sin_port = htons(args->port);

    int rc = inet_pton(AF_INET, args->laddr, &bind_addr.sin_addr);
    if (rc < 1) {
        logf(ERROR, "invalid listening address %s", args->laddr);
        perror("inet_pton");
        return 1;
    }

    rc = bind(sockfd, (const struct sockaddr *)&bind_addr, sizeof(bind_addr));
    if (rc == -1) {
        log(ERROR, "couldn't bind");
        perror("bind");
        return 1;
    }
    return sockfd;
}

/**
 * @return epoll fd on success, -1 on error
*/
static int create_epoll() {
    int epollfd = epoll_create1(0);
    if (epollfd == -1) {
        log(ERROR, "couldn't create epoll instance");
        perror("epoll_create1");
    }
    return epollfd;
}

struct sockdata *sockdata_ctor(int fd, struct client *data) {
    struct sockdata *s = malloc(sizeof(struct sockdata));
    s->data = data;
    s->fd = fd;
    return s;
}

void sockdata_dtor(struct sockdata **sockdata) {
    if (sockdata == NULL) return;
    if (*sockdata == NULL) return;
    struct sockdata *sd = *sockdata;

    /* if sd->data->sockfd is -1, that means it was closed in the client 7
    module when recv returned 0 */
    if (sd->fd != -1 and sd->data != NULL and sd->data->sockfd != -1) {
        close(sd->fd);
    }
    client_dtor(&sd->data);
    free(sd);
    *sockdata = NULL;
}

/**
 * Adds `sockfd` to `epollfd` and puts pointer to `struct sockdata`
 * (defined in `server.h`) to the `event->data->ptr` field. This struct
 * will contain `sockfd` and `p`. It also adds te struct to the clientlist
 * module
 * @return 0 on success, 1 on error
*/
static int add_sock_to_epoll(int sockfd, int epollfd, void *p) {
    logf(DEBUG, "adding %d to epoll", sockfd);

    struct sockdata *sockdata = sockdata_ctor(sockfd, p);

    struct epoll_event sock_event = {
        .events = EPOLLIN,
        .data.ptr = sockdata
    };


    int rc = epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &sock_event);
    if (rc == -1) {
        log(ERROR, "couldn't add socket to epoll");
        perror("epoll_ctl");
        return 1;
    }

    rc = clist_add(sockdata);
    if (rc == 1) {
        return 1;
    }

    return 0;
}

/**
 * return 0 on success else 1
*/
static int handle_socket_event(struct epoll_event *event, int tcpfd, int udpfd) {

    /* what would life be without a little of type unsafety */
    int eventfd = ((struct sockdata *)event->data.ptr)->fd;

    if (event->events & EPOLLERR) {
        log(ERROR, "EPOLLERR event");
        return 1;
    }

    if (eventfd == tcpfd) {
        log(DEBUG, "tcp welcome socket event");
        handle_tcp_welcome_socket(tcpfd);

    } else if (eventfd == udpfd) {
        log(DEBUG, "udp welcome socket event");
        /* todo: recvfrom here */
    } else {
        client_recv(((struct sockdata *)event->data.ptr)->data);
    }

    return 0;
}

int start_server(struct args *args) {

    logf(INFO, "starting server at %s:%hu", args->laddr, args->port);

    int rc;

    if (signal(SIGINT, sigint_handler) == SIG_ERR) {
        log(ERROR, "couldn't register signal handler");
        return 1;
    }

    /* create and bind sockets, create epoll */
    int udp_sock = create_udp_welcome_socket();
    int tcp_sock = create_tcp_welcome_socket();
    if (udp_sock == -1 or tcp_sock == -1) return 1;
    rc = bind_socket(udp_sock, args);
    if (rc == 1) return 1;
    rc = bind_socket(tcp_sock, args);
    if (rc == 1) return 1;
    epollfd = create_epoll();  // global variable

    /* add sockets to epoll */
    rc = add_sock_to_epoll(udp_sock, epollfd, NULL);
    if (rc == 1) return 1;
    rc = add_sock_to_epoll(tcp_sock, epollfd, NULL);
    if (rc == 1) return 1;

    rc = listen(tcp_sock, INT32_MAX);
    if (rc == -1) {
        log(ERROR, "error with listen");
        perror("listen");
        return 1;
    }

    struct epoll_event events[1];

    int timeout_ms = args->udp_timeout;

    while (true) {
        rc = epoll_wait(epollfd, events, 1, timeout_ms);
        if (rc == -1) {
            log(ERROR, "error with epoll_wait");
            perror("epoll_wait");
        }
        if (rc == 0) {
            // log(DEBUG, "epoll_waid timed out");
        }
        if (rc == 1) {
            log(DEBUG, "socket event");
            rc = handle_socket_event(events, tcp_sock, udp_sock);
            if (rc == 1) return 1;
        }
        /* something to do whether it was a socket event or timeout */
        if (stopper(false)) {
            log(INFO, "server done");
            break;
        }
        // getchar();
    }

    clist_remove(udp_sock);
    close(udp_sock);
    clist_remove(tcp_sock);
    close(tcp_sock);

    /* remove client structures one by one (possibly do another actions?) */
    unsigned int len = 0;
    struct sockdata **carr = clist_get_arr(&len);
    for (unsigned int i = 0; i < len; i++) {
        if (carr[i] != NULL) {
            clist_remove(carr[i]->fd);
        }
    }

    close(epollfd);
    clist_free();

    return rc;
}
