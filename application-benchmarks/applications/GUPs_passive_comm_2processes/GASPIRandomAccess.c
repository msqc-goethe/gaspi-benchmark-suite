/* -*- mode: C; tab-width: 2; indent-tabs-mode: nil; -*- */

/*
 * This code has been contributed by the DARPA HPCS program.  Contact
 * David Koester <dkoester@mitre.org> or Bob Lucas <rflucas@isi.edu>
 * if you have questions.
 *
 *
 * GUPS (Giga UPdates per Second) is a measurement that profiles the memory
 * architecture of a system and is a measure of performance similar to MFLOPS.
 * The HPCS HPCchallenge RandomAccess benchmark is intended to exercise the
 * GUPS capability of a system, much like the LINPACK benchmark is intended to
 * exercise the MFLOPS capability of a computer.  In each case, we would
 * expect these benchmarks to achieve close to the "peak" capability of the
 * memory system. The extent of the similarities between RandomAccess and
 * LINPACK are limited to both benchmarks attempting to calculate a peak system
 * capability.
 *
 * GUPS is calculated by identifying the number of memory locations that can be
 * randomly updated in one second, divided by 1 billion (1e9). The term "randomly"
 * means that there is little relationship between one address to be updated and
 * the next, except that they occur in the space of one half the total system
 * memory.  An update is a read-modify-write operation on a table of 64-bit words.
 * An address is generated, the value at that address read from memory, modified
 * by an integer operation (add, and, or, xor) with a literal value, and that
 * new value is written back to memory.
 *
 * We are interested in knowing the GUPS performance of both entire systems and
 * system subcomponents --- e.g., the GUPS rating of a distributed memory
 * multiprocessor the GUPS rating of an SMP node, and the GUPS rating of a
 * single processor.  While there is typically a scaling of FLOPS with processor
 * count, a similar phenomenon may not always occur for GUPS.
 ________________________________________________________________________
  This file is adapted by Diana Waschbuesch, Heidelberg University. 
 *
 */
#include <sched.h>
//#include <hpcc.h>
#include <stdio.h>
#include <stdlib.h>
#include "RandomAccess.h"
#include "comm_util.h"
#ifdef USE_MPI3
  MPI_Info winfo;
#endif


int main(int argc, char **argv)
{
  //printf("\nStart Program\n");
  u64Int *HPCC_Table; /* Allocate main table */
  s64Int i;
  int logNumProcs;
  s64Int LocalTableSize;    /* Local table width */
  u64Int MinLocalTableSize; /* Integer ratio TableSize/NumProcs */
  u64Int logTableSize, TableSize;

  double CPUTime;               /* CPU  time to update table */
  double RealTime;              /* Real time to update table */

  double TotalMem;
  int PowerofTwo;

  u64Int NumUpdates_Default; /* Number of updates to table (suggested: 4x number of table entries) */
  u64Int NumUpdates;  /* actual number of updates to table - may be smaller than
                       * NumUpdates_Default due to execution time bounds */
  s64Int ProcNumUpdates; /* number of updates per processor */
#ifdef RA_TIME_BOUND
  s64Int GlbNumUpdates;  /* for reduction */
#endif
  FILE *outFile = NULL;
  double *GUPs;
  double *temp_GUPs;
  int numthreads;
  
#ifdef USE_MPI3 
  int iProc,NumProcs,oProc;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &NumProcs);
  MPI_Comm_rank(MPI_COMM_WORLD, &iProc);
  oProc = NumProcs-1-iProc;
  MPI_Info_create(&winfo);
  MPI_Info_set(winfo, "same_size", "true");
  MPI_Info_set(winfo, "alloc_shm", "false");
  window_size = 20000000;
  window_base = malloc(window_size);
  MPI_Win_create(window_base, window_size,  1, winfo, MPI_COMM_WORLD, &window);
  MPI_Win_lock_all(0, window);
  next_window_address = window_base;
  MPI_Info_free(&winfo);
  if(iProc == 0) printf("Running with MPI-3, world size is %d\n", NumProcs);
#else
#ifdef USE_GASPI
  //current rank, number of ranks
  gaspi_rank_t iProc, NumProcs;
  segment_size = 50000000;
  //init GASPIS
  SUCCESS_OR_DIE( gaspi_proc_init(GASPI_BLOCK) );
  //set current rank
  SUCCESS_OR_DIE( gaspi_proc_rank(&iProc));
  //get number of ranks
  SUCCESS_OR_DIE( gaspi_proc_num(&NumProcs));
  gaspi_rank_t oProc = NumProcs-1-iProc;
  //create Segment, only one Group, Memory is unitilized
  SUCCESS_OR_DIE ( gaspi_segment_create ( segment_id, segment_size , GASPI_GROUP_ALL, GASPI_BLOCK, GASPI_MEM_INITIALIZED ) );
  //initilaize array
  SUCCESS_OR_DIE( gaspi_segment_ptr (segment_id, &segment_base_ary) );
  segment_next_ary = segment_base_ary;
  if(iProc == 0)printf("Running with GASPI,rank %d world size is %d\n", iProc, NumProcs);
#else
  int iProc,NumProcs;
  shmem_init();
  NumProcs = shmem_n_pes();
  iProc = shmem_my_pe();
