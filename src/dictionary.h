
#ifndef _DICTIONARY_H__
#define _DICTIONARY_H__


#include <stdint.h>
#include <stdlib.h>

#include "status.h"

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
} Dictionary;


void deleteDictionary(Dictionary* dictionary);
Dictionary* newDictionary(hash_t (*hash)(void*), bool (*equal)(void*, void*));
Status insertDictionary(Dictionary* dictionary, void* key, void* val);
Entry getDictionary(Dictionary* dictionary, void* key);
void deleteFromDictionary(Dictionary* dictionary, void* key);

#endif /* _DICTIONARY_H__ */
