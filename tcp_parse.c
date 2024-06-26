/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**              **
**   Created:   **
**  2024-03-21  **
*****************/

/**
 * @note maximal field lengths are hard_coded here, i tried to do it so that
 * it wont compile if they're changed in iota.h (via static_assert)
*/

#define _POSIX_C_SOURCE 200809L  // for strdup

#include <string.h>
#include <assert.h>
#include <regex.h>
#include <stdlib.h>
#include "tcp_parse.h"
#include "utils.h"
#include "iota.h"

/* a buffer for errors by `regerror`, it will be on stack so keep it small */
#define ERRMSG_BUFSIZE 80

/* informs about regex compile error, expects a buffer `errmsgbuf` sized
`ERRMSG_BUFSIZE` and a regex_t object `rgp`  */
#define regcomperr(errmsgbuf, rgp) log(ERROR, "couldn't compile regex"); \
regerror(rc, &rgp, errmsg, ERRMSG_BUFSIZE); \
fprintf(stderr, "regexec: %s\n", errmsg); \
log(ERROR, "couldn't compile regex")

#define IPK_CST_MSG "sadly, these constants are hard-coded here, if you " \
"change them in ipk24chat.h, you need to change them here too"

/* use only in tcp_parse_any where there is bool *err */
#define check_null(ptr_to_check) if (ptr_to_check == NULL) { \
    log(ERROR, MEMFAIL_MSG); \
    *err = true; \
    return NULL; \
}

/**
 * Private struct for all the regex patterns
*/
struct regpatterns {
    regex_t msg_pat;
    regex_t reply_pat;
    regex_t err_pat;
    regex_t auth_pat;
    regex_t bye_pat;
    regex_t join_pat;
};

/* flags for regcomp */
const int rflg = REG_ICASE | REG_EXTENDED;

static void check_ipk_constants() {
    static_assert(MAX_DNAME_LEN == 20, IPK_CST_MSG);
    static_assert(MAX_CHID_LEN == 20, IPK_CST_MSG);
    static_assert(MAX_MSGCONT_LEN == 1400, IPK_CST_MSG);
}

static struct regpatterns *get_regpatterns(bool do_free) {

    static struct regpatterns *output = NULL;

    if (do_free and output == NULL) {
        /* do nothing */
        return NULL;
    }

    if (do_free) {
        regfree(&output->msg_pat);
        regfree(&output->reply_pat);
        regfree(&output->err_pat);
        regfree(&output->auth_pat);
        regfree(&output->bye_pat);
        regfree(&output->join_pat);
        free(output);
        output = NULL;
        return NULL;
    }

    if (output != NULL) {
        return output;
    }

    check_ipk_constants();
    char errmsg[ERRMSG_BUFSIZE];
    int rc;

    output = malloc(sizeof(struct regpatterns));

    rc = regcomp(&output->msg_pat, "MSG FROM ([!-~]{1,20}) IS ([ -~]{1,1400})", rflg);
    if (rc != 0) {
        regcomperr(errmsg, output->msg_pat);
        return NULL;
    }

    /* only for the start of the string */
    rc = regcomp(&output->reply_pat, "\\s*REPLY", rflg);
    if (rc != 0) {
        regcomperr(errmsg, output->reply_pat);
        return NULL;
    }

    rc = regcomp(&output->err_pat, "ERR FROM ([!-~]{1,20}) IS ([ -~]{1,1400})", rflg);
    if (rc != 0) {
        regcomperr(errmsg, output->err_pat);
        return NULL;
    }

    rc = regcomp(&output->auth_pat, "AUTH ([A-Za-z0-9-]{1,20}) AS "
    "([!-~]{1,20}) USING ([A-Za-z0-9-]{1,128})", rflg);
    if (rc != 0) {
        regcomperr(errmsg, output->auth_pat);
        return NULL;
    }

    rc = regcomp(&output->bye_pat, "BYE", rflg);
    if (rc != 0) {
        regcomperr(errmsg, output->bye_pat);
        return NULL;
    }

