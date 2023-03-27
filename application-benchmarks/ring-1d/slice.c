#include "assert.h"
#include "slice.h"
#ifdef USE_GASPI
#include "success_or_die.h"
#include "testsome.h"
gaspi_rank_t iProc, nProc;
#else
int iProc,nProc;
#endif



#include "constant.h"
#include "data.h"
#include "topology.h"
#include "comm_util.h"

void init_slices (slice *ssl)
{
  for (int l = 0; l < NTHREADS; ++l)
  {
    ssl[l].stage = 0;
    ssl[l].index = l+1;
    ssl[l].left = ssl + ((l + NTHREADS - 1) % NTHREADS);
    ssl[l].next = ssl + ((l + NTHREADS + 1) % NTHREADS);
    omp_init_lock (&ssl[l].lock);
  }

}


void handle_slice( slice *sl, double* array
		   , unsigned short* left_data_available
		   , unsigned short* right_data_available
		   , unsigned char segment_id
		   , unsigned char queue_id
		   , int num
       , int VLEN
		   )
{
  const int right_halo = NTHREADS + 1;
  const int left_halo   = 0;

  ASSERT (sl->stage < num);

  int const new_buffer_id = (sl->stage + NWAY + 1 ) % NWAY;
  int const old_buffer_id = (sl->stage + NWAY) % NWAY;


  int stagecount = 0;
  volatile int left_stage = sl->left->stage;
  volatile int right_stage = sl->next->stage;
  volatile int my_stage = sl->stage;
  
  if (sl->index == left_halo + 1)
  {
    if (my_stage>right_stage||!test_or_die(LEFT(iProc, nProc),segment_id,left_data_available[old_buffer_id],1))
    {

      return;
    }
  }
  if (sl->index == right_halo - 1)
  {
    if (my_stage>left_stage||!test_or_die(RIGHT(iProc, nProc),segment_id,right_data_available[old_buffer_id],1))
    {
      return;
    }
  }
  if ((sl->index > left_halo + 1) && (sl->index < right_halo - 1))
  {
    if (my_stage>right_stage || my_stage>left_stage)
    {
      return;
    }
  }
  
  data_compute (array, new_buffer_id, old_buffer_id, sl->index,VLEN);

  if (sl->index == left_halo + 1)
  {
   PUTMEM(array+array_OFFSET_lower (new_buffer_id, right_halo, 0), \
         array+array_OFFSET_lower (new_buffer_id, left_halo + 1, 0), \
         VLEN, LEFT(iProc, nProc));

    SEND_NOTIFY(LEFT(iProc, nProc), right_data_available[new_buffer_id], 1);
    QUIET(iProc);
  }
  if (sl->index == right_halo - 1)
  {
    PUTMEM(array+array_OFFSET_upper (new_buffer_id, left_halo, 0), \
          array+array_OFFSET_upper (new_buffer_id, right_halo - 1, 0),\
          VLEN, RIGHT(iProc, nProc));
    SEND_NOTIFY( RIGHT(iProc, nProc), left_data_available[new_buffer_id], 1);
    QUIET(iProc);
  }

  ++sl->stage;
}

slice* get_slice_and_lock (slice* const ssl, const int num)
{

  int const tid = omp_get_thread_num();

  int slices_done;
  do
  {
    slices_done = 0;

    for (int i = 0; i < NTHREADS; ++i)
    {
      int const id = (tid + i) % NTHREADS;

      if (ssl[id].stage == num)
      {
        ++slices_done;
      }
      else if (omp_test_lock (&ssl[id].lock))
      {
        //! \note need to recheck as there is a race between ==num and lock
        if (ssl[id].stage == num)
        {
          ++slices_done;
        }
        else
        {
          return ssl + id;
        }
      }
    }
  }
  while (slices_done < NTHREADS);
  return NULL;
}
