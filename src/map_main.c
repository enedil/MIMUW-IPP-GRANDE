// needed for getline
#define _XOPEN_SOURCE 700

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "parser.h"

int main() {
    Map* m = newMap();
    if (m == NULL) {
        return 0;
    }

    char* line = NULL;
    size_t line_len = 0;
    while (getline(&line, &line_len, stdin) != -1) {
        if (errno == ENOMEM) {
            exit(0);
        }
        struct operation op = parse(line, strlen(line));
        switch (op.op) {
        case OP_ADD_ROAD:
            printf("addRoad");
            break;
        case OP_ERROR:
            printf("error");
            break;
        case OP_NEW_ROUTE:
            printf("newRoute");
            break;
        case OP_NOOP:
            printf("noop");
            break;
        case OP_REPAIR_ROAD:
            printf("repairRoad");
            break;
        case OP_ROUTE_DESCRIPTION:
            printf("routeDescription");
            break;
        }
        printf(" %s\n", op.arg);
    }

    deleteMap(m);
    return 0;
}
