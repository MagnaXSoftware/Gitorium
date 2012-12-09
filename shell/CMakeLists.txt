ADD_EXECUTABLE(gitorium-shell shell.c)
SET_TARGET_PROPERTIES(gitorium-shell PROPERTIES LANGUAGE C)
TARGET_LINK_LIBRARIES(gitorium-shell gitorium)
