#include "list.h"
#include "dictionary.h"
#include "utils.h"
#include <stdlib.h>

List *newList(void) {
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

Status listInsertAfter(List *list, Node *node, int elem) {
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

Status listInsertBefore(List *list, Node *node, int elem) {
    if (list == NULL || node == NULL) {
        return false;
    }
    if (node == list->begin) {
        return listInsertAfter(list, node, elem);
    }
    return listInsertAfter(list, node->prev, elem);
}

void deleteList(List *list) {
    if (list == NULL || list->begin == NULL) {
        return;
    }
    Node *begin = list->begin;
    Node *it = list->begin->next;
    while (it != list->end && it->next) {
        it = it->next;
        it->prev->next = NULL;
        free(it->prev);
        it->prev = NULL;
    }
    list->begin = list->end =NULL;
    free(begin);
}

void deleteListNode(List *list, Node *node) {
    if (list == NULL || node == NULL) {
        return;
    }
    if (node == list->begin || node == list->end) {
        return;
    }
    Node *prev = node->prev;
    Node *next = node->next;
    prev->next = next;
    next->prev = prev;

    // zapobiegaj przypadkowemu "use after free"
    node->next = node->prev = NULL;
    free(node);
}

List *copyList(List *list) {
    List *ret = newList();
    CHECK_RET(ret);
    for (Node *n = list->begin->next; n != list->end; n = n->next) {
        if (listInsertAfter(ret, ret->end, n->value) == false) {
            deleteList(ret);
            return NULL;
        }
    }
    return ret;
}

Status listInsertUnique(List *list, int el) {
    CHECK_RET(list);
    for (Node *n = list->begin->next; n != list->end; n = n->next) {
        if (n->value == el) {
            return true;
        }
    }
    return listInsertAfter(list, list->end, el);
}

void listEmplaceNode(List *list, Node *after, Node *val) {
    if (after == list->end) {
        after = after->prev;
    }
    Node *x = after->next;
    after->next = val;
    val->next = x;
    val->prev = after;
    x->prev = val;
}

void insertListAfterElement(List *l, List *new, int el) {
    for (Node *n = l->begin->next; n != l->end; n = n->next) {
        if (n->value == el) {
            Node *x = n->next;
            n->next = new->begin->next;
            new->begin->next->prev = n;
            new->end->prev->next = x;
            x->prev = new->end->prev;
            // free(new->begin);
            return;
        }
    }
}

ssize_t listPos(List *l, int el) {
    size_t ret = 0;
    Node *n;
    for (n = l->begin->next; n != l->end; n = n->next) {
        if (n->value == el) {
            break;
        }
        ret++;
    }
    if (n == l->end) {
        return -1;
    }
    return ret;
}

void listReverse(List *l) {
    for (Node *b = l->begin->next, *e = l->end->prev; b != e && b->next != e;
         b = b->next, e = e->prev) {
        swap(&b->value, &e->value);
    }
}

Node *listFind(List *l, int el) {
    if (l == NULL) {
        return NULL;
    }
    for (Node *n = l->begin->next; n != l->end; n = n->next) {
        if (n->value == el) {
            return n;
        }
    }
    return NULL;
}

bool isEmptyList(List *l) {
    if (!l->begin) {
        return true;
    }
    return l->begin->next == l->end;
}
