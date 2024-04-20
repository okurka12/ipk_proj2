/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**     IPK2     **
**     2024     **
*****************/

#include "argparse.h"  // struct args

#ifndef _S_E_R_V_E_R_H_
#define _S_E_R_V_E_R_H_

/**
 * a structure for both socket and client data
 * it's useful because we need to identify welcome sockets
 * which represent no clients
 */
struct sockdata {
    int fd;
    void *data;
};

/**
 * allocates + initializes struct sockdata
*/
struct sockdata *sockdata_ctor(int fd, void *data);

/**
 * frees struct sockdata + sets the pointer to null
*/
void sockdata_dtor(struct sockdata **sockdata);

/**
 * open sockets, bind them, start accepting clients
 * @return 0 on success, 1 on failure
*/
int start_server(struct args *args);

#endif  // ifndef _S_E_R_V_E_R_H_
