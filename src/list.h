#ifndef __LIST_H__
#define __LIST_H__

#include "status.h"


typedef struct Node {
    struct Node *prev;
    struct Node *next;
    int value;
} Node;

typedef struct List {
    Node *begin;
    Node *end;
} List;

List* newList();
Status listInsertAfter(List* list, Node* node, int elem);
Status listInsertBefore(List* list, Node* node, int elem);
void deleteListNode(List* list, Node* node);
void deleteList(List* list);

#endif /* __LIST_H__ */
