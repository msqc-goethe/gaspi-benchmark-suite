/**
* @file success_or_die.h
* @brief Macro to handle GASPI errors and terminate the
*		 program as soon as errors occur.
* @author MSQC
* @version 2020-11-27
*/

#ifndef SUCCESS_OR_DIE_H
#define SUCCESS_OR_DIE_H

#include <GASPI.h>
#include <stdio.h>
#include <stdlib.h>

#define SUCCESS_OR_DIE(__f__)                                            \
  do                                                                    \
  {                                                                     \
    const gaspi_return_t __r__ = __f__;                                         \
                                                                        \
    if (__r__ != GASPI_SUCCESS)                                             \
    {                                                                   \
      printf ("Error: '%s' [%s:%i]: %i\n", #__f__, __FILE__, __LINE__, __r__); \
                                                                        \
      exit (EXIT_FAILURE);                                              \
    }                                                                   \
  } while (0)

#endif
