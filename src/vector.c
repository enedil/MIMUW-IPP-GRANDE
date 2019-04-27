#include <stdio.h>
#include <stdlib.h>
#include "vector.h"

Vector* newVector() {
    Vector* n = malloc(sizeof(Vector));
    if (n == NULL) {
        return NULL;
    }

    const size_t initial_capacity = 4;
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
        // TODO: how to handle this? Should I free something?
        return false;
    }
    vector->arr = n;
    return vectorAppend(vector, element);
}
/*
int main()
{

    Vector* n = newVector();

    int *x[10];
    
    for (int i = 0; i < 10; ++i) {
        x[i] = calloc(1, sizeof(x));
        vectorAppend(n, x[i]);
    }
    for (int i = 0; i < n->size; ++i) {
        printf("%d ", *(int*)n->arr[i]);
    }
    vectorDeleteFreeContent(n);
}
*/
