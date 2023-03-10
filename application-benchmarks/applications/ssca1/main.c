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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef USE_GASPI
#include "success_or_die.h"
#endif
#ifdef USE_PAPI
#include <papi.h>
#include <assert.h>
#endif
#include "parameters.h"
#include "gen_sim_matrix.h"
#include <sys/time.h>
#include <time.h>
#include "gen_scal_data.h"
#include "pairwise_align.h"
#include "scan_backwards.h"
#include <limits.h>
#include "util.h"

#ifdef USE_PAPI
#define PAPI_CHECK(stmt)                                   \
	do {                                                  \
		int retval = (stmt);                           \
		if (retval >= 0) \
			break; \
		if (retval != PAPI_OK) {                   \
			fprintf(stderr,                               \
			        "[%s:%d] PAPI call failed with %d \n", \
			        __FILE__,                             \
			        __LINE__,                             \
			        retval);                           \
			exit(EXIT_FAILURE);                           \
		}                                                 \
		assert(retval == PAPI_OK);                 \
	} while (0)
#endif
unsigned int random_seed;
#ifdef USE_GASPI
gaspi_rank_t num_nodes;
gaspi_rank_t rank;
#else
int num_nodes;
int rank;
#endif

/* unsigned int get_dev_rand() - 
     routine to get an unsigned int from /dev/random.
   Intput-
     None
   Output-
     unsigned int- value from /dev/random
 */

unsigned int get_dev_rand()
{
  int rand_file = open("/dev/urandom", O_RDONLY);
  unsigned int value;
  ssize_t read_value = read(rand_file, &value, sizeof(unsigned int));
  close(rand_file);
  if(read_value != sizeof(unsigned int))
  {
    fprintf(stderr, "Error reading random values, aborting\n");
    abort();
  }
  return value;
}

/* void display_elapsed(struct timeval *start_time)
       Given a start time, display how much time has elapsed since thing
     Input- 
       struct timeval *start_time- pointer to the struct with the starting time
    Output-
       None
 */

void display_elapsed(struct timeval *start_time)
{
  struct timeval now;
  gettimeofday(&now, NULL);
  long hours_elapsed = 0;
  long minutes_elapsed = 0;
  time_t seconds_elapsed = now.tv_sec - start_time->tv_sec;
  long miliseconds_elapsed = 0;
  long u_elapsed = now.tv_usec - start_time->tv_usec;
  if(u_elapsed < 0)
  {
    seconds_elapsed--;
    u_elapsed = 1000000 + u_elapsed;
  }

  if(seconds_elapsed > 60)
  {
    minutes_elapsed = seconds_elapsed / 60;
    seconds_elapsed = seconds_elapsed % 60;
  }

  if(minutes_elapsed > 60)
  {
    hours_elapsed = minutes_elapsed / 60;
    minutes_elapsed = minutes_elapsed % 60;
  }

  if(u_elapsed > 1000)
  {
    miliseconds_elapsed = u_elapsed / 1000;
    u_elapsed = u_elapsed % 1000;
  }

  printf("\n\tElapsed time: %li hour(s), %li minute(s), %li second(s), %li milliseconds,  %li micro second(s).\n", hours_elapsed, minutes_elapsed, seconds_elapsed, miliseconds_elapsed, u_elapsed);
}

/* int main(int argc, char **argv)
     Entry routine.  Calls each kernel once, displaying elapsed time
 */

