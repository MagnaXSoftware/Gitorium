# - Find libconfig
# Find the native Libconfig headers and libraries.
#
#  LIBCONFIG_INCLUDE_DIRS - where to find config.h, etc.
#  LIBCONFIG_LIBRARIES    - List of libraries when using libconfig.
#  LIBCONFIG_FOUND        - True if libconfig found.

# Look for the header file.
FIND_PATH(LIBCONFIG_INCLUDE_DIR 
	NAMES libconfig.h
)
MARK_AS_ADVANCED(LIBCONFIG_INCLUDE_DIR)

# Look for the library.
FIND_LIBRARY(LIBCONFIG_LIBRARY
	NAMES config
)
MARK_AS_ADVANCED(LIBCONFIG_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set LIBCONFIG_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBCONFIG DEFAULT_MSG LIBCONFIG_LIBRARY LIBCONFIG_INCLUDE_DIR)

IF(LIBCONFIG_FOUND)
  SET(LIBCONFIG_LIBRARIES ${LIBCONFIG_LIBRARY})
  SET(LIBCONFIG_INCLUDE_DIRS ${LIBCONfIG_INCLUDE_DIR})
ENDIF(LIBCONFIG_FOUND)
