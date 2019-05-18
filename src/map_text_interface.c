#include "map_text_interface.h"
#include "utils.h"
#include <stdio.h>

Status execAddRoad(Map *map, char *arg) {
    Status ret = false;
    size_t len = strlen(arg) + 1;
    char city1[len];
    char city2[len];
    memset(city1, 0, len);
    memset(city2, 0, len);
    CHECK_RET(extractCityName(arg, city1));
    arg = strchr(arg, ';') + 1;
    if (!extractCityName(arg, city2)) {
        goto CLEANUP;
    }
    arg = strchr(arg, ';') + 1;
    unsigned long long length;
    if (!extractRoadLength(arg, &length)) {
        goto CLEANUP;
    }
    arg = strchr(arg, ';') + 1;
    int year;
    if (!extractYear(arg, &year)) {
        goto CLEANUP;
    }

    ret = addRoad(map, city1, city2, (unsigned)length, year);
CLEANUP:
    return ret;
}

Status execRepairRoad(Map *map, char *arg) {
    Status ret = false;
    size_t len = strlen(arg) + 1;
    char city1[len];
    char city2[len];
    memset(city1, 0, len);
    memset(city2, 0, len);
    CHECK_RET(extractCityName(arg, city1));
    arg = strchr(arg, ';') + 1;
    if (!extractCityName(arg, city2)) {
        goto CLEANUP;
    }
    arg = strchr(arg, ';') + 1;
    int year;
    if (!extractYear(arg, &year)) {
        goto CLEANUP;
    }
    ret = repairRoad(map, city1, city2, year);
CLEANUP:
    return ret;
}

Status execGetRouteDescription(Map *map, char *arg) {
    unsigned routeId;
    CHECK_RET(extractRouteId(arg, &routeId));
    const char *s = getRouteDescription(map, routeId);
    if (s != NULL) {
        printf("%s\n", s);
        free((void *)s);
        return true;
    } else {
        return false;
    }
}

Road getRoadFromName(Map *map, char *city1, char *city2) {
    Entry e1 = getDictionary(&map->city_to_int, (void *)city1);
    Entry e2 = getDictionary(&map->city_to_int, (void *)city2);
    if (NOT_FOUND(e1) || NOT_FOUND(e2)) {
        return (const Road){0};
    }
    int id1 = decodeCityId(e1.val);
    int id2 = decodeCityId(e2.val);
    return getRoad(map, id1, id2);
}

/** @brief Zwraca wskaźnik po n-tym średniku w stringu.
 * @param[in] ptr     - string, który przeszukujemy
 * @param[in] n       - numer średnika
 * @return Wskaźnik po n-tym średniku w stringu, lub NULL, jeśli taki nie
 * istnieje.
 */
static char *nextNthSemicolon(char *ptr, size_t n) {
    for (size_t i = 0; i < n && ptr - 1; ++i) {
        ptr = strchr(ptr, ';') + 1;
    }
    return ptr == (void *)1 ? NULL : ptr;
}

Status execNewRoute(Map *map, char *arg) {
    if (*arg < '0' || *arg > '9') {
        return false;
    }
    unsigned routeId;
    CHECK_RET(extractRouteId(arg, &routeId));
    if (!isEmptyList(&map->routes[routeId].cities)) {
        return false;
    }
    size_t len = strlen(arg) + 1;
    char xx[len];
    char yy[len];
    memset(xx, 0, len);
    memset(yy, 0, len);

    char *prev = nextNthSemicolon(arg, 1);
    char *ptr = nextNthSemicolon(prev, 3);

    // check if every edge can be inserted
    while (prev && ptr) {
        if (!extractCityName(prev, xx) || !extractCityName(ptr, yy)) {
            return false;
        }
        Road r = getRoadFromName(map, xx, yy);
        if (r.length != 0) {
            unsigned long long len;
            int year;
            if (!extractRoadLength(nextNthSemicolon(prev, 1), &len) ||
                len != r.length ||
                !extractYear(nextNthSemicolon(prev, 2), &year) ||
                year < r.builtYear) {
                return false;
            }
        }
        prev = nextNthSemicolon(prev, 3);
        ptr = nextNthSemicolon(ptr, 3);
    }

    List *l = newList();
    CHECK_RET(l);
    map->routes[routeId].cities = *l;
    free(l);

    prev = nextNthSemicolon(arg, 1);
    if (!extractCityName(prev, xx)) {
        return false;
    }
    if (!addCity(map, xx)) {
        return false;
    }
    Entry e = getDictionary(&map->city_to_int, xx);
    int id = decodeCityId(e.val);
    listInsertAfter(&map->routes[routeId].cities,
                    map->routes[routeId].cities.begin, id);
    ptr = nextNthSemicolon(prev, 3);
    while (prev && ptr) {
        if (!extractCityName(prev, xx) || !extractCityName(ptr, yy)) {
            return false;
        }
        unsigned long long length;
        int year;
        if (extractRoadLength(nextNthSemicolon(prev, 1), &length) &&
            extractYear(nextNthSemicolon(prev, 2), &year)) {
            if (!addRoadRepair(map, xx, yy, (unsigned)length, year)) {
                return false;
            }
        }
        Road r = getRoadFromName(map, xx, yy);
        if (!listInsertAfter(&map->routes[routeId].cities,
                             map->routes[routeId].cities.end, r.end)) {
            return false;
        }
        Entry e =
            getDictionary(&map->routesThrough, encodeEdgeAsPtr(r.start, r.end));
        if (NOT_FOUND(e)) {
            return false;
        }
        List *routesthrough = e.val;
        if (!listInsertAfter(routesthrough, routesthrough->end, routeId)) {
            return false;
        }

        prev = nextNthSemicolon(prev, 3);
        ptr = nextNthSemicolon(ptr, 3);
    }
    return true;
}
