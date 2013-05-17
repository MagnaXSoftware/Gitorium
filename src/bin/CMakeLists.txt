ADD_EXECUTABLE(gitorium main.c cmd_setup.c cmd_config.c ../perms.c ../config.c ../common.c)
TARGET_LINK_LIBRARIES(gitorium ${GIT2_LIBRARIES} ${LIBCONFIG_LIBRARIES})
INSTALL(TARGETS gitorium RUNTIME DESTINATION bin)
