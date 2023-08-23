#ifndef __UTIL_GASPI_H__
#define __UTIL_GASPI_H__
#include <GASPI.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void allocate_gaspi_memory(const gaspi_segment_id_t,
                           const size_t,
                           const char c);
void allocate_gaspi_memory_initialized(const gaspi_segment_id_t id, const size_t size);
void free_gaspi_memory(const gaspi_segment_id_t);
void allocate_memory(void**, const size_t);
void free_memory(void*);
#endif
