ADD_EXECUTABLE(gitorium-hook-post-update main.c ../config.c)
TARGET_LINK_LIBRARIES(gitorium-hook-post-update ${GIT2_LIBRARIES} ${LIBCONFIG_LIBRARIES})
