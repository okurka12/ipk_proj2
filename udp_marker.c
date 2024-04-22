/*****************
**  Vit Pavlik  **
**   xpavli0a   **
**    251301    **
**     IPK2     **
**     2024     **
*****************/

/**
 * marks message ids as seen (max 65535 or 0xffff messages)
*/

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "udp_marker.h"
#include "utils.h"

#define WELCOME_INIT_SIZE 128

/**
 * private structure for udpm_welcome functions
*/
struct udpm_welcome {

    /* this will be false for empty array slots*/
    bool active;

    uint16_t id;
    uint32_t addr;
    uint16_t port;
};

static struct udpm_welcome *welcome_data = NULL;
static unsigned int welcome_len = 0;


/**
 * return pointer to array (operate with udpm_mark or udpm_seen)
*/
int32_t *udpm_ctor() {
    int32_t *output = malloc(UDPM_MAX * sizeof(int32_t));
    if (output == NULL) {
        log(ERROR, MEMFAIL_MSG);
        return NULL;
    }
    for (uint32_t i = 0; i < UDPM_MAX; i++) {
        output[i] = -1;
    }
    return output;
}

void udpm_mark(uint16_t id, int32_t *udpm_data) {
    for (uint32_t i = 0; i < UDPM_MAX; i++) {
        if (udpm_data[i] == id) return;
        if (udpm_data[i] == -1) {
            udpm_data[i] = id;
            return;
        }
    }
}

bool udpm_seen(uint16_t id, int32_t *udpm_data) {

    for (uint32_t i = 0; i < UDPM_MAX; i++) {
        if (udpm_data[i] == id) return true;
    }
    return false;
}

/**
 * frees the data and sets the pointer to null
*/
void udpm_dtor(int32_t **udpm_data) {
    if (udpm_data == NULL) return;
    free(*udpm_data);
    *udpm_data = NULL;
}


void udpm_welcome_mark(uint16_t id, uint32_t addr, uint16_t port) {

    if (welcome_len == 0) {

        welcome_data =
            calloc(WELCOME_INIT_SIZE * sizeof(struct udpm_welcome), 1);

        welcome_len = WELCOME_INIT_SIZE;

        welcome_data[0].active = true;
        welcome_data[0].id = id;
        welcome_data[0].addr = addr;
        welcome_data[0].port = port;
        return;
    }

    for (unsigned int i = 0; i < welcome_len; i++) {
        if (welcome_data[i].active) continue;
        welcome_data[i].active = true;
        welcome_data[i].id = id;
        welcome_data[i].addr = addr;
        welcome_data[i].port = port;
        return;
    }
    log(WARNING, "expanding the welcoma data array was not yet implemented "
    "(probably too many UDP clients since starting the server)");
    /* todo */
}

bool udpm_welcome_seen(uint16_t id, uint32_t addr, uint16_t port) {
    for (unsigned int i = 0; i < welcome_len; i++) {
        bool c1 = welcome_data[i].active;
        bool c2 = welcome_data[i].id == id;
        bool c3 = welcome_data[i].addr == addr;
        bool c4 = welcome_data[i].port == port;
        if (c1 and c2 and c3 and c4) return true;
    }
    return false;
}

void udpm_welcome_free_res() {
    welcome_len = 0;
    free(welcome_data);
    welcome_data = NULL;
}
