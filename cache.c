#include "cache.h"

#define LOG_2(A) ceil( log(A)/log(2) )
#define F(x) pow(0.5, cacheP->lambda*(x))

//-------------------------POSSIBLE FUNCTIONS----------------------------------//
/*!proto*/
//1. cacheAllocate(cSize, bSize, assoc)
//   Function allocates an array to of the appropriate sizes
cachePT cacheAllocate(int c, int b, int s, int wp, double lambda, petriDishPT t ) 
// Hello
/*!endproto*/
{
   // Obtain the number of rows from the given parameters
   if(c == 0 || s == 0) return NULL;

   cachePT cacheP          = (cachePT)calloc(1, sizeof(cacheT));
   cacheP->cSize           = c;
   cacheP->bSize           = b;
   cacheP->assoc           = s;
   cacheP->writePolicy     = wp;
   cacheP->replacePolicy   = lambda == LFU ? LFU : (lambda == LRU ? LRU : LRFU );
   cacheP->lambda          = lambda;
   cacheP->rows            = cacheP->cSize/(cacheP->bSize*cacheP->assoc);
   
   // Allocate memory for the cache as a 2D- array
   cacheP->tagStore        = (blockPT*)calloc(cacheP->rows, sizeof(blockPT));
   for(int i = 0; i < cacheP->rows; i++) {
      cacheP->tagStore[i]  = (blockPT)calloc(cacheP->assoc, sizeof(blockT));
      if(cacheP->replacePolicy == LRU) {
         for(int j = 0; j < cacheP->assoc; j++) {
            cacheP->tagStore[i][j].count = j;
         }
      }
   }
   cacheP->hitTime         = t->a + (t->b * (cacheP->cSize/t->c)) + (t->d * (cacheP->bSize/t->e)) + (t->f * cacheP->assoc);
   cacheP->missPenalty     = t->g + t->h*(cacheP->bSize/t->i);
   return cacheP;
}

/*!proto*/
//2. getIndexAndTag( address )
//   Obtain index and tag from the decoded address
void getIndexAndTag(cachePT cacheP, unsigned int address) 
/*!endproto*/   
{
   // Obtain the index bits, tag bits and offset bits from the parameters
   // Obtain Index and tag from the derived bits
   int indexBits      = LOG_2(cacheP->rows); 
   int offsetBits     = LOG_2(cacheP->bSize);
   int tagBits        = ADDRESS_WIDTH - (indexBits+offsetBits);

   cacheP->index      = ( tagBits + offsetBits >= ADDRESS_WIDTH ) ? 0 : (address << tagBits) >> (tagBits+offsetBits);
   cacheP->tag        = address >> (indexBits+offsetBits);
}


/*!proto*/
//3. read (index, tag)
bool read ( cachePT cacheP, int address )
/*!endproto*/
{
   // Simulating memory (cacheP == NULL) by doing a hit
   if( cacheP == NULL ) return true;

   cacheP->reads++;
   cacheP->globalCount++;
   int hitColumn;
   getIndexAndTag(cacheP, address);
   if(!searchTagStore(cacheP, &hitColumn)) { //MISS
      if(cacheP->victimCache != NULL) {
         int vicHitCol;
         if(victimHitMiss(cacheP->victimCache, address, &vicHitCol)) {
            cacheSwap(cacheP, address, vicHitCol);
            return true;
         }
      }
      cacheP->readMisses++;
      cacheMiss(cacheP, address);
   }
   else { // HIT
      updateCounters(cacheP, hitColumn);
      return true;
   }
   return false;
}

