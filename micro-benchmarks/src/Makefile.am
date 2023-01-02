microbenchmarkdir = $(pkglibexecdir)
microbenchmark_PROGRAMS = microbenchmark 
microbenchmark_SOURCES = gbs_main.c \
	       argparser.c \
	       argparser.h \
	       gbs_abench_grid.c \
	       gbs_abench_grid.h \
	       gbs_benchmark.c \
	       gbs_benchmark.h \
	       gbs_benchmark_utils.c \
	       gbs_benchmark_utils.h \
	       gbs_microbenchmarks.c \
	       gbs_microbenchmarks.h \
	       gbs_ubench_allreduce.c \
	       gbs_ubench_atomic.c \
	       gbs_ubench_barrier.c \
	       gbs_ubench_get_all_bdir.c \
	       gbs_ubench_get_all_udir.c \
	       gbs_ubench_get_single_bdir.c \
	       gbs_ubench_get_single_udir.c \
	       gbs_ubench_headers.h \
	       gbs_ubench_noti_rate.c \
	       gbs_ubench_ping_pong.c \
	       gbs_ubench_put_aggregate_single_udir.c \
	       gbs_ubench_put_all_bdir.c \
	       gbs_ubench_put_all_udir.c \
	       gbs_ubench_put_single_bdir.c \
	       gbs_ubench_put_single_udir.c \
	       gbs_ubench_put_solo_single_udir.c \
	       gbs_ubench_true_exchange.c \
	       gbs_ubench_twosided_ping_pong.c \
	       success_or_die.h
microbenchmark_LDADD = ../inc/libarraylist.a ../inc/libstopwatch.a ../inc/libutility.a
AM_LDFLAGS = -L$(top_builddir)/inc/libarraylist.a \
	     -L$(top_builddir)/inc/libstopwatch.a \
	     -L$(top_builddir)/inc/libutility.a -lGPI2 \
	     $(gaspilib)
AM_CFLAGS=  $(gaspiinc) -I$(abs_top_srcdir)/inc -Wall -Wextra -pedantic \
	   -std=c99 -D_POSIX_C_SOURCE=199309L -pthread
