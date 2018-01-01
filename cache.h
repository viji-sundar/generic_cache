#include<stdio.h>
#include<malloc.h>
#include<math.h>
#include<stdlib.h>
#include<stdbool.h>

#define ADDRESS_WIDTH 32
#define WBWA          0
#define WTNA          1
#define LRU           2
#define LFU           3

//Pointers of the defined structures
typedef struct _blockT*     blockPT;
typedef struct _cacheT*     cachePT;
typedef struct _petriDishT* petriDishPT;

// Structure for a block
typedef struct _blockT {
   int        tag;
   int        validBit;
   int        dirtyBit;
   int        count;
}blockT;

typedef struct _petriDishT {
   float      a; //0.25
   float      b; //2.5
   float      c; //524288.0
   float      d; //0.025
   float      e; //16.0
   float      f; //0.025
   float      g; //20
   float      h; //0.5
   float      i; //16.0
}petriDishT;

// Structure for a generic cache
typedef struct _cacheT {
   // Config params
   int        cSize;
   int        bSize;
   int        assoc;
   int        writePolicy;
   int        replacePolicy;

   cachePT    nextLevelCache;

   int        reads;
   int        writes;
   int        readMisses;
   int        writeMisses;
   int        writeBacks;
   float      missPenalty;
   float      hitTime;

   int        rows;

   int        index;
   int        tag;

   // Tag store model
   blockPT*   tagStore;
}cacheT;

#include "build/cache_proto.h"

