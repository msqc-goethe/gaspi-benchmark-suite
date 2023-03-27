#ifndef __CHECK_H__
#define __CHECK_H__

#include <GASPI.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef GASPI_CHECK
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
#endif
#endif
