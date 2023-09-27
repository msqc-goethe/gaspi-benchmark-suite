find_path(
  GPI2_INCLUDE_DIR
  NAMES "GASPI.h"
  PATH_SUFFIXES "include"
)

find_library(
  GPI2_LIBRARY
  NAMES libGPI2.so libGPI2.a GPI2
  PATH_SUFFIXES "lib" "lib64"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  GPI2 REQUIRED_VARS GPI2_INCLUDE_DIR GPI2_LIBRARY
)

if(GPI2_FOUND)
  # Define a target only if none has been defined yet.
  if(NOT TARGET GPI2::GPI2)
    add_library(GPI2::GPI2 UNKNOWN IMPORTED)
    set_target_properties(
      GPI2::GPI2 PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${GPI2_INCLUDE_DIR}
                            IMPORTED_LOCATION ${GPI2_LIBRARY}
    )
  endif()
endif()

mark_as_advanced(GPI2_INCLUDE_DIR GPI2_LIBRARY)
