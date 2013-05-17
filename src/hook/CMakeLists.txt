ADD_EXECUTABLE(gitorium-hook main.c ../config.c ../common.c)
TARGET_LINK_LIBRARIES(gitorium-hook ${GIT2_LIBRARIES} ${LIBCONFIG_LIBRARIES})
INSTALL(TARGETS gitorium-hook RUNTIME DESTINATION bin)
