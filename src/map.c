#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map_struct.h"
#include "shortest_paths.h"
#include "utils.h"
#include "map.h"

#define INFINITY UINT64_MAX

static void deleteRoutes(Map *map) {
    for (int i = 0; i < ROUTE_MAX; ++i) {
        deleteList(&map->routes[i].cities);
    }
}

static void deleteAdjacencyDictionaries(Map *map) {
    if (map == NULL) {
        return;
    }
    for (size_t i = 0; i < map->neighbours.size; ++i) {
        deleteDictionary(map->neighbours.arr[i]);
        free(map->neighbours.arr[i]);
        map->neighbours.arr[i] = NULL;
    }
}

static void free_(void *ptr) {
    if (ptr != DELETED) {
        free(ptr);
    }
}

static void free_list(void *ptr) {
    List *l = ptr;
    if (l != NULL && l != DELETED) {
        deleteList(l);
        free(l->begin);
        free(l);
    }
}

Map *newMap(void) {
    Map *map = calloc(1, sizeof(Map));
    CHECK_RET(map);
    /// route number 0 is invalid, as per task descritption
    memset(map->routes, 0, ROUTE_MAX * sizeof(Route));

    Dictionary *d =
        newDictionary(hashString, undereferencing_strcmp, free_, empty);
    if (d == NULL) {
        goto DELETE_ROUTES;
    }
    map->city_to_int = *d;
    free(d);

    Vector *n = newVector();
    if (n == NULL) {
        goto DELETE_CITY_TO_INT;
    }
    map->neighbours = *n;
    free(n);

    Vector *c = newVector();
    if (c == NULL) {
        goto DELETE_INT_TO_CITY;
    }
    map->int_to_city = *c;
    free(c);

    Dictionary *edges = newDictionary(hashEdge, cmpEdges, empty, free_list);
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
    deleteMap(map);
    return NULL;
}

void deleteMap(Map *map) {
    deleteRoutes(map);
    // deleteDictionary(&map->city_to_int);
    free(map->city_to_int.array);
    deleteAdjacencyDictionaries(map);
    vectorDeleteFreeContent(&map->neighbours);
    // deleteDictionaryOfLists(&map->routesThrough);
    deleteDictionary(&map->routesThrough);
    vectorDeleteFreeContent(&map->int_to_city);

    free(map);
}

static bool areRoadsConsistent(Road *r1, Road *r2) {
    CHECK_RET(r1);
    CHECK_RET(r2);
    CHECK_RET(r1->builtYear == r2->builtYear);
    CHECK_RET(r1->length == r2->length);
    CHECK_RET(r1->end == r2->start);
    CHECK_RET(r2->start == r1->end);
    return true;
}

Status addCity(Map *map, const char *city) {
    CHECK_RET(map);
    CHECK_RET(city);
    if (!NOT_FOUND(getDictionary(&map->city_to_int, (void *)city))) {
        return true;
    }

    size_t len = strlen(city);
    char *c = malloc(len + 1);
    CHECK_RET(c);
    strcpy(c, city);

    void *city_id = encodeCityId(map->city_to_int.size);
    CHECK_RET(insertDictionary(&map->city_to_int, (void *)c, city_id));

    Dictionary *d = newDictionary(hashInt, equalInt, empty, free_);
    if (d == NULL) {
        goto DELETE_FROM_DICTIONARY;
    }
    if (vectorAppend(&map->neighbours, d) == false) {
        goto DELETE_DICT;
    }
    if (vectorAppend(&map->int_to_city, (void *)c) == false) {
        goto DELETE_V;
    }
    return true;

DELETE_V:
    vectorRemoveLast(&map->neighbours, false);
DELETE_DICT:
    deleteDictionary(d);
    free(d);
DELETE_FROM_DICTIONARY:
    deleteFromDictionary(&map->city_to_int, (void *)c);
    free(c);
    return false;
}

