AM_CFLAGS =
AM_LDFLAGS =
gupsdir = $(pkglibexecdir)/GUPs
gups_PROGRAMS = gups 
gups_SOURCES = SHMEMRandomAccess.c \
	       RandomAccess.h \
	       comm_util.h \
	       success_or_die.h 
gups_LDADD = ../../inc/libarraylist.a ../../inc/libstopwatch.a ../../inc/libutility.a

AM_LDFLAGS += -L$(top_builddir)/inc/libarraylist.a \
	     -L$(top_builddir)/inc/libstopwatch.a \
	     -L$(top_builddir)/inc/libutility.a
AM_CFLAGS +=  -I$(abs_top_srcdir)/inc -pipe -w -lm -ldl \
	     -frename-registers -Wall -pedantic \
	     -std=c99 -D_POSIX_C_SOURCE=199309L -pthread
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

if PAPI
AM_LDFLAGS += -lpapi $(papilib)
AM_CFLAGS += -DUSE_PAPI $(papiinc)
endif
