#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "map_struct.h"
#include "shortest_paths.h"
#include "utils.h"

/// Nieskończoność - długość najprawdopodobniej dłuższa niż długość każdej
/// ścieżki.
#define INFINITY UINT64_MAX

/** @brief Usuwa drogi krajowe.
 * @param[in,out] map       - mapa, z której usuwamy drogi krajowe
 */
static void deleteRoutes(Map *map) {
    for (int i = 0; i < ROUTE_MAX; ++i) {
        deleteList(&map->routes[i].cities);
    }
}

/** @brief Usuwa listę sąsiedztwa, która reprezentuje drogi między miastami.
 * @param[in,out] map       - mapa, z której usuwamy drogi
 */
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

/** @brief Zwalnia pamięć z wskaźnika @p ptr o ile ten nie jest @p DELETED.
 * @param[in,out] ptr           - wskaźnik, spod którego opróżniamy pamięć
 */
static void free_(void *ptr) {
    if (ptr != DELETED) {
        free(ptr);
    }
}

/** @brief Opróżnia listę (zwalnia pamięć).
 * Współpracuje z listami, które są elementami słownika.
 * @param[in,out] ptr          - wskaźnik na listę
 */
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
    /// Droga krajowa o numerze 0 jest niepoprawna.
    memset(map->routes, 0, ROUTE_MAX * sizeof(Route));

    Dictionary *d =
        newDictionary(hashString, undereferencing_strcmp, free_, empty);
    if (d == NULL) {
        goto DELETE;
    }
    map->city_to_int = *d;
    free(d);

    Vector *n = newVector();
    if (n == NULL) {
        goto DELETE;
    }
    map->neighbours = *n;
    free(n);

    Vector *c = newVector();
    if (c == NULL) {
        goto DELETE;
    }
    map->int_to_city = *c;
    free(c);

    Dictionary *edges = newDictionary(hashEdge, cmpEdges, empty, free_list);
    if (edges == NULL) {
        goto DELETE;
    }
    map->routesThrough = *edges;
    free(edges);
    return map;

DELETE:
    deleteRoutes(map);
    deleteMap(map);
    return NULL;
}

void deleteMap(Map *map) {
    deleteRoutes(map);
    deleteDictionary(&map->city_to_int);
    free(map->city_to_int.array);
    deleteAdjacencyDictionaries(map);
    vectorDeleteFreeContent(&map->neighbours);
    deleteDictionary(&map->routesThrough);
    vectorDelete(&map->int_to_city);
    free(map);
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

/** @brief Stwierdza, czy drogi @p r1 i @p r2 są tą samą drogą, tylko w
 * przeciwnym kierunku.
 * @param r1
 * @param r2
 * @return
 */
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
    if (!insertDictionary(&map->city_to_int, (void *)c, city_id)) {
        free(c);
        return false;
    }

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
        goto FREE_L;
    }
    *r1 = (const Road){
        .builtYear = builtYear, .length = length, .start = id1, .end = id2};
    *r2 = *r1;
    r2->start = r1->end;
    r2->end = r1->start;

    void *edge = encodeEdgeAsPtr(id1, id2);

    l = newList();
    if (!l) {
        goto FREE_L;
    }
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
    deleteFromDictionary(&map->routesThrough, edge);
    l = NULL;
FREE_L:
    deleteList(l);
    free(l);
    free(r1);
    free(r2);

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

Status addRoadRepair(Map *map, char *city1, char *city2, unsigned length,
                     int builtYear) {
    if (getRoadFromName(map, city1, city2).length == 0) {
        return addRoad(map, city1, city2, length, builtYear);
    } else {
        return repairRoad(map, city1, city2, builtYear);
    }
}

/** @brief Dodaje do drogi krajowej fragment wskazany przez tablicę @p prev.
 * @param[in,out] routesThrough    - słownik dróg krajowych  przebiagających
 * przez drogi
 * @param[in] routeId              - numer drogi krajowej, którą modyfikujemy
 * @param[in,out] route            - droga krajowa, którą modyfikujemy
 * @param[in] prev                 - tablica wyznaczonych poprzedników elementów
 * @param[in] after                - element, po którym należy wstawiać nowe
 * wierzchołki
 * @return Status powodzenia operacji.
 */
