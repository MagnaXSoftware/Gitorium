# - Find libgit2
# Find the native Libgit2 headers and libraries.
#
#  GIT2_INCLUDE_DIRS - where to find git2.h, etc.
#  GIT2_LIBRARIES    - List of libraries when using libgit2.
#  GIT2_FOUND        - True if libgit2 found.

# Look for the header file.
FIND_PATH(GIT2_INCLUDE_DIR 
	NAMES git2.h
)
MARK_AS_ADVANCED(GIT2_INCLUDE_DIR)

# Look for the library.
FIND_LIBRARY(GIT2_LIBRARY
	NAMES git2
)
MARK_AS_ADVANCED(GIT2_LIBRARY)

# handle the QUIETLY and REQUIRED arguments and set GIT2_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GIT2 DEFAULT_MSG GIT2_LIBRARY GIT2_INCLUDE_DIR)

IF(GIT2_FOUND)
  SET(GIT2_LIBRARIES ${GIT2_LIBRARY})
  SET(GIT2_INCLUDE_DIRS ${GIT2_INCLUDE_DIR})
ENDIF(GIT2_FOUND)