    rc = regcomp(&output->join_pat, "JOIN ([A-Za-z0-9-]{1,20}) AS ([!-~]{1,20})", rflg);
    if (rc != 0) {
        regcomperr(errmsg, output->bye_pat);
        return NULL;
    }

    return output;
}

void tcp_parse_free_resources() {
    get_regpatterns(true);
}

// bool tcp_parse_rep
msg_t *tcp_parse_any(char *data, bool *err) {

    /* we're going to insert zero bytes so we need to save the strlen for
    later */
    size_t data_len = strlen(data);

    /* lets hope SIGINT wont come between regcomp and regfree :shrug: */
    logf(DEBUG, "parsing: '%s'", data);

    struct regpatterns *rpats = get_regpatterns(false);
    if (rpats == NULL) {
        log(ERROR, "could not get regex patterns");
        *err = true;
        return NULL;
    }

    const size_t nmatch = 4;  // like above, max three fields (auth)
    regmatch_t rms[nmatch];

    /* initialize values so that valgrind calms down */
    rms[0].rm_eo = -1;
    rms[0].rm_so = -1;

    msg_t *output = msg_ctor();
    check_null(output);
    output->type = MTYPE_UNKNOWN;

    /* MSG */
    if (regexec(&rpats->msg_pat, data, nmatch, rms, 0) == 0) {
        data[rms[1].rm_eo] = '\0';  // displayname
        data[rms[2].rm_eo] = '\0';  // content
        // printf("%s: %s\n", data + rms[1].rm_so, data + rms[2].rm_so);
        output->type = MTYPE_MSG;
        output->dname = strdup(data + rms[1].rm_so);
        output->content = strdup(data + rms[2].rm_so);
        check_null(output->dname);
        check_null(output->content);

    /* ERR */
    } else if (regexec(&rpats->err_pat, data, nmatch, rms, 0) == 0) {
        data[rms[1].rm_eo] = '\0';
        data[rms[2].rm_eo] = '\0';
        fprintf(stderr, "ERR FROM %s: %s\n", data + rms[1].rm_so,
            data + rms[2].rm_so);
        output->type = MTYPE_ERR;
        output->dname = strdup(data + rms[1].rm_so);
        output->content = strdup(data + rms[2].rm_so);
        check_null(output->dname);
        check_null(output->content);
    /* BYE */
    } else if (regexec(&rpats->bye_pat, data, nmatch, rms, 0) == 0) {
        output->type = MTYPE_BYE;

    /* REPLY */
    } else if (regexec(&rpats->reply_pat, data, nmatch, rms, 0) == 0) {
        output->type = MTYPE_REPLY;
        /* we dont need the fields because client should never send us
        REPLY anyway */

    /* AUTH */
    } else if (regexec(&rpats->auth_pat, data, nmatch, rms, 0) == 0) {
        output->type = MTYPE_AUTH;
        data[rms[1].rm_eo] = '\0';  // username
        data[rms[2].rm_eo] = '\0';  // displayname
        data[rms[3].rm_eo] = '\0';  // secret
        output->username = strdup(data + rms[1].rm_so);
        output->dname    = strdup(data + rms[2].rm_so);
        output->secret   = strdup(data + rms[3].rm_so);
        check_null(output->username);
        check_null(output->dname);
        check_null(output->secret);

    } else if (regexec(&rpats->join_pat, data, nmatch, rms, 0) == 0) {
        output->type = MTYPE_JOIN;
        data[rms[1].rm_eo] = '\0';  // channel id
        data[rms[2].rm_eo] = '\0';  // displayname
        output->chid  = strdup(data + rms[1].rm_so);
        output->dname = strdup(data + rms[2].rm_so);
        check_null(output->chid);
        check_null(output->dname);
    }

    /** assert the whole match starts at index 0
     * this prevents server from accepting something like
     * `mmsg from h is hello`
     */

    if (rms[0].rm_so != 0) {
        output->type = MTYPE_UNKNOWN;
    }

    /**
     * assert it matched the string to the end
     * this prevents server from accepting something like
     * `auth c as c using c:`
    */
    if ((unsigned int)rms[0].rm_eo != data_len) {
        output->type = MTYPE_UNKNOWN;
    }

    return output;
}
