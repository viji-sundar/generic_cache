#include "cache.h"

#define LOG_2(A) ceil( log(A)/log(2) )

//-------------------------POSSIBLE FUNCTIONS----------------------------------//
//1. cacheAllocate(cSize, bSize, assoc)
//   Function allocates an array to of the appropriate sizes
/*!proto*/
cachePT cacheAllocate(int c, int b, int s, int wp, int rp, petriDishPT t ) 
/*!endproto*/
{
   // Obtain the number of rows from the given parameters
   if(c == 0 || s == 0) return NULL;

   cachePT cacheP          = (cachePT)calloc(1, sizeof(cacheT));
   cacheP->cSize           = c;
   cacheP->bSize           = b;
   cacheP->assoc           = s;
   cacheP->writePolicy     = wp;
   cacheP->replacePolicy   = rp;
   cacheP->rows            = cacheP->cSize/(cacheP->bSize*cacheP->assoc);
   
   // Allocate memory for the cache as a 2D- array
   cacheP->tagStore        = (blockPT*)calloc(cacheP->rows, sizeof(blockPT));
   for(int i = 0; i < cacheP->rows; i++) {
      cacheP->tagStore[i]  = (blockPT)calloc(cacheP->assoc, sizeof(blockT));
      if(rp == LRU) {
         for(int j = 0; j < cacheP->assoc; j++) {
            cacheP->tagStore[i][j].count = j;
         }
      }
   }
   cacheP->hitTime         = t->a + (t->b * (cacheP->cSize/t->c)) + (t->d * (cacheP->bSize/t->e)) + (t->f * cacheP->assoc);
   cacheP->missPenalty     = t->g + t->h*(cacheP->bSize/t->i);
   return cacheP;
}

//2. getIndexAndTag( address )
//   Obtain index and tag from the decoded address
/*!proto*/
void getIndexAndTag(cachePT cacheP, unsigned int address) 
/*!endproto*/   
{
   // obtain the index bits, tag bits and offset bits from the parameters
   // obtain Index and tag from the derived bits
   int indexBits      = LOG_2(cacheP->rows); 
   int offsetBits     = LOG_2(cacheP->bSize);
   int tagBits        = ADDRESS_WIDTH - (indexBits+offsetBits);

   cacheP->index      = (address << tagBits) >> (tagBits+offsetBits);
   cacheP->tag        = address >> (indexBits+offsetBits);
}


//3. read (index, tag)
/*!proto*/
bool read ( cachePT cacheP, int address )
/*!endproto*/
{
   if( cacheP == NULL ) return true;

   cacheP->reads++;
   int hitIndex;
   getIndexAndTag(cacheP, address);
   bool cond = searchTagStore(cacheP, &hitIndex);
   if(!cond) { // MISS
      cacheP->readMisses++;
      cacheMiss(cacheP, address);
   }
   else { // HIT
      updateCounters(cacheP, hitIndex);
   }
   return cond;
}

//4. write (index, tag)
/*!proto*/
bool write (cachePT cacheP, int address) 
/*!endproto*/
{
   if( cacheP == NULL ) return true;

   cacheP->writes++;
   int hitIndex;
   getIndexAndTag(cacheP, address);
   bool cond = searchTagStore(cacheP, &hitIndex);
   if(!cond) { // MISS
      cacheP->writeMisses++;
      if(cacheP->writePolicy == WBWA) {
        int dirtyAt = cacheMiss(cacheP, address);
        cacheP->tagStore[cacheP->index][dirtyAt].dirtyBit = 1;
      }
   }
   else { // HIT
      updateCounters(cacheP, hitIndex);
      if(cacheP->writePolicy == WBWA) {
         cacheP->tagStore[cacheP->index][hitIndex].dirtyBit = 1;
      }
   }
   if(cacheP->writePolicy == WTNA) 
      write (cacheP->nextLevelCache, address);
   return cond;
}
//5. searchTagStore( )
//   using index, look for matches with the tag. 
//   if match found - Hit; if mach not found - Miss
/*!proto*/
bool searchTagStore (cachePT cacheP, int* hitIndex)
/*!endproto*/
{
   // Scan through the tagStore to check for a match
   for(int j = 0; j < cacheP->assoc; j++)
   {
      if(cacheP->tagStore[cacheP->index][j].tag == cacheP->tag && cacheP->tagStore[cacheP->index][j].validBit == 1) {
         *hitIndex = j;
         return true;
      }
   }
   return false;
}

/*!proto*/
void updateCounters( cachePT cacheP, int hitIndex )
/*!endproto*/
{
   if(cacheP->replacePolicy == LRU)
      updateLRU( cacheP, hitIndex );
   else if(cacheP->replacePolicy == LFU)
      updateLFU( cacheP, hitIndex );
}

/*!proto*/
void updateLFU(cachePT cacheP, int hitIndex)
/*!endproto*/
{
   cacheP->tagStore[cacheP->index][hitIndex].count++;
}

