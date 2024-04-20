/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**     IPK2     **
**     2024     **
*****************/

#include <stdint.h>
#include <stdbool.h>

#ifndef _A_R_G_P_A_R_S_E_H_
#define _A_R_G_P_A_R_S_E_H_


struct args {

    /* listening ip address */
    char *laddr;

    uint16_t port;

    uint16_t udp_timeout;

    uint8_t udp_retransmissions;

    bool help;

};

/**
 * parse arguments or fill out the defaults
 * @return 0 on success else 1
 * @note args->laddr needs to be freed using `stdlib.h`'s `free`
*/
int parse_arguments(int argc, char **argv, struct args *args);

/**
 * frees the contents of struct args
*/
void free_argstruct(struct args *args);

#endif  // ifndef _A_R_G_P_A_R_S_E_H_
