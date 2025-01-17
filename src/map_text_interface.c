/** @file
 * Implementacja interfejsu tekstowego obsługi mapy.
 */
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

/** @brief Zwraca wskaźnik po n-tym średniku w stringu.
 * @param[in] ptr     - string, który przeszukujemy
 * @param[in] n       - numer średnika
 * @return Wskaźnik po n-tym średniku w stringu, lub NULL, jeśli taki nie
 * istnieje.
 */
static char *nextNthSemicolon(char *ptr, size_t n) {
    for (size_t i = 0; i < n && (ptr - 1) != NULL; ++i) {
        ptr = strchr(ptr, ';') + 1;
    }
    return ptr == (void *)1 ? NULL : ptr;
}

Status execNewRouteThrough(Map *map, char *arg) {
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

Status execNewRoute(Map *map, char *arg) {
    unsigned rid;
    extractRouteId(arg, &rid);
    char *city1 = strchr(arg, ';') + 1;
    char *city2 = strchr(city1, ';') + 1;
    //*city1 = 0;
    *(city2 - 1) = 0;
    return newRoute(map, rid, city1, city2);
}

Status execExtendRoute(Map *map, char *arg) {
    unsigned rid;
    extractRouteId(arg, &rid);
    char *city = strchr(arg, ';') + 1;
    return extendRoute(map, rid, city);
}

Status execRemoveRoute(Map *map, char *arg) {
    unsigned rid;
    extractRouteId(arg, &rid);
    return removeRoute(map, rid);
}

Status execRemoveRoad(Map *map, char *arg) {
    size_t len = strlen(arg);
    char city1[len];
    char city2[len];
    extractCityName(arg, city1);
    extractCityName(strchr(arg, ';') + 1, city2);
    return removeRoad(map, city1, city2);
}
