#include "data.h"
#include "assert.h"


static double value (int iProc, int slice_id)
{
  return iProc * NTHREADS + slice_id;
}


void data_init (int iProc, int buffer_id, double* array, int VLEN)
{

  for (int slice_id = 1; slice_id <= NTHREADS; ++slice_id)
  {
    for (int j = 0; j < VLEN; ++j)
    {
      array_ELEM_lower (buffer_id, slice_id, j) = value (iProc, slice_id);
    }
    for (int j = 0; j < VLEN; ++j)
    {
      array_ELEM_upper (buffer_id, slice_id, j) = value (iProc, slice_id);
    }
  }

}

void data_verify (int iProc, int buffer_id, double* array, int VLEN)
{


  for (int slice_id = 1; slice_id <= NTHREADS; ++slice_id)
  {
    for (int j = 0; j < VLEN; ++j)
    {
      //printf("Hallo");
      ASSERT (  array_ELEM_lower (buffer_id, slice_id, j)
             == value (iProc, slice_id)
             );
    }
    for (int j = 0; j < VLEN; ++j)
    {
      ASSERT (  array_ELEM_upper (buffer_id, slice_id, j)
             == value (iProc, slice_id)
             );
    }

  }

}

void data_compute_lower (double* array, int buffer_to, int buffer_from, int slice_id, int VLEN)
{

  for (int j = 0; j < VLEN; ++j)
    {
      array_ELEM_lower (buffer_to, slice_id, j) = array_ELEM_lower (buffer_from, slice_id + 1, j);
    }

}

void data_compute_upper (double* array, int buffer_to, int buffer_from, int slice_id, int VLEN)
{

  for (int j = 0; j < VLEN; ++j)
    {
      array_ELEM_upper (buffer_to, slice_id, j) = array_ELEM_upper (buffer_from, slice_id - 1, j);
    }

}


void data_compute (double* array, int buffer_to, int buffer_from, int slice_id, int VLEN)
{

  data_compute_lower (array, buffer_to, buffer_from, slice_id,VLEN);
  data_compute_upper (array, buffer_to, buffer_from, slice_id,VLEN);  

}
