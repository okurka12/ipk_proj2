/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**     IPK2     **
**     2024     **
*****************/

/**
 * marks message ids as seen (max 65535 or 0xffff messages)
*/

#include <stdint.h>
#include <stdbool.h>


#ifndef _U_D_P_M_A_R_K_E_R_H_
#define _U_D_P_M_A_R_K_E_R_H_

#define UDPM_MAX 0xffff

/**
 * return pointer to array (operate with udpm_mark or udpm_seen)
*/
int32_t *udpm_ctor();

void udpm_mark(uint16_t id, int32_t *udpm_data);

bool udpm_seen(uint16_t id, int32_t *udpm_data);

/**
 * frees the data and sets the pointer to null
*/
void udpm_dtor(int32_t **udpm_data);

/**
 * the server welcome socket needs to differentiate between ids from different
 * addresses. Hence, it will use this function to mark `id` from `address`
 * and `port` as seen
 *
 * That way, it can receive AUTH message id=0 from 127.0.0.1:12345
 * but also AUTH message id=0 from 127.0.0.1:34343
 *
 * @note this function has static data, so there can be only one instance
 * of seen data, it will grow as the server operates and accepts udp clients
 *
 * @note caller has no way of knowing if the function completed successfully,
 * but given that not much will happen if it doesn't, it's a risk im
 * willing to take
*/
void udpm_welcome_mark(uint16_t id, uint32_t addr, uint16_t port);

bool udpm_welcome_seen(uint16_t id, uint32_t addr, uint16_t port);

void udpm_welcome_free_res();



#endif  // ifndef _U_D_P_M_A_R_K_E_R_H_
