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
    unsigned length;
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

    Dictionary *d = newDictionary(hash_int, equal_int, true, true);
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
    if (builtYear == 0) {
        return false;
    }
    CHECK_RET(validCityName(city1) == false);
    CHECK_RET(validCityName(city2) == false);
    CHECK_RET(strcmp(city1, city2) != 0);

    Entry e1 = getDictionary(&map->city_to_int, (void*)city1);
    Entry e2 = getDictionary(&map->city_to_int, (void*)city2);
    if (NOT_FOUND(e1)) {
        addCity(map, city1);
    }
    if (NOT_FOUND(e2)) {
        addCity(map, city2);
    }




    return true;
}

