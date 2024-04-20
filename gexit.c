/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**     IPK2     **
**     2024     **
*****************/

/**
 *
 * implementation of the `gexit` - gracefully exit module
 * see gexit.h
 *
 * @note malloc here is not null-checked because it doesn't affect the
 * program's ability to function correctly, it just doesn't free memory
 * (but only if it's being exited with SIGINT!)
 *
 */

#include <unistd.h>  // close
#include <stdlib.h>  // free
#include <stdbool.h>
// #include <threads.h>  // thrd_join, mtx_unlock...
// #include <sys/socket.h>  // shutdown

#include "gexit.h"
#include "utils.h"

// extern mtx_t gcl;

/* how long should the array of the pointers be */
#define NUM_PTRS 256

/**
 * Private: initialize all positions of `arr` to NULL
 * @param arr the array to initialize
 * @param len length of the array
*/
void gexit_init_arr(void **arr, unsigned int len) {
    for (unsigned int i = 0; i < len; i++) {
        arr[i] = NULL;
    }
}

/**
 * Private: register a pointer to be freed if exiting via gexit
 * @param ptr_arr pointer to array of pointers (void ***)
 * @param len pointer to the length of the array
 * @param p the pointer to register
 * @note can be called with `p` equal to NULL (in that case, it does nothing)
 * @note if allocation fails, does nothing, therefore the pointer is not
 * registered and won't be freed if exiting via gexit (which is a risk i am
 * willing to take)
*/
void gexit_regptr(void ***ptr_arr, unsigned int *len, void *p) {

    if (p == NULL) {
        return;
    }

    /* case: first call to gexit_regptr - allocate a ptr_arr */
    if (*len == 0) {
        *ptr_arr = malloc(sizeof(void *) * NUM_PTRS);
        if (*ptr_arr == NULL) return;
        *len = NUM_PTRS;
        gexit_init_arr(*ptr_arr, *len);
        (*ptr_arr)[0] = p;
    }

    /* write `p` to the first empty position and return*/
    for (unsigned int i = 0; i < *len; i++) {
        if ((*ptr_arr)[i] == NULL) {
            (*ptr_arr)[i] = p;
            return;
        }
    }

    /* if we got here, that means the array is not long enough */
    log(DEBUG, "array not long enough, reallocating");
    *ptr_arr = realloc(*ptr_arr, sizeof(void *) * (*len + NUM_PTRS));
    if (*ptr_arr == NULL) {
        return;
    }

    /* init the new part of the array */
    gexit_init_arr(*ptr_arr + *len, NUM_PTRS);

    (*ptr_arr)[*len] = p;
    *len += NUM_PTRS;
}


/**
 * Private: unregister a pointer
 * @param ptr_arr pointer to array of pointers (void ***)
 * @param len pointer to the length of the array
 * @param p the pointer to unregister
 *
*/
void gexit_unregptr(void ***ptrs, unsigned int *len, void *p) {
    for (unsigned int i = 0; i < *len; i++) {
        if ((*ptrs)[i] == p) {
            (*ptrs)[i] = NULL;
            return;
        }
    }
}


/**
 * Private: free all the registered pointers and also free the array used
 * to store them
 * @param ptr_arr pointer to array of pointers (void ***)
 * @param len pointer to the length of the array
*/
void gexit_free_all(void ***ptrs, unsigned int *len) {
    for (unsigned int i = 0; i < *len; i++) {
        if ((*ptrs)[i] != NULL) {
            free((*ptrs)[i]);
        }
    }
    free(*ptrs);
}


void gexit(enum gexit_statement statement, void *p) {
    static int epollfd = -1;

    /* array of the pointers to free */
    static void **ptrs = NULL;
    static unsigned int ptrs_len = 0;

    switch (statement) {

    case GE_REGISTER_PTR:
        // mtx_lock(&gcl);
        gexit_regptr(&ptrs, &ptrs_len, p);
        // mtx_unlock(&gcl);
        break;

    case GE_UNREG_PTR:
        // mtx_lock(&gcl);
        gexit_unregptr(&ptrs, &ptrs_len, p);
        // mtx_unlock(&gcl);
        break;

    case GE_SET_EPOLLFD:
        epollfd = *((int *)p);
        break;

    case GE_UNSET_EPOLLFD:
        epollfd = -1;
        break;

    case GE_TERMINATE:
        log(INFO, "the program was interrupted, exiting");
        int rc = 0;

        if (epollfd != -1) close(epollfd);
        gexit_free_all(&ptrs, &ptrs_len);
        exit(rc);

    case GE_FREE_RES:
        free(ptrs);
        ptrs = NULL;
        break;

    }
}
