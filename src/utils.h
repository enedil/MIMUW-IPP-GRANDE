#ifndef __STRING_UTILS__
#define __STRING_UTILS__

#include "dictionary.h"
#include "vector.h"

/** @brief Skraca stringa podanego na wejściu.
 * Skraca stringa, tak aby można go było umieścić słowniku.
 * @param[in] str      - napis w stylu C do skrócenia.
 * @return Skrót stringa.
 */
hash_t hash_string(void* str);

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
bool undereferencing_strcmp(void* x, void* y);

/** @brief Stwierdza, czy nazwa podana może być nazwą miasta.
 * Nazwa każdego miasta musi być dodatniej długości, a każdy jej znak musi być
 * z przedziału <1, 32) i nie zawierać średnika.
 * @param[in] city    - nazwa do sprawdzenia
 * @return Wartość @p true jeśli nazwa spełnia warunek, wartość @p dalse w
 * przeciwnym wypadku.
 */
bool validCityName(const char* city);

/** @brief Stwierdza, czy droga o końcach w city1 i city2 spełnia założenia.
 * Droga może być poprawna, jeśli city1 i city2 mogą być poprawnymi miastami,
 * oraz są różne.
 * @param[in] city1   - nazwa pierwszego miasta
 * @param[in] city2   - nazwa drugiego miasta
 * @return Wartość @p true jeśli nazwy spełniają warunek, wartość @p dalse w
 * przeciwnym wypadku.
 */
bool possiblyValidRoad(const char* city1, const char* city2);

void deleteDictionaryOfLists(Dictionary* d);
hash_t hashEdge(void* key);
bool cmpEdges(void* e1, void* e2);

/** @brief Znajduje długość liczby całkowitej w zapisie dziesiętnym.
 * @param x[in]       - liczba, której długości szukamy
 * @return Długośc zapisu dziesiętnego (wraz z ewentualnym znakiem '-')
 * liczby x.
 */
size_t intLength(int64_t x);
void* encodeEdgeAsPtr(int a, int b);
void* encodeCityId(int id);
int decodeCityId(void* p);

/** @brief Wybiera mniejszą z dwóch liczb.
 * @param a[in]       - pierwsza z liczb
 * @param b[in]       - druga z liczb.
 * @return Mniejsza z liczb a, b.
 */
int min(int a, int b);
hash_t hashInt(void* p);
bool equalInt(void* p1, void* p2);
void deleteVectorOfLists(Vector* vector);
void swap(int* x, int* y);
void empty(void*);

#endif /* __STRING_UTILS__ */
