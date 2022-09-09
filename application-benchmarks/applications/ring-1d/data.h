#ifndef DATA_H
#define DATA_H

#include "constant.h"

//1D index of element
#define POSITION_upper(buffer_id,slice_id,vector_id) ((vector_id) +        2*VLEN*(slice_id) + 2*VLEN*(NTHREADS+2) * (buffer_id))
#define POSITION_lower(buffer_id,slice_id,vector_id) ((vector_id) + VLEN + 2*VLEN*(slice_id) + 2*VLEN*(NTHREADS+2) * (buffer_id))

//Pointer to element
#define array_ELEM_upper(buffer_id,slice_id,vector_id) ((double *               )array)[POSITION_upper (buffer_id,slice_id,vector_id)]
#define array_ELEM_lower(buffer_id,slice_id,vector_id) ((double *)array)[POSITION_lower (buffer_id,slice_id,vector_id)]

//Byte offset for element
#define array_OFFSET_upper(buffer_id,slice_id,vector_id) (POSITION_upper        (buffer_id,slice_id,vector_id) * sizeof(double))
#define array_OFFSET_lower(buffer_id,slice_id,vector_id) (POSITION_lower (buffer_id,slice_id,vector_id) * sizeof(double))

void data_init (int iProc, int buffer_id, double*, int VLEN);
void data_verify (int iProc, int buffer_id, double*, int VLEN);
void data_compute_lower (double*, int buffer_to, int buffer_from, int tid, int VLEN);
void data_compute_upper (double*, int buffer_to, int buffer_from, int tid, int VLEN);
void data_compute (double*, int buffer_to, int buffer_from, int tid, int VLEN);

#endif
