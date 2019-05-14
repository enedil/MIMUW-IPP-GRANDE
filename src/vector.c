#include <stdio.h>
#include <stdlib.h>
#include "vector.h"

Vector* newVector(void) {
    Vector* n = malloc(sizeof(Vector));
    if (n == NULL) {
        return NULL;
    }

    const size_t initial_capacity = 8;
    n->arr = calloc(initial_capacity, sizeof(void*));
    if (n->arr == NULL) {
        free(n);
        return NULL;
    }
    n->size = 0;
    n->capacity = initial_capacity;
    return n;
}

void vectorDelete(Vector* vector) {
    if (vector == NULL) {
        return;
    }
    free(vector->arr);
    vector->size = 0;   // avoid potential "use after free"
    vector->capacity = 0;
//    free(vector);
}

void vectorDeleteFreeContent(Vector* vector) {
    if (vector == NULL) {
        return;
    }
    for (size_t i = 0; i < vector->size; ++i) {
        free(vector->arr[i]);
        vector->arr[i] = NULL;
    }
    vectorDelete(vector);
}

Status vectorAppend(Vector *vector, void *element) {
    if (vector == NULL) {
        return false;
    }
    if (vector->size < vector->capacity) {
        vector->arr[vector->size] = element;
        vector->size++;
        return true;
    }
    vector->capacity *= 2;
    void *n = realloc(vector->arr, vector->capacity * sizeof(void*));
    if (n == NULL) {
        return false;
    }
    vector->arr = n;
    return vectorAppend(vector, element);
}

void vectorRemoveLast(Vector* vector, bool free_) {
    if (vector == NULL) {
        return;
    }
    if (vector->size == 0) {
        return;
    }
    vector->size--;
    /// NOTE: perhaps you want to free recursively
    if (free_) {
        free(vector->arr[vector->size]);
    }
}
