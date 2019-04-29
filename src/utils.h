#ifndef __STRING_UTILS__
#define __STRING_UTILS__

#include "dictionary.h"
#include "vector.h"

hash_t hash_string(void* str);
bool undereferencing_strcmp(void* x, void* y);
bool validCityName(const char* city);
bool possiblyValidRoad(const char* city1, const char* city2);
void deleteDictionaryOfLists(Dictionary* d);
hash_t hashEdge(void* key);
bool cmpEdges(void* e1, void* e2);
size_t intLength(int64_t x);
void* encodeEdgeAsPtr(int a, int b);
void* encodeCityId(int id);
int decodeCityId(void* p);
int min(int a, int b);
hash_t hashInt(void* p);
bool equalInt(void* p1, void* p2);
void deleteVectorOfLists(Vector* vector);
void swap(int* x, int* y);
void empty(void*);

#endif /* __STRING_UTILS__ */
