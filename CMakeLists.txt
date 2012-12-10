PROJECT(gitorium C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# Compilation flags
SET(CMAKE_C_FLAGS "-O2 -g -Wall -Wextra -Wno-missing-field-initializers -Wstrict-aliasing=2 -Wstrict-prototypes -Wmissing-prototypes ${CMAKE_C_FLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

SET(dir ${CMAKE_SOURCE_DIR}/build)
SET(EXECUTABLE_OUTPUT_PATH ${dir} CACHE PATH "Build directory" FORCE)
SET(LIBRARY_OUTPUT_PATH ${dir} CACHE PATH "Build directory" FORCE)

# Build Debug by default
IF (NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF ()

INCLUDE_DIRECTORIES(src src/bin src/shell)

ADD_SUBDIRECTORY(src/bin)
#ADD_SUBDIRECTORY(src/shell)