bool addRoad(Map *map, const char *city1, const char *city2, unsigned length,
             int builtYear) {
    CHECK_RET(builtYear);
    CHECK_RET(length);
    CHECK_RET(map);
    CHECK_RET(possiblyValidRoad(city1, city2));

    Entry e1 = getDictionary(&map->city_to_int, (void *)city1);
    Entry e2 = getDictionary(&map->city_to_int, (void *)city2);

    // don't bother with deleting this, in case of further failure
    if (NOT_FOUND(e1)) {
        CHECK_RET(addCity(map, city1));
    }
    if (NOT_FOUND(e2)) {
        CHECK_RET(addCity(map, city2));
    }

    e1 = getDictionary(&map->city_to_int, (void *)city1);
    e2 = getDictionary(&map->city_to_int, (void *)city2);
    int id1 = decodeCityId(e1.val);
    int id2 = decodeCityId(e2.val);

    Entry edge12 = getDictionary((map->neighbours.arr)[id1], &id2);
    Entry edge21 = getDictionary((map->neighbours.arr)[id2], &id1);
    if (!NOT_FOUND(edge12) || !NOT_FOUND(edge21)) {
        return false;
    }

    List *l = NULL;
    Road *r1 = calloc(1, sizeof(Road));
    Road *r2 = calloc(1, sizeof(Road));
    if (r1 == NULL || r2 == NULL) {
        goto FREE_R;
    }
    *r1 = (const Road){
        .builtYear = builtYear, .length = length, .start = id1, .end = id2};
    *r2 = *r1;
    r2->start = r1->end;
    r2->end = r1->start;

    void *edge = encodeEdgeAsPtr(id1, id2);

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

bool repairRoad(Map *map, const char *city1, const char *city2,
                int repairYear) {
    CHECK_RET(map);
    CHECK_RET(repairYear);
    CHECK_RET(possiblyValidRoad(city1, city2));

    Entry e1 = getDictionary(&map->city_to_int, (void *)city1);
    CHECK_RET(NOT_FOUND(e1) == false);
    Entry e2 = getDictionary(&map->city_to_int, (void *)city2);
    CHECK_RET(NOT_FOUND(e2) == false);

    int id1 = decodeCityId(e1.val);
    int id2 = decodeCityId(e2.val);

    Entry edge12 = getDictionary((map->neighbours.arr)[id1], &id2);
    Entry edge21 = getDictionary((map->neighbours.arr)[id2], &id1);

    if (NOT_FOUND(edge12) || NOT_FOUND(edge21)) {
        return false;
    }
    Road *r1 = edge12.val;
    Road *r2 = edge21.val;

    assert(areRoadsConsistent(r1, r2));

    CHECK_RET(repairYear >= r1->builtYear);
    r1->builtYear = repairYear;
    r2->builtYear = repairYear;
    return true;
}

Status canInsertEdge(Map *map, char *city1, char *city2) {
    CHECK_RET(map);
    CHECK_RET(city1);
    CHECK_RET(city2);
    Entry e1 = getDictionary(&map->city_to_int, (void *)city1);
    if (NOT_FOUND(e1)) {
        return true;
    }
    Entry e2 = getDictionary(&map->city_to_int, (void *)city2);
    if (NOT_FOUND(e2)) {
        return true;
    }
    int id1 = decodeCityId(e1.val);
    int id2 = decodeCityId(e2.val);
    Entry edge12 = getDictionary((map->neighbours.arr)[id1], &id2);
    Entry edge21 = getDictionary((map->neighbours.arr)[id2], &id1);
    return (NOT_FOUND(edge12) || NOT_FOUND(edge21));
}

Status addRoadRepair(Map *map, char *city1, char *city2, unsigned length,
                     int builtYear) {
    if (canInsertEdge(map, city1, city2)) {
        return addRoad(map, city1, city2, length, builtYear);
    } else {
        return repairRoad(map, city1, city2, builtYear);
    }
}

bool appendPath(Dictionary *routesThrough, unsigned routeId, List *route,
                int *prev, Node *after) {
    int current = after->value;
    int inserted_count = 0;
    // Node* x;
    if (after == route->begin) {
        current = route->begin->next->value;
        //    x = after->next;
    } else if (after == route->end) {
        current = route->end->prev->value;
        //    x = after->prev;
    } else {
        fprintf(stderr, "WAT\n");
        //    x = after;
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
        if (routesThrough != NULL) {
            Entry e = getDictionary(routesThrough, encodeEdgeAsPtr(p, current));
            assert(!NOT_FOUND(e));
            List *l = e.val;
            if (listInsertAfter(l, l->begin, routeId) == false) {
                // TODO: co tutaj zrobić?
            }
        }
        inserted_count++;
    }
    return true;
}

bool newRoute(Map *map, unsigned routeId, const char *city1,
              const char *city2) {
    CHECK_RET(map);
    CHECK_RET(possiblyValidRoad(city1, city2));
    CHECK_RET(1 <= routeId && routeId < ROUTE_MAX);
    CHECK_RET(map->routes[routeId].cities.begin == NULL);
    CHECK_RET(map->routes[routeId].cities.end == NULL);

    Entry e1 = getDictionary(&map->city_to_int, (void *)city1);
    CHECK_RET(NOT_FOUND(e1) == false);
    Entry e2 = getDictionary(&map->city_to_int, (void *)city2);
    CHECK_RET(NOT_FOUND(e2) == false);
    int id1 = decodeCityId(e1.val);
    int id2 = decodeCityId(e2.val);

    Status ret = false;

    bool *visited = NULL;
    int *prev = NULL;
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
    if (d == UINT64_MAX) {
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

Road getRoad(Map *map, int id1, int id2) {
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
    return *(Road *)edge12.val;
}

static size_t getRouteDescriptionLength(Map *map, unsigned routeId) {
    Route *route = &map->routes[routeId];
    size_t string_length = intLength(routeId) + 1;
    Node *node = route->cities.begin->next;
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

char const *getRouteDescription(Map *map, unsigned routeId) {
    if (map == NULL || routeId == 0 || routeId >= ROUTE_MAX) {
        char *c = malloc(1);
        if (c == NULL) {
            return NULL;
        }
        c[0] = 0;
        return c;
    }
    Route *route = &map->routes[routeId];
    // list is empty - that means that there is no such route
    if (route->cities.begin == route->cities.end ||
        route->cities.begin->next == route->cities.end) {
        char *c = malloc(1);
        if (c == NULL) {
            return NULL;
        }
        c[0] = 0;
        return c;
    }

    size_t string_length = getRouteDescriptionLength(map, routeId);
    char *description = malloc(string_length + 1);
    CHECK_RET(description);
    int bytes_written;
    char *ptr = description;
    sprintf(description, "%u;%n", routeId, &bytes_written);
    ptr += bytes_written;
    Node *node = route->cities.begin->next;
    while (node != route->cities.end->prev) {
        int current = node->value;
        int next = node->next->value;
        Road r = getRoad(map, current, next);
        sprintf(ptr, "%s;%n", (char *)map->int_to_city.arr[current],
                &bytes_written);
        ptr += bytes_written;
        sprintf(ptr, "%" PRIu64 ";%n", r.length, &bytes_written);
        ptr += bytes_written;
        sprintf(ptr, "%d;%n", r.builtYear, &bytes_written);
        ptr += bytes_written;

        node = node->next;
    }
    sprintf(ptr, "%s%n", (char *)map->int_to_city.arr[node->value],
            &bytes_written);
    ptr += bytes_written;
    *ptr = 0;
    return description;
}

bool extendRoute(Map *map, unsigned routeId, const char *city) {
    CHECK_RET(map);
    CHECK_RET(1 <= routeId && routeId < ROUTE_MAX);
    CHECK_RET(validCityName(city));
    CHECK_RET(map->routes[routeId].cities.begin != NULL);
    CHECK_RET(map->routes[routeId].cities.end != NULL);

    Entry e = getDictionary(&map->city_to_int, (void *)city);
    CHECK_RET(NOT_FOUND(e) == false);
    int id = decodeCityId(e.val);

    List *route = &map->routes[routeId].cities;
    size_t cities_no = map->city_to_int.size;
    uint64_t d1, d2;
    int w1, w2;

    CHECK_RET(id != route->begin->next->value && id != route->end->prev->value);

    Status ret = false;
    bool *visited1 = NULL;
    bool *visited2 = NULL;
    int *prev1 = NULL;
    int *prev2 = NULL;

    visited1 = calloc(cities_no, sizeof(bool));
    prev1 = malloc(cities_no * sizeof(int));
    if (visited1 == NULL || prev1 == NULL) {
        goto FREE;
    }
    memset(prev1, 0xff, cities_no * sizeof(int));
    for (Node *n = route->begin->next; n != route->end; n = n->next) {
        visited1[n->value] = true;
    }
    visited1[route->begin->next->value] = false;
    if (shortestPaths(map, id, route->begin->next->value, visited1, prev1, &d1,
                      &w1, false) == false) {
        goto FREE;
    }

    visited2 = calloc(cities_no, sizeof(bool));
    prev2 = malloc(cities_no * sizeof(int));
    if (visited2 == NULL || prev2 == NULL) {
        goto FREE;
    }
    memset(prev2, 0xff, cities_no * sizeof(int));

    for (Node *n = route->begin->next; n != route->end; n = n->next) {
        visited2[n->value] = true;
    }
    visited2[route->end->prev->value] = false;
    if (shortestPaths(map, id, route->end->prev->value, visited2, prev2, &d2,
                      &w2, false) == false) {
        goto FREE;
    }
    if (d1 == INFINITY && d2 == INFINITY) {
        goto FREE;
    }
    if (d1 < d2 || (d1 == d2 && w1 >= w2)) {
        if (appendPath(&map->routesThrough, routeId, route, prev1,
                       route->begin) == false) {
            goto FREE;
        }
    } else if (appendPath(&map->routesThrough, routeId, route, prev2,
                          route->end) == false) {
        goto FREE;
    }

    ret = true;
FREE:
    free(visited1);
    free(visited2);
    free(prev1);
    free(prev2);
    return ret;
}

Status extendPathFromPrev(List *route, int *prev, int start, int end) {
    int current = start;
    while (current != -1 && current != end) {
        if (listInsertAfter(route, route->begin, current) == false) {
            // deleteList(route);
            return false;
        }
        current = prev[current];
    }
    return true;
}

List *repairRoute(Map *map, unsigned routeId, int id1, int id2) {
    const uint64_t infinity = UINT64_MAX;
    List *cities = &map->routes[routeId].cities;
    size_t cities_no = map->city_to_int.size;

    bool *visited;
    int *prev;
    List *ret = NULL;

    visited = calloc(cities_no, sizeof(bool));
    prev = calloc(cities_no, sizeof(int));
    if (visited == NULL || prev == NULL) {
        goto FREE;
    }
    for (size_t i = 0; i < cities_no; ++i) {
        prev[i] = -1;
    }
    for (Node *n = cities->begin->next; n != cities->end; n = n->next) {
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

    ret = newList();
    if (ret == NULL) {
        goto FREE;
    }
    if (extendPathFromPrev(ret, prev, id1, id2) == false) {
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

static void vectorDeleteFreeListContent(Vector *vector, bool x) {
    if (vector == NULL) {
        return;
    }
    for (size_t i = 0; i < vector->size; ++i) {
        if (x) {
            free(((List *)vector->arr[i])->begin);
        } else {
            deleteList(vector->arr[i]);
        }
        free(vector->arr[i]);
        vector->arr[i] = NULL;
    }
    vectorDeleteFreeContent(vector);
}

bool routeIsValid(Map *map, Route *route) {
    List l = route->cities;
    Node *n = l.begin->next;
    Node *nn = l.begin->next->next;
    while (nn != l.end) {
        Road r = getRoad(map, n->value, nn->value);
        if (r.builtYear == 0) {
            return false;
        }
        n = n->next;
        nn = nn->next;
    }
    return true;
}

bool removeRoad(Map *map, const char *city1, const char *city2) {

    CHECK_RET(map);
    CHECK_RET(possiblyValidRoad(city1, city2));

    Entry e1 = getDictionary(&map->city_to_int, (void *)city1);
    CHECK_RET(NOT_FOUND(e1) == false);
    Entry e2 = getDictionary(&map->city_to_int, (void *)city2);
    CHECK_RET(NOT_FOUND(e2) == false);

    int id1 = decodeCityId(e1.val);
    int id2 = decodeCityId(e2.val);
    Road road = getRoad(map, id1, id2);
    if (road.builtYear == 0 || road.start != id1 || road.end != id2) {
        return false;
    }

    Entry e = getDictionary(&map->routesThrough, encodeEdgeAsPtr(id1, id2));
    CHECK_RET(NOT_FOUND(e) == false);
    List *routesThrough = e.val;
    Vector *new_routes = newVector();
    Status ret = false;

    if (new_routes == NULL) {
        return false;
    }
    for (Node *n = routesThrough->begin->next; n != routesThrough->end;
         n = n->next) {
        List *l = repairRoute(map, (unsigned)n->value, id1, id2);
        if (l == NULL) {
            goto FREE;
        }
        if (listInsertBefore(l, l->end, id1) == false) {
            goto FREE;
        }
        if (listInsertAfter(l, l->begin, id2) == false) {
            goto FREE;
        }
        if (vectorAppend(new_routes, l) == false) {
            goto FREE;
        }
    }
    size_t index = 0;
    for (Node *n = routesThrough->begin->next; n != routesThrough->end;
         n = n->next) {
        unsigned routeId = n->value;
        // uzupełnienie map routesThrough
        List *r = &((Route *)new_routes->arr[index])->cities;
        for (Node *n = r->begin->next; n != r->end->prev; n = n->next) {
            int a = n->value;
            int b = n->next->value;
            Entry e = getDictionary(&map->routesThrough, encodeEdgeAsPtr(a, b));
            List *l = e.val;
            if (listInsertAfter(l, l->end, routeId) == false) {
                size_t rindex = index;
                for (Node *nn = n; nn != routesThrough->begin; nn = nn->prev) {
                    unsigned routeId = nn->value;
                    // uzupełnienie map routesThrough
                    List *r = &((Route *)new_routes->arr[rindex])->cities;
                    for (Node *n = r->begin->next; n != r->end; n = n->next) {
                        int a = n->value;
                        int b = n->next->value;
                        Entry e = getDictionary(&map->routesThrough,
                                                encodeEdgeAsPtr(a, b));
                        List *l = e.val;
                        if (l->end->prev->value == (int)routeId) {
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
    for (Node *n = routesThrough->begin->next; n != routesThrough->end;
         n = n->next) {
        unsigned routeId = n->value;
        List *r = &((Route *)new_routes->arr[index])->cities;

        int a = r->begin->next->value;
        int b = r->end->prev->value;
        assert(a == id1 || a == id2);
        assert(b == id2 || b == id1);
        deleteListNode(r, r->begin->next);
        deleteListNode(r, r->end->prev);
        Node *anode = listFind(&map->routes[routeId].cities, id1);
        if (anode == NULL) {
            // abort();
        }
        if (anode->next->value == b) {
            insertListAfterElement(&map->routes[routeId].cities, r, a);
        } else if (anode->prev->value == b) {
            listReverse(r);
            insertListAfterElement(&map->routes[routeId].cities, r, b);
        } else {
            int x = 0;
            x++;

            // abort();
        }
        // if (listPos(&map->routes[routeId].cities, a) <=
        // listPos(&map->routes[routeId].cities, b)) {
        // } else {
        // }
        index++;
    }

    deleteList(routesThrough);
    deleteFromDictionary(&map->routesThrough, encodeEdgeAsPtr(id1, id2));
    deleteFromDictionary(map->neighbours.arr[id1], &id2);
    deleteFromDictionary(map->neighbours.arr[id2], &id1);

    ret = true;
FREE:
    vectorDeleteFreeListContent(new_routes, ret);
    free(new_routes);
    return ret;
}
