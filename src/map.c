#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "dictionary.h"
#include "list.h"
#include "map.h"
#include "string_utils.h"
#include "vector.h"

#define ROUTE_MAX 1000

typedef struct Route {
    uint64_t length;
    List cities;
} Route;

typedef struct Road {
    int start;
    int end;
    uint64_t length;
    int builtYear;
} Road;

typedef struct Map {
    Route routes[ROUTE_MAX];
    // each neighbour holds a Dictionary[int, Road]
    Vector neighbours;
    Dictionary city_to_int;
    Dictionary routesThrough;
} Map;

static void deleteRoutes(Map* map) {
    for (int i = 0; i < ROUTE_MAX; ++i) {
        deleteList(&map->routes[i].cities);
    }
}

static void deleteAdjacencyDictionaries(Map* map) {
    if (map == NULL) {
        return;
    }
    for (size_t i = 0; i < map->neighbours.size; ++i) {
        deleteDictionary(map->neighbours.arr[i]);
// hudsihd iudhias uh uisa
        free(map->neighbours.arr[i]);
        map->neighbours.arr[i] = NULL;
    }
}


Map* newMap(void) {
    Map* map = calloc(1, sizeof(Map));
    CHECK_RET(map);
    // route number 0 is invalid, as per task descritption
    for (int i = 1; i < ROUTE_MAX; ++i) {
        map->routes[i].length = 0;
        List *l = newList();
        if (l == NULL) {
            goto DELETE_ROUTES;
        }
        map->routes[i].cities = *l;
        free(l);
    }

    Dictionary* d = newDictionary(hash_string, undereferencing_strcmp, false, true);
    if (d == NULL) {
        goto DELETE_ROUTES;
    }
    map->city_to_int = *d;
    free(d);

    Vector* n = newVector();
    if (n == NULL) {
        goto DELETE_CITY_TO_INT;
    }
    map->neighbours = *n;
    free(n);


    return map;

    DELETE_CITY_TO_INT:
        deleteDictionary(&map->city_to_int);
    DELETE_ROUTES:
        deleteRoutes(map);
    //DELETE_MAP:
        deleteMap(map);
    return NULL;
}


void deleteMap(Map *map) {
    deleteRoutes(map);
    deleteDictionary(&map->city_to_int);
    deleteAdjacencyDictionaries(map);
    vectorDeleteFreeContent(&map->neighbours);
    free(map);
}

static bool validCityName(const char* city) {
    char* name = (char*)city;
    CHECK_RET(name);
    CHECK_RET(*name);
    while (*name) {
        if (*name >= 32 || *name < 0 || *name == ';') {
            return false;
        }
        name++;
    }
    return true;
}

static bool possiblyValidRoad(const char* city1, const char* city2) {
    CHECK_RET(city1);
    CHECK_RET(city2);
    CHECK_RET(validCityName(city1) == false);
    CHECK_RET(validCityName(city2) == false);
    CHECK_RET(strcmp(city1, city2) != 0);
    return true;
}

static bool areRoadsConsistent(Road* r1, Road* r2) {
    CHECK_RET(r1);
    CHECK_RET(r2);
    CHECK_RET(r1->builtYear == r2->builtYear);
    CHECK_RET(r1->length == r2->length);
    CHECK_RET(r1->end == r2->start);
    CHECK_RET(r2->start == r1->end);
    return true;
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

Status addCity(Map *map, const char* city) {
    CHECK_RET(map);
    CHECK_RET(city);

    int* city_id = malloc(sizeof(int));
    *city_id = (int)map->city_to_int.size;
    CHECK_RET(insertDictionary(&map->city_to_int, (void*)city, city_id));

    Dictionary *d = newDictionary(hash_int, equal_int, false, true);
    if (d == NULL) {
        goto DELETE_FROM_DICTIONARY;
    }
    if (vectorAppend(&map->neighbours, d) == false) {
        goto DELETE_DICT;
    }

    return true;

DELETE_DICT:
    deleteDictionary(d);
    free(d);

DELETE_FROM_DICTIONARY:
    deleteFromDictionary(&map->city_to_int, (void*)city);
    return false;
}

bool addRoad(Map *map, const char *city1, const char *city2,
             unsigned length, int builtYear) {
    CHECK_RET(builtYear);
    CHECK_RET(length);
    CHECK_RET(map);
    CHECK_RET(possiblyValidRoad(city1, city2));

    Entry e1 = getDictionary(&map->city_to_int, (void*)city1);
    Entry e2 = getDictionary(&map->city_to_int, (void*)city2);

    // don't bother with deleting this, in case of further failure
    if (NOT_FOUND(e1)) {
        CHECK_RET(addCity(map, city1));
    }
    if (NOT_FOUND(e2)) {
        CHECK_RET(addCity(map, city2));
    }

    e1 = getDictionary(&map->city_to_int, (void*)city1);
    e2 = getDictionary(&map->city_to_int, (void*)city2);

    int id1 = *(int*)e1.val;
    int id2 = *(int*)e2.val;
    Entry edge12 = getDictionary((map->neighbours.arr)[id1], &id2);
    Entry edge21 = getDictionary((map->neighbours.arr)[id2], &id1);
    if (!NOT_FOUND(edge12) || !NOT_FOUND(edge21)) {
        return false;
    }

    Road* r1 = calloc(1, sizeof(Road));
    Road* r2 = calloc(1, sizeof(Road));
    if (r1 == NULL || r2 == NULL) {
        goto FREE_R;
    }
    *r1 = (const Road){
            .builtYear = builtYear,
            .length = length,
            .start = id1,
            .end = id2
        };
    *r2 = *r1;
    r2->start = r1->end;
    r2->end = r1->start;

    if (insertDictionary(map->neighbours.arr[id1], &(r1->end), r1) == false) {
        goto FREE_R;
    }
    if (insertDictionary(map->neighbours.arr[id2], &(r2->end), r2) == false) {
        goto FREE_R;
    }
    return true;

FREE_R:
    free(r1);
    free(r2);

    return false;
}


bool repairRoad(Map *map, const char *city1, const char *city2, int repairYear)
{
    CHECK_RET(map);
    CHECK_RET(repairYear);
    CHECK_RET(possiblyValidRoad(city1, city2));

    Entry e1 = getDictionary(&map->city_to_int, (void*)city1);
    CHECK_RET(NOT_FOUND(e1) == false);
    Entry e2 = getDictionary(&map->city_to_int, (void*)city2);
    CHECK_RET(NOT_FOUND(e2) == false);

    int id1 = *(int*)e1.val;
    int id2 = *(int*)e2.val;

    Entry edge12 = getDictionary((map->neighbours.arr)[id1], &id2);
    Entry edge21 = getDictionary((map->neighbours.arr)[id2], &id1);

    if (NOT_FOUND(edge12) || NOT_FOUND(edge21)) {
        return false;
    }
    Road* r1 = edge12.val;
    Road* r2 = edge21.val;

    assert(areRoadsConsistent(r1, r2));

    CHECK_RET(repairYear >= r1->builtYear);
    r1->builtYear = repairYear;
    r2->builtYear = repairYear;

    return true;
}


