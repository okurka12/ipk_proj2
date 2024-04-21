/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**     IPK2     **
**     2024     **
*****************/

/**
 * a module to keep track of clients in an array
*/

#include "server.h"  // struct sockdata

#ifndef _C_L_I_E_N_T_L_I_S_T_H_
#define _C_L_I_E_N_T_L_I_S_T_H_

/**
 * add a client to the client list
 * @return 0 on success else 1
*/
int clist_add(struct sockdata *client);

/**
 * return an array of all the clients (of length n)
 * The array will have empty slots represented by NULL pointers
*/
struct sockdata **clist_get_arr(unsigned int *n);

/**
 * remove client from the client list (set its position in the client array
 * to NULL), free data accompanying data structures
*/
void clist_remove(int sockfd);

/**
 * Free internal data structures
*/
void clist_free();

#endif  // ifndef _C_L_I_E_N_T_L_I_S_T_H_
