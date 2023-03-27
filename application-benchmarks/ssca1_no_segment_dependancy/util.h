/*
   This file is part of SSCA1.

   Copyright (C) 2008-2015, UT-Battelle, LLC.

   This product includes software produced by UT-Battelle, LLC under Contract No.
   DE-AC05-00OR22725 with the Department of Energy.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the New BSD 3-clause software license (LICENSE).

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   LICENSE for more details.

   For more information please contact the SSCA1 developers at:
   bakermb@ornl.gov
 ________________________________________________________________________
  This file is adapted by Diana Waschbuesch, Heidelberg University. 
*/

#ifndef __UTIL_H
#define __UTIL_H
#include "pairwise_align.h"
#include "types.h"
#include <stdio.h>
#include <unistd.h>
#ifdef USE_MPI3
#include <mpi.h>
extern MPI_Comm world;
extern MPI_Win window;
extern void *window_base;
extern size_t window_size;
extern void *next_window_address;
extern MPI_Request request;
#else
#ifdef SGI_SHMEM
#include <mpp/shmem.h>
#else
#ifdef USE_GASPI
#include <GASPI.h>
extern void *segment_base_ary; //first Element of GASPI Segment
extern void *segment_next_ary;
extern size_t segment_ary_size;
extern size_t segment_size;
extern int seg_buf;
extern int seg_buf_write;
#define segment_id 0
#define queue 0
#define timeout GASPI_BLOCK
#else
#include <shmem.h>
#endif
#endif
#endif


#ifdef USE_MPI3
#define QUIET()		MPI_Win_flush_all(window)
#else
#ifdef USE_GASPI
#define QUIET() gaspi_wait(queue, GASPI_BLOCK);
#else
#define QUIET()		shmem_quiet()
#endif
#endif

#ifdef USE_MPI3
#define SHORT_GET(target, source, num_elems, rank)	MPI_Get(target, num_elems, MPI_SHORT, rank, (void *)source - window_base, num_elems, MPI_SHORT, window); QUIET()
#else
#ifdef USE_GASPI
static inline void SHORT_GET(short *target, short *source, int num_elems, int iProc){
  /*printf("\nget mem Source: %ld\n", (void *)source - segment_base_ary);
  printf("\nget mem Target: %ld\n", ((void *)&(segment_base_ary[seg_buf])) - (void *)segment_base_ary);
  printf("\n Read Source %d  ", *source);
  printf("Target %d  ", *target);
  printf("rank %d  iProc %d", rank,iProc);*/
  gaspi_read(segment_id,
             ((void *)&(segment_base_ary[seg_buf])) - (void *)segment_base_ary,
             iProc,
             segment_id,
             ((void *)source - (void *)segment_base_ary),
             num_elems * sizeof(short),
             queue,
             timeout);
  //printf("\nFailure %d\n", myfailure);
  //cast bytes (segment_base array is an array of bytes) to uint16;
  QUIET();
  *target = ((char *)segment_base_ary)[seg_buf] 
              | (uint16_t)((char *)segment_base_ary)[seg_buf + 1] << 8;
  //printf("  Target %d after\n", *target);
  
};
#else
#define SHORT_GET(target, source, num_elems, pe)	shmem_short_get(target, source, num_elems, pe)
#endif
#endif

#ifdef USE_MPI3
#define SHORT_GET_NB(target, source, num_elems, rank)	MPI_Get(target, num_elems, MPI_SHORT, rank, (void *)source - window_base, num_elems, MPI_SHORT, window)
#else
#ifdef USE_GASPI
static inline void SHORT_GET_NB(short *target, short *source, int num_elems, int rank){
  gaspi_read(segment_id,
             ((void *)&(segment_base_ary[seg_buf])) - (void *)segment_base_ary,
             rank, 
             segment_id,
             ((void *)source - (void *)segment_base_ary),
             num_elems * sizeof(uint16_t),
             queue,
             timeout);
  //printf("\nFailure %d\n", myfailure);
  //cast bytes (segment_base array is an array of bytes) to uint16;
  QUIET();
  *target = ((char *)segment_base_ary)[seg_buf] 
              | (uint16_t)((char *)segment_base_ary)[seg_buf + 1] << 8;
};
#else
#define SHORT_GET_NB(target, source, num_elems, pe)	shmem_short_get_nbi(target, source, num_elems, pe)
#endif
#endif

