#include "cache.h"

void tracify(cachePT cacheP, char* traceFile ) {
   char rw;
   int  address;
   FILE *trace;

   trace = fopen(traceFile, "r");
   do
   {
      fscanf(trace,"%c %x\n", &rw, &address);

      if((rw == 'r') || (rw == 'R'))
         read(cacheP, address);
      else
         write(cacheP, address);
   }
   while(!feof(trace));
   fclose(trace);
}

void printHeader(int b, int c, int s, int rp, int wp, char* traceFile, int c2, int s2, int vs) {
   printf(" ===== Simulator configuration =====\n");
   printf(" L1_BLOCKSIZE:                    %d\n", b);
   printf(" L1_SIZE:                         %d\n", c);
   printf(" L1_ASSOC:                        %d\n", s);
   printf(" Victim_Cache_SIZE:                 %d\n", vs);
   printf(" L2_SIZE:                         %d\n", c2);
   printf(" L2_ASSOC:                        %d\n", s2);
   printf(" trace_file:                      %s\n", traceFile);
   printf(" Replacement Policy:              %s\n", (rp == LFU) ? "LFU" : "LRU");
   printf(" ===================================\n\n");
}

void printFooter(cachePT cacheP) {
   int reads, writes, readMisses, writeMisses, writeBacks, memTraffic;
   float missRate, AAT;

   getResults(cacheP, &reads, &writes, &readMisses, &writeMisses, &missRate, &writeBacks, &memTraffic );


   printf("\n ====== Simulation results (raw) ======\n\n");
   printf(" a. number of L1 reads:          %d\n", reads);
   printf(" b. number of L1 read misses:    %d\n", readMisses);
   printf(" c. number of L1 writes:         %d\n", writes);
   printf(" d. number of L1 write misses:   %d\n", writeMisses);
   printf(" e. L1 miss rate:                %.4f\n", missRate );
   printf(" f. number of swaps:              \n");
   printf(" g. number of victim cache writeback:  \n");
   
   getResults(cacheP->nextLevelCache, &reads, &writes, &readMisses, &writeMisses, &missRate, &writeBacks, &memTraffic);

   printf(" h. number of L2 reads:          %d\n", reads);
   printf(" i. number of L2 read misses:    %d\n", readMisses);
   printf(" j. number of L2 writes:         %d\n", writes);
   printf(" k. number of L2 write misses:   %d\n", writeMisses);
   printf(" l. L2 miss rate:                %.4f\n", missRate );
   printf(" m. number of L2 writeback:      %d\n", writeBacks );
   printf(" n. total memory traffic:         %d\n", memTraffic);

   printf("\n ==== Simulation results (performance) ====\n");
   printf(" 1. average access time:         %.4f ns\n", getAAT(cacheP)); 
}




int main ( int argc, char* argv[] ) {

   int b    = atoi(argv[1]);
   int l1c  = atoi(argv[2]);
   int l1s  = atoi(argv[3]);
   int vs   = atoi(argv[4]);
   int l2c  = atoi(argv[5]);
   int l2s  = atoi(argv[6]);
   int rp   = atoi(argv[7]);


   // print simulation config := by main.c
   printHeader(b, l1c, l1s, rp, WBWA, argv[8], l2c, l2s, vs);

   petriDishT timeL1 = { 0.25, 2.5, 524288.0, 0.025, 16.0, 0.025, 20.0, 0.5, 16.0 };
   petriDishT timeL2 = { 2.5 , 2.5, 524288.0, 0.025, 16.0, 0.025, 20.0, 0.5, 16.0 };
   cachePT cacheP         = cacheAllocate(l1c, b, l1s, WBWA, rp, &timeL1);
   cacheP->nextLevelCache = cacheAllocate(l2c, b, l2s, WBWA, rp, &timeL2);

   tracify(cacheP, argv[8]);

   // print cache contents
   printTagstore(cacheP, "L1");
   printTagstore(cacheP->nextLevelCache, "L2");

   // print sim results (raw) c/m
   printFooter(cacheP);

   // print sim results (perf) c/m
}

