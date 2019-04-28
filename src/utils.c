#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "list.h"
#include "utils.h"

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

bool validCityName(const char* city) {
    char* name = (char*)city;
    CHECK_RET(name);
    CHECK_RET(*name);
    while (*name) {
        if ((*name > 0 && *name < 32) || *name == ';') {
            return false;
        }
        name++;
    }
    return true;
}

bool possiblyValidRoad(const char* city1, const char* city2) {
    CHECK_RET(city1);
    CHECK_RET(city2);
    CHECK_RET(validCityName(city1));
    CHECK_RET(validCityName(city2));
    CHECK_RET(strcmp(city1, city2) != 0);
    return true;
}

void deleteDictionaryOfLists(Dictionary* d) {
    for (size_t i = 0; i < d->array_size; ++i) {
        List* l = d->array[i].val;
        deleteList(l);
    }
   deleteDictionary(d);
}

hash_t hashEdge(void* key) {
    return (hash_t)key;
}

bool cmpEdges(void* e1, void* e2) {
    if (e1 == NULL || e1 == DELETED || e2 == NULL || e2 == DELETED) {
        return false;
    }
    return e1 == e2;
}

size_t int_length(int64_t x) {
    // 3 == ceil(log10(256))
    char b[sizeof(x)*3+1];
    sprintf(b, "%"PRId64, x);
    return strlen(b);
}

void* encodeEdgeAsPtr(int a, int b) {
    uint64_t ret = 0;
    if (a < b) {
        ret |= a;
        ret |= ((uint64_t)b) << 32;
    } else {
        return encodeEdgeAsPtr(b, a);
    }
    return (void*)ret;
}


int min(int a, int b) {
    if (a < b) {
        return a;
    }
    return b;
}

hash_t hash_int(void* p) {
    unsigned *c = p;
    return *c;
}

bool equal_int(void* p1, void* p2) {
    CHECK_RET(p1);
    CHECK_RET(p2);
    return 0 == memcmp(p1, p2, sizeof(int));
}