#ifdef USE_MPI3
#define LONG_GET(target, source, num_elems, rank)	MPI_Get(target, num_elems, MPI_LONG, rank, (void *)source - window_base, num_elems, MPI_LONG, window); QUIET()
#else
#ifdef USE_GASPI
static inline void print_array(gaspi_pointer_t array, int size, int iProc){
    int i;
    printf("Rank:%d    ", iProc);
    for (i = 0; i < size; i++)

    {
        printf("%d ", ((char *)array)[i]);
    }
    printf("\n");
}
static inline void LONG_GET(index_t *target, index_t *source, int num_elems, int rank){
  /**source = 5;
  num_elems = 1;
  printf("\nget mem Source: %ld\n", (void *)source - segment_base_ary);
  printf("\nget mem Target: %ld\n", ((void *)&(segment_base_ary[0])) - (void *)segment_base_ary);
  printf("\n Source %d\n", *source);
  printf("\n Target %d\n", *target);
  printf("\n Size %d\n", num_elems);
  int myfailure =*/
  gaspi_read(segment_id,
             ((void *)&(segment_base_ary[seg_buf])) - (void *)segment_base_ary,
             rank,
             segment_id,
             (void *)source - (void *)segment_base_ary,
             num_elems * sizeof(index_t),
             queue,
             timeout);
  //printf("\nFailure %d\n", myfailure);
  QUIET();
  //cast bytes (segment_base array is an array of bytes) to uint16;
  *target = ((char *)segment_base_ary)[seg_buf] 
            | (uint64_t)((char *)segment_base_ary)[seg_buf + 1] << 8 
            | (uint64_t)((char *)segment_base_ary)[seg_buf + 2] << 16 
            | (uint64_t)((char *)segment_base_ary)[seg_buf + 3] << 24 
            | (uint64_t)((char *)segment_base_ary)[seg_buf + 4] << 32 
            | (uint64_t)((char *)segment_base_ary)[seg_buf + 5] << 40 
            | (uint64_t)((char *)segment_base_ary)[seg_buf + 6] << 48 
            | (uint64_t)((char *)segment_base_ary)[seg_buf + 7] << 56;
  
  //printf("\n Target %d after\n", *target);
};
#else
#define LONG_GET(target, source, num_elems, pe)		shmem_long_get((long*)target, (long*)source, num_elems, pe)
#endif
#endif

#ifdef USE_MPI3
#define GETMEM(target, source, length, rank)		MPI_Get(target, length, MPI_BYTE, rank, (void *)source - window_base, length, MPI_BYTE, window); QUIET()
#else
#ifdef USE_GASPI
static inline void print_mystruct(current_ends_t* str, int iProc){
    printf("Rank:%d    ", iProc);
    printf("%p ", (void*)str->goodEnds[0]);
    printf("%p ", (void*)str->goodEnds[1]);
    printf("%p ", (void*)str->goodScores);
    printf("%d ", str->report);
    printf("%d ", str->size);
    printf("%d ", str->min_score);
    printf("\n");
}

static inline void GETMEM(void *target, void *source, int length, int rank){
  /*printf("\nget mem Target: %ld\n", target - segment_base_ary);
  printf("\nget mem Source: %ld\n", source - segment_base_ary);*/
  /*printf("\nSource: ");
  print_mystruct(source, rank);
  printf("Target: ");
  print_mystruct(target,rank);*/
  //print_array(&segment_base_ary[37030], length, rank);
  //int failure_my = 
  gaspi_read(segment_id,
             target - segment_base_ary,
             rank,
             segment_id,
             source - segment_base_ary,
             length,
             queue,
             timeout);
  /*printf("\nFailure Code %d\n", failure_my);
  print_array(&segment_base_ary[37030], length, rank);*/
  //printf("After \nTarget: ");
  //print_mystruct(target, rank);
  QUIET();
};
#else
#define GETMEM(target, source, length, pe)		shmem_getmem(target, source, length, pe)
#endif
#endif

#ifdef USE_MPI3
#define SHORT_PUT(target, source, num_elems, rank)	MPI_Put(source, num_elems, MPI_SHORT, rank, (void *)target - window_base, num_elems, MPI_SHORT, window); QUIET()
#else
#ifdef USE_GASPI
static inline void SHORT_PUT(short *target, short *source, int num_elems, int iProc){
  //*source = 5;
  //num_elems = 1;
  //printf("\nget mem Source: %ld\n", (void *)source - segment_base_ary);
  //printf("get mem Target: %ld\n", ((void *)target - (void *)segment_base_ary));
  //printf("rank %d  ", rank);
 /* printf(" Source %d  ", *source);
   //int myfailure =
  printf("Target %d  ", *target);
  printf("Size %d  ", num_elems);*/
 /*printf("\n Writ Source %d  ", *source);
  printf("Target %d  ", *target);
  printf("rank %d  iProc %d", rank,iProc);*/

  ((unsigned char *)segment_base_ary)[seg_buf_write] = *source& 0xff;
  ((unsigned char *)segment_base_ary)[seg_buf_write+1] = (*source >> 8) & 0xff;
  gaspi_write(segment_id,
                               (&segment_base_ary[seg_buf_write] - (void *)segment_base_ary),
                               iProc,
                               segment_id,
                               (void*)target -segment_base_ary,
                               num_elems * sizeof(short),
                               queue,
                               timeout);
  QUIET();
  //printf("Target after %d\n", *target);
};

