#define _GNU_SOURCE
#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "dictionary.h"
#include "map_struct.h"
#include "parser.h"
#include "utils.h"

/** @brief Sprawdza, czy łańcuch znaków tworzy poprawną liczbę całkowitą.
 * @param data[in]          - łańcuch znaków do sprawdzenia
 * @param data_len[in]      - długość łańcucha znakóœ
 * @return
 */
static bool validNumeral(char *data, size_t data_len) {
    if (data_len == 0) {
        return false;
    }
    // handle single '-' properly
    if (data_len == 1) {
        return isdigit(data[0]);
    }
    if (!(isdigit(data[0]) || data[0] == '-')) {
        return false;
    }
    for (size_t i = 1; i < data_len; ++i) {
        if (!(isdigit(data[i]))) {
            return false;
        }
    }
    return true;
}

/** @brief Zlicza wystąpienia znaku w stringu.
 * @param str               - string do przeszukania
 * @param c                 - znak do zliczenia
 * @return Liczba wystąpień znaku @p c w stringu @p str.
 */
static size_t countChar(char *str, char c) {
    size_t count = 0;
    while (*str) {
        if (*str == c) {
            count++;
        }
        str++;
    }
    return count;
}

/** @brief Liczy skrót (hasz) stringa zakończonego bajtem zerowym, bądź
 * średnikiem.
 * @param ptr                 - string
 * @return wartość skrótu
 */
static hash_t hashSemicolonTerminated(void *ptr) {
    uint8_t *str = ptr;
    hash_t ret = 1;
    while (*str && *str != ';') {
        ret *= *str + 1;
        str++;
    }
    return ret;
}

static bool cmpSemicolonTerminated(void *ptr1, void *ptr2) {
    if (ptr1 == DELETED || ptr2 == DELETED) {
        return false;
    }
    uint8_t *str1 = ptr1, *str2 = ptr2;
    while (*str1 && *str1 != ';' && *str2 && *str2 != ';') {
        if (*str1 != *str2) {
            return false;
        }
        str1++;
        str2++;
    }
    return !((*str1 && *str1 != ';') || (*str2 && *str2 != ';'));
}

Status extractCityName(char *arg, char *city) {
    if (arg == NULL || city  == NULL) {
        return false;
    }
    char *p = strchrnul(arg, ';');
    strncpy(city, arg, p - arg);
    city[p - arg] = 0;
    return validCityName(city);
}

Status extractRoadLength(char *arg, unsigned long long *length) {
    if (arg == NULL || length == NULL) {
        return false;
    }
    char *out;
    errno = 0;
    *length = strtoull(arg, &out, 10);
    return (*length > 0) && (*length <= UINT32_MAX) && (errno == 0) &&
           (*out == 0 || *out == ';');
}

Status extractYear(char *arg, int *year) {
    if (arg == NULL || year == NULL) {
        return false;
    }
    char *out;
    errno = 0;
    long x = 1;
    x = strtol(arg, &out, 10);
    if (x == 0 || errno != 0 || (*out != 0 && *out != ';')) {
        return false;
    }
    if (x != (int)x) {
        return false;
    }
    *year = (int)x;
    return true;
}

Status extractRouteId(char* arg, unsigned *routeId) {
    if (arg == NULL || routeId == NULL) {
        return false;
    }
    char* out;
    unsigned long x = strtoul(arg, &out, 10);
    if (errno != 0 || (*out != 0 && *out != ';')) {
        return false;
    }
    *routeId = x;
    return true;
}

static bool vNewRoute(char *arg) {
    Status ret = false;
    if (*arg == 0 || *arg == ';') {
        return false;
    }
    size_t semicolon_count = countChar(arg, ';');
    if ((semicolon_count % 3 != 1) || semicolon_count <= 1) {
        return false;
    }

    char *ptr = arg;

    int routeId;
    if (!extractYear(ptr, &routeId)) {
        return false;
    }
    if (routeId >= ROUTE_MAX) {
        return false;
    }
    ptr = strchr(ptr + 1, ';') + 1;
    Dictionary *detect_duplicates = newDictionary(
        hashSemicolonTerminated, cmpSemicolonTerminated, empty, empty);
    if (detect_duplicates == NULL) {
        return false;
    }
    for (size_t i = 0; i < semicolon_count / 3; ++i) {
        Entry e = getDictionary(detect_duplicates, ptr);
        // If found, the route is invalid. If not, try to insert element.
        if (!NOT_FOUND(e) || !insertDictionary(detect_duplicates, ptr, ptr)) {
            goto CLEANUP;
        }
        if (!nValidCityName(ptr, strchr(ptr, ';') - ptr)) {
            goto CLEANUP;
        }
        ptr = strchr(ptr + 1, ';');
        unsigned long long length;
        if (!extractRoadLength(ptr + 1, &length)) {
            goto CLEANUP;
        }
        ptr = strchr(ptr + 1, ';');
        int year;
        if (!extractYear(ptr + 1, &year)) {
            goto CLEANUP;
        }
        ptr = strchr(ptr + 1, ';') + 1;
    }
    Entry e = getDictionary(detect_duplicates, ptr);
    if (!NOT_FOUND(e)) {
        goto CLEANUP;
    }
    if (!validCityName(ptr)) {
        goto CLEANUP;
    }
    ret = true;
CLEANUP:
    deleteDictionary(detect_duplicates);
    free(detect_duplicates);
    return ret;
}

