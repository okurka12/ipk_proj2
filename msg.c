/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**              **
**   Created:   **
**  2024-03-04  **
*****************/

#include <stdlib.h>
#include "msg.h"
#include "utils.h"

msg_t *msg_ctor(void) {
    msg_t *p = malloc(sizeof(msg_t));
    if (p == NULL) { log(ERROR, MEMFAIL_MSG); return NULL; }

    p->id = 0;
    p->ref_msgid = 0;
    p->result = 0;
    p->type = 0;

    /* char * */
    p->secret = NULL;
    p->username = NULL;
    p->chid = NULL;
    p->content = NULL;
    p->dname = NULL;

    return p;
}


void msg_dtor(msg_t *p) {
    if (p == NULL) {
        return;
    }
    if (p->secret != NULL) free(p->secret);
    if (p->username != NULL) free(p->username);
    if (p->chid != NULL) free(p->chid);
    if (p->content != NULL) free(p->content);
    if (p->dname != NULL) free(p->dname);
    free(p);
}

const char *mtype_str(uint8_t mtype) {
    switch (mtype) {
        case MTYPE_CONFIRM: return "CONFIRM";
        case MTYPE_REPLY:   return "REPLY";
        case MTYPE_AUTH:    return "AUTH";
        case MTYPE_JOIN:    return "JOIN";
        case MTYPE_MSG:     return "MSG";
        case MTYPE_ERR:     return "ERR";
        case MTYPE_BYE:     return "BYE";
    }
    return "unknown";
}