int main(int argc, char **argv)
{
  printf("Program Start\n");
  
  parameters_t global_parameters;
  sim_matrix_t *sim_matrix;
  seq_data_t *seq_data;
  struct timeval start_time;
  good_match_t *A;

/* #ifdef USE_PAPI */
/* #define NUM_NETWORK_EVENTS 2 */
/* 	int network_event_set = PAPI_NULL,flop_event_set = PAPI_NULL; */
/* 	int flop_event_codes[NUM_FLOP_EVENTS] = {PAPI_SP_OPS,PAPI_DP_OPS,PAPI_TOT_INS}; */
/* 	long long network_values[NUM_NETWORK_EVENTS] = {0LL,0LL}; */
/* 	long long flop_values[NUM_FLOP_EVENTS]; */
/* 	const PAPI_component_info_t *component_info = NULL; */
/* 	int num_components,cid; */
/* 	int code,papi_retval; */
	
/* 	papi_retval = PAPI_library_init(PAPI_VER_CURRENT); */
/* 	if(papi_retval != PAPI_VER_CURRENT){ */
/* 		printf("PAPI_library_init failed with %d\n",papi_retval); */
/* 		exit(EXIT_FAILURE); */
/* 	} */
/* 	// Find infiniband component */
	
/* 	num_components = PAPI_num_components(); */
/* 	for(cid = 0; cid < num_components; ++cid){ */
/* 		if((component_info = PAPI_get_component_info(cid)) == NULL) { */
/* 			printf("PAPI_get_component_info failed with\n"); */
/* 			return -1; */
/* 		} */
/* 		if(strstr(component_info->name,"infiniband")){ */
/* 			break; */
/* 		} */
/* 	} */
/* 	// Create infiniband events */
/* 	PAPI_CHECK(PAPI_create_eventset(&network_event_set)); */
/* 	PAPI_CHECK(PAPI_event_name_to_code("infiniband:::mlx4_0_1:port_rcv_data",&code)); */
/* 	PAPI_CHECK(PAPI_add_event(network_event_set,code)); */
/* 	PAPI_CHECK(PAPI_event_name_to_code("infiniband:::mlx4_0_1:port_xmit_data",&code)); */
/* 	PAPI_CHECK(PAPI_add_event(network_event_set,code)); */
	
/* 	// Create event to count FLOPs */
/* 	PAPI_CHECK(PAPI_create_eventset(&flop_event_set)); */
/* 	PAPI_CHECK(PAPI_add_events(flop_event_set,flop_event_codes,NUM_FLOP_EVENTS)); */
/* #endif // end of PAPI infiniband component setup */

#ifdef USE_MPI3
  MPI_Info winfo;
  MPI_Init(&argc, &argv);
  MPI_Comm_size(MPI_COMM_WORLD, &num_nodes);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Info_create(&winfo);
  MPI_Info_set(winfo, "same_size", "true");
  MPI_Info_set(winfo, "alloc_shm", "false");
  window_size = 10000000;
  window_base = malloc(window_size);
  MPI_Win_create(window_base, window_size,  1, winfo, MPI_COMM_WORLD, &window);
  MPI_Win_lock_all(0, window);
  next_window_address = window_base;
  MPI_Info_free(&winfo);
  printf("Running with MPI-3, world size is %d\n", num_nodes);
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
  seg_buf = segment_ary_size - (rank * sizeof(uint64_t)) - sizeof(uint64_t);
  seg_buf_write = segment_ary_size -(rank * sizeof(uint64_t)) - sizeof(uint64_t)-2000;
  if(rank == 0)
    printf("Running with GASPI, world size is %d\n", num_nodes);
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
#ifdef USE_PREFETCH
  if(rank == 0) printf("Using Prefetching\n");
#endif

  init_parameters(&global_parameters);

  if(argc > 1 && !strcmp(argv[1],"--threads"))
  {
    global_parameters.threads = atoi(argv[2]);
  }
  else
  {
    global_parameters.threads = 1;
  }

  good_match_t *S[global_parameters.K2_MAX_REPORTS];
  memset(S, 0, sizeof(good_match_t *)*global_parameters.K2_MAX_REPORTS);
  malloc_all(sizeof(index_t), (void **)&scan_back_main);
  malloc_all(sizeof(score_t), (void **)&scan_back_match);
  if(global_parameters.ENABLE_VERIF || global_parameters.CONSTANT_RNG)
  {
    if(rank == 0) printf("\n\tVerification run, using constant seed for RNG\n");
    // interesting values that have uncovered bugs in the past,
    // 2613174141 -- segfault caused by insert_validation producting two identical values
    // -550696422 -- segfault caused by the RNG producing 0.
    random_seed = (unsigned int)2613174141;
  }
  else
  {
    random_seed = (unsigned int)time(NULL); // casting from time_t to unsigned int we can lose precision... no big deal here 
    random_seed += get_dev_rand();
    distribute_rng_seed(random_seed);
  }

  if(rank == 0){
    printf("HPCS SSCA #1 Bioinformatics Sequence Alignment Executable Specification:\nRunning...\n");

    printf("Using seed %u\n", random_seed);

    printf("\nScalable Data Generator - genScalData() beginning execution...\n");
  }

  gettimeofday(&start_time, NULL);

  sim_matrix = gen_sim_matrix(global_parameters.SIM_EXACT, global_parameters.SIM_SIMILAR, global_parameters.SIM_DISSIMILAR, global_parameters.GAP_START, global_parameters.GAP_EXTEND, global_parameters.MATCH_LIMIT);

  seq_data = gen_scal_data(sim_matrix, global_parameters.MAIN_SEQ_LENGTH, global_parameters.MATCH_SEQ_LENGTH, global_parameters.CONSTANT_RNG);

  if(rank == 0){
    display_elapsed(&start_time);

  	if(global_parameters.ENABLE_VERIF)
  	{
  	  verifyData(sim_matrix, seq_data);
  	}
  }

  // Kernel 1 run 

  if(rank == 0){
  printf("\nBegining Kernel 1 execution.\n");
/* #ifdef USE_PAPI */
/* 	PAPI_CHECK(PAPI_start(network_event_set)); */
/* 	PAPI_CHECK(PAPI_start(flop_event_set)); */
/* #endif */

  gettimeofday(&start_time, NULL);
  }
#ifdef USE_MPI3  
  QUIET();
#endif
#ifdef USE_PAPI
PAPI_CHECK(PAPI_hl_region_begin("Kernel 1"));
#endif
  A=pairwise_align(seq_data, sim_matrix, global_parameters.K1_MIN_SCORE, global_parameters.K1_MAX_REPORTS, global_parameters.K1_MIN_SEPARATION);
#ifdef USE_PAPI 
  PAPI_CHECK(PAPI_hl_region_end("Kernel 1"));
#endif
  if (rank == 0)
  {
    display_elapsed(&start_time);
/* #ifdef USE_PAPI */
/* 	PAPI_CHECK(PAPI_stop(network_event_set,network_values)); */
/* 	PAPI_CHECK(PAPI_stop(flop_event_set,flop_values)); */
/* 	printf("port_rcv_data: %lld\n",network_values[0]); */
/* 	printf("port_xmit_data: %lld\n",network_values[1]); */
/* 	printf("PAPI_SP_OPS: %lld\n",flop_values[0]); */
/* 	printf("PAPI_DP_OPS: %lld\n",flop_values[1]); */
/* 	printf("PAPI_TOT_INS: %lld\n",flop_values[2]); */
/* 	PAPI_CHECK(PAPI_cleanup_eventset(network_event_set)); */
/* 	PAPI_CHECK(PAPI_cleanup_eventset(flop_event_set)); */
/* #endif */
  }
    
  // Kernel 2 run 

  if(rank == 0){
  printf("\nBegining Kernel 2 execution.\n");

  gettimeofday(&start_time, NULL);

  scanBackward(A, global_parameters.K2_MAX_REPORTS, global_parameters.K2_MIN_SEPARATION);

  display_elapsed(&start_time);

  if(global_parameters.ENABLE_VERIF)
  {
    verify_alignment(A, global_parameters.K2_DISPLAY);
  }
  }

  //release_good_match(A);
  release_sim_matrix(sim_matrix);
  release_scal_data(seq_data);

  BARRIER_ALL();
#ifdef USE_MPI3
  MPI_Win_unlock_all(window);
  MPI_Finalize();
#endif
#ifdef USE_PAPI
  PAPI_shutdown();
#endif
  return 0;
}
