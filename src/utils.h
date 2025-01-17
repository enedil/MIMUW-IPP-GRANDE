/** @file
 * Różne pomocnicze funkcje.
 */
#ifndef __STRING_UTILS__
#define __STRING_UTILS__

#include "dictionary.h"
#include "vector.h"

/** @brief Skraca stringa podanego na wejściu.
 * Skraca stringa, tak aby można go było umieścić słowniku.
 * @param[in] str      - napis w stylu C do skrócenia.
 * @return Skrót stringa.
 */
hash_t hashString(void *str);

/** @brief Skraca stringa podanego na wejściu, czytając maksymalnie @p len
 * bajtów. Skraca stringa, tak aby można go było umieścić słowniku.
 * @param[in] str      - napis w stylu C do skrócenia.
 * @param[in] len      - maksymalna przeczytana długośc napisu.
 * @return Skrót stringa.
 */
hash_t nHashString(void *str, size_t len);

/** @brief Porównuje x i y jako stringi, o ile nie są równe NULL ani DELETED.
 * Jeżeli x lub y jest NULLem lub jest usunięty ze słownika (wartość DELETED),
 * zwraca fałsz. W przeciwnym wypadku, stwierdza, czy x i y zawierają tego
 * samego stringa. Przydatne do tworzenia słownika ze stringami jako klucze.
 * @param[in] x       - jeden z napisów w stylu C do porównania
 * @param[in] y       - drugi z napisów w stylu C do porównania
 * @return Wartość @p true jeśli x nie jest NULLem, x nie jest usunięty,
 * y nie jest NULLem, y nie jest usunięty, oraz x i y wskazują na takie same
 * napisy w stylu C. Wartość @p false w przeciwnym wypadku.
 */
bool undereferencing_strcmp(void *x, void *y);

/** @brief Stwierdza, czy nazwa podana może być nazwą miasta.
 * Nazwa każdego miasta musi być dodatniej długości, a każdy jej znak musi być
 * z przedziału <1, 32) i nie zawierać średnika.
 * @param[in] city    - nazwa do sprawdzenia
 * @return Wartość @p true jeśli nazwa spełnia warunek, wartość @p false w
 * przeciwnym wypadku.
 */
bool validCityName(const char *city);

/** @brief Stwierdza, czy nazwa podana może być nazwą miasta.
 * Różni się od funkcji @ref validCityName faktem, że czyta maksymalnie @p n
 * znaków.
 * @param[in] city   - nazwa do sprawdzenia
 * @param[in] n      - maksymalna liczba znaków do sprawdzenia
 * @return Wartość @p true jeśli nazwa spełnia warunek, wartość @p false w
 * przeciwnym wypadku.
 */
bool nValidCityName(const char *city, size_t n);

/** @brief Stwierdza, czy droga o końcach w city1 i city2 spełnia założenia.
 * Droga może być poprawna, jeśli city1 i city2 mogą być poprawnymi miastami,
 * oraz są różne.
 * @param[in] city1   - nazwa pierwszego miasta
 * @param[in] city2   - nazwa drugiego miasta
 * @return Wartość @p true jeśli nazwy spełniają warunek, wartość @p dalse w
 * przeciwnym wypadku.
 */
bool possiblyValidRoad(const char *city1, const char *city2);

/** @brief Porównuje (stwierdza równość bądź różność) krawędzi.
 * Krawędzie są równe, jeśli zbiory ich końców są równe. Nadaje się do
 * stosowania w słowniku.
 * @param[in] e1          - krawędź pierwsza
 * @param[in] e2          - krawędź druga
 * @return Wartość logiczna określająca równość @p e1 i @p e2.
 */
bool cmpEdges(void *e1, void *e2);

/** @brief Znajduje długość liczby całkowitej w zapisie dziesiętnym.
 * @param[in] x       - liczba, której długości szukamy
 * @return Długośc zapisu dziesiętnego (wraz z ewentualnym znakiem '-')
 * liczby x.
 */
size_t intLength(int64_t x);

/** @brief Zakodowuje drogę jako wskaźnik, który można wstawić do słownika.
 * encodeEdgeAsPtr(a, b) == encodeEdgeAsPtr(b, a)
 * @param[in] a       - początek drogi
 * @param[in] b       - koniec drogi
 * @return Wskaźnik reprezentujący drogę z @p a do @p b (i na odwrót).
 */
void *encodeEdgeAsPtr(int a, int b);

/** @brief Koduje liczbę jako wskaźnik, który można wstawić do słownika.
 * @param[in] id      - liczba do zakodowania
 * @return Wskaźnik unikalnie reprezentujący liczbę @p id
 */
void *encodeCityId(int id);

/** @brief Funkcja odwrotna do @ref encodeCityId.
 * Dekoduje wskaźnik jako liczbę, by móc operować na wartości po wyjęciu ze
 * słownika.
 * @param[in] p       - wskaźnik do dekodowania
 * @return
 */
int decodeCityId(void *p);

/** @brief Funkcja skrótu dla krawędzi.
 * @param[in] key     - krawędź zakodowana za pomocą @ref encodeEdgeAsPtr
 * @return Skrót (hasz) krawędzi.
 */
hash_t hashEdge(void *key);

/** @brief Wybiera mniejszą z dwóch liczb.
 * @param[in] a          - pierwsza z liczb
 * @param[in] b          - druga z liczb.
 * @return Mniejsza z liczb a, b.
 */
int min(int a, int b);

/** @brief Funkcja skrótu dla liczb całkowitych (int)
 * @param[in] p          - liczba całkowita do skrócenia
 * @return
 */
hash_t hashInt(void *p);

/** @brief Porównuje dwie liczby
 * @param[in] p1         - pierwsza z liczb do porównania
 * @param[in] p2         - druga z liczb do porównania
 * @return wartośc logiczna czy p1 jest równe p2
 */
bool equalInt(void *p1, void *p2);

/** @brief Usuwa wektor list, zwalniając przeznaczoną na elementy pamięć.
 * @param[in,out] vector - wektor do opróżnienia
 */
void deleteVectorOfLists(Vector *vector);

/** @brief Zamienia wartościami zawartości wskaźników @p x i @p y
 * @param[in,out] x       - wskaźnik na pierwszą z liczb
 * @param[in,out] y       - wskaźnik na drugą z licz
 */
void swap(int *x, int *y);

/** @brief Pusta funkcja.
 * Funkcja służy do wstawienia do słownika, w którym klucze lub wartości nie
 * wymagają zwalniania (np. znajdują się na stosie, nie wskazują na istotne dane
 * i nigdy nie są dereferencjonowane, bądź wskazują na zmienne globalne).
 * @param[in] ptr         - nieistotny wskaźnik
 */
void empty(void *ptr);

#endif /* __STRING_UTILS__ */