#else
#define SHORT_PUT(target, source, num_elems, pe)	shmem_short_put(target, source, num_elems, pe)
#endif
#endif



#ifdef USE_MPI3
#define BARRIER_ALL()	QUIET(); MPI_Barrier(MPI_COMM_WORLD)
#else
#ifdef USE_GASPI
#define BARRIER_ALL() QUIET(); gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK)
#else
#define BARRIER_ALL()	shmem_barrier_all()
#endif
#endif

#ifdef USE_MPI3
static inline int malloc_all(size_t size, void **address) {
  *address = next_window_address;
  next_window_address += size;
  MPI_Barrier(MPI_COMM_WORLD);
  if (next_window_address - window_base > window_size) {
    printf("ran out of memory!\n");
    return -1;
  } else
    return 0;
}
#else
#ifdef USE_GASPI
static inline int malloc_all(size_t size, void **address) {
  *address = segment_next_ary;
  segment_next_ary += size;
  BARRIER_ALL();
  if (segment_next_ary - segment_base_ary > segment_size)
  {
    printf("ran out of memory!\n");
    return -1;
  }
  else
    return 0;
}
#else
static inline int malloc_all(size_t size, void **address) {
  *address = shmalloc(size);
  if (*address == NULL)
    return -1;
  else
    return 0;
}
#endif
#endif

#ifdef USE_MPI3
#define FREE_ALL(address) /* unable to free memory like this */
#else
#ifdef USE_GASPI
#define FREE_ALL(address)/* unable to free memory like this */
#else
#define FREE_ALL(address) shfree(address)
#endif
#endif

static inline int global_index_to_rank(const seq_t *in, const index_t codon_index){
  return codon_index / in->local_size;
}

static inline int global_index_to_local_index(const seq_t *in, const index_t codon_index){
  return codon_index % in->local_size;
}


static inline void fetch_from_seq(const seq_t *in, index_t const codon_index, codon_t *out){
  int target_ep = global_index_to_rank(in,codon_index);
  int local_index = global_index_to_local_index(in,codon_index);
  short *typed_seq = (short *)in->sequence;
  //printf("\nRead rank:%d target r:%d localIndex:%d Offset : %d", rank, target_ep, local_index,(void*)&(typed_seq[local_index])-segment_base_ary);
  SHORT_GET((short *)out, &(typed_seq[local_index]), 1, target_ep);
}


static inline void fetch_from_seq_nb(const seq_t *in, index_t const codon_index, codon_t *out){
  int target_ep = global_index_to_rank(in,codon_index);
  int local_index = global_index_to_local_index(in,codon_index);
  short *typed_seq = (short *)in->sequence;
  SHORT_GET_NB((short *)out, &(typed_seq[local_index]), 1, target_ep);
}

static inline void write_to_seq(const seq_t *in, const index_t codon_index, codon_t data){
  int target_ep = global_index_to_rank(in,codon_index);
  int local_index = global_index_to_local_index(in,codon_index);
  short *typed_seq = (short *)in->sequence;
  short typed_data = (short)data;
  //printf("\nWrit rank:%d target r:%d localIndex:%d Offset : %d", rank, target_ep, local_index,(void*)&(typed_seq[local_index])-segment_base_ary);
  SHORT_PUT(&(typed_seq[local_index]), (short *)&typed_data, 1, target_ep);
}
#ifdef USE_MPI3
#define WAIT_NB() QUIET()
#else
#ifdef USE_GASPI
#define WAIT_NB() QUIET()
#else
#define WAIT_NB() QUIET()
#endif
#endif

void distribute_rng_seed(unsigned int new_seed);
void seed_rng(int adjustment);
void touch_memory(void *mem, index_t size);
index_t scrub_hyphens(good_match_t *A, seq_t *dest, seq_t *source, index_t length);
void assemble_acid_chain(good_match_t *A, char *result, seq_t *chain, index_t length);
void assemble_codon_chain(good_match_t *A, char *result, seq_t *chain, index_t length);
score_t simple_score(good_match_t *A, seq_t *main, seq_t *match);
seq_t *alloc_global_seq(index_t seq_size);
seq_t *alloc_local_seq(index_t seq_size);
void free_global_seq(seq_t *doomed);
void free_local_seq(seq_t *doomed);

#endif
