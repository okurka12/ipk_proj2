/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**     IPK2     **
**     2024     **
*****************/

#include <stddef.h>
#include "clientlist.h"
#include "client.h"
#include "server.h"  // struct sockdata
#include "mmal.h"
#include "utils.h"

const unsigned int clist_initial_length = 512;

static struct sockdata **clist_arr = NULL;
static unsigned int clist_arrlen = 0;

/**
 * initializes elements from `from` to `clist_arrlen` to NULL
*/
static void clist_init(unsigned int from) {
    for (unsigned int i = from; i < clist_arrlen; i++) {
        clist_arr[i] = NULL;
    }
}

int clist_add(struct sockdata *client) {

    /* case: first time call to clist module */
    if (clist_arr == NULL) {
        clist_arr = mmal(clist_initial_length * sizeof(struct sockdata *));
        if (clist_arr == NULL) {
            log(ERROR, MEMFAIL_MSG);
            return 1;
        }
        clist_arrlen = clist_initial_length;
        clist_init(0);
        clist_arr[0] = client;
        return 0;
    }

    /* case: clist array is already allocated */
    for (unsigned int i = 0; i < clist_arrlen; i++) {
        if (clist_arr[i] == NULL) {
            clist_arr[i] = client;
            return 0;
        }
    }

    log(DEBUG, "clist array full, reallocating");
    unsigned int original_length = clist_arrlen;
    clist_arr = mrealloc(clist_arr, 2 * clist_arrlen);
    if (clist_arr == NULL) {
        clist_arrlen = 0;
        return 1;
    }
    clist_arrlen *= 2;
    clist_init(original_length);
    clist_arr[original_length] = client;
    return 0;
}

struct sockdata **clist_get_arr(unsigned int *n) {
    *n = clist_arrlen;
    return clist_arr;
}

void clist_remove(int sockfd) {
    for (unsigned int i = 0; i < clist_arrlen; i++) {
        if (clist_arr[i] != NULL and clist_arr[i]->fd == sockfd) {
            sockdata_dtor(clist_arr + i);
            clist_arr[i] = NULL;
        }
    }
}

void clist_free() {
    mfree(clist_arr);
    clist_arr = NULL;
    clist_arrlen = 0;
}