static bool vAddRoad(char *arg) {
    const int max_semicolons = 3;
    char *semicolons[max_semicolons];
    int semicolon_count = 0;
    char *p = arg;
    while (*p) {
        if (*p == ';') {
            semicolons[semicolon_count] = p;
            *p = 0;
            semicolon_count++;
        }
        p++;
    }
    if (semicolon_count != max_semicolons) {
        return false;
    }
    if (!possiblyValidRoad(arg, 1 + semicolons[0])) {
        return false;
    }

    unsigned long long length;
    if (!extractRoadLength(semicolons[1] + 1, &length)) {
        return false;
    }
    int year;
    if (!extractYear(semicolons[2] + 1, &year)) {
        return false;
    }
    for (size_t i = 0; i < 3; ++i) {
        *semicolons[i] = ';';
    }
    return true;
}

/** @brief Stwierdza, czy linia wejścia jest składniowo poprawna, jako operacja
 * newRoute. W szczególności, sprawdza czy wszyskie zmienne liczbowe mieszczą
 * się w odpowiednich zakresach, a także czy żadne miasto nie występuje na
 * liście więcej niż raz.
 * @param[in] arg    - linia wejścia
 * @return Wartość @p false w przypadku błędu alokacji, a przeciwnym razie
 * wartość logiczna poprawności linii wejścia.
 */
static bool vRepairRoad(char *arg) {
    char *first_semicolon = strchr(arg, ';');
    if (first_semicolon == NULL) {
        return false;
    }
    char c1[first_semicolon - arg + 1];
    memset(c1, 0, first_semicolon - arg + 1);
    strncpy(c1, arg, first_semicolon - arg);
    char *second_semicolon = strchr(first_semicolon + 1, ';');
    if (second_semicolon == NULL) {
        return false;
    }
    char c2[second_semicolon - first_semicolon + 1];
    memset(c2, 0, second_semicolon - first_semicolon + 1);
    strncpy(c2, first_semicolon + 1, second_semicolon - first_semicolon - 1);

    if (!possiblyValidRoad(c1, c2)) {
        return false;
    }
    int year;
    return extractYear(second_semicolon + 1, &year) != 0;
}

static bool vRouteDescription(char *arg) {
    unsigned id;
    if (*arg < '0' || *arg > '9') {
        return false;
    }
    return extractRouteId(arg, &id);
}

static void validateArgs(struct operation *ret) {
    bool valid = true;
    switch (ret->op) {
    case OP_ROUTE_DESCRIPTION:
        valid = vRouteDescription(ret->arg);
        break;
    case OP_REPAIR_ROAD:
        valid = vRepairRoad(ret->arg);
        break;
    case OP_ADD_ROAD:
        valid = vAddRoad(ret->arg);
        break;
    case OP_NEW_ROUTE:
        valid = vNewRoute(ret->arg);
        break;
    case OP_ERROR:
    case OP_NOOP:
        break;
    }
    if (!valid) {
        ret->op = OP_ERROR;
    }
}

struct operation parse(char *line, size_t length) {
    struct operation ret = {OP_ERROR, NULL};
    if (line == NULL) {
        return ret;
    }
    if (line[length - 1] == '\n') {
        line[length - 1] = 0;
        length--;
    }
    if (length == 0) {
        ret.op = OP_NOOP;
        return ret;
    }
    if (line[0] == '#') {
        ret.op = OP_NOOP;
        return ret;
    }
    char *first_semicolon = strchr(line, ';');
    if (first_semicolon == NULL) {
        return ret;
    }
    if (line[0] == '0') {
        return ret;
    }
    size_t op_name_length = first_semicolon - line;
    ret.arg = first_semicolon + 1;
    if (validNumeral(line, op_name_length)) {
        ret.arg = line;
        ret.op = OP_NEW_ROUTE;
    } else if (strncmp(line, "addRoad", op_name_length) == 0) {
        ret.op = OP_ADD_ROAD;
    } else if (strncmp(line, "repairRoad", op_name_length) == 0) {
        ret.op = OP_REPAIR_ROAD;
    } else if (strncmp(line, "getRouteDescription", op_name_length) == 0) {
        ret.op = OP_ROUTE_DESCRIPTION;
    }
    validateArgs(&ret);
    return ret;
}
