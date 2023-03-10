AM_CFLAGS =
AM_LDFLAGS =
gups_threaded_different_queuesdir = $(pkglibexecdir)/GUPs_threaded_different_queues
gups_threaded_different_queues_PROGRAMS = gups_threaded_different_queues 
gups_threaded_different_queues_SOURCES = SHMEMRandomAccess.c \
	       RandomAccess.h \
	       comm_util.h \
	       success_or_die.h 
gups_threaded_different_queues_LDADD = ../../inc/libarraylist.a ../../inc/libstopwatch.a ../../inc/libutility.a

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
