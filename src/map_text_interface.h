/** @file
 * Interfejs tekstowy.
 */
#ifndef __MAP_TEXT_INTERFACE_H__
#define __MAP_TEXT_INTERFACE_H__

#include "map.h"
#include "parser.h"
#include "status.h"
#include <stdlib.h>
#include <string.h>

/** @brief Dodaje drogę
 * @param[in,out] map   - mapa, do której dodajemy drogę
 * @param[in] arg       - string w formacie wejściowym
 * @return
 */
Status execAddRoad(Map *map, char *arg);

/** @brief Naprawia drogę
 * @param[in,out] map   - mapa, z której naprawiamy drogę
 * @param[in] arg       - string w formacie wejściowym
 * @return
 */
Status execRepairRoad(Map *map, char *arg);

/** @brief Wyświetla drogę krajową.
 * Jeśli droga nie istnieje, nic nie robi.
 * @param[in,out] map   - mapa, z której wyświetlamy drogę krajową
 * @param[in] arg       - string w formacie wejściowym
 * @return
 */
Status execGetRouteDescription(Map *map, char *arg);

/** Dodaje drogę krajową przechodzącą przez miasta.
 * @brief execNewRoute
 * @param[in,out] map   - mapa, do której dodajemy drogę krajową
 * @param[in] arg       - string w formacie wejściowym
 * @return
 */
Status execNewRouteThrough(Map *map, char *arg);

/** Dodaje nową drogę krajową z punktu do punktu.
 * @param[in,out] map   - mapa, do której dodajemy drogę krajową
 * @param[in] arg       - string w formacie wejściowym
 * @return
 */
Status execNewRoute(Map *map, char *arg);

/** Rozszerza drogę krajową do podanego miasta
 * @param[in,out] map   - mapa, z której drogę krajową rozszerzamy
 * @param[in] arg       - string w formacie wejściowym
 * @return
 */
Status execExtendRoute(Map* map, char* arg);

/** Usuwa drogę krajową z mapy
 * @param[in,out] map   - mapa, z której drogę krajową usuwamy
 * @param[in] arg       - string w formacie wejściowym
 * @return
 */
Status execRemoveRoute(Map* map, char* arg);

/** Usuwa drogę z mapy
 * @param[in,out] map   - mapa, z której drogę usuwamy
 * @param[in] arg       - string w formacie wejściowym
 * @return
 */
Status execRemoveRoad(Map* map, char* arg);

#endif /* __MAP_TEXT_INTERFACE_H__ */
