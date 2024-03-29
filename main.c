#include "cache.h"

void tracify(cachePT cacheP, char* traceFile ) {
   char rw;
   int  address;
   FILE *trace;

   trace = fopen(traceFile, "r");

   do{
      fscanf(trace,"%c %x\n", &rw, &address);

      if((rw == 'r') || (rw == 'R'))
         read(cacheP, address);
      else
         write(cacheP, address);

   } while(!feof(trace));
   fclose(trace);
}

void printHeader(int b, int c, int s, double lambda, char* traceFile, int c2, int s2, int vs) {
   printf(" ===== Simulator configuration =====\n");
   printf(" L1_BLOCKSIZE:                    %d\n", b);
   printf(" L1_SIZE:                         %d\n", c);
   printf(" L1_ASSOC:                        %d\n", s);
   printf(" Victim_Cache_SIZE:               %d\n", vs);
   printf(" L2_SIZE:                         %d\n", c2);
   printf(" L2_ASSOC:                        %d\n", s2);
   printf(" trace_file:                      %s\n", traceFile);
   printf(" Replacement Policy:              %s\n", (lambda == LFU) ? "LFU" : (lambda == LRU ? "LRU" : "LRFU"));
   if( lambda <= 1 )
      printf(" lambda:                     %.2f\n", lambda);
   printf(" ===================================\n\n");
}

void printFooter(cachePT cacheL1P, cachePT cacheVictimP, cachePT cacheL2P) {
   int   reads, writes, readMisses, writeMisses, writeBacks, memTraffic1, memTraffic2, memTrafficV, swaps;
   float missRate, AAT;

   getResults(cacheL1P, &reads, &writes, &readMisses, &writeMisses, &missRate, &writeBacks, &memTraffic1, &swaps );
   int l1Miss = readMisses + writeMisses;
   printf("\n ====== Simulation results (raw) ======\n\n");
   printf(" a. number of L1 reads:          %d\n"     , reads);
   printf(" b. number of L1 read misses:    %d\n"     , readMisses);
   printf(" c. number of L1 writes:         %d\n"     , writes);
   printf(" d. number of L1 write misses:   %d\n"     , writeMisses);
   printf(" e. L1 miss rate:                %.4f\n"   , missRate );
   printf(" f. number of swaps:             %d\n"     , swaps);

   getResults(cacheVictimP, &reads, &writes, &readMisses, &writeMisses, &missRate, &writeBacks, &memTrafficV, &swaps );

   printf(" g. number of victim cache writeback: %d\n", writeBacks);

   getResults(cacheL2P, &reads, &writes, &readMisses, &writeMisses, &missRate, &writeBacks, &memTraffic2, &swaps );
   printf(" h. number of L2 reads:          %d\n"  , reads);
   printf(" i. number of L2 read misses:    %d\n"  , readMisses);
   printf(" j. number of L2 writes:         %d\n"  , writes);
   printf(" k. number of L2 write misses:   %d\n"  , writeMisses);
   printf(" l. L2 miss rate:                %.4f\n", missRate );
   printf(" m. number of L2 writeback:      %d\n"  , writeBacks );
   printf(" n. total memory traffic:        %d\n"  , cacheL2P == NULL ? (cacheVictimP == NULL ? memTraffic1: memTrafficV + l1Miss) : memTraffic2);

   printf("\n ==== Simulation results (performance) ====\n");
   printf(" 1. average access time:         %.4f ns\n", getAAT(cacheL1P)); 
}

int main ( int argc, char* argv[] ) {
   int    b               = atoi(argv[1]);
   int    l1c             = atoi(argv[2]);
   int    l1s             = atoi(argv[3]);
   int    vs              = atoi(argv[4]);
   int    l2c             = atoi(argv[5]);
   int    l2s             = atoi(argv[6]);
   double rp              = atof(argv[7]);
   int    tNo             = 8;

   // print simulation config
   printHeader(b, l1c, l1s, rp, argv[tNo], l2c, l2s, vs);
   
   //                            a,   b,        c,     d,    e,     f,    g,   h,    i
   petriDishT timeL1      = { 0.25, 2.5, 524288.0, 0.025, 16.0, 0.025, 20.0, 0.5, 16.0 };
   petriDishT timeL2      = { 2.5 , 2.5, 524288.0, 0.025, 16.0, 0.025, 20.0, 0.5, 16.0 };

   cachePT cacheL1P       = cacheAllocate(l1c, b, l1s, WBWA, rp, &timeL1);
   cachePT cacheVictimP   = cacheAllocate(vs,  b, vs/b, WBWA, LRU, &timeL1); 
   cachePT cacheL2P       = cacheAllocate(l2c, b, l2s, WBWA, LRU, &timeL2);

   connectVictim( cacheL1P, cacheVictimP );
   connectL( cacheL1P, cacheL2P );

   tracify(cacheL1P, argv[tNo]);

   // print cache contents
   printTagstore(cacheL1P    , "L1");
   printTagstore(cacheVictimP, "Victim Cache");
   printTagstore(cacheL2P    , "L2");

   // print sim results (raw)
   // print sim results (perf)
   printFooter(cacheL1P, cacheVictimP, cacheL2P);
}

