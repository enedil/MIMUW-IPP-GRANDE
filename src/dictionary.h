/** @file
 * Interfejs dostarczajacy strukturę słownika.
 */
#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__

#include <stdint.h>
#include <stdlib.h>

#include "status.h"

/// Tak oznaczane są wierzchołki, które usuwamy ze słownika.
#define DELETED ((void *)1)

/// Określa, czy Entry e reprezentuje poprawne dane (czyli, czy zostało
/// znalezione w słowniku).
#define NOT_FOUND(e) ((e).key == NULL || (e).key == DELETED)

/**
 * @brief Taki typ zwraca każda funkcja skrótu używana w słowniku.
 */
typedef uint64_t hash_t;

/**
 * Struktura reprezentująca element słownika.
 */
typedef struct Entry {
    /// klucz, za pomocą którego szukamy wartości
    void *key;
    /// skojarzona wartość
    void *val;
} Entry;

/**
 * Struktura reprezentująca słownik.
 */
typedef struct Dictionary {
    /// funkcja skrótu używana w słowniku
    hash_t (*hash)(void *);
    /// funkcja porównująca klucze na równość
    bool (*equal)(void *, void *);
    /// tablica przechowująca pary klucz-wartość
    Entry *array;
    /// rozmiar tablicy
    size_t array_size;
    /// liczba elementów w słowniku
    size_t size;
    /// funkcja zwalniająca pamięć po kluczach
    void (*free_key)(void *);
    /// funkcja zwalniająca pamięć po wartościach
    void (*free_val)(void *);
} Dictionary;

/** @brief Usuwa istniejący słownik wraz z elementami.
 * Usuwa strukturę słownika. Zajmuje się zwalnianiem przechowywanych elementów
 * za pomocą funkcji @p free_key i @p free_val.
 * @param[in,out] dictionary   - słownik do usunięcia.
 */
void deleteDictionary(Dictionary *dictionary);

/** @brief Tworzy nowy słownik.
 * Tworzy nową strukturę tablicy haszującej. Jeśli elementy są równe według
 * funkcji @p equal, muszą mieć także ten sam skrót. Funkcje @p free_key i
 * @p free_val służą, by można było ręcznie decydować, kto zajmuje się
 * zwalnianiem pamięci, czy wskaźniki mogą leżeć na stosie etc.
 * @param[in] hash             - funkcja skrótu (hasz).
 * @param[in] equal            - funkcja porównująca klucze
 * @param[in] free_key         - funkcja zwalniająca klucze
 * @param[in] free_val         - funkcja zwalniająca wartości
 * @return Wskaźnik na nowy słownik lub NULL, gdy nie udało się zaalokować
 * pamięci.
 */
Dictionary *newDictionary(hash_t (*hash)(void *), bool (*equal)(void *, void *),
                          void (*free_key)(void *), void (*free_val)(void *));
/** @brief Wstawia wartość do słownika.
 * Wstawia element do słownika. Jeśli element znajduje się w słowniku, zastępuje
 * starą wartość nową.
 * @param[in,out] dictionary   - słownik, do którego wstawiamy wartości
 * @param[in] key              - klucz, pod którym ma się znaleźć wartość
 * @param[in] val              - wartość, którą wstawiamy
 * @return @p true jeśli wstawienie się powiodło, @p false jeśli nie (np. nie
 * udało się zaalokować pamięci)
 */
Status insertDictionary(Dictionary *dictionary, void *key, void *val);

/** @brief Znajduje element w słowniku.
 * Jeśli element znajduje się w słowniku, zwraca go w postaci struktury Entry,
 * o polach key i val. Jeśli elementu w tym słowniku nie ma, zwraca wartość typu
 * Entry, gdzie key i val są ustawione na NULL.
 * @param[in] dictionary       - słownik, z którego wybieramy wartości
 * @param[in] key              - klucz elementu, którego poszukujemy
 * @return Element (para key val) przechowująca klucz i odpowiadającą jemu
 * wartość, jeśli taki występował w słowniku, lub para dwóch wskaźników NULL w
 * przeciwnym razie.
 */
Entry getDictionary(Dictionary *dictionary, void *key);

/** @brief Usuwa element ze słownika.
 * Usuwa element ze słownika o kluczu @p key. Jeśli takiego klucza nie ma,
 * nic nie robi. Zwalnia pamięć przydzieloną na klucz i wartość za pomocą
 * funkcji @p free_key i @p free_val.
 * @param[in,out] dictionary   - słownik, z którego usuwa
 * @param[in] key              - klucz, który chcemy usunąć ze słownika.
 */
void deleteFromDictionary(Dictionary *dictionary, void *key);

#endif /* __DICTIONARY_H__ */
