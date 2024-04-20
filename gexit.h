/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**     IPK2     **
**     2024     **
*****************/

/**
 *
 * API for the `gexit` - gracefully exit module
 * `gexit` is meant to handle the program interrupt
 *
 */

#ifndef _G_E_X_I_T_H_
#define _G_E_X_I_T_H_

/* statements that `gexit` can be called with */
enum gexit_statement {


    /* call this statement when the program is terminated */
    GE_TERMINATE,

    /* register a pointer to be freed when exiting via gexit (note: you can
    register a NULL pointer which has no effect) */
    GE_REGISTER_PTR,

    /* unregister a pointer registered with GE_REGISTER_PTR */
    GE_UNREG_PTR,

    /* free internal gexit resources (this wipes registered pointers) */
    GE_FREE_RES,

    /* set epoll instance file descriptor */
    GE_SET_EPOLLFD,

    /* unset epoll instance file descriptor */
    GE_UNSET_EPOLLFD

};

/**
 * gracefully exit
 *
 * this function keeps static variables for the config structure and possibly
 * a socket file descriptor that is currently open, so that allocated resources
 * (address string, open sockets) can be freed upon termination with `C-c`
 *
 * if the program is terminated in a normal way, `gexit(GE_FREE_RES, NULL)`
 * needs to be called beforehand
 *
 * Call this function with `statement` and a pointer `p` pointing to the
 * desired property you want to set (sockfd, confp, etc..) or, if the desired
 * property is a pointer, `p` is the pointer itself
 *
 * @note
 * this function contains a few `log` statements from `utils.h` which may
 * result in an undefined behavior (see `man 7 signal-safety`). eliminate
 * this by defining NDEBUG
*/
void gexit(enum gexit_statement statement, void *p);

#endif  // ifndef _G_E_X_I_T_H_
