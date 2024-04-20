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

#include "server.h"
#include "utils.h"


static int create_udp_welcome_socket() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        log(ERROR, "couldn't create socket");
        perror("socket");
    }
    return sockfd;
}
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

/**
 * @return 0 on success, 1 on error
*/
static int add_sock_to_epoll(int sockfd, int epollfd) {
    struct epoll_event sock_event = { .events = EPOLLIN };
    int rc = epoll_ctl(epollfd, EPOLL_CTL_ADD, sockfd, &sock_event);
    if (rc == -1) {
        log(ERROR, "couldn't add socket to epoll");
        perror("epoll_ctl");
        return 1;
    }
    return 0;
}

int start_server(struct args *args) {

    logf(INFO, "starting server at %s:%hu", args->laddr, args->port);

    int rc;

    /* create and bind sockets, create epoll */
    int udp_sock = create_udp_welcome_socket();
    int tcp_sock = create_tcp_welcome_socket();
    if (udp_sock == -1 or tcp_sock == -1) return 1;
    rc = bind_socket(udp_sock, args);
    if (rc == 1) return 1;
    rc = bind_socket(tcp_sock, args);
    if (rc == 1) return 1;
    int epollfd = create_epoll();

    /* add sockets to epoll */
    rc = add_sock_to_epoll(udp_sock, epollfd);
    if (rc == 1) return 1;
    rc = add_sock_to_epoll(tcp_sock, epollfd);
    if (rc == 1) return 1;

    struct epoll_event events[1];

    int timeout_ms = args->udp_timeout;

    while (true) {
        rc = epoll_wait(epollfd, events, 1, timeout_ms);
        if (rc == -1) {
            log(ERROR, "error with epoll_wait");
            perror("epoll_wait");
        }
        if (rc == 0) {
            log(DEBUG, "epoll_waid timed out");
        }
        if (rc == 1) {
            log(DEBUG, "socket event");
        }
    }


    return 0;
}
