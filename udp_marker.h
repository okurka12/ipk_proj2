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



#endif  // ifndef _U_D_P_M_A_R_K_E_R_H_
