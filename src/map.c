#include <assert.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "dictionary.h"
#include "list.h"
#include "map.h"
#include "string_utils.h"
#include "vector.h"

#define ROUTE_MAX 1000

typedef struct Route {
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
// hudsihd iudhias uh uisa
        free(map->neighbours.arr[i]);
        map->neighbours.arr[i] = NULL;
    }
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

Map* newMap(void) {
    Map* map = calloc(1, sizeof(Map));
    CHECK_RET(map);
    // route number 0 is invalid, as per task descritption
    memset(map->routes, 0, ROUTE_MAX * sizeof(Route));


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

void deleteDictionaryOfLists(Dictionary* d) {
    for (size_t i = 0; i < d->array_size; ++i) {
        List* l = d->array[i].val;
        deleteList(l);
    }
   deleteDictionary(d);
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

Status shortestPaths(Map* map, int A, int B, bool visited[], int prev[], uint64_t* d) {
    const uint64_t infinity = -1;

    size_t cities_no = map->city_to_int.size;
    if (cities_no != map->neighbours.size) {
        abort();
    }

    uint64_t *dist = NULL;
    List* queue = NULL;
    bool vis = false;

    dist = malloc(cities_no * sizeof (uint64_t));
    CHECK_RET(dist);

    for (size_t i = 0; i < cities_no; ++i) {
        dist[i] = infinity;
    }
    dist[A] = 0;

    queue = newList();
    if (queue == NULL) {
        goto FREE_MEMORY;
    }
    listInsertAfter(queue, queue->begin, A);

    while (queue->begin->next != queue->end) {
        int x = queue->begin->next->value;
        visited[x] = true;
        deleteListNode(queue, queue->begin->next);
        // reset counter
        Road road = nextNeighbour(map, x, true);
        Dictionary* neighbours = map->neighbours.arr[x];

        for (size_t i = 0; i < neighbours->size; ++i) {
            road = nextNeighbour(map, x, false);
            if (visited[road.end] == false) {
                if (dist[road.end] > road.length + dist[x]) {
                    prev[road.end] = x;
                    dist[road.end] = road.length + dist[x];
                }
                listInsertAfter(queue, queue->end, road.end);
            }
        }
    }

    vis = visited[B];
    if (vis == true) {
        *d = dist[B];
    }

FREE_MEMORY:
    deleteList(queue);
    free(queue);
    free(dist);

    return vis;
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

    int id1 = *(int*)e1.val;
    int id2 = *(int*)e2.val;


    bool ret = false;

    bool* visited = NULL;
    int* prev = NULL;
    List *l = newList();
    CHECK_RET(l);
    map->routes[routeId].cities = *l;
    free(l);

    size_t cities_no = map->city_to_int.size;
    uint64_t d;
    visited = calloc(cities_no, sizeof(bool));
    prev = malloc(cities_no * sizeof(int));
    if (visited == NULL || prev == NULL) {
        ret = false;
        goto FREE_ROUTE;
    }
    memset(prev, 0xff, cities_no * sizeof(int));
    if (shortestPaths(map, id1, id2, visited, prev, &d) == false) {
        ret = false;
        goto FREE_ROUTE;
    }
    int current = id2;
    l = &map->routes[routeId].cities;
    while (current != id1) {
        if (listInsertAfter(l, l->begin, current) == false) {
            ret = false;
            goto FREE_ROUTE;
        }
        current = prev[current];
    }
    if (listInsertAfter(l, l->begin, current) == false) {
        ret = false;
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

size_t int_length(int64_t x) {
    // 3 == ceil(log10(256))
    char b[sizeof(x)*3+1];
    sprintf(b, "%"PRId64, x);
    return strlen(b);
}

char const* getRouteDescription(Map *map, unsigned routeId) {
    if (map == NULL || routeId == 0 || routeId >= 1000) {
        return NULL;
    }
    Route* route = &map->routes[routeId];
    // list is empty - that means that there is no such route
    if (route->cities.begin->next == route->cities.end) {
        return NULL;
    }

    size_t string_length = int_length(routeId) + 1;
    Node* node = route->cities.begin->next;
    while (node != route->cities.end->prev) {
        int current = node->value;
        int next = node->next->value;
        Road r = getRoad(map, current, next);

        string_length += strlen(map->int_to_city.arr[current]) + 1;
        string_length += int_length(r.length) + 1;
        string_length += int_length(r.builtYear) + 1;

        node = node->next;
    }
    string_length += strlen(map->int_to_city.arr[node->value]);

    char* description = malloc(string_length + 1);
    CHECK_RET(description);

    int bytes_written;
    char* ptr = description;
    sprintf(description, "%u;%n", routeId, &bytes_written);
    ptr += bytes_written;
    node = route->cities.begin->next;
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
