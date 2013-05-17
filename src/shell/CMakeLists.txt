ADD_EXECUTABLE(gitorium-shell main.c cmd_int_list.c cmd_int_repo.c ../perms.c ../config.c ../repo.c ../gitio.c ../common.c)
TARGET_LINK_LIBRARIES(gitorium-shell ${GIT2_LIBRARIES} ${LIBCONFIG_LIBRARIES})
INSTALL(TARGETS gitorium-shell RUNTIME DESTINATION bin)
