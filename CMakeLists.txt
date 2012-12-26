PROJECT(gitorium C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

FIND_PACKAGE(git2 REQUIRED)
FIND_PACKAGE(libconfig REQUIRED)
INCLUDE_DIRECTORIES(${GIT2_INCLUDE_DIRS} ${LIBCONFIG_INCLUDE_DIRS})

# Compilation flags
SET(CMAKE_C_FLAGS "-O2 -g -Wall -Wextra -Wno-missing-field-initializers -Wstrict-aliasing=2 -Wstrict-prototypes -Wmissing-prototypes ${CMAKE_C_FLAGS} -std=c99")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

# Build in build directory
SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/build" CACHE PATH "Build directory" FORCE)

# Build Debug by default
IF (NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF ()

# Targets

INCLUDE_DIRECTORIES(src src/bin src/shell src/hook-admin src/hook-update src/hook-post-update)

ADD_SUBDIRECTORY(src/bin)
ADD_SUBDIRECTORY(src/shell)
ADD_SUBDIRECTORY(src/hook-admin)
ADD_SUBDIRECTORY(src/hook-update)
ADD_SUBDIRECTORY(src/hook-post-update)
