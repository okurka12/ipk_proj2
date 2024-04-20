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
 * open sockets, bind them, start accepting clients
 * @return 0 on success, 1 on failure
*/
int start_server(struct args *args);

#endif  // ifndef _S_E_R_V_E_R_H_
