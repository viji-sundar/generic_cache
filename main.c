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

void printHeader(int b, int c, int s, int rp, int wp, char* traceFile) {
   printf("===== Simulator configuration =====\n");
   printf("L1_BLOCKSIZE:                    %d\n", b);
   printf("L1_SIZE:                         %d\n", c);
   printf("L1_ASSOC:                        %d\n", s);
   printf("L1_REPLACEMENT_POLICY:           %d\n", rp);
   printf("L1_WRITE_POLICY:                 %d\n", wp);
   printf("trace_file:                      %s\n", traceFile);
   printf("===================================\n\n");
}

void printFooter(cachePT cacheP) {
   int reads, writes, readMisses, writeMisses, writeBacks, memTraffic;
   float missRate, AAT;

   getResults(cacheP, &reads, &writes, &readMisses, &writeMisses, &missRate, &writeBacks, &memTraffic, &AAT);


   printf("\n====== Simulation results (raw) ======\n");
   printf("a. number of L1 reads:          %d\n", reads);
   printf("b. number of L1 read misses:    %d\n", readMisses);
   printf("c. number of L1 writes:         %d\n", writes);
   printf("d. number of L1 write misses:   %d\n", writeMisses);
   printf("e. L1 miss rate:                %.4f\n", missRate );
   printf("f. number of writebacks from L1: %d\n", writeBacks);
   printf("g. total memory traffic:         %d\n", memTraffic);

   printf("\n==== Simulation results (performance) ====\n");
   printf("1. average access time:         %.4f ns\n", AAT); 
}


int main ( int argc, char* argv[] ) {

   int b  = atoi(argv[1]);
   int c  = atoi(argv[2]);
   int s  = atoi(argv[3]);
   int rp = atoi(argv[4]);
   int wp = atoi(argv[5]);

   // print simulation config := by main.c
   printHeader(b, c, s, rp, wp, argv[6]);

   cachePT cacheP = cacheAllocate(c, b, s, wp, rp);
   
   tracify(cacheP, argv[6]);

   // print cache contents
   printTagstore(cacheP);
   // print sim results (raw) c/m
   printFooter(cacheP);

   // print sim results (perf) c/m
}

