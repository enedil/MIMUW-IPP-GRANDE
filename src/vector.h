
#ifndef _VECTOR_H__
#define _VECTOR_H__

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


#endif /* _VECTOR_H__ */
