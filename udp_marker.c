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
