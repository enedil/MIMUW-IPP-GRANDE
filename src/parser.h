#ifndef __PARSER_H__
#define __PARSER_H__

#include <ctype.h>

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

#endif /* __PARSER_H__ */
