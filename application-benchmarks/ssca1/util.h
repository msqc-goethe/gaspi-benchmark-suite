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
*/

#ifndef __UTIL_H
#define __UTIL_H

#include <stdio.h>
#include "pairwise_align.h"
#include "types.h"

#ifdef USE_MPI3
#include <mpi.h>
extern MPI_Comm world;
extern MPI_Win window;
extern void* window_base;
extern size_t window_size;
extern void* next_window_address;
extern MPI_Request request;
#elif USE_GASPI
#include <assert.h>
#include "GASPI.h"
extern const gaspi_segment_id_t gaspi_segment_id;
extern const gaspi_queue_id_t gaspi_queue_id;
extern gaspi_pointer_t gaspi_segment_base_addr;
extern gaspi_pointer_t gaspi_segment_next_addr;
extern gaspi_size_t gaspi_segment_size;

#define GASPI_CHECK(stmt)                                    \
	do {                                                     \
		const gaspi_return_t gaspi_err = (stmt);             \
		if (gaspi_err != GASPI_SUCCESS) {                    \
			fprintf(stderr,                                  \
			        "[%s:%d]: GASPI call failed with %d \n", \
			        __FILE__,                                \
			        __LINE__,                                \
			        gaspi_err);                              \
                                                             \
			exit(EXIT_FAILURE);                              \
		}                                                    \
		assert(gaspi_err == GASPI_SUCCESS);                  \
	} while (0)

static inline void setup_gaspi_segment(const gaspi_size_t segment_size) {
	GASPI_CHECK(gaspi_segment_create(gaspi_segment_id,
	                                 segment_size,
	                                 GASPI_GROUP_ALL,
	                                 GASPI_BLOCK,
	                                 GASPI_MEM_UNINITIALIZED));
	GASPI_CHECK(gaspi_segment_ptr(gaspi_segment_id, &gaspi_segment_base_addr));
	gaspi_segment_next_addr = gaspi_segment_base_addr;
}
#else
#include <shmem.h>
#endif

#ifdef USE_MPI3
#define SHORT_GET(target, source, num_elems, rank) \
	MPI_Get(target,                                \
	        num_elems,                             \
	        MPI_SHORT,                             \
	        rank,                                  \
	        (void*) source - window_base,          \
	        num_elems,                             \
	        MPI_SHORT,                             \
	        window);                               \
	QUIET()
#elif USE_GASPI
#define SHORT_GET(target, source, num_elems, rank)                   \
	GASPI_CHECK(gaspi_read(gaspi_segment_id,                         \
	                       (void*) target - gaspi_segment_base_addr, \
	                       rank,                                     \
	                       gaspi_segment_id,                         \
	                       (void*) source - gaspi_segment_base_addr, \
	                       num_elems * sizeof(short),                \
	                       gaspi_queue_id,                           \
	                       GASPI_BLOCK));                            \
	QUIET()
#else
#define SHORT_GET(target, source, num_elems, pe) \
	shmem_short_get(target, source, num_elems, pe)
#endif

#ifdef USE_MPI3
#define SHORT_GET_NB(target, source, num_elems, rank) \
	MPI_Get(target,                                   \
	        num_elems,                                \
	        MPI_SHORT,                                \
	        rank,                                     \
	        (void*) source - window_base,             \
	        num_elems,                                \
	        MPI_SHORT,                                \
	        window)
#elif USE_GASPI
#define SHORT_GET_NB(target, source, num_elems, rank)                \
	GASPI_CHECK(gaspi_read(gaspi_segment_id,                         \
	                       (void*) target - gaspi_segment_base_addr, \
	                       rank,                                     \
	                       gaspi_segment_id,                         \
	                       (void*) source - gaspi_segment_base_addr, \
	                       num_elems * sizeof(short),                \
	                       gaspi_queue_id,                           \
	                       GASPI_BLOCK))
#else
#define SHORT_GET_NB(target, source, num_elems, pe) \
	shmem_short_get_nbi(target, source, num_elems, pe)
#endif

#ifdef USE_MPI3
#define LONG_GET(target, source, num_elems, rank) \
	MPI_Get(target,                               \
	        num_elems,                            \
	        MPI_LONG,                             \
	        rank,                                 \
	        (void*) source - window_base,         \
	        num_elems,                            \
	        MPI_LONG,                             \
	        window);                              \
	QUIET()
#elif USE_GASPI
#define LONG_GET(target, source, num_elems, rank)                    \
	GASPI_CHECK(gaspi_read(gaspi_segment_id,                         \
	                       (void*) target - gaspi_segment_base_addr, \
	                       rank,                                     \
	                       gaspi_segment_id,                         \
	                       (void*) source - gaspi_segment_base_addr, \
	                       num_elems * sizeof(short),                \
	                       gaspi_queue_id,                           \
	                       GASPI_BLOCK));                            \
	QUIET()
#else
#define LONG_GET(target, source, num_elems, pe) \
	shmem_long_get((long*) target, (long*) source, num_elems, pe)
#endif

#ifdef USE_MPI3
#define GETMEM(target, source, length, rank) \
	MPI_Get(target,                          \
	        length,                          \
	        MPI_BYTE,                        \
	        rank,                            \
	        (void*) source - window_base,    \
	        length,                          \
	        MPI_BYTE,                        \
	        window);                         \
	QUIET()
#elif USE_GASPI
#define GETMEM(target, source, length, rank)                         \
	GASPI_CHECK(gaspi_read(gaspi_segment_id,                         \
	                       (void*) target - gaspi_segment_base_addr, \
	                       rank,                                     \
	                       gaspi_segment_id,                         \
	                       (void*) source - gaspi_segment_base_addr, \
	                       length,                                   \
	                       gaspi_queue_id,                           \
	                       GASPI_BLOCK));                            \
	QUIET()
