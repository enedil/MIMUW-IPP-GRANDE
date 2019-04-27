#include <stdlib.h>

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
    int cost;
} Road;

typedef struct Map {
    Route routes[ROUTE_MAX];
    // each neighbour holds a Dictionary[int, Road]
    Vector neighbours;
    Dictionary city_to_int;


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
        Dictionary* d = map->neighbours.arr[i];
// hudsihd iudhias uh uisa
        deleteDictionary(d);
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

    Dictionary* d = newDictionary(hash_string, undereferencing_strcmp);
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
    free(map);
}



