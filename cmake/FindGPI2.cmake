find_path(GPI2_PREFIX NAMES include/GASPI.h )

find_library(GPI2_LIBRARIES NAMES libGPI2.so libGPI2.a GPI2
	HINTS ${GPI2_PREFIX}/lib64 
)

find_path(GPI2_INCLUDE_DIRS NAMES GASPI.h HINTS ${GPI2_PREFIX}/include)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GPI2 DEFAULT_MSG GPI2_LIBRARIES GPI2_INCLUDE_DIRS)

mark_as_advanced(
	GPI2_PREFIX_DIRS
	GPI2_LIBRARIES
	GPI2_INCLUDE_DIRS
)
