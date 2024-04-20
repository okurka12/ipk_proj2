/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**     IPK2     **
**     2024     **
*****************/

/**
 * argparse.c - implementation of argparse.h
*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <unistd.h>  // getopt
#include <getopt.h>
extern char *optarg;

#include "argparse.h"
#include "mmal.h"
#include "utils.h"

int parse_arguments(int argc, char **argv, struct args *args) {

    int rc = 0;

    args->laddr = mstrdup("0.0.0.0");
    if (args->laddr == NULL)
    args->port = 4567;
    args->udp_retransmissions = 3;
    args->udp_timeout = 250;
    args->help = false;

    int c;
    while ((c = getopt(argc, argv, "l:p:d:r:h")) != -1) {
        switch (c)
        {
        case 'l':
            mfree(args->laddr);
            args->laddr = mstrdup(optarg);
            if (args->laddr == NULL) {
                log(ERROR, MEMFAIL_MSG);
                return 1;
            }
            break;
        case 'p':
            rc = sscanf(optarg, "%hu", &(args->port));
            if (rc != 1) {
                logf(ERROR, "invalid value %s for option p", optarg);
                return 1;
            }
            break;
        case 'd':
            rc = sscanf(optarg, "%hu", &(args->udp_timeout));
            if (rc != 1) {
                logf(ERROR, "invalid value %s for option d", optarg);
                return 1;
            }
            break;
        case 'r':
            rc = sscanf(optarg, "%hhu", &(args->udp_retransmissions));
            if (rc != 1) {
                logf(ERROR, "invalid value %s for option r", optarg);
                return 1;
            }
            break;
        case 'h':
            args->help = true;
            break;

        default:
            logf(WARNING, "unknown option %c", c);
            break;
        }
    }
    logf(INFO, "parsed listnening address:  %s", args->laddr);
    logf(INFO, "parsed listnening port:     %hu", args->port);
    logf(INFO, "parsed udp timeout:         %hu", args->udp_timeout);
    logf(INFO, "parsed udp retransmissions: %hhu",
        args->udp_retransmissions);
    logf(INFO, "should print help:          %s",
        args->help ? "true" : "false");
    return 0;
}

void free_argstruct(struct args *args) {
    mfree(args->laddr);
}
