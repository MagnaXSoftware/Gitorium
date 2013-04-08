ADD_DEFINITIONS(-D_USE_FCGI_STDIO)
ADD_EXECUTABLE(gitorium-cgi main.c ../gitio.c ../repo.c ../perms.c ../config.c ../common.c)
TARGET_LINK_LIBRARIES(gitorium-cgi ${GIT2_LIBRARIES} ${LIBCONFIG_LIBRARIES} ${LIBFCGI_LIBRARIES})
INSTALL(TARGETS gitorium-cgi RUNTIME DESTINATION bin)
