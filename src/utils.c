#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "list.h"
#include "utils.h"

hash_t nHashString(void *str, size_t len) {
    const uint8_t *c = str;
    hash_t ret = 0;
    for (size_t i = 0; i < len; ++i) {
        ret = ret * 149 + c[i];
    }
    return ret;

    size_t x = sizeof(hash_t) * (len / sizeof(hash_t));
    for (size_t i = 0; i < x; i += sizeof(hash_t)) {
        ret ^= *(const hash_t *)(c + i);
    }
    if (x < len) {
        uint8_t rest[sizeof(hash_t)] = {0};
        memcpy(rest, c, len - x);
        ret ^= *(const hash_t *)rest;
    }
    return ret;
}

hash_t hashString(void *str) { return nHashString(str, strlen((char *)str)); }

bool undereferencing_strcmp(void *x, void *y) {
    const char *a = x;
    const char *b = y;
    if (a == NULL || a == DELETED) {
        return false;
    }
    if (b == NULL || b == DELETED) {
        return false;
    }
    return strcmp(a, b) == 0;
}

bool nValidCityName(const char *city, size_t n) {
    char *name = (char *)city;
    CHECK_RET(name);
    CHECK_RET(*name);
    while (*name && (size_t)(name - city) < n) {
        if ((*name > 0 && *name < 32) || *name == ';') {
            return false;
        }
        name++;
    }
    return true;
}

bool validCityName(const char *city) { return nValidCityName(city, SIZE_MAX); }

bool possiblyValidRoad(const char *city1, const char *city2) {
    CHECK_RET(city1);
    CHECK_RET(city2);
    CHECK_RET(validCityName(city1));
    CHECK_RET(validCityName(city2));
    CHECK_RET(strcmp(city1, city2) != 0);
    return true;
}

hash_t hashEdge(void *key) { return (hash_t)key; }

bool cmpEdges(void *e1, void *e2) {
    if (e1 == NULL || e1 == DELETED || e2 == NULL || e2 == DELETED) {
        return false;
    }
    return e1 == e2;
}

size_t intLength(int64_t x) {
    // 3 == ceil(log10(256))
    char b[sizeof(x) * 3 + 1];
    sprintf(b, "%" PRId64, x);
    return strlen(b);
}

void *encodeCityId(int id) {
    uint64_t ret = id;
    ret <<= 32;
    ret |= 3;
    return (void *)ret;
}

void *encodeEdgeAsPtr(int a, int b) {
    uint64_t ret = 0;
    if (a <= b) {
        ret |= a;
        ret |= ((uint64_t)b) << 32;
    } else {
        return encodeEdgeAsPtr(b, a);
    }
    return (void *)ret;
}

int decodeCityId(void *p) {
    uint64_t x = (uint64_t)p;
    return x >> 32;
}

int min(int a, int b) {
    if (a < b) {
        return a;
    }
    return b;
}

hash_t hashInt(void *p) {
    unsigned *c = p;
    return *c;
}

bool equalInt(void *p1, void *p2) {
    CHECK_RET(p1);
    CHECK_RET(p2);
    if (p1 == DELETED || p2 == DELETED) {
        return false;
    }
    return 0 == memcmp(p1, p2, sizeof(int));
}

void swap(int *x, int *y) {
    int z = *x;
    *x = *y;
    *y = z;
}

void empty(void *v) {
    int *x = v;
    x++;
}
