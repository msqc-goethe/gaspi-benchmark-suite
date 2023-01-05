AM_CFLAGS =
AM_LDFLAGS =
ssca1_threaded_segmentdir = $(pkglibexecdir)/ssca1_threaded_segment
ssca1_threaded_segment_PROGRAMS = ssca1_threaded_segment 
ssca1_threaded_segment_SOURCES = gen_scal_data.c \
		gen_scal_data.h \
		gen_sim_matrix.h \
		gen_sim_matrix.c \
		glibc_sort.c \
		main.c \
		pairwise_align.c \
		pairwise_align.h \
		parameters.c \
		parameters.h \
		scan_backwards.c \
		scan_backwards.h \
		sort.h \
		types.h \
		util.c \
		util.h \
	        success_or_die.h 
ssca1_threaded_segment_LDADD = ../../inc/libarraylist.a ../../inc/libstopwatch.a ../../inc/libutility.a

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