#endif
#endif
  /*Allocate symmetric memory*/
  malloc_all(sizeof(double), (void **)&GUPs);
  malloc_all(sizeof(double), (void **)&temp_GUPs);

  *GUPs = -1;

  if (0 == iProc) {
    outFile = stdout;
    setbuf(outFile, NULL);
  }

  TotalMem = 20000000; // max single node memory 
  TotalMem *= NumProcs;             // max memory in NumProcs nodes 

  TotalMem /= sizeof(u64Int);

  // calculate TableSize --- the size of update array (must be a power of 2) 
  for (TotalMem *= 0.5, logTableSize = 0, TableSize = 1;
       TotalMem >= 1.0;
       TotalMem *= 0.5, logTableSize++, TableSize <<= 1)
    ; // EMPTY 


  MinLocalTableSize = (TableSize / NumProcs);
  LocalTableSize = MinLocalTableSize;

  //Shmalloc HPCC_Table for RMA
  malloc_all(sizeof(u64Int)*LocalTableSize , (void **)&HPCC_Table);
  BARRIER_ALL();

  // Default number of global updates to table: 4x number of table entries 
  NumUpdates_Default = 4 * TableSize;
  ProcNumUpdates = 4*LocalTableSize;
  NumUpdates = NumUpdates_Default;

  if (iProc == 0) {
    fprintf( outFile, "Running on %d processors%s\n", NumProcs, PowerofTwo ? " (PowerofTwo)" : "");
    fprintf( outFile, "Total Main table size = 2^" FSTR64 " = " FSTR64 " words\n",logTableSize, TableSize );
    if (PowerofTwo)
        fprintf( outFile, "PE Main table size = 2^" FSTR64 " = " FSTR64 " words/PE\n",
                 (logTableSize - logNumProcs), TableSize/NumProcs );
      else
        fprintf( outFile, "PE Main table size = (2^" FSTR64 ")/%d  = " FSTR64 " words/PE MAX\n",
                 logTableSize, NumProcs, LocalTableSize);

    fprintf( outFile, "Default number of updates (RECOMMENDED) = " FSTR64 "\tand actually done = %d\n", NumUpdates_Default,ProcNumUpdates*NumProcs);
  }

  // Initialize main table 
  for (i=0; i<LocalTableSize; i++)
    HPCC_Table[i] = iProc;

  BARRIER_ALL();

  int iterate;
  int remote_proc;

  s64Int *updates;
  s64Int *ran;
  s64Int *ran_remote;

  malloc_all(sizeof(s64Int) , (void **)&ran);
  malloc_all(sizeof(s64Int) , (void **)&ran_remote);
  malloc_all(sizeof(s64Int) * NumProcs , (void **)&updates);// An array of length npes to avoid overwrites

  *ran = starts(4*(MinLocalTableSize * iProc));
  
  for (i = 0; i < NumProcs; i++){
    updates[i] = 0;
  }
  
  long long *remote_val;
  malloc_all(sizeof(long long) , (void **)&remote_val);
  BARRIER_ALL();
  // Begin timed section  

  RealTime = -RTSEC();
  ProcNumUpdates = 100;
  for (iterate = 0; iterate < ProcNumUpdates; iterate++)
  {
    if (iProc == 0)
    {
    //genereate next address
    *ran = (*ran << 1) ^ ((s64Int)*ran < ZERO64B ? POLY : ZERO64B);
    remote_proc = (*ran >> (logTableSize - logNumProcs)) & (NumProcs - 1);
    //send address
    PASSIVE_SEND(ran, oProc);
    //receive adress
    PASSIVE_RECV(ran_remote, oProc);
    //receive value
    PASSIVE_RECV(remote_val,oProc)
    //send value
    PASSIVE_SEND(&HPCC_Table[*ran_remote & (LocalTableSize - 1)],oProc)
    //update value
    *remote_val ^= *ran_remote;
    //send updated value
    PASSIVE_SEND(remote_val,oProc)
    //receive updated value
    PASSIVE_RECV(remote_val,oProc)
    }
    else
    {
    //genereate next address
    *ran = (*ran << 1) ^ ((s64Int)*ran < ZERO64B ? POLY : ZERO64B);
    remote_proc = (*ran >> (logTableSize - logNumProcs)) & (NumProcs - 1);
    //receive adress
    PASSIVE_RECV(ran_remote, oProc);
    //send address
    PASSIVE_SEND(ran, oProc);
    //send value
    PASSIVE_SEND(&HPCC_Table[*ran_remote & (LocalTableSize - 1)],oProc)
    //receive value
    PASSIVE_RECV(remote_val,oProc)
    //update value
    *remote_val ^= *ran_remote;
    //receive updated value
    PASSIVE_RECV(remote_val,oProc)
    //send updated value
    PASSIVE_SEND(remote_val,oProc)
    }

  }
  BARRIER_ALL();
  // End timed section 
  RealTime += RTSEC();


 //  Print timing results 
  if (iProc == 0){
    *GUPs = 1e-9*NumUpdates / RealTime;
    fprintf( outFile, "Real time used = %.6f seconds\n", RealTime );
    fprintf( outFile, "%.9f Billion(10^9) Updates    per second [GUP/s]\n",
             *GUPs );
    fprintf( outFile, "%.9f Billion(10^9) Updates/PE per second [GUP/s]\n",
             *GUPs / NumProcs );
  }
 
  BARRIER_ALL();
  // End verification phase 


  FREE_ALL(updates);
  FREE_ALL(ran);
  BARRIER_ALL();

  // Deallocate memory (in reverse order of allocation which should
  //     help fragmentation) 

  failed_table:

  if (0 == iProc) if (outFile != stderr) fclose( outFile );

  BARRIER_ALL();

  FINALIZE();
  printf("\nFinished Program\n");
  return 0;
}

