SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-unused-parameter")
FILE(GLOB LIB_SRC "*.c")
ADD_LIBRARY(gitorium STATIC ${LIB_SRC})
