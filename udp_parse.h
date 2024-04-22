/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**     IPK2     **
**     2024     **
*****************/

#include <stdint.h>
#include "msg.h"

#ifndef _U_D_P_P_A_R_S_E_H_
#define _U_D_P_P_A_R_S_E_H_

/**
 * Parses UDP message, expects that the buffer is large enough (larger than
 * the message itself) and that its contens after the message are zeroed
 * @return msg_t * or NULL on failure
*/
msg_t *udp_parse(char *buf);

/**
 * reads two bytes of char and returns ntohs'd id
*/
uint16_t get_id(char *src);

#endif  // ifndef _U_D_P_P_A_R_S_E_H_
