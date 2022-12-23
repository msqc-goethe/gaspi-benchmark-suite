AC_INIT([gaspi-benchmark],[0.1])
AM_INIT_AUTOMAKE([foreign])
AC_PROG_RANLIB
AC_PROG_CC
AC_ARG_WITH([GASPI],
	[AS_HELP_STRING([--with-gaspi=@<:@GASPI installation path@:>@],
	[Provide path to GASPI installation])],
	[AS_CASE([$with_gaspi],
		[yes|no],[],
		[CFLAGS="-I$with_gaspi/include $CFLAGS"
		 LDFLAGS="-L$with_gaspi/lib64 -Wl,-rpath=$with_gaspi/lib64 $LDFLAGS"])])

AC_ARG_WITH([gaspi-include],[AS_HELP_STRING([--with-gaspi-include=@<:@GASPI include path@:>@],[Provide path to GASPI include files])],[AS_CASE([$with_gaspi_include],[yes|no],[],[CFLAGS="-I$with_gaspi_include $CFLAGS"])])

AC_ARG_WITH([gaspi-libpath],
	[AS_HELP_STRING([--with-gaspi-libpath=@<:@GASPI library path@:>@],
	[Provide path to GASPI library files])],
	[AS_CASE([$with_gaspi_libpath],
	[yes|no],[],
	[LDFLAGS="-L$with_gaspi_libpath $LDFLAGS"])])

	
AC_CONFIG_FILES([Makefile inc/Makefile src/Makefile])
AC_OUTPUT
