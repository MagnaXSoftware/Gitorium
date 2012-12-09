PROJECT(gitorium C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Platform specific compilation flags
#SET(CMAKE_C_FLAGS "-O2 -g -Wall -Wextra -Wno-missing-field-initializers -Wstrict-aliasing=2 -Wstrict-prototypes -Wmissing-prototypes ${CMAKE_C_FLAGS}")
#SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")
#SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

# Build Debug by default
IF (NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF ()

INCLUDE_DIRECTORIES(include lib shell)
 
# add the executable
ADD_SUBDIRECTORY(lib)
ADD_SUBDIRECTORY(shell)
