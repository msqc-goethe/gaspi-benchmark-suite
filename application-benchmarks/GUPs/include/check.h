#ifndef __CHECK_H__
#define __CHECK_H__
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef USE_GASPI
#include "GASPI.h"
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
#elif USE_MPI3
#include "mpi.h"
#define MPI_CHECK(stmt)                                   \
	do {                                                  \
		int mpi_errno = (stmt);                           \
		if (MPI_SUCCESS != mpi_errno) {                   \
			fprintf(stderr,                               \
			        "[%s:%d] MPI call failed with %d \n", \
			        __FILE__,                             \
			        __LINE__,                             \
			        mpi_errno);                           \
			exit(EXIT_FAILURE);                           \
		}                                                 \
		assert(MPI_SUCCESS == mpi_errno);                 \
	} while (0)
#else
#endif
#endif
