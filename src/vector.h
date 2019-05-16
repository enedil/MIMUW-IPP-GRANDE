#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdlib.h>

#include "status.h"

/**
 * Struktura opisująca tablicę dynamiczną (wektor).
 */
typedef struct Vector {
    void **arr;
    size_t size;
    size_t capacity;
} Vector;

/** @brief Tworzy nowy wektor.
 * @return Wskaźnik na nowy, pusty wektor, lub NULL w przypadku błędu alokacji.
 */
Vector *newVector(void);

/** @brief Usuwa zawartość wektora.
 * @param vector[in,out]        - wektor, którego zawartość chcemy usunąć
 */
void vectorDelete(Vector *vector);

/** @brief Usuwa wektor wraz z zawartością.
 * @param vector[in,out]        - wektor, którego zawartość chcemy usunąć
 */
void vectorDeleteFreeContent(Vector *vector);

/** @brief Dodaje element na koniec wektora.
 * @param vector[in,out]        - wektor, który chcemy poszerzyć
 * @param element[in]           - element, o który chcemy poszerzyć wektor
 * @return wartość logiczna statusu powodzenia operacji, która może się nie
 * udać w przypadku błędu alokacji pamięci.
 */
Status vectorAppend(Vector *vector, void *element);

/** @brief Usuwa ostatni element z wektora.
 * @param vector[in,out]        - wektor, z którego usuwamy ostatni element
 * @param free_[in]             - parametr określa, czy powinniśmy zwolnić
 * pamięć po zwalnianym elemencie przed wymazaniem z wektora.
 */
void vectorRemoveLast(Vector *vector, bool free_);

#endif /* __VECTOR_H__ */
