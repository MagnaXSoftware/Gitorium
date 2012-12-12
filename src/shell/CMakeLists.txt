ADD_EXECUTABLE(gitorium-shell shell.c ../config.c)
TARGET_LINK_LIBRARIES(gitorium-shell ${GIT2_LIBRARIES} ${LIBCONFIG_LIBRARIES})
