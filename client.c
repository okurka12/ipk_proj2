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
#include "tcp_parse.h"
#include "iota.h"
#include "tcp_render.h"
#include "server.h"  // broadcast
#include "udp_render.h"
#include "udp_parse.h"
#include "udp_marker.h"

/* buffer for the whole tcp payload */
const unsigned int tcp_bufsize = 1048576;

/* buffer for the whole udp payload */
const unsigned int udp_bufsize = 70000;

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
    c->channel = strdup(DEFAULT_CHANNEL);
    c->udpm_data = udpm_ctor();
    if (c->tcp_incomplete_buf == NULL or c->channel == NULL or
        c->udpm_data == NULL) {
        log(ERROR, MEMFAIL_MSG);
        return NULL;
    }

    logf(DEBUG, "%s:%hu connected (tcp)", c->address, c->port);
    return c;
}

static void client_set_inactive(struct client *client) {
    if (not client->active) return;

    shutdown(client->sockfd, SHUT_RDWR);
    close(client->sockfd);
    client->sockfd = -1;
    client->active = false;
    server_broadcast_leave(client->dname, client->channel);
    logf(INFO, "%s:%hu (%s) disconnected", client->address, client->port,
        client->dname);
}

/**
 * return true if the user's credentials are valid
*/
bool authenticate_user(const char *username, const char *secret) {
    logf(DEBUG, "authenticating user %s with secret=%s", username, secret);
    return true;
}

/**
 * Process a single message from client that we already know is complete
 * (ends with CRLF)
*/
static int client_process_message(struct client *client, char *message) {
    bool err = false;

    msg_t *msg = NULL;
    if (client->tproto == T_TCP) {
        msg = tcp_parse_any(message, &err);
        if (msg == NULL and err) {
            log(ERROR, "couldn't parse message due to internal error");
            return 1;
        }
    } else {
        msg = udp_parse(message);
        if (msg == NULL) {
            log(ERROR, "something went wrong");
            return 1;
        }
    }

    msg_t bye_msg = { .type = MTYPE_BYE };
    msg_t parse_err_msg = {
        .type = MTYPE_ERR,
        .dname = SDNAME,
        .content = "couldn't parse your message"
    };

    /* this code should be ok for both TCP and UDP */
    switch (msg->type)
    {

    case MTYPE_CONFIRM:
        /* todo */
        break;

    case MTYPE_MSG:
        printf("RECV %s:%hu | MSG DisplayName=%s Content=%s\n",
            client->address, client->port, msg->dname, msg->content);
        /* todo: broadcast it */
        server_broadcast(msg, client->channel, client->sockfd);

        /* update client displayname */
        free(client->dname);
        client->dname = msg->dname;
        msg->dname = NULL;  // so its not freed by msg_dtor
        break;

    case MTYPE_AUTH:
        printf("RECV %s:%hu | AUTH Username=%s Displayname=%s Secret=%s\n",
            client->address, client->port, msg->username, msg->dname,
            msg->secret);
        if (authenticate_user(msg->username, msg->secret)) {
            msg_t reply_ok = {
                .type = MTYPE_REPLY,
                .result = 1,
                .content = "auth success"
            };
            client_send(client, &reply_ok, false);
            free(client->dname);
            client->dname = msg->dname;
            msg->dname = NULL;  // so its not freed by msg_dtor
            client->authenticated = true;
            server_broadcast_join(
                client->dname, DEFAULT_CHANNEL, client->sockfd);
        }
        break;

    case MTYPE_JOIN:;  // supress gcc warning with the :;

        /* send reply ok */
        msg_t reply_ok = {
            .type = MTYPE_REPLY,
            .result = 1,
            .content = "join success",
            .ref_msgid = msg->id
        };
        client_send(client, &reply_ok, false);

        /* change client channel, broadcast leave */
        char *original_channel = client->channel;
        client->channel = msg->chid;
        msg->chid = NULL;  // so its not freed by msg_dtor
        server_broadcast_leave(client->dname, original_channel);
        free(original_channel);
        original_channel = NULL;

        /* change client dname, broadcast join */
        free(client->dname);
        client->dname = msg->dname;
        msg->dname = NULL;  // so its not freed by msg_dtor
        logf(DEBUG, "%s's channel is now %s", client->dname, client->channel);
        server_broadcast_join(client->dname, client->channel, client->sockfd);

        break;

    case MTYPE_BYE:
        printf("RECV %s:%hu | BYE\n", client->address, client->port);
        logf(INFO, "BYE from %s:%hu (%s)", client->address, client->port,
            client->dname);
        client_set_inactive(client);
        break;

    case MTYPE_UNKNOWN:;  // supress gcc warning with the :;
        client_send(client, &parse_err_msg, false);
        client_send(client, &bye_msg, false);
        client_set_inactive(client);
        break;

    case MTYPE_ERR:;
        client_send(client, &bye_msg, false);
        client_set_inactive(client);
        break;

    default:
        logf(WARNING, "unhandled message type 0x%hhx", msg->type);
        break;
    }
    msg_dtor(msg);
    fflush(stdout);
    fflush(stderr);
    return 0;
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
        strncpy(buf_whole, client->tcp_incomplete_buf, tcp_bufsize - 1);
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
        client_set_inactive(client);
        free(buf_whole);
        free(buf_single);
        return 0;
    }

    /* current position in buf_whole */
    char *buf = buf_whole;

    bool done = false;
    while (not done) {
        char *msg_end = strstr(buf, "\r\n");

        /* current message doesnt end with CRLF*/
        if (msg_end == NULL and strlen(buf) > 0) {
            logf(DEBUG, "incomplete message: '%s'", buf);
            free(client->tcp_incomplete_buf);
            client->tcp_incomplete_buf = strdup(buf);
            break;
        }

        /* we reached the end of the buffer */
        if (msg_end == NULL) break;

        /* copy the message to a separate buffer `buf_single` */
        unsigned int msg_len = msg_end - buf;
        memcpy(buf_single, buf, msg_len);  // without the CRLF
        logf(DEBUG, "Processing message: '%s'", buf_single);
        /* main processing function */
        client_process_message(client, buf_single);

        /* zero out buf_single, move `buf` to the start of a new message */
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
        char *buf = malloc(udp_bufsize);
        recv(client->sockfd, buf, udp_bufsize, 0);
        uint8_t type = buf[0];
        uint16_t id = get_id(buf + 1);
        msg_t confirm = { .type = MTYPE_CONFIRM, .ref_msgid = id };
        if (type != MTYPE_CONFIRM) {
            client_send(client, &confirm, false);
        }
        if (not udpm_seen(id, client->udpm_data)) {
            client_process_message(client, buf);
        }
        free(buf);
    }
    return 0;
}


