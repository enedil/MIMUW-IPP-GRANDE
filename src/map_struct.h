/** @file
 * Definicje struktur, które współtworzą strukturę mapy.
 */
#ifndef __MAP_STRUCT_H__
#define __MAP_STRUCT_H__

#include "dictionary.h"
#include "list.h"
#include "vector.h"

/// Maksymalna liczba dróg krajowych.
#define ROUTE_MAX 1000

/**
 * Struktura przechowująca informację o drodze krajowej.
 */
typedef struct Route {
    /// Lista miast leżących na drodze krajowej.
    List cities;
} Route;

/**
 * Struktura przechowująca informację o odcinku drogowym.
 */
typedef struct Road {
    /// Długość odcinka drogowego.
    uint64_t length;
    /// Rok budowy lub ostatniej naprawy.
    int builtYear;
    /// Miasto startowe.
    int start;
    /// Miasto końcowe.
    int end;
} Road;

/**
 * Struktura przechowująca informację o mapie połączeń.
 */
typedef struct Map {
    /// Przechowuje wszystkie drogi krajowe.
    Route routes[ROUTE_MAX];
    /// Każdy z sąsiadów przechowuje Dictionary[int, Road], słownik sąsiadów.
    Vector neighbours;
    /// Każdy element przechowuje char*, nazwę miasta o odpowiednim indeksie.
    Vector int_to_city;
    /// Słownik Dictionary[char*, int], który służy do uzyskania
    /// identyfikatora, na podstawie tekstowej nazwy miasta.
    Dictionary city_to_int;
    /// Słownik Dictionary[(int, int), List[int]] dla każdej krawędzi przechowuje
    /// listę dróg krajowych, które przez nią przebiegają.
    Dictionary routesThrough;
} Map;

#endif /* __MAP_STRUCT_H__ */
