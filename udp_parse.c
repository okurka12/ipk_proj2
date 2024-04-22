/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**     IPK2     **
**     2024     **
*****************/

#define _POSIX_C_SOURCE 200809L  // for strdup

#include <stdint.h>
#include <arpa/inet.h>  // htons, ntohs
#include <string.h>
#include <stdlib.h>
#include "udp_parse.h"
#include "msg.h"
#include "utils.h"

/* check fields validity with the tcp parse regular expressions */
#include "tcp_parse.h"
#include "tcp_render.h"

#define check_null(ppttrr) \
do { \
    if ((ppttrr) == NULL) { log(ERROR, MEMFAIL_MSG); return NULL; } \
} while (0)

uint16_t get_id(char *src) {
    return ntohs(* ((uint16_t *)(src)));
}

msg_t *udp_parse(char *buf) {

    msg_t *msg = msg_ctor();
    check_null(msg);
    msg->type = MTYPE_UNKNOWN;

    uint8_t type = buf[0];
    uint16_t id = get_id(buf + 1);

    size_t dname_len = 0;
    size_t username_len = 0;
    size_t chid_len = 0;

    /* lets say the server doesnt know message type REPLY */
    switch (type)
    {
    case MTYPE_CONFIRM:
        msg->type = MTYPE_CONFIRM;
        msg->ref_msgid = id;
        break;
    case MTYPE_AUTH:
        msg->type = MTYPE_AUTH;
        msg->id = id;
        msg->username = strdup(buf + 3);
        check_null(msg->username);
        username_len = strlen(msg->username);
        msg->dname = strdup(buf + 3 + username_len + 1);
        check_null(msg->dname);
        dname_len = strlen(msg->dname);
        msg->secret = strdup(buf + 3 + username_len + 1 + dname_len + 1);
        check_null(msg->secret);
        break;
    case MTYPE_ERR: case MTYPE_MSG:
        msg->type = type;
        msg->id = id;
        msg->dname = strdup(buf + 3);
        check_null(msg->dname);
        dname_len = strlen(msg->dname);
        msg->content = strdup(buf + 3 + dname_len + 1);
        check_null(msg->content);
        break;
    case MTYPE_JOIN:
        msg->type = MTYPE_JOIN;
        msg->id = id;
        msg->chid = strdup(buf + 3);
        check_null(msg->chid);
        chid_len = strlen(msg->chid);
        msg->dname = strdup(buf + 3 + chid_len + 1);
        check_null(msg->dname);
        break;
    case MTYPE_BYE:
        msg->type = MTYPE_BYE;
        break;

    default:
        logf(WARNING, "unknown message type %hhx", type);
        break;
    }

    if (msg->type == MTYPE_UNKNOWN) {
        return msg;
    }

    char *rendered = NULL;
    msg_t *parsed_rendered_msg = NULL;
    if (msg->type != MTYPE_CONFIRM) {
        rendered = tcp_render(msg);
        check_null(rendered);
        size_t rendered_len = strlen(rendered);
        rendered[rendered_len - 2] = '\0';  // strip CRLF
        bool err = false;
        parsed_rendered_msg = tcp_parse_any(rendered, &err);
        if (parsed_rendered_msg == NULL and err) {
            log(ERROR, "internal error in tcp_parse");
            return NULL;
        }
        if (parsed_rendered_msg->type == MTYPE_UNKNOWN) {
            msg->type = MTYPE_UNKNOWN;
        }
    }

    logf(DEBUG, "parsed %s", mtype_str(msg->type));
    logf(DEBUG, "    id %hu", msg->id);
    logf(DEBUG, "    refid %hu", msg->ref_msgid);
    logf(DEBUG, "    dname %s", msg->dname);
    logf(DEBUG, "    username %s", msg->username);
    logf(DEBUG, "    chid %s", msg->chid);
    logf(DEBUG, "    content %s", msg->content);
    logf(DEBUG, "    secret %s", msg->secret);

    free(rendered);
    msg_dtor(parsed_rendered_msg);
    return msg;
}