int client_send(struct client *client, const msg_t *msg, bool auth) {
    if (not client->active) {
        logf(DEBUG, "%s:%hu (%s) already disconnected, not sending",
            client->address, client->port, client->dname);
        return 0;
    }
    if (not client->authenticated and auth) {
        logf(DEBUG, "%s:%hu (%s) not yet authenticated, not sending",
            client->address, client->port, client->dname);
        return 0;
    }
    logf(DEBUG, "sending %s to %s:%hu (%s)", mtype_str(msg->type),
        client->address, client->port, client->dname);
    if (client->tproto == T_TCP) {
        char *rendered = tcp_render(msg);
        if (rendered == NULL) {
            log(ERROR, "couldn't render message");
            return 1;
        }
        ssize_t rc = send(client->sockfd, rendered, strlen(rendered), 0);
        free(rendered); rendered = NULL;
        if (rc == -1) {
            log(ERROR, "send error");
            perror("send");
            client_set_inactive(client);
            return 0;
        }
    } else {  // UDP

        /* msg is const but we need to edit the id */
        // msg_t shallow_copy = { .type = msg->type };
        // shallow_copy.chid = msg->chid;
        // shallow_copy.content = msg->content;
        // shallow_copy.dname = msg->dname;
        // shallow_copy.id = msg->id;
        // shallow_copy.ref_msgid = msg->ref_msgid;
        // shallow_copy.result = msg->result;
        // shallow_copy.secret = msg->secret;
        // shallow_copy.username = msg->username;
        msg_t shallow_copy;
        memcpy(&shallow_copy, msg, sizeof(msg_t));

        shallow_copy.id = client->msgcount;
        client->msgcount += 1;

        unsigned int rendered_length = 0;
        char *rendered = udp_render_message(&shallow_copy, &rendered_length);
        struct sockaddr_in addr = { .sin_family = AF_INET };
        addr.sin_port = htons(client->port);
        inet_pton(AF_INET, client->address, &addr.sin_addr);
        sendto(client->sockfd, rendered, rendered_length, 0,
            (struct sockaddr *)&addr, sizeof(addr));
        free(rendered);
    }
    printf("SENT %s:%hu | %s\n", client->address, client->port,
        mtype_str(msg->type));
    fflush(stdout);  // just to be sure

    return 0;
}


void client_dtor(struct client **client) {
    /* dont close the socket here, sockdata_dtor will do that */
    if (*client == NULL) return;
    struct client *c = *client;
    udpm_dtor(&c->udpm_data);
    free(c->address);
    free(c->dname);
    free(c->tcp_incomplete_buf);
    free(c->channel);
    free(c);
    *client = NULL;
}

/******************************************************************************/
/******************************************************************************/

int client_udp_auth(struct client *client, msg_t *authmsg) {
    if (authenticate_user(authmsg->username, authmsg->secret)) {
        msg_t reply_ok = {
            .type = MTYPE_REPLY,
            .result = 1,
            .content = "auth success",
            .ref_msgid = authmsg->id
        };

        client->authenticated = true;

        free(client->dname);
        client->dname = authmsg->dname;
        authmsg->dname = NULL;

        free(client->channel);
        client->channel = strdup(DEFAULT_CHANNEL);
        if (client->channel == NULL) {
            log(ERROR, MEMFAIL_MSG);
            return 1;
        }

        server_broadcast_join(client->dname, client->channel, client->sockfd);

        client_send(client, &reply_ok, false);
    }
    return 0;
}
