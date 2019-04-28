#include <assert.h>
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dictionary.h"
#include "list.h"
#include "map.h"
#include "utils.h"
#include "vector.h"

#define ROUTE_MAX 1000

typedef struct Route {
    List cities;
} Route;

typedef struct Road {
    uint64_t length;
    int start;
    int end;
    int builtYear;
} Road;

typedef struct Map {
    Route routes[ROUTE_MAX];
    /// each neighbour holds a Dictionary[int, Road]
    Vector neighbours;
    Vector int_to_city;
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
        free(map->neighbours.arr[i]);
        map->neighbours.arr[i] = NULL;
    }
}

Map* newMap(void) {
    Map* map = calloc(1, sizeof(Map));
    CHECK_RET(map);
    /// route number 0 is invalid, as per task descritption
    memset(map->routes, 0, ROUTE_MAX * sizeof(Route));

    Dictionary* d = newDictionary(hash_string, undereferencing_strcmp, true, false);
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

    Vector* c = newVector();
    if (c == NULL) {
        goto DELETE_INT_TO_CITY;
    }
    map->int_to_city = *c;
    free(c);

    Dictionary* edges = newDictionary(hashEdge, cmpEdges, false, true);
    if (edges == NULL) {
        goto DELETE_EDGES_TO_LIST_OF_ROUTES;
    }
    map->routesThrough = *edges;
    free(edges);
    return map;

    DELETE_EDGES_TO_LIST_OF_ROUTES:
        vectorDelete(&map->int_to_city);
    DELETE_INT_TO_CITY:
        vectorDelete(&map->neighbours);
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
    vectorDelete(&map->int_to_city);
    deleteDictionaryOfLists(&map->routesThrough);
    free(map);
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

Status addCity(Map *map, const char* city) {
    CHECK_RET(map);
    CHECK_RET(city);

    size_t len = strlen(city);
    char* c = malloc(len + 1);
    CHECK_RET(c);
    strcpy(c, city);

    //int* city_id = malloc(sizeof(int));
    //*city_id = (int)map->city_to_int.size;
    void* city_id = (void*)((map->city_to_int.size << 32) + 1);
    CHECK_RET(insertDictionary(&map->city_to_int, (void*)c, city_id));

    Dictionary *d = newDictionary(hash_int, equalInt, false, true);
    if (d == NULL) {
        goto DELETE_FROM_DICTIONARY;
    }
    if (vectorAppend(&map->neighbours, d) == false) {
        goto DELETE_DICT;
    }
    if (vectorAppend(&map->int_to_city, (void*)city) == false) {
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

    //int id1 = *(int*)e1.val;
    //int id2 = *(int*)e2.val;
    int id1 = ((uint64_t)e1.val >> 32);
    int id2 = ((uint64_t)e2.val >> 32);
    Entry edge12 = getDictionary((map->neighbours.arr)[id1], &id2);
    Entry edge21 = getDictionary((map->neighbours.arr)[id2], &id1);
    if (!NOT_FOUND(edge12) || !NOT_FOUND(edge21)) {
        return false;
    }

    List* l = NULL;
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

    void* edge = encodeEdgeAsPtr(id1, id2);

    l = newList();
    CHECK_RET(l);
    if (insertDictionary(&map->routesThrough, edge, l) == false) {
        goto FREE_L;
    }
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
FREE_L:
    free(l);
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

    //int id1 = *(int*)e1.val;
    //int id2 = *(int*)e2.val;
    int id1 = ((uint64_t)e1.val >> 32);
    int id2 = ((uint64_t)e2.val >> 32);


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

Road nextNeighbour(Map* map, int src, bool reset) {
    static unsigned x;
    if (reset) {
        x = 0;
        return (const Road){0};
    }
    Dictionary* neighbours = map->neighbours.arr[src];
    do {
        x = (x + 1) % neighbours->array_size;
    } while (NOT_FOUND(neighbours->array[x]));
    return *(Road*)(neighbours->array[x].val);
}

Status shortestPaths(Map* map, int A, int B, bool visited[], int prev[], uint64_t* d, int* w) {
    const uint64_t infinity = -1;

    size_t cities_no = map->city_to_int.size;
    if (cities_no != map->neighbours.size) {
        abort();
    }

    uint64_t *dist = NULL;
    List* queue = NULL;
    bool vis = false;
    bool* is_in_queue = NULL;
    int *time = NULL;


    dist = malloc(cities_no * sizeof (uint64_t));
    CHECK_RET(dist);

    for (size_t i = 0; i < cities_no; ++i) {
        dist[i] = infinity;
    }
    dist[A] = 0;

    is_in_queue = calloc(cities_no, sizeof(bool));
    if (is_in_queue == NULL) {
        goto FREE_MEMORY;
    }

    queue = newList();
    if (queue == NULL) {
        goto FREE_MEMORY;
    }
    listInsertAfter(queue, queue->begin, A);

    time = calloc(cities_no, sizeof (int));
    if (time == NULL) {
        goto FREE_MEMORY;
    }
    for (size_t i = 0; i < cities_no; ++i) {
        time[i] = INT_MAX;
    }

    while (queue->begin->next != queue->end) {
        int x = queue->begin->next->value;
        visited[x] = true;
        deleteListNode(queue, queue->begin->next);
        is_in_queue[x] = false;
        // reset counter
        Road road = nextNeighbour(map, x, true);
        Dictionary* neighbours = map->neighbours.arr[x];

        for (size_t i = 0; i < neighbours->size; ++i) {
            road = nextNeighbour(map, x, false);
            if (visited[road.end] == false) {
                bool p1 = dist[road.end] > road.length + dist[x];
                bool p2 = dist[road.end] == road.length + dist[x];
                if (p1 || (p2 && time[road.end] <= min(time[x], road.builtYear))) {
                    prev[road.end] = x;
                    dist[road.end] = road.length + dist[x];
                    time[road.end] = min(time[x], road.builtYear);
                    if (is_in_queue[road.end] == false) {
                        listInsertAfter(queue, queue->end, road.end);
                        is_in_queue[road.end] = true;
                    }
                }
            }
        }
    }
    vis = visited[B];
    *d = dist[B];
    *w = time[B];


FREE_MEMORY:
    deleteList(queue);
    free(queue);
    free(dist);
    free(is_in_queue);
    free(time);

    return vis;
}

bool appendPath(Dictionary* routesThrough, int routeId, List* route, int* prev, bool to_end) {
    int current;
    if (to_end) {
        current = route->end->prev->value;
    } else {
        current = route->begin->next->value;
    }
    int inserted_count = 0;
    while (current != -1 && prev[current] != -1) {
        int p = current;
        current = prev[current];
        bool v;
        if (to_end) {
            v = listInsertBefore(route, route->end, current);
        } else {
            v = listInsertAfter(route, route->begin, current);
        }
        if (v == false) {
            for (int i = 0; i < inserted_count; ++i) {
                if (to_end) {
                    deleteListNode(route, route->end->prev);
                } else {
                    deleteListNode(route, route->begin->next);
                }
            }
            return false;
        }
        inserted_count++;
        Entry e = getDictionary(routesThrough, encodeEdgeAsPtr(p, current));
        List* l = e.val;
        listInsertAfter(l, l->begin, routeId);
    }
    return true;
}

bool newRoute(Map *map, unsigned routeId,
              const char *city1, const char *city2) {
    CHECK_RET(map);
    CHECK_RET(possiblyValidRoad(city1, city2));
    CHECK_RET(1 <= routeId && routeId < ROUTE_MAX);
    CHECK_RET(map->routes[routeId].cities.begin == NULL);
    CHECK_RET(map->routes[routeId].cities.end   == NULL);

    Entry e1 = getDictionary(&map->city_to_int, (void*)city1);
    CHECK_RET(NOT_FOUND(e1) == false);
    Entry e2 = getDictionary(&map->city_to_int, (void*)city2);
    CHECK_RET(NOT_FOUND(e2) == false);

//    int id1 = *(int*)e1.val;
//    int id2 = *(int*)e2.val;
    int id1 = ((uint64_t)e1.val >> 32);
    int id2 = ((uint64_t)e2.val >> 32);


    bool ret = false;

    bool* visited = NULL;
    int* prev = NULL;
    List *l = newList();
    CHECK_RET(l);
    map->routes[routeId].cities = *l;
    free(l);

    size_t cities_no = map->city_to_int.size;
    uint64_t d;
    int w;
    visited = calloc(cities_no, sizeof(bool));
    prev = malloc(cities_no * sizeof(int));
    if (visited == NULL || prev == NULL) {
        goto FREE_ROUTE;
    }
    memset(prev, 0xff, cities_no * sizeof(int));
    if (shortestPaths(map, id1, id2, visited, prev, &d, &w) == false) {
        goto FREE_ROUTE;
    }

    l = &map->routes[routeId].cities;
    if (listInsertAfter(l, l->begin, id2) == false) {
        goto FREE_ROUTE;
    }
    if (appendPath(&map->routesThrough, routeId, l, prev, false) == false) {
        goto FREE_ROUTE;
    }

    ret = true;
FREE_ROUTE:
    if (ret == false) {
       deleteList(&map->routes[routeId].cities);
    }
    free(visited);
    free(prev);

    return ret;
}

Road getRoad(Map* map, int id1, int id2) {
    Road null = {0};
    if (map == NULL || id1 == id2 || id1 < 0 || id2 < 0) {
        return null;
    }
    int64_t cities_no = map->city_to_int.size;
    if (id1 >= cities_no || id2 >= cities_no) {
        return null;
    }

    Entry edge12 = getDictionary((map->neighbours.arr)[id1], &id2);
    Entry edge21 = getDictionary((map->neighbours.arr)[id2], &id1);

    if (NOT_FOUND(edge12) || NOT_FOUND(edge21)) {
        return null;
    }
    if (areRoadsConsistent(edge12.val, edge21.val) == false) {
        return null;
    }
    return *(Road*)edge12.val;
}

size_t getRouteDescriptionLength(Map *map, unsigned routeId) {
    Route* route = &map->routes[routeId];
    size_t string_length = intLength(routeId) + 1;
    Node* node = route->cities.begin->next;
    while (node != route->cities.end->prev) {
        int current = node->value;
        int next = node->next->value;
        Road r = getRoad(map, current, next);

        string_length += strlen(map->int_to_city.arr[current]) + 1;
        string_length += intLength(r.length) + 1;
        string_length += intLength(r.builtYear) + 1;

        node = node->next;
    }
    string_length += strlen(map->int_to_city.arr[node->value]);
    return string_length;
}

char const* getRouteDescription(Map *map, unsigned routeId) {
    if (map == NULL || routeId == 0 || routeId >= ROUTE_MAX) {
        return NULL;
    }
    Route* route = &map->routes[routeId];
    // list is empty - that means that there is no such route
    if (route->cities.begin->next == route->cities.end) {
        return NULL;
    }

    size_t string_length = getRouteDescriptionLength(map, routeId);
    char* description = malloc(string_length + 1);
    CHECK_RET(description);
    int bytes_written;
    char* ptr = description;
    sprintf(description, "%u;%n", routeId, &bytes_written);
    ptr += bytes_written;
    Node* node = route->cities.begin->next;
    while (node != route->cities.end->prev) {
        int current = node->value;
        int next = node->next->value;
        Road r = getRoad(map, current, next);
        sprintf(ptr, "%s;%n", (char*)map->int_to_city.arr[current], &bytes_written);
        ptr += bytes_written;
        sprintf(ptr, "%"PRIu64";%n", r.length, &bytes_written);
        ptr += bytes_written;
        sprintf(ptr, "%d;%n", r.builtYear, &bytes_written);
        ptr += bytes_written;

        node = node->next;
    }
    sprintf(ptr, "%s%n", (char*)map->int_to_city.arr[node->value], &bytes_written);
    ptr += bytes_written;
    *ptr = 0;
    return description;
}


bool extendRoute(Map *map, unsigned routeId, const char *city) {
    CHECK_RET(map);
    CHECK_RET(1 <= routeId && routeId < ROUTE_MAX);
    CHECK_RET(validCityName(city));
    CHECK_RET(map->routes[routeId].cities.begin != NULL);
    CHECK_RET(map->routes[routeId].cities.end   != NULL);

    Entry e = getDictionary(&map->city_to_int, (void*)city);
    CHECK_RET(NOT_FOUND(e) == false);
    //int id = *(int*)e.val;
    int id = ((uint64_t)e.val >> 32);



    bool ret = false;
    bool* visited1 = NULL;
    bool* visited2 = NULL;
    int* prev1 = NULL;
    int* prev2 = NULL;

    List* route = &map->routes[routeId].cities;
    size_t cities_no = map->city_to_int.size;
    uint64_t d1, d2;
    int w1, w2;

    visited1 = calloc(cities_no, sizeof(bool));
    prev1 = malloc(cities_no * sizeof(int));
    if (visited1 == NULL || prev1 == NULL) {
        goto FREE;
    }
    memset(prev1, 0xff, cities_no * sizeof(int));
    for (Node* n =  route->begin->next; n != route->end; n = n->next) {
        visited1[n->value] = true;
    }
    visited1[route->begin->next->value] = false;
    if (shortestPaths(map, id, route->begin->next->value, visited1, prev1, &d1, &w1) == false) {
        goto FREE;
    }

    visited2 = calloc(cities_no, sizeof(bool));
    prev2 = malloc(cities_no * sizeof(int));
    if (visited2 == NULL || prev2 == NULL) {
        goto FREE;
    }
    memset(prev2, 0xff, cities_no * sizeof(int));

    for (Node* n =  route->begin->next; n != route->end; n = n->next) {
        visited2[n->value] = true;
    }
    visited2[route->end->prev->value] = false;
    if (shortestPaths(map, id, route->end->prev->value, visited2, prev2, &d2, &w2) == false) {
        goto FREE;
    }

    if (d1 < d2 || (d1 == d2 && w1 >= w2)) {
        if (appendPath(&map->routesThrough, routeId, route, prev1, false) == false) {
            goto FREE;
        }
    } else {
        if (appendPath(&map->routesThrough, routeId, route, prev2, true) == false) {
            goto FREE;
        }
    }

    ret = true;
FREE:
    free(visited1);
    free(visited2);
    free(prev1);
    free(prev2);
    return ret;
}

/*
bool removeRoad(Map *map, const char *city1, const char *city2) {
    CHECK_RET(map);
    CHECK_RET(possiblyValidRoad(city1, city2));

    Entry e1 = getDictionary(&map->city_to_int, (void*)city1);
    CHECK_RET(NOT_FOUND(e1) == false);
    Entry e2 = getDictionary(&map->city_to_int, (void*)city2);
    CHECK_RET(NOT_FOUND(e2) == false);

    getRoad(e1.val)
}
*/
