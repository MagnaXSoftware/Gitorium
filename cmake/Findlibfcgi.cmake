# - Find fcgi
# Find the native Libfcgi headers and libraries.
#
#  LIBFCGI_INCLUDE_DIRS - where to find fcgi_stdio.h, etc.
#  LIBFCGI_LIBRARIES    - List of libraries when using fcgi.
#  LIBFCGI_FOUND        - True if fcgi found.

# Look for the header file.
FIND_PATH(LIBFCGI_INCLUDE_DIR 
	NAMES fcgi_stdio.h
)
MARK_AS_ADVANCED(LIBFCGI_INCLUDE_DIR)

# Look for the library.
FIND_LIBRARY(LIBFCGI_LIBRARY
	NAMES fcgi
)
MARK_AS_ADVANCED(LIBFCGI_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set LIBFCGI_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBFCGI DEFAULT_MSG LIBFCGI_LIBRARY LIBFCGI_INCLUDE_DIR)

IF(LIBFCGI_FOUND)
  SET(LIBFCGI_LIBRARIES ${LIBFCGI_LIBRARY})
  SET(LIBFCGI_INCLUDE_DIRS ${LIBFCGI_INCLUDE_DIR})
ENDIF(LIBFCGI_FOUND)
