#ifndef __STRING_UTILS__
#define __STRING_UTILS__

#include "dictionary.h"

hash_t hash_string(void* str);
bool undereferencing_strcmp(void* x, void* y);
bool validCityName(const char* city);
bool possiblyValidRoad(const char* city1, const char* city2);
void deleteDictionaryOfLists(Dictionary* d);
hash_t hashEdge(void* key);
bool cmpEdges(void* e1, void* e2);
size_t int_length(int64_t x);

#endif /* __STRING_UTILS__ */
