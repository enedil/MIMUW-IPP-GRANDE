#include <stdlib.h>

#include "dictionary.h"


#define DICTIONARY_INITIAL_SIZE 4

#define INDEX(key) dictionary->hash((key)) % dictionary->array_size

void deleteDictionary(Dictionary* dictionary) {
    free(dictionary->array);
    dictionary->size = 0;
    dictionary->array_size = 0;
    dictionary->array = NULL;
    free(dictionary);
}

Dictionary* newDictionary(hash_t (*hash)(void*), bool (*equal)(void*, void*)) {
    Dictionary* dictionary = malloc(sizeof(Dictionary));
    if (dictionary == NULL) {
        return NULL;
    }
    dictionary->hash = hash;
    dictionary->equal = equal;
    dictionary->size = 0;
    dictionary->array_size = DICTIONARY_INITIAL_SIZE;
    dictionary->array = calloc(DICTIONARY_INITIAL_SIZE, sizeof(Entry));
    if (dictionary->array == NULL) {
        free(dictionary);
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
                free(copy.array);
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
    }
    hash_t index = INDEX(key);
    while (dictionary->array[index].key != NULL) {
        if (dictionary->equal(dictionary->array[index].key, key)) {
            if (dictionary->array[index].key != key) {
                free(dictionary->array[index].key);
            }
            if (dictionary->array[index].val != val) {
                free(dictionary->array[index].val);
            }
            break;
        }
        index = (index + 1) % dictionary->array_size;
    }
    dictionary->array[index].key = key;
    dictionary->array[index].val = val;
    return true;
}

Entry getDictionary(Dictionary* dictionary, void* key) {
    if (dictionary == NULL || key == NULL) {
        return (const Entry){NULL, NULL};
    }
    hash_t index = INDEX(key);
    while (dictionary->array[index].key != NULL) {
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
    hash_t index = INDEX(key);
    while (dictionary->array[index].key != NULL) {
        if (dictionary->equal(dictionary->array[index].key, key)) {
            free(dictionary->array[index].key);
            free(dictionary->array[index].val);
            dictionary->array[index].key = NULL;
            dictionary->array[index].val = NULL;
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
    if ((a == 0) || (a == (void*)1) || (b == 0) || (b == (void*)1)) {
        return false;
    }
    return *(int*)a == *(int*)b;
}

#include <stdio.h>
int main() {
    Dictionary* d = newDictionary(hsh, eq);
    int* a = malloc(sizeof(int));
    *a = 4;
    int* b = malloc(sizeof(int));
    *b = 3;
    for (int i = 0; i < d->array_size; ++i) {
        printf("%p, %p\n", (int*)d->array[i].key, d->array[i].val);
    }

    insertDictionary(d, a, b);
    Entry e = getDictionary(d, a);
    printf("%d, %d\n",*(int*)(e.key), *(int*)(e.val));
    for (int i = 0; i < d->array_size; ++i) {
        printf("%p, %p\n", (int*)d->array[i].key, d->array[i].val);
    }
    int c = 4;
    deleteFromDictionary(d, &c);


    deleteDictionary(d);
}