#else
#define GETMEM(target, source, length, pe) \
	shmem_getmem(target, source, length, pe)
#endif

#ifdef USE_MPI3
#define SHORT_PUT(target, source, num_elems, rank) \
	MPI_Put(source,                                \
	        num_elems,                             \
	        MPI_SHORT,                             \
	        rank,                                  \
	        (void*) target - window_base,          \
	        num_elems,                             \
	        MPI_SHORT,                             \
	        window);                               \
	QUIET()
#elif USE_GASPI
#define SHORT_PUT(target, source, num_elems, rank)                    \
	GASPI_CHECK(gaspi_write(gaspi_segment_id,                         \
	                        (void*) source - gaspi_segment_base_addr, \
	                        rank,                                     \
	                        gaspi_segment_id,                         \
	                        (void*) target - gaspi_segment_base_addr, \
	                        num_elems * sizeof(short),                \
	                        gaspi_queue_id,                           \
	                        GASPI_BLOCK));                            \
	QUIET()
#else
#define SHORT_PUT(target, source, num_elems, pe) \
	shmem_short_put(target, source, num_elems, pe)
#endif

#ifdef USE_MPI3
#define QUIET() MPI_Win_flush_all(window)
#elif USE_GASPI
#define QUIET() GASPI_CHECK(gaspi_wait(gaspi_queue_id, GASPI_BLOCK))
#else
#define QUIET() shmem_quiet()
#endif

#ifdef USE_MPI3
#define BARRIER_ALL() \
	QUIET();          \
	MPI_Barrier(MPI_COMM_WORLD)
#elif USE_GASPI
#define BARRIER_ALL() GASPI_CHECK(gaspi_barrier(GASPI_GROUP_ALL, GASPI_BLOCK))
#else
#define BARRIER_ALL() shmem_barrier_all()
#endif

#ifdef USE_MPI3
static inline int malloc_all(size_t size, void** address) {
	*address = next_window_address;
	next_window_address += size;
	MPI_Barrier(MPI_COMM_WORLD);
	if (next_window_address - window_base > window_size) {
		printf("ran out of memory!\n");
		return -1;
	}
	else
		return 0;
}
#elif USE_GASPI
static inline int malloc_all(size_t size, void** address) {
	*address = gaspi_segment_next_addr;
	gaspi_segment_next_addr += size;
	BARRIER_ALL();
	if (gaspi_segment_next_addr - gaspi_segment_base_addr >
	    gaspi_segment_size) {
		fprintf(stderr, "ran out of memory!\n");
		return -1;
	}
	return 0;
}
#else
static inline int malloc_all(size_t size, void** address) {
	*address = shmalloc(size);
	if (*address == NULL)
		return -1;
	else
		return 0;
}
#endif

#ifdef USE_MPI3
#define FREE_ALL(address) /* unable to free memory like this */
#elif USE_GASPI
#define FREE_ALL(address) /*unable to free memory like this */
#else
#define FREE_ALL(address) shfree(address)
#endif

static inline int global_index_to_rank(const seq_t* in,
                                       const index_t codon_index) {
	return codon_index / in->local_size;
}

static inline int global_index_to_local_index(const seq_t* in,
                                              const index_t codon_index) {
	return codon_index % in->local_size;
}

static inline void fetch_from_seq(const seq_t* in,
                                  index_t const codon_index,
                                  codon_t* out) {
	int target_ep = global_index_to_rank(in, codon_index);
	int local_index = global_index_to_local_index(in, codon_index);
	short* typed_seq = (short*) in->sequence;
	SHORT_GET((short*) out, &(typed_seq[local_index]), 1, target_ep);
}

static inline void fetch_from_seq_nb(const seq_t* in,
                                     index_t const codon_index,
                                     codon_t* out) {
	int target_ep = global_index_to_rank(in, codon_index);
	int local_index = global_index_to_local_index(in, codon_index);
	short* typed_seq = (short*) in->sequence;
	SHORT_GET_NB((short*) out, &(typed_seq[local_index]), 1, target_ep);
}

static inline void write_to_seq(const seq_t* in,
                                const index_t codon_index,
                                codon_t data) {
	int target_ep = global_index_to_rank(in, codon_index);
	int local_index = global_index_to_local_index(in, codon_index);
	short* typed_seq = (short*) in->sequence;
	short typed_data = (short) data;
	SHORT_PUT(&(typed_seq[local_index]), &typed_data, 1, target_ep);
}

#ifdef USE_MPI3
#define WAIT_NB() QUIET()
#elif USE_GASPI
#define WAIT_NB() QUIET()
#else
#define WAIT_NB() QUIET()
#endif

void distribute_rng_seed(unsigned int new_seed);
void seed_rng(int adjustment);
void touch_memory(void* mem, index_t size);
index_t scrub_hyphens(good_match_t* A,
                      seq_t* dest,
                      seq_t* source,
                      index_t length);
void assemble_acid_chain(good_match_t* A,
                         char* result,
                         seq_t* chain,
                         index_t length);
void assemble_codon_chain(good_match_t* A,
                          char* result,
                          seq_t* chain,
                          index_t length);
score_t simple_score(good_match_t* A, seq_t* main, seq_t* match);
seq_t* alloc_global_seq(index_t seq_size);
seq_t* alloc_local_seq(index_t seq_size);
void free_global_seq(seq_t* doomed);
void free_local_seq(seq_t* doomed);

#endif
