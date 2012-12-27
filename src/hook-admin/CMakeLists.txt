ADD_EXECUTABLE(gitorium-hook-admin main.c ssh.c repo.c ../config.c)
TARGET_LINK_LIBRARIES(gitorium-hook-admin ${GIT2_LIBRARIES} ${LIBCONFIG_LIBRARIES})
INSTALL(TARGETS gitorium-hook-admin RUNTIME DESTINATION bin)
