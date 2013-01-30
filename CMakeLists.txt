PROJECT(gitorium C)
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

FIND_PACKAGE(git2 REQUIRED)
FIND_PACKAGE(libconfig REQUIRED)
FIND_PACKAGE(libfcgi REQUIRED)
INCLUDE_DIRECTORIES(${GIT2_INCLUDE_DIRS} ${LIBCONFIG_INCLUDE_DIRS} ${LIBGFCGI_INCLUDE_DIRS})

# Compilation flags
SET(CMAKE_C_FLAGS "-std=c99 -O2 -g -Wall -Wextra -Wno-missing-field-initializers -Wstrict-aliasing=2 -Wstrict-prototypes -Wmissing-prototypes ${CMAKE_C_FLAGS}")
SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -std=c99")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

# Build in build directory
SET(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/build" CACHE PATH "Build directory" FORCE)

# Install directory
IF (NOT CMAKE_INSTALL_PREFIX)
	SET(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation prefix" FORCE)
ENDIF ()

# Build Debug by default
IF (NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF ()

# Specifics file
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/src/specific.h.cmake ${CMAKE_SOURCE_DIR}/src/specific.h)

# Non-binary installs
INSTALL(FILES data/config.cfg DESTINATION /etc/gitorium)

# Targets
INCLUDE_DIRECTORIES(src src/bin src/shell src/cgi src/hook-admin src/hook)

ADD_SUBDIRECTORY(src/bin)
ADD_SUBDIRECTORY(src/shell)
ADD_SUBDIRECTORY(src/cgi)
ADD_SUBDIRECTORY(src/hook)
ADD_SUBDIRECTORY(src/hook-admin)
