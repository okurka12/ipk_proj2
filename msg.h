/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**              **
**   Created:   **
**  2024-03-04  **
*****************/

/**
 * This module helps create and destroy message structures (msg_t)
 * module taken from the first project
*/

#include <stdint.h>

#ifndef _M_S_G_H_
#define _M_S_G_H_

#define MTYPE_CONFIRM 0x00
#define MTYPE_REPLY	  0x01
#define MTYPE_AUTH	  0x02
#define MTYPE_JOIN	  0x03
#define MTYPE_MSG	  0x04
#define MTYPE_UNKNOWN 0xEE
#define MTYPE_ERR	  0xFE
#define MTYPE_BYE	  0xFF

/* message struct */
typedef struct {

    /* message type (CONFIRM, REPLY, AUTH, JOIN, MSG, ERR, BYE) */
    uint8_t type;

    /* message id */
    uint16_t id;

    /* Username (for type AUTH) */
    char *username;

    /* DisplayName (for type AUTH, JOIN, MSG, ERR) */
    char *dname;

    /* Channel ID (for type JOIN) */
    char *chid;

    /* Secret (for type AUTH) */
    char *secret;

    /* MessageContents (for type REPLY, MSG, ERR) */
    char *content;

    /* Ref_MessageID (for type CONFIRM, REPLY)*/
    uint16_t ref_msgid;

    /* Result (for type REPLY) */
    uint8_t result;

} msg_t;


/**
 * allocates + initializes a msg_t structure on heap with all blank fields,
 * meaning char pointers will be null and integer fields will be zero
 * @note needs to be freed using `msg_dtor`
 * @return pointer to allocated blang msg_t structure or NULL on failure
*/
msg_t *msg_ctor(void);

/**
 * frees `p` and also frees all non-null char pointer fields
*/
void msg_dtor(msg_t *p);

/**
 * Returns a string representing the message type if it exists, else an
 * empty string
 *
 * Example: `mtype_str(0x04)` returns `"MSG"`
*/
const char *mtype_str(uint8_t mtype);

#endif  // ifndef _M_S_G_H_
