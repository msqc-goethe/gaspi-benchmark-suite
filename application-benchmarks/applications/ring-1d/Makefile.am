AM_CFLAGS =
AM_LDFLAGS =
ring_1ddir = $(pkglibexecdir)/ring-1d
ring_1d_PROGRAMS = ring_1d 
ring_1d_SOURCES = assert.h \
		  comm_util.h \
		  constant.h \
		  data.c \
		  data.h \
		  left_right_dataflow_halo.c \
 		  slice.c \
		  slice.h \
		  topology.h \
		  testsome.h \
		  testsome.c \
	          success_or_die.h 
ring_1d_LDADD = ../../inc/libarraylist.a ../../inc/libstopwatch.a ../../inc/libutility.a

AM_LDFLAGS += -L$(top_builddir)/inc/libarraylist.a \
	     -L$(top_builddir)/inc/libstopwatch.a \
	     -L$(top_builddir)/inc/libutility.a
AM_CFLAGS +=  -I$(abs_top_srcdir)/inc -pipe -w -lm -ldl \
	     -frename-registers -Wall -pedantic \
	     -std=c99 -D_POSIX_C_SOURCE=199309L -pthread -fopenmp
if MPI3
AM_CFLAGS += -DUSE_MPI3
endif

if GASPI
AM_LDFLAGS += -lGPI2 $(gaspilib)
AM_CFLAGS += -DUSE_GASPI $(gaspiinc)
endif

if SHMEM
AM_CFLAGS += -DUSE_SHMEM
endif
