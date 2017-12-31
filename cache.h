#include<stdio.h>
#include<malloc.h>
#include<math.h>
#include<stdlib.h>
#include<stdbool.h>

#define ADDRESS_WIDTH 32
#define WBWA          0
#define WTNA          1
#define LRU           0
#define LFU           1

//Pointers of the defined structures
typedef struct _blockT* blockPT;
typedef struct _cacheT* cachePT;

// Structure for a block
typedef struct _blockT {
   int        tag;
   int        validBit;
   int        dirtyBit;
   int        count;
}blockT;

// Structure for a generic cache
typedef struct _cacheT {
   // Config params
   int        cSize;
   int        bSize;
   int        assoc;
   int        writePolicy;
   int        replacePolicy;


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

