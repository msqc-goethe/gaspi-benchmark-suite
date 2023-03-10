AC_INIT([gaspi-application-benchmark],[2022.12])
AC_CONFIG_MACRO_DIR([../m4])
AM_INIT_AUTOMAKE([foreign])
AC_PROG_RANLIB
AC_PROG_CC([gcc mpicc oshcc])
AC_C_INLINE
AC_OPENMP
AC_CHECK_HEADERS([stdlib.h string.h sys/time.h unistd.h math.h omp.h])

AC_ARG_ENABLE([mpi],[AS_HELP_STRING([--enable-mpi],[build MPI version of application benchmarks])],[enable_mpi=yes],[enable_mpi=no])
AC_ARG_ENABLE([shmem],[AS_HELP_STRING([--enable-shmem],[build SHMEM version of application benchmarks])],[enable_shmem=yes],[enable_shmem=no])
AC_ARG_ENABLE([gaspi],[AS_HELP_STRING([--enable-gaspi],[build GASPI version of application benchmarks])],[AX_CHECK_GASPI enable_gaspi=yes],[enable_gaspi=no])

AS_IF([test "$enable_shmem" == yes], [shmem_library=true])
AS_IF([test "$enable_gaspi" == yes], [gaspi_library=true])
AS_IF([test "$enable_mpi"   == yes], [mpi3_library=true])

AC_CHECK_FUNC([MPI_Get_accumulate],[mpi3_library=true])
AC_CHECK_FUNC([shmem_barrier_all],[shmem_library=true])

AM_CONDITIONAL([MPI3],[test x$mpi3_library == xtrue && test "$enable_mpi" == yes])
AM_CONDITIONAL([GASPI],[test x$gaspi_library == xtrue && test "$enable_gaspi" == yes])
AM_CONDITIONAL([SHMEM],[test x$shmem_library == xtrue && test "$enable_shmem" == yes])

AC_CONFIG_FILES([Makefile 
	         inc/Makefile
		 applications/Makefile
		 applications/GUPs/Makefile
		 applications/GUPs_large_data/Makefile
		 applications/GUPs_passive_comm_2processes/Makefile
		 applications/GUPs_threaded_different_queues/Makefile
		 applications/put_get_performance/Makefile
		 applications/ring-1d/Makefile
		 applications/ssca1/Makefile
		 applications/ssca1_big_cluster/Makefile
		 applications/ssca1_no_segment_dependancy/Makefile
		 applications/ssca1_threaded_segment/Makefile
		])
AC_OUTPUT
