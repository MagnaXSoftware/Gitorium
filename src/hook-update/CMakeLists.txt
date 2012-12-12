ADD_EXECUTABLE(gitorium-hook-update main.c ../config.c)
TARGET_LINK_LIBRARIES(gitorium-hook-update ${GIT2_LIBRARIES} ${LIBCONFIG_LIBRARIES})
