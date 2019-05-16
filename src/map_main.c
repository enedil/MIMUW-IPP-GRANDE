// needed for getline
#define _XOPEN_SOURCE 700

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map.h"
#include "parser.h"
#include "map_text_interface.h"

static size_t line_no = 0;

static void error(bool condition) {
    if (condition) {
        fprintf(stderr, "%zu ERROR\n", line_no);
    }
}

int main() {
    Map *m = newMap();
    if (m == NULL) {
        return 0;
    }

    char *line = NULL;
    size_t line_len = 0;
    while (line_no++, getline(&line, &line_len, stdin) != -1) {
        if (errno == ENOMEM) {
            exit(0);
        }
        struct operation op = parse(line, strlen(line));
        //printf("%d %s\n", op.op, op.arg);
        switch (op.op) {
        case OP_ROUTE_DESCRIPTION:

            break;
        case OP_REPAIR_ROAD:
            error(!execRepairRoad(m, op.arg));
            break;
        case OP_ADD_ROAD:
            error(!execAddRoad(m, op.arg));
            break;
        case OP_NEW_ROUTE:

            break;
        case OP_ERROR:
            error(true);
            break;
        case OP_NOOP:
            break;
        }
    }

    deleteMap(m);
    return 0;
}