/*!proto*/
//4. write (index, tag)
bool write (cachePT cacheP, int address) 
/*!endproto*/
{
   // Simulating memory (cacheP == NULL) by doing a hit
   if( cacheP == NULL ) return true;

   cacheP->writes++;
   cacheP->globalCount++;
   int hitColumn;
   getIndexAndTag(cacheP, address);
   bool cond = searchTagStore(cacheP, &hitColumn);
   if(!cond) { // MISS
      if(cacheP->writePolicy == WBWA) {
         int dirtyAt;
         if(cacheP->victimCache != NULL) {
            int vicHitCol;
            if(victimHitMiss(cacheP->victimCache, address, &vicHitCol)) {
               dirtyAt = cacheSwap(cacheP, address, vicHitCol);
               cacheP->tagStore[cacheP->index][dirtyAt].dirtyBit = 1;
               return true;
            }
         }
         cacheP->writeMisses++;
         dirtyAt = cacheMiss(cacheP, address);
         cacheP->tagStore[cacheP->index][dirtyAt].dirtyBit = 1;
      }
   }
   else { // HIT
      updateCounters(cacheP, hitColumn);
      if(cacheP->writePolicy == WBWA) {
         cacheP->tagStore[cacheP->index][hitColumn].dirtyBit = 1;
      }
   }
   if(cacheP->writePolicy == WTNA) 
      write (cacheP->nextLevelCache, address);
   return cond;
}

/*!proto*/
//5. searchTagStore( )
//   using index, look for matches with the tag. 
//   if match found - Hit; if mach not found - Miss
bool searchTagStore (cachePT cacheP, int* hitColumn)
/*!endproto*/
{
   // Scan through the tagStore to check for a match
   for(int j = 0; j < cacheP->assoc; j++)
   {
      if(cacheP->tagStore[cacheP->index][j].tag == cacheP->tag && cacheP->tagStore[cacheP->index][j].validBit == 1) {
         *hitColumn = j;
         return true;
      }
   }
   return false;
}

/*!proto*/
void updateCounters( cachePT cacheP, int hitColumn )
/*!endproto*/
{
   if(cacheP->replacePolicy == LRU)
      updateLRU(cacheP, hitColumn);
   else if(cacheP->replacePolicy == LFU)
      updateLFU(cacheP, hitColumn);
   else
      updateLRFU(cacheP, hitColumn);
}

/*!proto*/
void updateLFU(cachePT cacheP, int hitColumn)
/*!endproto*/
{
   cacheP->tagStore[cacheP->index][hitColumn].count++;
}

/*!proto*/
// 6. updateLRU ( )
//    set current index counter to 0
//    Increment rest until present counter if a hit; Increment all if a miss
void updateLRU(cachePT cacheP, int hitColumn)
/*!endproto*/
{
   // Update LRU for hits
   int col = cacheP->tagStore[cacheP->index][hitColumn].count;
   for(int j = 0; j < cacheP->assoc; j++) {
      if(cacheP->tagStore[cacheP->index][j].count < col) 
         cacheP->tagStore[cacheP->index][j].count++;
   }
   cacheP->tagStore[cacheP->index][hitColumn].count = 0;
}

/*!proto*/
void updateLRFU( cachePT cacheP, int hitColumn)
/*!endproto*/
{
    double crf                                         = cacheP->tagStore[cacheP->index][hitColumn].crf;
    int    gc                                          = cacheP->globalCount;
    int    lastRef                                     = cacheP->tagStore[cacheP->index][hitColumn].count;

    cacheP->tagStore[cacheP->index][hitColumn].crf     = 1.0 + F(gc - lastRef)*crf;
    cacheP->tagStore[cacheP->index][hitColumn].count   = gc;
}

/*!proto*/
unsigned int addressDecoder(cachePT cacheP, unsigned int tag)
/*!endproto*/
{
   int indexBits      = LOG_2(cacheP->rows); 
   int offsetBits     = LOG_2(cacheP->bSize);
   return (tag << (indexBits + offsetBits)) | (cacheP->index << (offsetBits));
}


