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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "success_or_die.h"
#include <sys/time.h>
#include <time.h>
#include <limits.h>
#include "util.h"
#ifdef _OPENMP
#include <omp.h>
#endif
unsigned int random_seed;
#ifdef USE_GASPI
gaspi_rank_t num_nodes;
gaspi_rank_t rank;
#else
int num_nodes;
int rank;
#endif


int main(int argc, char **argv)
{
  printf("Program Start\n");
 
  
#ifdef USE_MPI3
  MPI_Info winfo;
#endif
  int thread_num = 1;
  omp_set_num_threads(thread_num);

#ifdef USE_MPI3
  int provided;
  //MPI_Init(&argc, &argv);
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  MPI_Comm_size(MPI_COMM_WORLD, &num_nodes);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Info_create(&winfo);
  MPI_Info_set(winfo, "same_size", "true");
  MPI_Info_set(winfo, "alloc_shm", "false");
  window_size = 1000000;
  window_base = malloc(window_size);
  MPI_Win_create(window_base, window_size,  1, winfo, MPI_COMM_WORLD, &window);
  MPI_Win_lock_all(0, window);
  next_window_address = window_base;
  MPI_Info_free(&winfo);
  if(rank == 0)
    printf("Running with MPI, world size is %d\n", num_nodes);
#else
#ifdef USE_GASPI
  //segment size
  segment_size = 10000000;
  gaspi_size_t const seg_size = segment_size*sizeof(double);
  //segment_ary_size = 5; //128 geht nicht, 127 geht noch, sizeof(char)
  //init GASPIS
  SUCCESS_OR_DIE(gaspi_proc_init(GASPI_BLOCK));
  //set current rank
  SUCCESS_OR_DIE(gaspi_proc_rank(&rank));
  //get number of ranks
  SUCCESS_OR_DIE(gaspi_proc_num(&num_nodes));
  //create Segment, only one Group, Memory is unitilized
  SUCCESS_OR_DIE(gaspi_segment_create(segment_id, seg_size, GASPI_GROUP_ALL, GASPI_BLOCK, GASPI_MEM_UNINITIALIZED));
  //initilaize array
  SUCCESS_OR_DIE(gaspi_segment_ptr(segment_id, &segment_base_ary));
  segment_next_ary = segment_base_ary;
  segment_ary_size = segment_size;
  noti_num = rank*omp_get_max_threads();
  if(rank == 0)
    printf("Running with GASPI, world size is %d\n", num_nodes);
  for (int i = 0; i < Queue_num;i++){
        gaspi_queue_create(&queues[i], GASPI_BLOCK);
    }
#else
#ifdef USE_SHMEM
  start_pes(0);
  num_nodes=shmem_n_pes();
  rank=shmem_my_pe();
  if(rank == 0)
    printf("Running with OpenSHMEM, world size is %i\n", num_nodes);
#else
  num_nodes=1;
  rank=0;
#endif
#endif
#endif
  BARRIER_ALL();
  short *target;
  short *source;
  malloc_all(sizeof(short), (void **)&target);
  malloc_all(sizeof(short), (void **)&source);
  
  if (rank==0){
  printf("Warmup\n");
  struct timeval start, end;
  for (int i = 0; i < 1000; i++)
  {
    SHORT_GET_NB(target, source, 1, 1);
    SHORT_PUT_NB(target, source, 1, 1);
    SHORT_GET_NB(target, source, 1, 0);
    SHORT_PUT_NB(target, source, 1, 0);
    QUIET();
}
int max_time = 1000;
  //GET
  gettimeofday(&start, NULL);
  for (int i = 0; i < max_time; i++)
  {
  SHORT_GET_NB(target, source, 1, 0);
  QUIET();
  } 
  gettimeofday(&end, NULL);
  printf("Local Short_GET_B+QUIET(): %ld micro seconds*%d\n",((end.tv_sec * 1000000 + end.tv_usec) -(start.tv_sec * 1000000 + start.tv_usec)), max_time);

  gettimeofday(&start, NULL);
    for (int i = 0; i < max_time; i++)
  {
  SHORT_GET_NB(target, source, 1, 1);
  QUIET();
  } 
  gettimeofday(&end, NULL);
  printf("Remote Short_GET_B+QUIET(): %ld micro seconds*%d\n",((end.tv_sec * 1000000 + end.tv_usec) -(start.tv_sec * 1000000 + start.tv_usec)), max_time);

  //PUT
  gettimeofday(&start, NULL);
    for (int i = 0; i < max_time; i++)
  {
  SHORT_PUT_NB(target, source, 1, 0);
  QUIET();
  } 
  gettimeofday(&end, NULL);
  printf("Local Short_PUT_B+QUIET(): %ld micro seconds*%d\n",((end.tv_sec * 1000000 + end.tv_usec) -(start.tv_sec * 1000000 + start.tv_usec)), max_time);
  
  gettimeofday(&start, NULL);
    for (int i = 0; i < max_time; i++)
  {
  SHORT_PUT_NB(target, source, 1, 1);
  QUIET();
  } 
  gettimeofday(&end, NULL);
  printf("Remote Short_PUT_B+QUIET(): %ld micro seconds*%d\n",((end.tv_sec * 1000000 + end.tv_usec) -(start.tv_sec * 1000000 + start.tv_usec)), max_time);
  
  //local sequence
  gettimeofday(&start, NULL);
    for (int i = 0; i < max_time; i++)
  {
  SHORT_GET_NB(target, source, 1, 0);
  SHORT_GET_NB(target, source, 1, 0);
  SHORT_PUT_NB(target, source, 1, 0);
  SHORT_PUT_NB(target, source, 1, 0);
  QUIET();
  } 
  gettimeofday(&end, NULL);
  printf("Local Series: %ld micro seconds*%d\n",((end.tv_sec * 1000000 + end.tv_usec) -(start.tv_sec * 1000000 + start.tv_usec)), max_time);
  
  //remote sequence
  gettimeofday(&start, NULL);
    for (int i = 0; i < max_time; i++)
  {
  SHORT_GET_NB(target, source, 1, 1);
  SHORT_GET_NB(target, source, 1, 1);
  SHORT_PUT_NB(target, source, 1, 1);
  SHORT_PUT_NB(target, source, 1, 1);
  QUIET();
  } 
  gettimeofday(&end, NULL);
  printf("Remote Series: %ld micro seconds*%d\n",((end.tv_sec * 1000000 + end.tv_usec) -(start.tv_sec * 1000000 + start.tv_usec)), max_time);
  }
  BARRIER_ALL();
#ifdef USE_MPI3
  MPI_Win_unlock_all(window);
#endif
  return 0;
}
