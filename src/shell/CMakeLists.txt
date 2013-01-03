ADD_EXECUTABLE(gitorium-shell main.c perms.c ../config.c)
TARGET_LINK_LIBRARIES(gitorium-shell ${GIT2_LIBRARIES} ${LIBCONFIG_LIBRARIES})
INSTALL(TARGETS gitorium-shell RUNTIME DESTINATION bin)
