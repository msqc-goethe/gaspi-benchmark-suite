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
#define USE_PREFETCH 1
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
#define Request_Queue_Num 100
extern int request_num[64];
extern MPI_Request request[Request_Queue_Num*64];
extern MPI_Request request_block[64];
#else
#ifdef SGI_SHMEM
#include <mpp/shmem.h>
#else
#ifdef USE_GASPI
#include <GASPI.h>
extern void *segment_base_ary; //first Element of GASPI Segment
extern void *segment_next_ary;
extern size_t segment_ary_size;
extern gaspi_notification_t noti_num;
extern size_t segment_size;
extern int seg_buf;
extern int seg_buf_write;
#define Queue_num 8
extern gaspi_queue_id_t queues[Queue_num];
#define segment_id 0
#define queue 0
#define timeout GASPI_BLOCK
#else
#include <shmem.h>
#endif
#endif
#endif
#ifdef _OPENMP
#include <omp.h>
#endif
#ifdef USE_MPI3
#define QUIET()	MPI_Win_flush_local_all(window)
#else
#ifdef USE_GASPI
#define QUIET() gaspi_wait(queues[omp_get_thread_num()], GASPI_BLOCK);
#else
#define QUIET()		shmem_quiet()
#endif
#endif

#ifdef USE_MPI3
#define SHORT_GET(target, source, num_elems, rank) MPI_Get(target, num_elems, MPI_SHORT, rank, (void *)source - window_base, num_elems, MPI_SHORT, window);QUIET()
//#define SHORT_GET(target, source, num_elems, rank)	 MPI_Rget(target, num_elems, MPI_SHORT, rank,(void *)source - window_base, num_elems, MPI_SHORT, window, &request_block[omp_get_thread_num()]);MPI_Wait(&request_block[omp_get_thread_num()],MPI_STATUS_IGNORE)
#else
#ifdef USE_GASPI
#define SHORT_GET(target, source, num_elems, rank)	gaspi_read(segment_id,((void *)target - (void *)segment_base_ary),rank,segment_id,((void *)source - (void *)segment_base_ary),num_elems * sizeof(short), queues[omp_get_thread_num()],timeout);QUIET()
#else
#define SHORT_GET(target, source, num_elems, pe)	shmem_short_get(target, source, num_elems, pe)
#endif
#endif

#ifdef USE_MPI3
//#define SHORT_GET_NB(target, source, num_elems, rank)	 MPI_Rget(target, num_elems, MPI_SHORT, rank,(void *)source - window_base, num_elems, MPI_SHORT, window, &request_block[omp_get_thread_num()]);MPI_Wait(&request_block[omp_get_thread_num()],MPI_STATUS_IGNORE)

#define SHORT_GET_NB(target, source, num_elems, rank)	MPI_Get(target, num_elems, MPI_SHORT, rank, (void *)source - window_base, num_elems, MPI_SHORT, window)
static inline void SHORT_GET_NB_(short* target,short* source,int num_elems,int rank){
MPI_Rget(target, num_elems, MPI_SHORT, rank,
          (void *)source - window_base, num_elems, MPI_SHORT, window, &request[omp_get_thread_num()*Request_Queue_Num+request_num[omp_get_thread_num()]]);
request_num[omp_get_thread_num()]++;
}
#else
#ifdef USE_GASPI
#define SHORT_GET_NB(target, source, num_elems, rank) gaspi_read(segment_id, ((void *)target - (void *)segment_base_ary), rank, segment_id, ((void *)source - (void *)segment_base_ary), num_elems * sizeof(uint16_t), queues[omp_get_thread_num()], timeout);
#else
#define SHORT_GET_NB(target, source, num_elems, pe)	shmem_short_get_nbi(target, source, num_elems, pe)
#endif
#endif

