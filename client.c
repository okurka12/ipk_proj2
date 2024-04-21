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
#include "client.h"
#include "utils.h"

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
    c->todo = 0; // todo
    logf(DEBUG, "%s:%hu connected (tcp)", c->address, c->port);
    return c;
}

int client_recv(struct client *client);


int client_send(struct client *client, int message, bool auth);


void client_dtor(struct client **client) {
    if (*client == NULL) return;
    struct client *c = *client;
    free(c->address);
    free(c->dname);
    free(c);
    *client = NULL;
}
