#include<stdio.h>
#include<malloc.h>
#include<math.h>
#include<stdlib.h>
#include<stdbool.h>

// BUGS FIXED:
// 1.  Missing init counters for replacement policies. (All 0s for LFU, 
//     [0,assoc-1] for LRU)
// 2.  During write hit, dirty bit was not being set (WBWA) which 
//     caused issues in read followed by writes (Write hit)
// 3.  Valid bit was not being set to 1 during data placement
// 4.  For writeback, address corresponding to the block being evicted
//     was not passed.
// 5.  For misses (W/R), writeback has to be done prior to reading 
//     and allocating the block
// 6.  Read miss counter wrongly placed when victim was a hit
// 7.  Transfer dirty bit along with address during cache to victim transfer
// 8.  cache2Victim has to be done IFF !emptyFromI & victimCache != NULL
// 9.  L2 has to be updated when victim is present and has a writeback
// 10. Cache swap is considered a miss counter update (matters in LRFU)

// EXISTING BUGS:
// -NA-

#define ADDRESS_WIDTH 32
#define WBWA          0
#define WTNA          1
#define LRFU          0
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
   double     crf;
}blockT;

typedef struct _petriDishT {
   float      a; // 0.25
   float      b; // 2.5
   float      c; // 524288.0
   float      d; // 0.025
   float      e; // 16.0
   float      f; // 0.025
   float      g; // 20
   float      h; // 0.5
   float      i; // 16.0
}petriDishT;

// Structure for a generic cache
typedef struct _cacheT {
   // Config params
   int        cSize;
   int        bSize;
   int        assoc;
   int        writePolicy;
   int        replacePolicy;
   double     lambda; 

   cachePT    nextLevelCache;
   cachePT    victimCache;

   int        reads;
   int        writes;
   int        readMisses;
   int        writeMisses;
   int        writeBacks;
   float      missPenalty;
   float      hitTime;
   int        swaps;
   int        victimBit;
   int        globalCount;

   int        rows;

   int        index;
   int        tag;

   // Tag store model
   blockPT*   tagStore;
}cacheT;

#include "build/cache_proto.h"

