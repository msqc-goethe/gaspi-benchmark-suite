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

#ifndef _TYPES_H
#define _TYPES_H

#include <stdint.h>

typedef uint64_t index_t;

#ifdef USE_GASPI
#include <GASPI.h>
extern gaspi_rank_t num_nodes;
extern gaspi_rank_t rank;
#else
extern int num_nodes;
extern int rank;
#endif

#if defined(USE_SHMEM) || defined(USE_MPI3)|| defined(USE_GASPI)
typedef uint16_t codon_t;
typedef int16_t score_t;
#else
typedef int_fast8_t codon_t;
typedef int_fast16_t score_t;
#endif

codon_t *scan_back_main;
codon_t *scan_back_match;

typedef struct _sequence_t {
  codon_t *sequence;
  index_t length;
  index_t backing_memory;
  index_t local_size;
} seq_t;

typedef struct _seq_data_t {
  seq_t *main;
  seq_t *match;
  index_t max_validation;
} seq_data_t;

typedef struct
{
  index_t *goodEnds[2];
  score_t *goodScores;
  int report;
  int size;
  score_t min_score;
} current_ends_t;
#endif
