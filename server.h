/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**     IPK2     **
**     2024     **
*****************/

#include "argparse.h"  // struct args
#include "client.h"

#ifndef _S_E_R_V_E_R_H_
#define _S_E_R_V_E_R_H_

/**
 * a structure for both socket and client data
 * it's useful because we need to identify welcome sockets
 * which represent no clients
 */
struct sockdata {
    int fd;
    struct client *data;
};

/**
 * allocates + initializes struct sockdata
*/
struct sockdata *sockdata_ctor(int fd, struct client *data);

/**
 * frees struct sockdata + sets the pointer to null
*/
void sockdata_dtor(struct sockdata **sockdata);

/**
 * open sockets, bind them, start accepting clients
 * @return 0 on success, 1 on failure
*/
int start_server(struct args *args);

/**
 * broadcasts `msg` to all clients in `channel`, except the client
 * with file descriptor `whence`
*/
void server_broadcast(const msg_t *msg, const char *channel, int whence);

/**
 * broadcasts that `dname` joined to `channel` to all clients in `channel`
 *
 * if BROAD macro is defined, broadcasts to all clients in channel,
 * including client with file descriptor `whence`
*/
void server_broadcast_join(const char *dname, const char *channel, int whence);


#endif  // ifndef _S_E_R_V_E_R_H_
