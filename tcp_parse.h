/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**              **
**   Created:   **
**  2024-03-21  **
*****************/

/**
 * a module for parsing TCP IPK24CHAT data
 * extended module from the first project
*/

#include <stdbool.h>
#include "msg.h"

#ifndef _T_C_P_P_A_R_S_E_H_
#define _T_C_P_P_A_R_S_E_H_

// enum parse_result {

//     /* message was a REPLY message with result=success */
//     PR_REPLY_OK,

//     /* message was a REPLY message with result=failure */
//     PR_REPLY_NOK,

//     /* message was a MSG message*/
//     PR_MSG,

//     /* message was an ERR message */
//     PR_ERR,

//     /* message was a BYE message */
//     PR_BYE,

//     /* couldn't match the message to any of the types expected from server */
//     PR_UNKNOWN,

//     /* internal error while parsing */
//     PR_ERR_INTERNAL

// };

/**
 * Parses reply
 * @return reply success, sets `content` to the position in `data` to where
 * the content starts, if the message is invalid, sets
 * `content` to NULL and returns false, if an internal error occurs, sets
 * `content` to NULL, returns false and sets `err` to true
 * @note it also changes the CR character in `data` to NULL
*/
bool tcp_parse_reply(char *data, char **content, bool *err);

/**
 * parses any message from server and prints it to the corresponding
 * output
 * expects `data` without trailing CRLF
 * @return corresponding msg_t structure, or NULL if message was invalid
 * or NULL if an internal error occured, in which case sets `err` to true
 * @note it modifies the data buffer for its own purposes (it adds
 * zero bytes to separate the fields)
*/
msg_t *tcp_parse_any(char *data, bool *err);

void tcp_parse_free_resources();

#endif  // ifndef _T_C_P_P_A_R_S_E_H_
