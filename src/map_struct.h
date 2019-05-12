#ifndef __MAP_STRUCT_H__
#define __MAP_STRUCT_H__

#include "dictionary.h"
#include "list.h"
#include "map.h"
#include "vector.h"

#define ROUTE_MAX 1000

typedef struct Route {                                                           
    List cities;                                                                 
} Route;                                                                         
                                                                                 
typedef struct Road {                                                            
    uint64_t length;
    int builtYear;
    int start;                                                                   
    int end;
} Road;                                                                          
                                                                                 
typedef struct Map {                                                             
    Route routes[ROUTE_MAX];                                                     
    /// each neighbour holds a Dictionary[int, Road]                             
    Vector neighbours;                                                           
    /// each int_to_city element holds a char*                                   
    Vector int_to_city;                                                          
    /// Dictionary[char*, int]                                                   
    Dictionary city_to_int;                                                      
    /// Dictionary[(int, int), List[int]]                                        
    Dictionary routesThrough;                                                    
} Map;                                                                           

#endif /* __MAP_STRUCT_H__ */