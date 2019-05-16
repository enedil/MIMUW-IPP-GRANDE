#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "shortest_paths.h"
#include "utils.h"

#define INFINITY UINT64_MAX

/** @brief Iteruje po sąsiadach.
 * Iterację po sąsiadach wierzchołka należy zacząć od wywołania nextNeighbour(NULL, 0, true).
 * Kolejne wywołania funkcji nextNeighbour (z @p reset false). zwracają kolejne odcinki dróg
 *  wychodzących z wierzchołka src. Jeśli pomiędzy wywołaniem nextNeighbour(m, x, false) i
 * nextNeighbour(m, y, false) nie znalazło się wywołanie nextNeighbour(m, x, true), wynik
 * operacji jest nieokreślony.
 * @param map[in]       - mapa dróg
 * @param src[in]       - wierzchołek, którego sąsiadów przeglądamy
 * @param reset[in]     - zacznij od nowa iterację
 * @return kolejny z odcinków drogowych wychodzących z wierzchołka src.
 */
static Road nextNeighbour(Map* map, int src, bool reset) {
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

void freeStructures(uint64_t **dist, List** queue, bool** is_in_queue, int **time) {
    deleteList(*queue);
    free(*queue);
    free(*dist);
    free(*is_in_queue);
    free(*time);
}

static Status allocateStructures(int initial_vertex, size_t cities_no, uint64_t **dist, List** queue, bool** is_in_queue, int **time) {
    *dist = malloc(cities_no * sizeof (uint64_t));
    CHECK_RET(*dist);
    for (size_t i = 0; i < cities_no; ++i) {
        (*dist)[i] = INFINITY;
    }
    (*dist)[initial_vertex] = 0;
    *is_in_queue = calloc(cities_no, sizeof(bool));
    if (*is_in_queue == NULL) {
        goto FREE_MEMORY;
    }
    *time = calloc(cities_no, sizeof (int));
    if (*time == NULL) {
        goto FREE_MEMORY;
    }
    for (size_t i = 0; i < cities_no; ++i) {
        (*time)[i] = INT_MAX;
    }
    *queue = newList();
    if (*queue == NULL) {
        goto FREE_MEMORY;
    }
    if (listInsertAfter(*queue, (*queue)->begin, initial_vertex) == false) {
        goto FREE_MEMORY;
    }
    return true;
FREE_MEMORY:
    freeStructures(dist, queue, is_in_queue, time);
    return false;
}

Status shortestPathsHelper(
        Map* map,
        int A, int B,
        uint64_t dist[],
        List* queue,
        bool is_in_queue[],
        int time[],
        bool visited[],
        int prev[],
        uint64_t* d,
        int* w,
        bool fixing,
        bool insert_begin) {
    bool ret = false;

    while (queue->begin->next != queue->end) {
        int x;
        if (insert_begin) {
            x = queue->begin->next->value;
            deleteListNode(queue, queue->begin->next);
        } else {
            x = queue->end->prev->value;
            deleteListNode(queue, queue->end->prev);
        }
        if (visited[x] == true) {
            continue;
        }
        is_in_queue[x] = false;

        // reset counter
        Road road = nextNeighbour(map, x, true);
        Dictionary* neighbours = map->neighbours.arr[x];

        for (size_t i = 0; i < neighbours->size; ++i) {
            road = nextNeighbour(map, x, false);
            if (fixing == false || encodeEdgeAsPtr(A, B) != encodeEdgeAsPtr(road.start, road.end)) {
                bool p1 = dist[road.end] > road.length + dist[x];
                bool p2 = dist[road.end] == road.length + dist[x];
                if (p1 || (p2 && time[road.end] <= min(time[x], road.builtYear))) {
                    prev[road.end] = x;
                    dist[road.end] = road.length + dist[x];
                    time[road.end] = min(time[x], road.builtYear);
                    if (is_in_queue[road.end] == false) {
                        if (listInsertAfter(queue, queue->end, road.end) == false) {
                            goto FREE_MEMORY;
                        }
                        is_in_queue[road.end] = true;
                    }
                }
            }
        }
    }
    ret = true;
    *d = dist[B];
    *w = time[B];

FREE_MEMORY:
    freeStructures(&dist, &queue, &is_in_queue, &time);
    return ret;
}

Status shortestPaths(Map* map,
                     int A, int B,
                     bool visited[],
                     int prev[],
                     uint64_t* d,
                     int* w,
                     bool fixing) {
    size_t cities_no = map->city_to_int.size;
    uint64_t *dist;
    List* queue;
    bool* is_in_queue;
    int *time;
    int prev_cp[cities_no];
    CHECK_RET(allocateStructures(A, cities_no, &dist, &queue, &is_in_queue, &time));
    CHECK_RET(shortestPathsHelper(map, A, B, dist, queue, is_in_queue, time, visited, prev, d, w, fixing, false));
    for (size_t i = 0; i < cities_no; ++i) {
        prev_cp[i] = prev[i];
    }
    CHECK_RET(allocateStructures(A, cities_no, &dist, &queue, &is_in_queue, &time));
    CHECK_RET(shortestPathsHelper(map, A, B, dist, queue, is_in_queue, time, visited, prev, d, w, fixing, true));
    int id1 = B, id2 = B;
    while (true) {
        if (id1 != id2) {
            return false;
        }
        if (id1 == -1 && id2 == -1) {
            break;
        }
        id1 = prev[id1];
        id2 = prev_cp[id2];
    }
    return true;
}
