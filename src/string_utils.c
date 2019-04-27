#include <string.h>

#include "string_utils.h"

hash_t hash_string(void* str) {
    const char* c = str;
    hash_t ret = 0;
    size_t len = strlen(c);
    size_t x = sizeof(hash_t) * (len / sizeof(hash_t));
    for (size_t i = 0; i < x; i += sizeof(hash_t)) {
        ret ^= *(const hash_t*)(c + i);
    }
    if (x < len) {
        uint8_t rest[sizeof(hash_t)] = {0};
        memcpy(rest, c, len - x);
        ret ^= *(const hash_t*)rest;
    }
    return ret;
}

bool undereferencing_strcmp(void* x, void* y) {
    const char* a = x;
    const char* b = y;
    if (a == NULL || a == DELETED) {
        return false;
    }
    if (b == NULL || b == DELETED) {
        return false;
    }
    return strcmp(a, b) == 0;
}

