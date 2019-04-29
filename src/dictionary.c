#include <stdlib.h>

#include "dictionary.h"


#define DICTIONARY_INITIAL_SIZE 4

#define INDEX(key) dictionary->hash((key)) % dictionary->array_size

void free_(void* ptr) {
    if (ptr != DELETED) {
        free(ptr);
    }
}

void deleteDictionary(Dictionary* dictionary) {
    if (dictionary == NULL) {
        return;
    }
    for (size_t i = 0; i < dictionary->array_size; ++i) {
        dictionary->free_key(dictionary->array[i].key);
        dictionary->free_val(dictionary->array[i].val);
    }
    free_(dictionary->array);
    dictionary->size = 0;
    dictionary->array_size = 0;
    dictionary->array = NULL;
}

Dictionary* newDictionary(hash_t (*hash)(void*), bool (*equal)(void*, void*), void (*free_key)(void*), void (*free_val)(void*)) {
    Dictionary* dictionary = calloc(1, sizeof(Dictionary));
    if (dictionary == NULL) {
        return NULL;
    }
    dictionary->hash = hash;
    dictionary->equal = equal;
    dictionary->size = 0;
    dictionary->array_size = DICTIONARY_INITIAL_SIZE;
    dictionary->array = calloc(DICTIONARY_INITIAL_SIZE, sizeof(Entry));
    dictionary->free_key = free_key;
    dictionary->free_val = free_val;
    if (dictionary->array == NULL) {
        free_(dictionary);
        return NULL;
    }
    return dictionary;
}

static Status rehashDictionary(Dictionary* dictionary, size_t new_size) {
    CHECK_RET(dictionary);
    Entry* p = calloc(new_size, sizeof(Entry));
    if (p == NULL) {
        deleteDictionary(dictionary);
        return false;
    }
    Dictionary copy = *dictionary;
    dictionary->size = 0;
    dictionary->array_size = new_size;
    dictionary->array = p;

    for (size_t i = 0; i < copy.array_size; ++i) {
        Entry e = copy.array[i];
        if (e.key != NULL) {
            if (insertDictionary(dictionary, e.key, e.val) == false) {
                deleteDictionary(dictionary);
                *dictionary = copy;
                return false;
            }
        }
    }

    free(copy.array);
    return true;
}

Status insertDictionary(Dictionary* dictionary, void* key, void* val) {
    CHECK_RET(dictionary);
    CHECK_RET(key);
    CHECK_RET(val);
    if (dictionary->size * 10 > dictionary->array_size * 9) {
        CHECK_RET(rehashDictionary(dictionary, 2*dictionary->array_size));
    } else if (dictionary->size + 1 >= dictionary->array_size) {
        CHECK_RET(rehashDictionary(dictionary, 2*dictionary->array_size));
    }

    bool free_key = dictionary->memlocation[0];
    bool free_val = dictionary->memlocation[1];
    hash_t index = INDEX(key);
    while (dictionary->array[index].key != NULL) {
        if (dictionary->equal(dictionary->array[index].key, key)) {
            if (dictionary->array[index].key != key && free_key) {
                free_(dictionary->array[index].key);
            }
            if (dictionary->array[index].val != val && free_val) {
                free_(dictionary->array[index].val);
            }
            break;
        }
        index = (index + 1) % dictionary->array_size;
    }
    dictionary->array[index].key = key;
    dictionary->array[index].val = val;
    dictionary->size++;
    return true;
}

Entry getDictionary(Dictionary* dictionary, void* key) {
    if (dictionary == NULL || key == NULL) {
        return (const Entry){NULL, NULL};
    }
    hash_t index = INDEX(key);
    while (NOT_FOUND(dictionary->array[index]) == false) {
        if (dictionary->equal(dictionary->array[index].key, key)) {
            return dictionary->array[index];
        }
        index = (index + 1) % dictionary->array_size;
    }
    return (const Entry){NULL, NULL};
}

void deleteFromDictionary(Dictionary* dictionary, void* key) {
    if (dictionary == NULL || key == NULL) {
        return;
    }
    bool free_key = dictionary->memlocation[0];
    bool free_val = dictionary->memlocation[1];
    hash_t index = INDEX(key);
    while (dictionary->array[index].key != NULL) {
        if (dictionary->equal(dictionary->array[index].key, key)) {
            if (free_key) {
                free_(dictionary->array[index].key);
            }
            if (free_val) {
                free_(dictionary->array[index].val);
            }
            dictionary->array[index].key = DELETED;
            dictionary->array[index].val = DELETED;
            dictionary->size--;
            return;
        }
        index = (index + 1) % dictionary->array_size;
    }
}

hash_t hsh(void* key) {
    hash_t *x = key;
    return *x;
}

bool eq(void* a, void* b) {
    if ((a == 0) || (a == DELETED) || (b == 0) || (b == DELETED)) {
        return false;
    }
    return *(int*)a == *(int*)b;
}

/*
#include <stdio.h>
static void printdict(Dictionary* d)
{
    if (d == NULL) {
        return;
    }
    printf("Dictionary at %p:\n", d);
    for (int i = 0; i < d->array_size; ++i) {
        printf("%p, %p\n", d->array[i].key, d->array[i].val);
    }
    printf("\n");
}
int main() {
    Dictionary* d = newDictionary(hsh, eq);

    int INSERT = 1;
    int GET = 2;
    int DELETE = 3;

    int command;
    int key;
    int val;
    while (true) {
        printf("cmd, key, val ");
        scanf("%d%d%d", &command, &key, &val);
        if (command == INSERT) {
            int* k = malloc(sizeof(int));
            *k = key;
            int* v = malloc(sizeof(int));
            *v = val;
            insertDictionary(d, k, v);
        } else if (command == GET) {
            Entry e = getDictionary(d, &key);
            printf("ret: %p %p\n", e.key, e.val);
        } else if (command == DELETE) {
            deleteFromDictionary(d, &key);
        } else {
            break;
        }
        printdict(d);
    }

    deleteDictionary(d);
    free(d);
}
*/
