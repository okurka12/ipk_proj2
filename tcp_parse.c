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
#include "tcp_parse.h"
#include "utils.h"
#include "iota.h"

/* a buffer for errors by `regerror`, it will be on stack so keep it small */
#define ERRMSG_BUFSIZE 80

/* informs about regex compile error, expects a buffer `errmsgbuf` sized
`ERRMSG_BUFSIZE` and a regex_t object `rgp`  */
#define regcomperr(errmsgbuf, rgp) log(ERROR, "couldn't compile regex"); \
regerror(rc, &rgp, errmsg, ERRMSG_BUFSIZE); \
fprintf(stderr, "regexec: %s", errmsg); \
log(ERROR, "couldn't compile regex")

#define IPK_CST_MSG "sadly, these constants are hard-coded here, if you " \
"change them in ipk24chat.h, you need to change them here too"

/* use only in tcp_parse_any where there is bool *err */
#define check_null(ptr_to_check) if (ptr_to_check == NULL) { \
    log(ERROR, MEMFAIL_MSG); \
    *err = true; \
    return NULL; \
}

/* flags for regcomp */
const int rflg = REG_ICASE | REG_EXTENDED;

static void check_ipk_constants() {
    static_assert(MAX_DNAME_LEN == 20, IPK_CST_MSG);
    static_assert(MAX_CHID_LEN == 20, IPK_CST_MSG);
    static_assert(MAX_MSGCONT_LEN == 1400, IPK_CST_MSG);
}


// bool tcp_parse_reply(char *data, char **content, bool *err) {
//     check_ipk_constants();

//     int rc = 0;
//     char errmsg[ERRMSG_BUFSIZE];

//     /* compile the regex */
//     regex_t pattern;
//     regcomp(&pattern, "REPLY (N?OK) IS ([ -~]{1,1400})", rflg);
//     if (rc != 0) {
//         regcomperr(errmsg, pattern);
//         *err = true;
//         *content = NULL;
//         return false;
//     }

//     /* todo: register `pattern` in gexit? */

//     /* try to match the regex */
//     const size_t nmatch = 3;  // whole match, first group, second group
//     regmatch_t rms[nmatch];
//     rc = regexec(&pattern, data, nmatch, rms, 0);
//     if (rc != 0) {
//         log(WARNING, "couldn't match REPLY");
//         regerror(rc, &pattern, errmsg, ERRMSG_BUFSIZE);
//         fprintf(stderr, "regexec: %s", errmsg);
//         *content = NULL;
//         regfree(&pattern);
//         return false;
//     }

//     /* change CR to null byte */
//     data[rms[0].rm_eo] = '\0';

//     /* return the content index in the original data */
//     *content = data + rms[2].rm_so;

//     // len("OK") is 2 and len("NOK") is 3
//     bool reply_ok = rms[1].rm_eo - rms[1].rm_so == 2;

//     logf(DEBUG, "successfully matched, reply_success=%d, content='%s'",
//         reply_ok, *content);

//     regfree(&pattern);
//     return reply_ok;
// }

msg_t *tcp_parse_any(char *data, bool *err) {
    check_ipk_constants();

    /* lets hope SIGINT wont come between regcomp and regfree :shrug: */
    logf(DEBUG, "parsing: '%s'", data);

    int rc = 0;
    char errmsg[ERRMSG_BUFSIZE];


    regex_t msg_pat;
    rc = regcomp(&msg_pat, "MSG FROM ([!-~]{1,20}) IS ([ -~]{1,1400})", rflg);
    if (rc != 0) {
        regcomperr(errmsg, msg_pat);
        *err = true;
        return NULL;
    }

    /* only for the start of the string? do the rest with tcp_parse_reply */
    regex_t reply_pat;
    rc = regcomp(&reply_pat, "\\s*REPLY", rflg);
    if (rc != 0) {
        regcomperr(errmsg, reply_pat);
        *err = true;
        return NULL;
    }

    regex_t err_pat;
    rc = regcomp(&err_pat, "ERR FROM ([!-~]{1,20}) IS ([ -~]{1,1400})", rflg);
    if (rc != 0) {
        regcomperr(errmsg, err_pat);
        *err = true;
        return NULL;
    }

    regex_t auth_pat;
    // rc = regcomp(&auth_pat, "AUTH ((?:[A-Z]|[a-z]|[0-9]|-){1,20}) AS "
    // "([!-~]{1,20}) USING ((?:[A-z]|[0-9]|-){1,128})", rflg);
    rc = regcomp(&auth_pat, "AUTH ([A-Z]|[a-z]|[0-9]|-){1,20} AS "
    "([!-~]{1,20}) USING ([A-z]|[0-9]|-){1,128}", rflg);
    if (rc != 0) {
        regcomperr(errmsg, auth_pat);
        *err = true;
        return NULL;
    }

    regex_t bye_pat;
    rc = regcomp(&bye_pat, "BYE", rflg);
    if (rc != 0) {
        regcomperr(errmsg, bye_pat);
        *err = true;
        return NULL;
    }

    const size_t nmatch = 4;  // like above, max three fields (auth)
    regmatch_t rms[nmatch];
    msg_t *output = msg_ctor();
    check_null(output);
    output->type = MTYPE_UNKNOWN;

    /* MSG */
    if (regexec(&msg_pat, data, nmatch, rms, 0) == 0) {
        data[rms[1].rm_eo] = '\0';  // displayname
        data[rms[2].rm_eo] = '\0';  // content
        // printf("%s: %s\n", data + rms[1].rm_so, data + rms[2].rm_so);
        output->type = MTYPE_MSG;
        output->dname = strdup(data + rms[1].rm_so);
        output->content = strdup(data + rms[2].rm_so);
        check_null(output->dname);
        check_null(output->content);

    /* ERR */
    } else if (regexec(&err_pat, data, nmatch, rms, 0) == 0) {
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
    } else if (regexec(&bye_pat, data, nmatch, rms, 0) == 0) {
        output->type = MTYPE_BYE;

    /* REPLY */
    } else if (regexec(&reply_pat, data, nmatch, rms, 0) == 0) {
        output->type = MTYPE_REPLY;
        /* we dont need the fields because client should never send us
        REPLY anyway */

    /* AUTH */
    } else if (regexec(&auth_pat, data, nmatch, rms, 0) == 0) {
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

    }

    regfree(&msg_pat);
    regfree(&err_pat);
    regfree(&reply_pat);
    regfree(&err_pat);
    regfree(&bye_pat);

    return output;
}
