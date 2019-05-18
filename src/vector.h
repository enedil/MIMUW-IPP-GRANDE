/** @file
 * Interfejs dostarczajacy strukturę tablicy dynamicznej (wektora).
 */
#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdlib.h>

#include "status.h"

/**
 * Struktura opisująca tablicę dynamiczną (wektor).
 */
typedef struct Vector {
    /// tablica z wskaźnikami na elementy
    void **arr;
    /// rozmiar tablicy
    size_t size;
     /// pojemność tablicy (zaalokowany rozmiar)
    size_t capacity;
} Vector;

/** @brief Tworzy nowy wektor.
 * @return Wskaźnik na nowy, pusty wektor, lub NULL w przypadku błędu alokacji.
 */
Vector *newVector(void);

/** @brief Usuwa zawartość wektora.
 * @param[in,out] vector        - wektor, którego zawartość chcemy usunąć
 */
void vectorDelete(Vector *vector);

/** @brief Usuwa wektor wraz z zawartością.
 * @param[in,out] vector        - wektor, którego zawartość chcemy usunąć
 */
void vectorDeleteFreeContent(Vector *vector);

/** @brief Dodaje element na koniec wektora.
 * @param[in,out] vector        - wektor, który chcemy poszerzyć
 * @param[in] element           - element, o który chcemy poszerzyć wektor
 * @return wartość logiczna statusu powodzenia operacji, która może się nie
 * udać w przypadku błędu alokacji pamięci.
 */
Status vectorAppend(Vector *vector, void *element);

/** @brief Usuwa ostatni element z wektora.
 * @param[in,out] vector        - wektor, z którego usuwamy ostatni element
 * @param[in] free_             - parametr określa, czy powinniśmy zwolnić
 * pamięć po zwalnianym elemencie przed wymazaniem z wektora.
 */
void vectorRemoveLast(Vector *vector, bool free_);

#endif /* __VECTOR_H__ */
