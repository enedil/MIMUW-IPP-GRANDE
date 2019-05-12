#ifndef __LIST_H__
#define __LIST_H__

#include <stdlib.h>
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

List* newList(void);
Status listInsertAfter(List* list, Node* node, int elem);
Status listInsertBefore(List* list, Node* node, int elem);
void deleteListNode(List* list, Node* node);
void deleteList(List* list);
List* copyList(List* list);
void listEmplaceNode(List* list, Node* after, Node* val);
void insertListAfterElement(List* l, List* new, int el);
size_t listPos(List* l, int el);
void listReverse(List* l);
Node* listFind(List* l, int el);

#endif /* __LIST_H__ */
