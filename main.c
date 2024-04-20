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

int main(int argc, char **argv) {
    int rc;
    struct args args;
    rc = parse_arguments(argc, argv, &args);

    free_argstruct(&args);
    gexit(GE_FREE_RES, NULL);
    return rc;
}
