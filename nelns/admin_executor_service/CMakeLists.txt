FILE(GLOB SRC *.cpp *.h)

ADD_EXECUTABLE(admin_executor_service ${SRC})

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${NELMISC_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(admin_executor_service ${PLATFORM_LINKFLAGS} ${LIBXML2_LIBRARIES} ${NELMISC_LIBRARY} ${NELNET_LIBRARY})
ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS admin_executor_service RUNTIME DESTINATION sbin)
INSTALL(FILES admin_executor_service.cfg common.cfg DESTINATION share/nel/nelns)
