#include "cache.h"


int main ( int argc, char* argv[] ) {

   int b  = atoi(argv[1]);
   int c  = atoi(argv[2]);
   int s  = atoi(argv[3]);
   int rp = atoi(argv[4]);
   int wp = atoi(argv[5]);

   FILE *trace;
   char rw;
   int  address;

   cachePT cacheP = cacheAllocate(c, b, s, wp, rp);

   trace = fopen(argv[6], "r");
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
   printTagstore(cacheP);
   // print simulation config := by main.c
   // print cache contents : c
   // print sim results (raw) c/m
   // print sim results (perf) c/m
}

