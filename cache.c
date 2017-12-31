#include "cache.h"

#define LOG_2(A) ceil( log(A)/log(2) )

//-------------------------POSSIBLE FUNCTIONS----------------------------------//
//1. cacheAllocate(cSize, bSize, assoc)
//   Function allocates an array to of the appropriate sizes
/*!proto*/
cachePT cacheAllocate(int c, int b, int s, int wp, int rp ) 
/*!endproto*/
{
   // Obtain the number of rows from the given parameters

   cachePT cacheP          = (cachePT)calloc(1, sizeof(cacheT));
   cacheP->cSize           = c;
   cacheP->bSize           = b;
   cacheP->assoc           = s;
   cacheP->writePolicy     = wp;
   cacheP->replacePolicy   = rp;
   cacheP->rows            = cacheP->cSize/(cacheP->bSize*cacheP->assoc);
   // Allocate memory for the cache as a 2D- array
   cacheP->tagStore       = (blockPT*)calloc(cacheP->rows, sizeof(blockPT));
   for(int i = 0; i < cacheP->rows; i++) {
      cacheP->tagStore[i] = (blockPT)calloc(cacheP->assoc, sizeof(blockT));
      for(int j = 0; j < cacheP->assoc; j++) {
         cacheP->tagStore[i][j].count = j;
      }
   }
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
   int hitIndex;
   getIndexAndTag(cacheP, address);
   bool cond = searchTagStore(cacheP, &hitIndex);
   if(!cond) { // MISS
      cacheMiss(cacheP);
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
   int hitIndex;
   getIndexAndTag(cacheP, address);
   bool cond = searchTagStore(cacheP, &hitIndex);
   if(!cond) { // MISS
      if(cacheP->writePolicy == WBWA) {
        int dirtyAt = cacheMiss(cacheP);
        cacheP->tagStore[cacheP->index][dirtyAt].dirtyBit = 1;
      }
   }
   else { // HIT
      updateCounters(cacheP, hitIndex);
      if(cacheP->writePolicy == WBWA) {
         cacheP->tagStore[cacheP->index][hitIndex].dirtyBit = 1;
      }
   }
   //if(cacheP->writePolicy == WTNA) 
   //FIXME: write (nextpointer)
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
      if(cacheP->tagStore[cacheP->index][j].tag == cacheP->tag) {
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
   updateLRU( cacheP, hitIndex );
}

void updateLFU(cachePT cacheP, int hitIndex)
{
   //
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
int cacheMiss(cachePT cacheP) 
/*!endproto*/
{
   // Search for an empty place
   int empty = -1;
   for(int j = 0; j < cacheP->assoc; j++) {
      if(cacheP->tagStore[cacheP->index][j].validBit == 0) {
         empty = j;
         break;
      }
   }
   // Search for victim block
   if(empty == -1) {
      for(int j = 0; j < cacheP->assoc; j++) {
         if(cacheP->tagStore[cacheP->index][j].count == cacheP->assoc-1) {
            empty = j;
            break;
         }
      } 
   }
   // Do a read request
   // read(cacheP, next-level-address);
   // Check if dirty. If yes, do a writeback
   if(cacheP->tagStore[cacheP->index][empty].dirtyBit == 1) {
      //write(cacheP, next-level-address);
      cacheP->tagStore[cacheP->index][empty].dirtyBit = 0; 
   }
   // Cache data at evicted/empty place
   cacheP->tagStore[cacheP->index][empty].tag = cacheP->tag;
   cacheP->tagStore[cacheP->index][empty].validBit = 1;
   // Update LRU of that block
   updateCounters(cacheP, empty);
   return empty;
}

/*!proto*/
void printTagstore (cachePT cacheP)
/*!endproto*/
{
   for(int i = 0; i < cacheP->rows; i++) {
      printf("set %d: ", i);
      for(int j = 0; j < cacheP->assoc; j++) {
         printf("%x %c ", cacheP->tagStore[i][j].tag, cacheP->tagStore[i][j].dirtyBit ? 'D' : ' ');   
      }
      printf("\n");
   }  
}
