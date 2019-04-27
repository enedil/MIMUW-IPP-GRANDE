#ifndef __DICTIONARY_H__
#define __DICTIONARY_H__


#include <stdint.h>
#include <stdlib.h>

#include "status.h"

#define DELETED ((void*)1)

#define NOT_FOUND(e) ((e).key == NULL || (e).key == DELETED)

typedef uint64_t hash_t;

typedef struct Entry {
    void*   key;
    void*   val;
} Entry;

typedef struct Dictionary {
    hash_t      (*hash)(void*);
    bool        (*equal)(void*, void*);
    Entry*      array;
    size_t      array_size;
    size_t      size;
    bool        memlocation[2];
} Dictionary;


void deleteDictionary(Dictionary* dictionary);
Dictionary* newDictionary(hash_t (*hash)(void*), bool (*equal)(void*, void*), bool free_key, bool free_val);
Status insertDictionary(Dictionary* dictionary, void* key, void* val);
Entry getDictionary(Dictionary* dictionary, void* key);
void deleteFromDictionary(Dictionary* dictionary, void* key);

#endif /* __DICTIONARY_H__ */
