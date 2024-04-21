/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**     IPK2     **
**     2024     **
*****************/

#include <stdint.h>
#include <stdbool.h>
// #include <sys/socket.h>  // struct sockaddr
#include <netinet/in.h>  // struct sockaddr_in


#ifndef _C_L_I_E_N_T_H_
#define _C_L_I_E_N_T_H_

/* transport protocol for the client */
enum tproto {
    T_TCP,
    T_UDP
};

/* all necessary information for the client */
struct client {
    int sockfd;
    char *address;
    enum tproto tproto;
    uint16_t port;
    unsigned int msgcount;
    char *dname;
    bool authenticated;
    bool active;

    /* queue for unconfirmed udp messages*/
    int todo;
};

/**
 * Allocates space for the client data and initializes them
 * @param protocol T_TCP or T_UDP
 * @returns pointer to the client structure or NULL on failure
*/
struct client *client_ctor(int sockfd, enum tproto protocol,
    struct sockaddr_in *addr);

/**
 * performs recv on the client's socket and performs necessary actions
 * @return 0 on success, else 1
*/
int client_recv(struct client *client);


/**
 * sends a message to the client
 * if `auth` is true, send a message only if client is authenticated, else
 * do nothing
 * @return 0 on success, else 1
*/
int client_send(struct client *client, int message, bool auth);


/**
 * frees all the client data
*/
void client_dtor(struct client **client);



#endif  // ifndef _C_L_I_E_N_T_H_
