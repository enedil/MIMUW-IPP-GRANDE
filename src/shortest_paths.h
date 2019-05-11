#ifndef __SHORTEST_PATHS_H__
#define __SHORTEST_PATHS_H__
#include "map_struct.h"

Status shortestPaths(Map* map, int A, int B, bool visited[], int prev[], uint64_t* d, int* w, bool fixing);
#endif /* __SHORTEST_PATHS_H__ */
