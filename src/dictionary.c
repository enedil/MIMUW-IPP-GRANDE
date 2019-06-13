#include "dictionary.h"
#include <stdlib.h>

#define LOAD_FACTOR 0.9
#define DICTIONARY_INITIAL_SIZE 4
#define INDEX(key) dictionary->hash((key)) & (dictionary->array_size - 1)
#define NEXT_INDEX(index)                                                      \
    {                                                                          \
        if (++(index) == dictionary->array_size) {                             \
            (index) = 0;                                                       \
        }                                                                      \
    }

void deleteDictionary(Dictionary *dictionary) {
    if (dictionary == NULL) {
        return;
    }
    for (size_t i = 0; i < dictionary->array_size; ++i) {
        dictionary->free_key(dictionary->array[i].key);
        dictionary->free_val(dictionary->array[i].val);
    }
    free(dictionary->array);
    dictionary->size = 0;
    dictionary->array_size = 0;
    dictionary->array = NULL;
}

Dictionary *newDictionary(hash_t (*hash)(void *), bool (*equal)(void *, void *),
                          void (*free_key)(void *), void (*free_val)(void *)) {
    Dictionary *dictionary = calloc(1, sizeof(Dictionary));
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
        free(dictionary);
        return NULL;
    }
    return dictionary;
}

static Status rehashDictionary(Dictionary *dictionary, size_t new_size) {
    CHECK_RET(dictionary);
    Entry *p = calloc(new_size, sizeof(Entry));
    if (p == NULL) {
        // deleteDictionary(dictionary);
        return false;
    }
    Dictionary copy = *dictionary;
    dictionary->size = 0;
    dictionary->array_size = new_size;
    dictionary->array = p;

    for (size_t i = 0; i < copy.array_size; ++i) {
        Entry e = copy.array[i];
        if (NOT_FOUND(e) == false) {
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

Status insertDictionary(Dictionary *dictionary, void *key, void *val) {
    CHECK_RET(dictionary);
    CHECK_RET(key);
    CHECK_RET(val);
    if (dictionary->size > dictionary->array_size * LOAD_FACTOR ||
        dictionary->size + 1 >= dictionary->array_size) {
        CHECK_RET(rehashDictionary(dictionary, 2 * dictionary->array_size));
    }
    hash_t index = INDEX(key);
    while (dictionary->array[index].key != NULL &&
           dictionary->array[index].key != DELETED) {
        if (dictionary->equal(dictionary->array[index].key, key)) {
            dictionary->free_key(dictionary->array[index].key);
            dictionary->free_val(dictionary->array[index].val);
            dictionary->size--;
            break;
        }
        NEXT_INDEX(index);
    }
    dictionary->array[index].key = key;
    dictionary->array[index].val = val;
    dictionary->size++;
    return true;
}

Entry getDictionary(Dictionary *dictionary, void *key) {
    if (dictionary == NULL || key == NULL) {
        return (const Entry){NULL, NULL};
    }
    hash_t index = INDEX(key);
    size_t n = 0;
    while (dictionary->array[index].key != NULL && n < dictionary->array_size) {
        n++;
        if (dictionary->equal(dictionary->array[index].key, key)) {
            return dictionary->array[index];
        }
        NEXT_INDEX(index);
    }
    return (const Entry){NULL, NULL};
}

void deleteFromDictionary(Dictionary *dictionary, void *key) {
    if (dictionary == NULL || key == NULL) {
        return;
    }
    hash_t index = INDEX(key);
    while (dictionary->array[index].key != NULL) {
        if (dictionary->equal(dictionary->array[index].key, key)) {
            dictionary->free_key(dictionary->array[index].key);
            dictionary->free_val(dictionary->array[index].val);
            dictionary->array[index].key = DELETED;
            dictionary->array[index].val = NULL;
            dictionary->size--;
            return;
        }
        NEXT_INDEX(index);
    }
}
