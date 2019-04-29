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
    /// each int_to_city element holds a char*
    Vector int_to_city;
    /// Dictionary[char*, int]
    Dictionary city_to_int;
    /// Dictionary[(int, int), List[int]]
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

    void* city_id = encodeCityId(map->city_to_int.size);
    CHECK_RET(insertDictionary(&map->city_to_int, (void*)c, city_id));

    Dictionary *d = newDictionary(hashInt, equalInt, false, true);
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
    int id1 = decodeCityId(e1.val);
    int id2 = decodeCityId(e2.val);

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

    int id1 = decodeCityId(e1.val);
    int id2 = decodeCityId(e2.val);


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

Status shortestPaths(Map* map, int A, int B, bool visited[], int prev[], uint64_t* d, int* w, bool fixing) {
    const uint64_t infinity = UINT64_MAX;

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
        deleteListNode(queue, queue->begin->next);
        if (visited[x] == true) {
            continue;
        }
        //visited[x] = true;
        is_in_queue[x] = false;
        // reset counter
        Road road = nextNeighbour(map, x, true);
        Dictionary* neighbours = map->neighbours.arr[x];

        for (size_t i = 0; i < neighbours->size; ++i) {
            road = nextNeighbour(map, x, false);
            if (fixing == false || encodeEdgeAsPtr(A, B) != encodeEdgeAsPtr(road.start, road.end)) {
   //         if (visited[road.end] == false) {
                bool p1 = dist[road.end] > road.length + dist[x];
                bool p2 = dist[road.end] == road.length + dist[x];
                if (p1 || (p2 && time[road.end] <= min(time[x], road.builtYear))) {
                    //if (road.end == B && p2 && time[road.end] == min(time[x], road.builtYear)) {
                    //    vis = false;
                    //    goto FREE_MEMORY;
                    //}
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
    vis = visited[B] < infinity;
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

bool appendPath(Dictionary* routesThrough, unsigned routeId, List* route, int* prev, Node* after) {
    int current = after->value;
    int inserted_count = 0;
    if (after == route->begin) {
        current = route->begin->next->value;
    } else if (after == route->end) {
        current = route->end->prev->value;
    }
    while (current != -1 && prev[current] != -1) {
        int p = current;
        current = prev[current];
        bool v = true;
        if (route != NULL) {
            v = listInsertAfter(route, after, current);
        }
        if (v == false) {
            for (int i = 0; i < inserted_count; ++i) {
                deleteListNode(route, after);
            }
            return false;
        }
        inserted_count++;
        if (routesThrough != NULL) {
            Entry e = getDictionary(routesThrough, encodeEdgeAsPtr(p, current));
            List* l = e.val;
            listInsertAfter(l, l->begin, routeId);
        }
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
    int id1 = decodeCityId(e1.val);
    int id2 = decodeCityId(e2.val);

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
    if (shortestPaths(map, id1, id2, visited, prev, &d, &w, false) == false) {
        goto FREE_ROUTE;
    }

    l = &map->routes[routeId].cities;
    if (listInsertAfter(l, l->begin, id2) == false) {
        goto FREE_ROUTE;
    }
    if (appendPath(&map->routesThrough, routeId, l, prev, l->begin) == false) {
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
    int id = decodeCityId(e.val);

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
    if (shortestPaths(map, id, route->begin->next->value, visited1, prev1, &d1, &w1, false) == false) {
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
    if (shortestPaths(map, id, route->end->prev->value, visited2, prev2, &d2, &w2, false) == false) {
        goto FREE;
    }

    if (d1 < d2 || (d1 == d2 && w1 >= w2)) {
        if (appendPath(&map->routesThrough, routeId, route, prev1, false) == false) {
            goto FREE;
        }
    } else {
        if (appendPath(&map->routesThrough, routeId, route, prev2, route->end) == false) {
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

List* repairRoute(Map *map, unsigned routeId, int id1, int id2) {
    const uint64_t infinity = UINT64_MAX;
    List* cities = &map->routes[routeId].cities;
    size_t cities_no = map->city_to_int.size;

    bool* visited;
    int* prev;
    List* ret = NULL;

    visited = calloc(cities_no, sizeof(bool));
    prev  = calloc(cities_no, sizeof(int));
    if (visited == NULL || prev == NULL) {
        goto FREE;
    }
    for (size_t i = 0; i < cities_no; ++i) {
        prev[i] = -1;
    }
    for (Node* n = cities->begin->next; n != cities->end; n = n->next) {
        visited[n->value] = true;
    }
    visited[id1] = false;
    visited[id2] = false;

    uint64_t d;
    int w;
    if (shortestPaths(map, id2, id1, visited, prev, &d, &w, true) == false) {
        goto FREE;
    }
    if (d == infinity) {
        goto FREE;
    }

    //ret = copyList(cities);
    ret = newList();
    if (ret == NULL) {
        goto FREE;
    }
    if (listInsertAfter(ret, ret->begin, prev[id1]) == false) {
        goto FREE;
    }
    if (appendPath(NULL, 0, ret, prev, ret->end) == false) {
        deleteList(ret);
        free(ret);
        ret = NULL;
        goto FREE;
    }
    deleteListNode(ret, ret->end->prev);
FREE:
    free(prev);
    free(visited);
    return ret;
}


bool removeRoad(Map *map, const char *city1, const char *city2) {

    CHECK_RET(map);
    CHECK_RET(possiblyValidRoad(city1, city2));

    Entry e1 = getDictionary(&map->city_to_int, (void*)city1);
    CHECK_RET(NOT_FOUND(e1) == false);
    Entry e2 = getDictionary(&map->city_to_int, (void*)city2);
    CHECK_RET(NOT_FOUND(e2) == false);

    int id1 = decodeCityId(e1.val);
    int id2 = decodeCityId(e2.val);
    Road road = getRoad(map, id1, id2);
    if (road.builtYear == 0 || road.start != id1 || road.end != id2) {
        return false;
    }

    Entry e = getDictionary(&map->routesThrough, encodeEdgeAsPtr(id1, id2));
    CHECK_RET(NOT_FOUND(e) == false);
    List* routesThrough = e.val;
    Vector* new_routes = newVector();
    bool ret = false;

    if (new_routes == NULL) {
        return false;
    }
    for (Node* n = routesThrough->begin->next; n != routesThrough->end; n = n->next) {
        List* l = repairRoute(map, (unsigned)n->value, id1, id2);
        if (l == NULL) {
            goto FREE;
        }
        if (listInsertBefore(l, l->begin, id1) == false) {
            goto FREE;
        }
        if (listInsertAfter(l, l->end, id2) == false) {
            goto FREE;
        }
        if (vectorAppend(new_routes, l) == false) {
            goto FREE;
        }
    }
    size_t index = 0;
    for (Node* n = routesThrough->begin->next; n != routesThrough->end; n = n->next) {
        unsigned routeId = n->value;
        // uzupełnienie map routesThrough
        List* r = &((Route*)new_routes->arr[index])->cities;
        for (Node* n = r->begin->next; n != r->end->prev; n = n->next) {
            int a = n->value;
            int b = n->next->value;
            Entry e = getDictionary(&map->routesThrough, encodeEdgeAsPtr(a, b));
            List* l = e.val;
            if (listInsertAfter(l, l->end, routeId) == false) {
                size_t rindex = index;
                for (Node* nn = n; nn != routesThrough->begin; nn = nn->prev) {
                    unsigned routeId = nn->value;
                    // uzupełnienie map routesThrough
                    List* r = &((Route*)new_routes->arr[rindex])->cities;
                    for (Node* n = r->begin->next; n != r->end; n = n->next) {
                        int a = n->value;
                        int b = n->next->value;
                        Entry e = getDictionary(&map->routesThrough, encodeEdgeAsPtr(a, b));
                        List* l = e.val;
                        if (l->end->prev->value == routeId) {
                            deleteListNode(l, l->end);
                        }
                    }
                    rindex -= 1;
                }
                goto FREE;
            }
        }
        index++;
    }


    index = 0;
    for (Node* n = routesThrough->begin->next; n != routesThrough->end; n = n->next) {
        unsigned routeId = n->value;
        List* r = &((Route*)new_routes->arr[index])->cities;
        int a = r->begin->next->value;
        int b = r->end->prev->value;
        deleteListNode(r, r->begin->next);
        deleteListNode(r, r->end->prev);
        if (listPos(&map->routes[routeId].cities, a) <= listPos(&map->routes[routeId].cities, b)) {
            insertListAfterElement(&map->routes[routeId].cities, r, a);
        } else {
            listReverse(r);
            insertListAfterElement(&map->routes[routeId].cities, r, b);
        }
        index++;
    }

    deleteList(routesThrough);
    deleteFromDictionary(&map->routesThrough, encodeEdgeAsPtr(id1, id2));
    vectorDeleteFreeContent(new_routes);

    ret = true;
FREE:
    //deleteVectorOfLists(new_routes);
    //vectorDeleteFreeContent(new_routes);
    free(new_routes);
    return ret;
}