// 6. updateLRU ( )
//    set current index counter to 0
//    Increment rest until present counter if a hit; Increment all if a miss

/*!proto*/
void updateLRU(cachePT cacheP, int hitIndex)
/*!endproto*/
{
   //update LRU for hits
   int col = cacheP->tagStore[cacheP->index][hitIndex].count;
   for(int j = 0; j < cacheP->assoc; j++) {
      if(cacheP->tagStore[cacheP->index][j].count < col) 
         cacheP->tagStore[cacheP->index][j].count++;
   }
   cacheP->tagStore[cacheP->index][hitIndex].count = 0;
}

// 7. cacheMiss ( )
//    Check for victim blocks based on the valid bit
//    replace / evict based on the replacement (LRU) policy

/*!proto*/
int cacheMiss(cachePT cacheP, int address) 
/*!endproto*/
{
   int indexBits      = LOG_2(cacheP->rows); 
   int offsetBits     = LOG_2(cacheP->bSize);
   // Search for an empty place
   int empty = -1;
   for(int j = 0; j < cacheP->assoc; j++) {
      if(cacheP->tagStore[cacheP->index][j].validBit == 0) {
         empty = j;
         break;
      }
   }

   // If an empty block is not found, search using replacement policy
   if(empty == -1) {
      // Search for victim block for LRU Policy
      if(cacheP->replacePolicy == LRU)
         empty = getEvictionLRU(cacheP);

      //Search for victim block for LFU policy
      else if(cacheP->replacePolicy == LFU) 
         empty = getEvictionLFU(cacheP);
   }

   // Check if dirty. If yes, do a writeback
   if(cacheP->tagStore[cacheP->index][empty].dirtyBit == 1) {
      unsigned int evictedAddress = (cacheP->tagStore[cacheP->index][empty].tag << (indexBits + offsetBits)) | (cacheP->index << (offsetBits));
      write(cacheP->nextLevelCache, evictedAddress);
      cacheP->writeBacks++;
      cacheP->tagStore[cacheP->index][empty].dirtyBit = 0; 
   }

   // Do a read request
   read(cacheP->nextLevelCache, address);

   // Cache data at evicted/empty place
   cacheP->tagStore[cacheP->index][empty].tag = cacheP->tag;
   cacheP->tagStore[cacheP->index][empty].validBit = 1;
   // Update LFU and LRU of that block
   updateCounters(cacheP, empty);
   return empty;
}

/*!proto*/
int getEvictionLRU(cachePT cacheP) 
/*!endproto*/
{
   for(int j = 0; j < cacheP->assoc; j++) {
      if(cacheP->tagStore[cacheP->index][j].count == cacheP->assoc-1) {
         return j;
      }
   } 
}

/*!proto*/
int getEvictionLFU(cachePT cacheP) 
/*!endproto*/
{
   int min   = cacheP->tagStore[cacheP->index][0].count; 
   int empty = 0; 
   for(int j = 1; j < cacheP->assoc; j++) {
      if(min > cacheP->tagStore[cacheP->index][j].count) {
         min = cacheP->tagStore[cacheP->index][j].count;
         empty = j;
      }
   } 
   return empty;
}


/*!proto*/
void printTagstore (cachePT cacheP, char* name)
/*!endproto*/
{
   printf("===== %s contents =====\n", name);
   for(int i = 0; i < cacheP->rows; i++) {
      printf("set %d: ", i);
      for(int j = 0; j < cacheP->assoc; j++) {
         printf("%x %c ", cacheP->tagStore[i][j].tag, cacheP->tagStore[i][j].dirtyBit ? 'D' : ' ');   
      }
      printf("\n");
   }  
}

/*!proto*/
void getResults ( cachePT     cacheP, 
                  int*        reads, 
                  int*        writes, 
                  int*        readMisses, 
                  int*        writeMisses,
                  float*      missRate,
                  int*        writeBacks, 
                  int*        memTraffic )
/*!endproto*/
{
   *reads              = cacheP->reads;
   *writes             = cacheP->writes;
   *readMisses         = cacheP->readMisses;
   *writeMisses        = cacheP->writeMisses;
   *writeBacks         = cacheP->writeBacks;
   *missRate           = (float)(*readMisses + *writeMisses)/(float)(*reads + *writes);

   if(cacheP->writePolicy == WBWA)
      *memTraffic      = *writeBacks + *readMisses + *writeMisses;
   else
      *memTraffic      = *readMisses + *writes;
}

/*!proto*/
float getAAT ( cachePT cacheP )
/*!endproto*/
{
   float missRate    = (float)(cacheP->readMisses + cacheP->writeMisses)/(float)(cacheP->reads + cacheP->writes);
   float missPenalty = (cacheP->nextLevelCache) ? getAAT ( cacheP->nextLevelCache ) : cacheP->missPenalty;
   return cacheP->hitTime + ( missRate ) * missPenalty;
}

