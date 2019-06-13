#include <stdlib.h>
#include "queue.h"

Queue newQueue(size_t size) {
    Queue q = {0};
    q.array = calloc(size, sizeof(int));
    if (q.array == NULL) {
        return q;
    }
    q.size = size;
    q.begin = 0;
    q.end = 0;
    return q;
}

int beginQueue(Queue* q) {
    return q->array[q->begin];
}
int endQueue(Queue* q) {
    return q->array[(q->end-1+q->size) % (q->size)];
}
void pushQueueBegin(Queue* q, int elem) {
    q->begin = q->begin > 0 ? q->begin - 1 : q->size - 1;
    q->array[q->begin] = elem;
}
void pushQueueEnd(Queue* q, int elem) {
    q->end++;
    if (q->end == q->size) {
        q->end = 0;
    }
    ssize_t prev = q->end - 1;
    if (prev == -1) {
        prev += q->size;
    }
    q->array[prev] = elem;
}
void popQueueBegin(Queue* q) {
    q->begin++;
    if (q->begin == q->size) {
        q->begin = 0;
    }
}
void popQueueEnd(Queue* q) {
    q->end--;
    if (q->end == -1) {
        q->end += q->size;
    }
}

bool isEmptyQueue(Queue* q) {
    return q->begin == q->end;
}