/*!proto*/
// 7. cacheMiss ( )
//    Check for victim blocks based on the valid bit
//    replace / evict based on the replacement (LRU) policy
//    FUnc is called IFF victim was also a miss
int cacheMiss(cachePT cacheP, int address) 
/*!endproto*/
{
   bool emptyFromI    = false;
   // Search for an empty place
   int empty = -1;
   for(int j = 0; j < cacheP->assoc; j++) {
      if(cacheP->tagStore[cacheP->index][j].validBit == 0) {
         empty      = j;
         emptyFromI = true;
         break;
      }
   }
   
   // If an empty block is not found, search using replacement policy
   if(empty == -1) 
      // Search for victim block for LRU and LFU Policy
      empty = getEviction(cacheP);

   //------------------- EVICTION BEGIN ------------
   unsigned int evictedAddress = addressDecoder(cacheP, cacheP->tagStore[cacheP->index][empty].tag); 
   // Check if dirty. If yes, do a writeback
   if(cacheP->tagStore[cacheP->index][empty].dirtyBit == 1 && cacheP->victimCache == NULL) {
      write(cacheP->nextLevelCache, evictedAddress);
      cacheP->writeBacks++;
   }
   if( !emptyFromI && cacheP->victimCache != NULL ){
      cache2Victim(cacheP->victimCache, evictedAddress, cacheP->tagStore[cacheP->index][empty].dirtyBit );
   }
   cacheP->tagStore[cacheP->index][empty].dirtyBit = 0; 
   //------------------- EVICTION END --------------

   //------------------- ALLOCATION BEGIN ----------
   // Do a read request
   if(cacheP->victimCache != NULL) 
      read(cacheP->victimCache->nextLevelCache, address);
   else if(!cacheP->victimBit)
      read(cacheP->nextLevelCache, address);
   //------------------- ALLOCATION END ------------

   // Cache data at evicted/empty place
   cacheP->tagStore[cacheP->index][empty].tag = cacheP->tag;
   cacheP->tagStore[cacheP->index][empty].validBit = 1;
   // Update LFU and LRU of that block
   updateCMiss(cacheP, empty);
   return empty;
}

#define GET_COL(j) cacheP->tagStore[cacheP->index][j]

/*!proto*/
void updateCMiss(cachePT cacheP, int empty)
/*!endproto*/
{
   if(cacheP->replacePolicy == LRU)
      updateLRU(cacheP, empty);
   else if(cacheP->replacePolicy == LFU)
      updateLFU(cacheP, empty);
   else {
      GET_COL(empty).crf   = 1.0;
      GET_COL(empty).count = cacheP->globalCount;
   }
}