#ifdef USE_MPI3
#define LONG_GET(target, source, num_elems, rank)	MPI_Get(target, num_elems, MPI_LONG, rank, (void *)source - window_base, num_elems, MPI_LONG, window);QUIET()
#else
#ifdef USE_GASPI
#define LONG_GET(target, source, num_elems, rank)	gaspi_read(segment_id, (void *)target - (void *)segment_base_ary, rank, segment_id, (void *)source - (void *)segment_base_ary, num_elems * sizeof(index_t),queues[omp_get_thread_num()], timeout); QUIET()
#else
#define LONG_GET(target, source, num_elems, pe)		shmem_long_get((long*)target, (long*)source, num_elems, pe)
#endif
#endif

#ifdef USE_MPI3
#define GETMEM(target, source, length, rank)		MPI_Get(target, length, MPI_BYTE, rank, (void *)source - window_base, length, MPI_BYTE, window);QUIET()
#else
#ifdef USE_GASPI
#define GETMEM(target, source, length, rank)		gaspi_read(segment_id, (void *)target - segment_base_ary, rank,segment_id,(void *)source - segment_base_ary, length, queues[omp_get_thread_num()], timeout); QUIET()
#else
#define GETMEM(target, source, length, pe)		shmem_getmem(target, source, length, pe)
#endif
#endif

#ifdef USE_MPI3
#define SHORT_PUT(target, source, num_elems, rank) MPI_Put(source, num_elems, MPI_SHORT, rank, (void *)target - window_base, num_elems, MPI_SHORT, window);QUIET()
#else
#ifdef USE_GASPI
#define SHORT_PUT(target, source, num_elems, rank)	gaspi_write(segment_id, (void*)source -segment_base_ary, rank, segment_id, (void*)target -segment_base_ary, num_elems * sizeof(short), queues[omp_get_thread_num()], timeout); QUIET()
#else
#define SHORT_PUT(target, source, num_elems, pe)	shmem_short_put(target, source, num_elems, pe)
#endif
#endif

#ifdef USE_MPI3
#define SHORT_PUT_NB(target, source, num_elems, rank) 	MPI_Put(source, num_elems, MPI_SHORT, rank, (void *)target - window_base, num_elems, MPI_SHORT, window); 
#else
#ifdef USE_GASPI
#define SHORT_PUT_NB(target, source, num_elems, rank)	gaspi_write(segment_id, (void*)source -segment_base_ary, rank, segment_id, (void*)target -segment_base_ary, num_elems * sizeof(short), queues[omp_get_thread_num()], timeout);
#else
#define SHORT_PUT_NB(target, source, num_elems, pe)	SHORT_PUT(target, source, num_elems, pe)
#endif
#endif

#ifdef USE_MPI3
#define BARRIER_ALL()	MPI_Win_flush_local_all(window); MPI_Barrier(MPI_COMM_WORLD)
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
  SHORT_GET((short *)out, &(typed_seq[local_index]), 1, target_ep);
}


static inline void fetch_from_seq_nb(const seq_t *in, index_t const codon_index, codon_t *out){
  int target_ep = global_index_to_rank(in,codon_index);
  int local_index = global_index_to_local_index(in,codon_index);
  short *typed_seq = (short *)in->sequence;
  SHORT_GET_NB((short *)out, &(typed_seq[local_index]), 1, target_ep);
}

static inline void write_to_seq(const seq_t *in, const index_t codon_index, codon_t *data){
  int target_ep = global_index_to_rank(in,codon_index);
  int local_index = global_index_to_local_index(in,codon_index);
  short *typed_seq = (short *)in->sequence;
  SHORT_PUT(&(typed_seq[local_index]),(short *) data, 1, target_ep);
}
#ifdef USE_MPI3
#define WAIT_NB() QUIET()
static inline void WAIT_NB_()
{
 // printf("ThreadNUm: %i Max NUm %d\n", omp_get_thread_num(),request_num[omp_get_thread_num()]);
  for (int h = 0; h < request_num[omp_get_thread_num()]; h++)
  {
    MPI_Wait(&request[omp_get_thread_num() * Request_Queue_Num + h], MPI_STATUS_IGNORE);
  }
  request_num[omp_get_thread_num()] = 0;
}
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
