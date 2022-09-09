
#ifndef __UTIL_H
#define __UTIL_H
#include <stdio.h>
#include <unistd.h>
static const int queue_id = 0;
static const int segment_id = 0;
int *notify;
#define Queue_num 16
#define MAXTHREADS 8
#ifdef USE_MPI3
#include <mpi.h>
MPI_Info winfo;
MPI_Comm world;
MPI_Win window;
void *window_base;
size_t window_size;
void *next_window_address;

#define Request_Queue_Num 100
int request_num[64];
MPI_Request request[Request_Queue_Num*64];
MPI_Request request_block[64];
int iProc, nProc;
#else
#ifdef USE_GASPI
#include <GASPI.h>
#include "success_or_die.h"

void *segment_base_ary; //first Element of GASPI Segment
void *segment_next_ary;
size_t segment_size;
gaspi_rank_t iProc, nProc;
#define timeout GASPI_BLOCK
#else
#include <shmem.h> 
int iProc, nProc;
#endif
#endif


#ifdef USE_MPI3
static inline void INIT(int win_size){
  
  int argc=1;
  char *argv[1];
  int provided;
  MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
  MPI_Comm_size(MPI_COMM_WORLD, &nProc);
  MPI_Comm_rank(MPI_COMM_WORLD, &iProc);
  MPI_Info_create(&winfo);
  MPI_Info_set(winfo, "same_size", "true");
  MPI_Info_set(winfo, "alloc_shm", "false");
  window_size = win_size;
  window_base = malloc(window_size);
  MPI_Win_create(window_base, window_size,  1, winfo, MPI_COMM_WORLD, &window);
  MPI_Win_lock_all(0, window);
  next_window_address = window_base;
  MPI_Info_free(&winfo);
  if(iProc == 0) printf("Running with MPI-3, world size is %d\n", nProc);
}
#else
#ifdef USE_GASPI
static inline void INIT(int seg_size){
  segment_size = seg_size;
  //init GASPIS
  SUCCESS_OR_DIE( gaspi_proc_init(GASPI_BLOCK) );
  //set current rank
  SUCCESS_OR_DIE( gaspi_proc_rank(&iProc));
  //get number of ranks
  SUCCESS_OR_DIE( gaspi_proc_num(&nProc));
  //create Segment, only one Group, Memory is unitilized
  SUCCESS_OR_DIE ( gaspi_segment_create ( segment_id, segment_size , GASPI_GROUP_ALL, GASPI_BLOCK, GASPI_MEM_INITIALIZED ) );
  //initilaize array
  SUCCESS_OR_DIE( gaspi_segment_ptr (segment_id, &segment_base_ary) );
  segment_next_ary = segment_base_ary;
    if (iProc == 0)
      printf("Running with GASPI,rank %d world size is %d\n", iProc, nProc);
}
#else
static inline void INIT(int nothing){
  shmem_init();
  nProc = shmem_n_pes();
  iProc = shmem_my_pe();
  if(iProc == 0) printf("Running with OpenSHMEM,rank %d world size is %d\n", iProc, nProc);
}
#endif
#endif

#ifdef USE_MPI3
#define TERMINATE()		MPI_Finalize(void)
#else
#ifdef USE_GASPI
#define TERMINATE() gaspi_proc_term (GASPI_BLOCK);
#else

#define TERMINATE()		shmem_finalize()
#endif
#endif

#ifdef USE_MPI3
#define QUIET(rank)  MPI_Win_flush_local(rank,window)
#else
#ifdef USE_GASPI
#define QUIET(rank) gaspi_wait(queue_id, GASPI_BLOCK)
#else
#define QUIET(rank)		shmem_quiet()
#endif
#endif

#ifdef USE_MPI3
static inline int SEND_NOTIFY(int rank,int id,int value) {
  MPI_Win_flush(rank,window);
  MPI_Put(&id, 1, MPI_INT, rank, ((void *)(notify +id+ iProc*10 + 1) - window_base), 1, MPI_INT, window); 
}
#else
#ifdef USE_GASPI
#define SEND_NOTIFY(rank,id,value) gaspi_notify(segment_id, rank, id, value, queue_id, GASPI_BLOCK)
#else
static inline int SEND_NOTIFY(int rank,int id,int value) {
  //shmem_quiet();
  shmem_int_put((notify +id+ iProc*10 + 1) ,&id, 1, rank);
}

#endif
#endif

#ifdef USE_MPI3

static inline int WAIT_NOTIFY(int rank,unsigned short notification_id, unsigned short* first_id){
  MPI_Win_flush(iProc,window);
  if (*(notify+notification_id + rank*10 + 1) == notification_id)
  {
    *first_id = *(notify+rank*10+notification_id+1);
    *(notify+rank*10+notification_id+1) = 255;
    return 0;
  }
  else
  {
    return 1;
  }
};
#else
#ifdef USE_GASPI
#define WAIT_NOTIFY(rank,notification_id, first_id) gaspi_notify_waitsome(segment_id, notification_id, 1, first_id, GASPI_TEST)
#else
static inline int WAIT_NOTIFY(int rank,unsigned short notification_id, unsigned short* first_id){
  //shmem_quiet();
  if (*(notify+notification_id + rank*10 + 1) == notification_id)
  {
    *first_id = *(notify+rank*10+notification_id+1);
    *(notify+rank*10+notification_id+1) = 255;
    return 0;
  }
  else
  {
    return 1;
  }
};
#endif
#endif

#ifdef USE_MPI3
static inline void NOTIFY_RESET(unsigned short id,unsigned short* value){
  *value = 1;
}
#else
#ifdef USE_GASPI
#define NOTIFY_RESET(id,value) gaspi_notify_reset(segment_id, id, value)
#else
static inline void NOTIFY_RESET(unsigned short id,unsigned short* value){
  *value = 1;
}
#endif
#endif

#ifdef USE_MPI3
#define PUTMEM(target, source,length, rank)	MPI_Put(source, length, MPI_DOUBLE, rank, (void *)target - window_base, length, MPI_DOUBLE, window);//MPI_Win_flush(iProc,window); 
#else
#ifdef USE_GASPI
#define PUTMEM(target, source, length, rank)		gaspi_write(segment_id, (void *)source - segment_base_ary, rank,segment_id,(void *)target - segment_base_ary, length*sizeof(double), queue_id, timeout);
#else
#define PUTMEM(target, source, length, pe)		shmem_double_put(target, source, length, pe)
#endif
#endif

#ifdef USE_MPI3
#define BARRIER_ALL()	MPI_Win_flush_all(window); MPI_Barrier(MPI_COMM_WORLD)
#else
#ifdef USE_GASPI
#define BARRIER_ALL() QUIET(); gaspi_barrier(GASPI_GROUP_ALL, timeout)
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
  *address = shmem_malloc(size);
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
#define FREE_ALL(address) shmem_free(address)
#endif
#endif

#ifdef USE_MPI3
#define FINALIZE() 
#else
#ifdef USE_GASPI 
#define FINALIZE() gaspi_proc_term(GASPI_BLOCK)
#else
#define FINALIZE() shmem_finalize()
#endif
#endif


#endif



