#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <stdlib.h>

#include "status.h"

typedef struct Vector {
    void **arr;
    size_t size;
    size_t capacity;
} Vector;

Vector* newVector();
void vectorDelete(Vector* vector);
void vectorDeleteFreeContent(Vector* vector);
Status vectorAppend(Vector *vector, void *element);
void vectorRemoveLast(Vector* vector, bool free_);


#endif /* __VECTOR_H__ */
