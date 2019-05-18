/** @file
 * Interfejs klasy parsującej polecenia.
 */
#ifndef __PARSER_H__
#define __PARSER_H__

#include "status.h"
#include <ctype.h>

/** @brief Typ wyliczeniowy, opisujący możliwe wyniki parsowania poleceń. Są
 * to, albo poprawne operacje (jedna z czterech), albo brak operacji (NOOP),
 * albo błąd parsowania (ERROR).
 */
enum opcode {
    OP_ERROR,
    OP_NOOP,
    OP_NEW_ROUTE,
    OP_ADD_ROAD,
    OP_REPAIR_ROAD,
    OP_ROUTE_DESCRIPTION
};

/** @brief Struktura reprezentująca typ operacji.
 */
struct operation {
    /// Element typu wyliczeniowego określający typ operacji.
    enum opcode op;
    /// Argument operacji, czyli wszystko co jest po pierwszym średniku.
    char *arg;
};

/** @brief Parsuje linię wejścia, zwracając typ operacji oraz skojarzony
 * argument.
 * @param[in] line      - linia wejścia
 * @param[in] length    - długość linii wejścia
 * @return struktura reprezentująca status parsowania linii
 */
struct operation parse(char *line, size_t length);

/** @brief Wyciąga długość drogi ze wskaźnika arg.
 * Funkcja zapisuje pod wskaźnikiem @p length znalezioną długość drogi, o ile
 * początkowy fragment @p arg jest poprawną długością drogi, i poprawny fragment
 * kończy się bajtem zerowym, bądź średnikiem.
 * @param[in] arg       - linia wejścia
 * @param[out] length   - długość drogi
 * @return Status powodzenia operacji.
 */
Status extractRoadLength(char *arg, unsigned long long *length);

/** @brief Wyciąga nazwę miasta ze wskaźnika arg.
 * Zapisuje pod @p city1 wskaźnik na zaalokowany na stercie napis zawierający
 * wyciągniętą nazwę miasta, o ile jest poprawna.
 * @param[in] arg       - linia wejścia
 * @param[out] city     - wskaźnik, pod którym znajdzie się nazwa miasta
 * @return Status powodzenia operacji.
 */
Status extractCityName(char *arg, char *city);

/** @brief Wyciąga rok budowy (naprawy) drogi ze wskaźnika arg.
 * Funkcja zapisuje pod wskaźnikiem @p year znaleziony rok budowy drogi, o ile
 * początkowy fragment @p arg jest poprawnym rokiem, i poprawny fragment
 * kończy się bajtem zerowym, bądź średnikiem.
 * @param[in] arg       - linia wejścia
 * @param[out] year   - rok budowy (naprawy)
 * @return Status powodzenia operacji.
 */
Status extractYear(char *arg, int *year);

/** @brief Wyciąga rok numer drogi krajowej ze wskaźnika arg.
 * Funkcja zapisuje pod wskaźnikiem @p routeId znaleziony numer drogi krajowej,
 * o ile początkowy fragment @p arg jest poprawny, i poprawny fragment kończy
 * się bajtem zerowym, bądź średnikiem.
 * @param[in] arg       - linia wejścia
 * @param[out] routeId  - rok budowy (naprawy)
 * @return Status powodzenia operacji.
 */
Status extractRouteId(char *arg, unsigned *routeId);

#endif /* __PARSER_H__ */