static Status appendPath(Dictionary *routesThrough, unsigned routeId,
                         List *route, int *prev, Node *after) {
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
        if (routesThrough != NULL) {
            Entry e = getDictionary(routesThrough, encodeEdgeAsPtr(p, current));
            assert(!NOT_FOUND(e));
            List *l = e.val;
            if (listInsertAfter(l, l->begin, routeId) == false) {
                return false;
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

/** @brief Liczy długość reprezentacji tekstowej drogi krajowej.
 * @param[in] map       - mapa dróg krajowych, w krórej znajduje się ta szukana
 * @param[in] routeId   - numer drogi krajowej
 * @return Długość reprezentacji tekstowej drogi krajowej, o ile ta istnieje.
 * W przeciwnym wypadku 0.
 */
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
    // this sets every element to -1
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

static Status extendPathFromPrev(List *route, int *prev, int start, int end) {
    int current = start;
    while (current != -1 && current != end) {
        if (listInsertAfter(route, route->begin, current) == false) {
            return false;
        }
        current = prev[current];
    }
    return true;
}

/** @brief repairRoute naprawia drogę krajową nr. @p routeId po usunięciu drogi.
 * Droga usuwana łączy @p id1 oraz @p id2.
 * @param[in,out] map       - mapa, którą modyfikujemy
 * @param[in] routeId       - numer drogi krajowej, którą naprawiamy
 * @param[in] id1           - początek usuwanej drogi
 * @param[in] id2           - koniec usuwanej drogi
 * @return lista wierzchołków, które należy wstawić pomiędzy @p id1 i @p id2 na
 * strukturze reprezentującej drogę krajową.
 */
static List *repairRoute(Map *map, unsigned routeId, int id1, int id2) {
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

/** @brief Zwalnia wektor.
 * Jeśli @p free_as_list true, opróźnia listę pod każdym elementem, w
 * przeciwnym wypadku tylko zwalnia głowę listy.
 * @param[in,out] vector        - wektor do opróżnienia
 * @param[in] free_as_list      - parametr opisuje, czy zwolnić całą listę, czy
 * tylko jej głowę i ogon (jeśli odpowiedzialność nad zawartością listy została
 * przeniesiona wcześniej gdzie indziej.
 */
static void vectorDeleteFreeListContent(Vector *vector, bool free_as_list) {
    if (vector == NULL) {
        return;
    }
    for (size_t i = 0; i < vector->size; ++i) {
        if (!free_as_list) {
            free(((List *)vector->arr[i])->begin);
        } else {
            deleteList(vector->arr[i]);
        }
        free(vector->arr[i]);
        vector->arr[i] = NULL;
    }
    vectorDeleteFreeContent(vector);
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
        if (!l || !listInsertBefore(l, l->end, id1) ||
            !listInsertAfter(l, l->begin, id2)) {
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
        if (listPos(&map->routes[routeId].cities, a) <=
            listPos(&map->routes[routeId].cities, b)) {
            insertListAfterElement(&map->routes[routeId].cities, r, a);
        } else {
            listReverse(r);
            insertListAfterElement(&map->routes[routeId].cities, r, b);
        }
        index++;
    }

    deleteList(routesThrough);
    deleteFromDictionary(&map->routesThrough, encodeEdgeAsPtr(id1, id2));
    deleteFromDictionary(map->neighbours.arr[id1], &id2);
    deleteFromDictionary(map->neighbours.arr[id2], &id1);

    ret = true;
FREE:
    vectorDeleteFreeListContent(new_routes, !ret);
    free(new_routes);
    return ret;
}

bool removeRoute(Map *map, unsigned routeId) {
    if (!map) {
        return false;
    }
    if (map->routes[routeId].cities.begin == NULL) {
        return false;
    }
    if (isEmptyList(&map->routes[routeId].cities)) {
        return false;
    }

    Node *n1 = map->routes[routeId].cities.begin->next;
    Node *n2 = map->routes[routeId].cities.begin->next->next;
    if (n2 == map->routes[routeId].cities.end) {
        return false;
    }
    while (n2 != map->routes[routeId].cities.end) {
        Entry e = getDictionary(&map->routesThrough,
                                encodeEdgeAsPtr(n1->value, n2->value));
        if (NOT_FOUND(e)) {
            return false;
        }
        removeFromList(e.val, routeId);
        n1 = n1->next;
        n2 = n2->next;
    }

    deleteList(&map->routes[routeId].cities);
    return true;
}
