/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**     IPK2     **
**     2024     **
*****************/


#include "argparse.h"
#include "gexit.h"
#include "utils.h"
#include "server.h"
#include "iota.h"


int main(int argc, char **argv) {
    int rc;
    struct args args;
    rc = parse_arguments(argc, argv, &args);
    if (rc == 1) {
        log(ERROR, "couldn't parse arguments");
        rc = 1;
        goto cleanup;
    }
    if (args.help) {
        printf(HELP_TXT);
        rc = 0;
        goto cleanup;
    }

    rc = start_server(&args);

    cleanup:
    free_argstruct(&args);
    gexit(GE_FREE_RES, NULL);
    return rc;
}
