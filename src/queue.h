#ifndef __QUEUE_H__
#define __QUEUE_H__

#include <sys/types.h>
#include "status.h"
#include "vector.h"

typedef struct Queue {
    ssize_t size;
    ssize_t begin;
    ssize_t end;
    int* array;
} Queue;

Queue newQueue(size_t size);

int beginQueue(Queue* q);
int endQueue(Queue* q);
void pushQueueBegin(Queue* q, int elem);
void pushQueueEnd(Queue* q, int elem);
void popQueueBegin(Queue* q);
void popQueueEnd(Queue* q);
bool isEmptyQueue(Queue* q);

#endif /* __QUEUE_H__ */
