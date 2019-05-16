#ifndef __PARSER_H__
#define __PARSER_H__

#include <ctype.h>
#include "status.h"

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
    enum opcode op;
    char* arg;
};

/** @brief Parsuje linię wejścia, zwracając typ operacji oraz skojarzony argument.
 * @param line[in]      - linia wejścia
 * @return struktura reprezentująca status parsowania linii
 */
struct operation parse(char* line, size_t length);

/** @brief Wyciąga długość drogi z wskaźnika arg.
 * Funkcja zapisuje pod wskaźnikiem @p length znalezioną długość drogi, o ile
 * początkowy fragment @p arg jest poprawną długością drogi, i poprawny fragment
 * kończy się bajtem zerowym, bądź średnikiem.
 * @param[in] arg       - linia wejścia
 * @param[out] length   - długość drogi
 * @return Status powodzenia operacji.
 */
Status extractRoadLength(char* arg, unsigned long long* length);

#endif /* __PARSER_H__ */