/*!proto*/
int getEviction(cachePT cacheP)
/*!endproto*/
{
   if(cacheP->replacePolicy == LRU)
      return getEvictionLRU(cacheP);
   else if(cacheP->replacePolicy == LFU)
      return getEvictionLFU(cacheP); 
   else
      return getEvictionLRFU(cacheP);
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
int getEvictionLRFU(cachePT cacheP)
/*!endproto*/
{
   double min   = F(cacheP->globalCount - GET_COL(0).count)*GET_COL(0).crf;
   int empty = 0; 
   for(int j = 1; j < cacheP->assoc; j++) {
      double crf = F(cacheP->globalCount - GET_COL(j).count)*GET_COL(j).crf;
      if(min > crf){
         min = crf;
         empty = j;
      }
   } 
   return empty;
}


/*!proto*/
void printTagstore (cachePT cacheP, char* name)
/*!endproto*/
{
   if( cacheP == NULL ) return;

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
                  int*        memTraffic,
                  int*        swaps )
   
/*!endproto*/
{
   if( cacheP == NULL ) {
      *reads              = 0; 
      *writes             = 0; 
      *readMisses         = 0;
      *writeMisses        = 0;
      *writeBacks         = 0;
      *missRate           = 0; 
      *swaps              = 0; 
      *memTraffic         = 0;
      return;
   }


   *reads              = cacheP->reads;
   *writes             = cacheP->writes;
   *readMisses         = cacheP->readMisses;
   *writeMisses        = cacheP->writeMisses;
   *writeBacks         = cacheP->writeBacks;
   if( *reads + *writes == 0 )
      *missRate        = 0;
   else
      *missRate        = (float)(*readMisses + *writeMisses)/(float)(*reads + *writes);
   *swaps              = cacheP->swaps;

   if(cacheP->writePolicy == WBWA)
      *memTraffic      = *writeBacks + *readMisses + *writeMisses;
   else
      *memTraffic      = *readMisses + *writes;
}

/*!proto*/
float getAAT ( cachePT cacheP )
/*!endproto*/
{
   if( cacheP == NULL ) return -1;

   float missRate    = (float)(cacheP->readMisses + cacheP->writeMisses)/(float)(cacheP->reads + cacheP->writes);
   float missPenalty;

   if(cacheP->victimCache == NULL)
      missPenalty = (cacheP->nextLevelCache) ? getAAT ( cacheP->nextLevelCache ) : cacheP->missPenalty;
   else
      missPenalty = (cacheP->victimCache->nextLevelCache) ? getAAT ( cacheP->victimCache->nextLevelCache ) : cacheP->missPenalty;

   return cacheP->hitTime + ( missRate ) * missPenalty;
}

/*!proto*/
// To check if it's a hit or a miss in the victim
bool victimHitMiss (cachePT cacheP, int address, int* vicHitCol) 
/*!endproto*/
{ 
   bool cond;
   getIndexAndTag(cacheP, address);
   // remember to pass ONLY THE ADDRESS of a pointer when there is a function-ception
   cond  = searchTagStore(cacheP, vicHitCol);
   return cond;
}

/*!proto*/
int cacheSwap (cachePT cacheP, int address, int vicHitCol)
/*!endproto*/
{
   cacheP->swaps++;
   int col                  = getEviction(cacheP);
   unsigned int addressL1   = addressDecoder (cacheP, cacheP->tagStore[cacheP->index][col].tag);

   //------------------ SWAP TAGS BEGIN --------------------------
   //                                                                            = new address tag
   cacheP->tagStore[cacheP->index][col].tag                                      = cacheP->tag;
   
   getIndexAndTag(cacheP->victimCache, addressL1);
   cacheP->victimCache->tagStore[cacheP->victimCache->index][vicHitCol].tag      = cacheP->victimCache->tag;
   //------------------ SWAP TAGS END ----------------------------

   //------------------ SWAP DIRTY BIT BEGIN ---------------------
   int l1Dirty        = cacheP->tagStore[cacheP->index][col].dirtyBit;
   int victimDirty    = cacheP->victimCache->tagStore[cacheP->victimCache->index][vicHitCol].dirtyBit;
   cacheP->tagStore[cacheP->index][col].dirtyBit                                 = victimDirty;
   cacheP->victimCache->tagStore[cacheP->victimCache->index][vicHitCol].dirtyBit = l1Dirty;
   //------------------ SWAP DIRTY BIT END -----------------------

   updateCMiss(cacheP->victimCache, vicHitCol);
   updateCMiss(cacheP, col);
   return col;
}

/*!proto*/
void cache2Victim (cachePT cacheP, unsigned int address, int dirty)
/*!endproto*/
{
   getIndexAndTag(cacheP, address);
   int col                                       = cacheMiss(cacheP, address);
   cacheP->tagStore[cacheP->index][col].dirtyBit = dirty;
}

/*!proto*/
void connectVictim(cachePT cacheLP, cachePT cacheVictimP)
/*!endproto*/
{
   if(cacheVictimP != NULL) {
      cacheLP->victimCache = cacheVictimP;
      cacheVictimP->victimBit = 1;
   }
}

/*!proto*/
void connectL(cachePT cacheLP, cachePT cacheLNextP)
/*!endproto*/
{
   if(cacheLP->victimCache == NULL)
      cacheLP->nextLevelCache = cacheLNextP;
   else
      cacheLP->victimCache->nextLevelCache = cacheLNextP;
}


