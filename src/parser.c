#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "parser.h"
#include "utils.h"
#include "dictionary.h"

/** @brief Sprawdza, czy łańcuch znaków tworzy poprawną liczbę całkowitą.
 * @param data[in]          - łańcuch znaków do sprawdzenia
 * @param data_len[in]      - długość łańcucha znakóœ
 * @return
 */
static bool validNumeral(char* data, size_t data_len) {
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
static size_t countChar(char* str, char c) {
    size_t count = 0;
    while (*str) {
        if (*str == c) {
            count++;
        }
        str++;
    }
    return count;
}

static hash_t hashSemicolonTerminated(void* ptr) {
    uint8_t* str = ptr;
    hash_t ret = 1;
    while (*str && *str != ';') {
        ret *= *str + 1;
        str++;
    }
    return ret;
}

static bool cmpSemicolonTerminated(void* ptr1, void* ptr2) {
    uint8_t* str1 = ptr1, * str2 = ptr2;
    while (*str1 && *str1 != ';' && *str2 && *str2 != ';') {
        if (*str1 != *str2) {
            return false;
        }
        str1++;
        str2++;
    }
    return !((*str1 && *str1 != ';') || (*str2 && *str2 != ';'));
}

static bool vNewRoute(char* arg) {
    bool ret = false;
    if (*arg == 0 || *arg == ';') {
        return false;
    }
    size_t semicolon_count = countChar(arg, ';');
    if (semicolon_count % 3 != 1) {
        return false;
    }

    char* ptr = arg;
    errno = 0;
    char* end;
    unsigned long routeId =  strtoul(ptr, &end, 10);
    if (errno || (*end != '\0' && *end != ';') || routeId == 0 || routeId >= 1000) {
        return false;
    }
    ptr = strchr(ptr+1, ';') + 1;
    Dictionary* detect_duplicates = newDictionary(hashSemicolonTerminated, cmpSemicolonTerminated, empty, empty);
    if (detect_duplicates == NULL) {
        return false;
    }
    for (size_t i = 0; i < semicolon_count; i += 3) {
        Entry e = getDictionary(detect_duplicates, ptr);
        // If found, the route is invalid. If not, try to insert element.
        if (!NOT_FOUND(e) || !insertDictionary(detect_duplicates, ptr, ptr)) {
            goto CLEANUP;
        }
        if (!nValidCityName(ptr, strchr(ptr, ';') - ptr)) {
            goto CLEANUP;
        }
        ptr = strchr(ptr + 1, ';');
        char* end;
        errno = 0;
        long long int length = strtoll(ptr+1, &end, 10);
        if (errno || (*end != '\0' && *end != ';') || length <= 0) {
            goto CLEANUP;
        }
        ptr = strchr(ptr + 1, ';');
        errno = 0;
        long long year = strtoll(ptr+1, &end, 10);
        if (errno || (*end != '\0' && *end != ';') || year == 0 || (int)year != year) {
            return false;
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

static bool vAddRoad(char* arg) {
    const int max_semicolons = 3;
    char* semicolons[max_semicolons];
    int semicolon_count = 0;
    char* p = arg;
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
    if (!possiblyValidRoad(arg, 1+semicolons[0])) {
        return false;
    }

    char* end;
    errno = 0;
    long long length = strtoll(1+semicolons[1], &end, 10);
  /*todo fix condition*/  if (errno || end == NULL || *end != '\0' || length <= 0) {
        return false;
    }
    errno = 0;
    long long year = strtoll(1+semicolons[2], &end, 10);
  /*todo*/   if (errno || end == NULL || *end != '\0' || year == 0 || year > INT_LEAST32_MAX) {
        return false;
    }
    return true;
}

static bool vRepairRoad(char* arg) {
    char* first_semicolon = strchr(arg, ';');
    if (first_semicolon == NULL) {
        return false;
    }
    char c1[first_semicolon - arg];
    strncpy(c1, arg, first_semicolon - arg);
    char* second_semicolon = strchr(first_semicolon + 1, ';');
    if (second_semicolon == NULL) {
        return false;
    }
    char c2[second_semicolon - first_semicolon];
    strncpy(c2, first_semicolon + 1, second_semicolon - first_semicolon - 1);

    if (!possiblyValidRoad(c1, c2)) {
        return false;
    }
    char* third_semicolon = strchr(second_semicolon + 1, ';');
    if (third_semicolon == NULL) {
        return false;
    }
    if (!validNumeral(third_semicolon+1, strlen(third_semicolon+1))) {
        return false;
    }
    if (strlen(third_semicolon+1) > 16) {
        // numbers this long don't fit into unsigned integer
        return false;
    }
    long long x = atoll(arg);
    return x != 0;
}

static bool vRouteDescription(char* arg) {
    if (strlen(arg) > 4) {
        // can't be a number less than 1000
        return false;
    }
    int x = atoi(arg);
    return x > 0 && x < 1000;
}

static void validateArgs(struct operation* ret) {
    bool valid = true;
    switch(ret->op) {
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

struct operation parse(char* line, size_t length) {
    struct operation ret = {OP_ERROR, NULL};
    if (line == NULL) {
        return ret;
    }
    if (line[length-1] == '\n') {
        line[length-1] = 0;
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
    char* first_semicolon = strchr(line, ';');
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
