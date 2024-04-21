/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**     IPK2     **
**     2024     **
*****************/

#define _POSIX_C_SOURCE 200809L  // for strdup

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <arpa/inet.h>  // inet_ntoa
#include <netinet/in.h>  // struct sockaddr_in
#include <unistd.h>  // close
#include <string.h>
#include "client.h"
#include "utils.h"

/* buffer for the whole tcp payload */
const unsigned int tcp_bufsize = 1048576;

/* buffer to copy a single message from the payload into */
const unsigned int tcp_single_msg_bufsize = 2048;


struct client *client_ctor(
    int sockfd, enum tproto protocol, struct sockaddr_in *addr
) {
    struct client *c = malloc(sizeof(struct client));
    if (c == NULL) return NULL;
    c->address = strdup(inet_ntoa(addr->sin_addr));
    if (c->address == NULL) {
        log(ERROR, MEMFAIL_MSG);
        return NULL;
    }
    c->port = ntohs(addr->sin_port);
    c->authenticated = false;
    c->dname = strdup("");
    if (c->dname == NULL) {
        log(ERROR, MEMFAIL_MSG);
        return NULL;
    }
    c->msgcount = 0;
    c->sockfd = sockfd;
    c->tproto = protocol;
    c->active = true;
    c->todo = 0; // todo
    c->tcp_incomplete_buf = strdup("");
    if (c->tcp_incomplete_buf == NULL) {
        log(ERROR, MEMFAIL_MSG);
        return NULL;
    }
    logf(DEBUG, "%s:%hu connected (tcp)", c->address, c->port);
    return c;
}

static int client_recv_tcp(struct client *client) {

    char *buf_whole = calloc(tcp_bufsize, 1);
    char *buf_single = calloc(tcp_single_msg_bufsize, 1);
    if (buf_whole == NULL or buf_single == NULL) {
        log(ERROR, MEMFAIL_MSG);
        return 1;
    }
    unsigned int offset = 0;

    /* did we receive incomplete message the last time? */
    if (strlen(client->tcp_incomplete_buf) > 0) {

        /* copy it to the main buffer and call recv with offset */
        strncpy(buf_whole, client->tcp_incomplete_buf, tcp_bufsize);
        offset = strlen(client->tcp_incomplete_buf);

        /* now we can get rid of the incomplete buffer */
        free(client->tcp_incomplete_buf);
        client->tcp_incomplete_buf = strdup("");
        if (client->tcp_incomplete_buf == NULL) {
            log(ERROR, MEMFAIL_MSG);
            return 1;
        }
    }

    int received_bytes =
        recv(client->sockfd, buf_whole + offset, tcp_bufsize - offset, 0);

    if (received_bytes == -1) {
        logf(ERROR, "recv error with %s:%hu", client->address, client->port);
        perror("recv");
        client->active = false;
        return 0;  // return 0 or 1? :hmm
    }
    if (received_bytes == 0) {
        logf(INFO, "orderly shutdown from %s:%hu",
            client->address, client->port);
        shutdown(client->sockfd, SHUT_RDWR);
        close(client->sockfd);
        client->sockfd = -1;
        client->active = false;
        free(buf_whole);
        free(buf_single);
        return 0;
    }

    /* current position in buf_whole */
    char *buf = buf_whole;

    bool done = false;
    while (not done) {
        char *msg_end = strstr(buf, "\r\n");
        if (msg_end == NULL and strlen(buf) > 0) {
            logf(DEBUG, "incomplete message: '%s'", buf);
            free(client->tcp_incomplete_buf);
            client->tcp_incomplete_buf = strdup(buf);
            break;
        }
        if (msg_end == NULL) break;
        unsigned int msg_len = msg_end - buf;
        memcpy(buf_single, buf, msg_len);  // + 2 ?
        logf(DEBUG, "Processing message: '%s'", buf_single);
        /* todo: processing */
        memset(buf_single, 0, msg_len);
        buf = buf + msg_len + 2;
    }

    free(buf_whole);
    free(buf_single);

    return 0;
}

int client_recv(struct client *client) {
    if (client->tproto == T_TCP) {
        return client_recv_tcp(client);
    } else {
        return 1; /* not implemented (todo) */
    }
}


int client_send(struct client *client, int message, bool auth);


void client_dtor(struct client **client) {
    /* dont close the socket here, sockdata_dtor will do that */
    if (*client == NULL) return;
    struct client *c = *client;
    free(c->address);
    free(c->dname);
    free(c->tcp_incomplete_buf);
    free(c);
    *client = NULL;
}
