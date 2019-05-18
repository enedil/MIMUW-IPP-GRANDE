/** @file
  * Znajduje najkrótrze ścieżki między miastami
  */
#ifndef __SHORTEST_PATHS_H__
#define __SHORTEST_PATHS_H__
#include "map_struct.h"

/** @brief Znajduje najkrótrze ścieżki między wierzchołkami A i B.
 * @param[in] map           - struktura mapy, na której wykonujemy wyszukania
 * @param[in] A             - wierzchołek początkowy
 * @param[in] B             - wierzchołek końcowy
 * @param[in] visited       - tablica odwiedzonych wierzchołków (przez które nie
 * wolno przechodzić)
 * @param[in,out] prev      - tablica przodków w najlepszych ścieżkach
 * (najlepsza ścieżka do x pochodzi od prev[x])
 * @param[out] d            - tutaj zapisywana jest długość najkrótszej ścieżki;
 * jeśli taka nie istnieje zwraca UINT64_MAX
 * @param[out] w            - tutaj zapisywany jest czas ostatniej
 *  naprawy/budowy drogi na znalezionej ścieżce.
 * @param[in] fixing        - jeśli @p fixing jest prawdziwy, wzięcie
 * bezpośredniej drogi z A do B jest zabronione
 * @return Wartośc logiczna, czy udało się przeprowadzić wyszukiwanie - czy
 * alokacje pamięci się powiodły, czy źródło jest różne od celu.
 */
Status shortestPaths(Map *map, int A, int B, bool visited[], int prev[],
                     uint64_t *d, int *w, bool fixing);
#endif /* __SHORTEST_PATHS_H__ */
