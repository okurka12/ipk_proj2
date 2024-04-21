/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**     IPK2     **
**     2024     **
*****************/

#include <stdint.h>
#include <stdbool.h>
#include <arpa/inet.h>  // inet_ntoa
#include <netinet/in.h>  // struct sockaddr_in
#include "mmal.h"
#include "client.h"
#include "utils.h"

struct client *client_ctor(
    int sockfd, enum tproto protocol, struct sockaddr_in *addr
) {
    struct client *c = mmal(sizeof(struct client));
    if (c == NULL) return NULL;
    c->address = mstrdup(inet_ntoa(addr->sin_addr));
    if (c->address == NULL) {
        log(ERROR, MEMFAIL_MSG);
        return NULL;
    }
    c->port = ntohs(addr->sin_port);
    c->authenticated = false;
    c->dname = mstrdup("");
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
    mfree(c->address);
    mfree(c->dname);
    mfree(c);
    *client = NULL;
}
