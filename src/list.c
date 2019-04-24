#include <stdlib.h>
#include "list.h"

List* newList() {
    List *n = calloc(1, sizeof(List));
    if (n == NULL) {
        return NULL;
    }
    n->begin = calloc(2, sizeof(Node));
    if (n->begin == NULL) {
        free(n);
        return NULL;
    }
    n->end = n->begin + 1;

    n->begin->next = n->end;
    n->begin->prev = NULL;
    n->end->next = NULL;
    n->end->prev = n->begin;

    return n;
}

Status listInsertAfter(List* list, Node* node, int elem) {
    if (list == NULL || node == NULL) {
        return false;
    }
    Node *new = calloc(1, sizeof(Node));
    new->value = elem;
    if (new == NULL) {
        return false;
    }

    if (node == list->end) {
        Node *before_end = list->end->prev;
        before_end->next = new;
        new->prev = before_end;
        list->end->prev = new;
        new->next = list->end;
    } else {
        Node *next = node->next;
        node->next = new;
        new->prev = node;
        next->prev = new;
        new->next = next;
    }
    return true;
}

Status listInsertBefore(List* list, Node* node, int elem) {
    if (list == NULL || node == NULL) {
        return false;
    }
    if (node == list->begin) {
        return listInsertAfter(list, node, elem);
    }
    return listInsertAfter(list, node->prev, elem);
}

void deleteList(List* list) {
    if (list == NULL) {
        return;
    }
    Node *it = list->begin->next;
    while (it != list->end) {
        it = it->next;
        free(it->prev);
    }
    free(list->begin);
    free(list);
}


#include <stdio.h>
int main()
{
    List* l = newList();
    listInsertAfter(l, l->begin, 1);
    listInsertAfter(l, l->begin, 2);
    listInsertAfter(l, l->end, 3);
    listInsertBefore(l, l->end->prev->prev, 5);
    listInsertAfter(l, l->begin->next, 4);
    deleteList(l);
}
