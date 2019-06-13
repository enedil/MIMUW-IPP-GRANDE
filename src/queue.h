#ifndef __QUEUE_H__
#define __QUEUE_H__
/** @file
 * Interfejs dostarczajacy strukturę kolejki.
 */

#include "status.h"
#include "vector.h"
#include <sys/types.h>

/**
 * Struktura reprezentująca kolejkę dwukońcową.
 */
typedef struct Queue {
    /// maksymalny rozmiar kolejki
    ssize_t size;
    /// indeks początkowego elementu kolejki
    ssize_t begin;
    /// indeks za ostatnim elementem kolejki
    ssize_t end;
    /// tablica przechowująca elementy kolejki
    int *array;
} Queue;

/** @brief newQueue tworzy nową kolejkę.
 * @param[in] size      - maksymalny rozmiar kolejki
 * @return kolekja o maksymalnym rozmiarze @p size, lub kolejka o q.array ==
 * NULL, gdy nie udało się zaalokować pamięci.
 */
Queue newQueue(size_t size);

/** @brief beginQueue zwraca pierwszy element kolejki.
 * @param[in] q         - kolejka
 * @return pierwszy element kolejki (jeśli kolejka jest pusta, zachowanie jest
 * niezdefiniowane).
 */
int beginQueue(Queue *q);

/** @brief endQueue zwraca ostatni element kolejki.
 * @param[in] q         - kolejka
 * @return pierwszy ostatnic kolejki (jeśli kolejka jest pusta, zachowanie jest
 * niezdefiniowane).
 */
int endQueue(Queue *q);

/** @brief pushQueueBegin wkłada element na poszątek kolejki.
 * @param[in] q         - kolejka
 * @param[in] elem      - element do wstawienia
 */
void pushQueueBegin(Queue *q, int elem);

/** @brief pushQueueEnd wkłada element na koniec kolejki.
 * @param[in] q         - kolejka
 * @param[in] elem      - element do wstawienia
 */
void pushQueueEnd(Queue *q, int elem);

/** @brief pushQueueEnd zciąga element z końca kolejki.
 * @param[in] q         - kolejka
 */
void popQueueBegin(Queue *q);

/** @brief pushQueueEnd zciąga element z początku kolejki.
 * @param[in] q         - kolejka
 */
void popQueueEnd(Queue *q);

/** @brief isEmptyQueue stwierdza, czy kolejka jest pusta.
 * @param q
 * @return
 */
bool isEmptyQueue(Queue *q);

#endif /* __QUEUE_H__ */
