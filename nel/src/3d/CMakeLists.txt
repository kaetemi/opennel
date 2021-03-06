FILE(GLOB SRC *.cpp *.h ../../include/nel/3d/*.h)

DECORATE_NEL_LIB("nel3d")
SET(NL3D_LIB ${LIBNAME})
DECORATE_NEL_LIB("nelmisc")
SET(NLMISC_LIB ${LIBNAME})

IF(NOT WIN32)
  ADD_LIBRARY(${NL3D_LIB} SHARED ${SRC})
ELSE(NOT WIN32)
  ADD_LIBRARY(${NL3D_LIB} STATIC ${SRC})
ENDIF(NOT WIN32)

INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS} ${JPEG_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(${NL3D_LIB} ${NLMISC_LIB} ${FREETYPE_LIBRARY} ${JPEG_LIBRARY})
SET_TARGET_PROPERTIES(${NL3D_LIB} PROPERTIES VERSION ${NL_VERSION})

IF(WIN32)
  SET_TARGET_PROPERTIES(${NL3D_LIB} PROPERTIES COMPILE_FLAGS "/Yustd3d.h")
  SET_SOURCE_FILES_PROPERTIES(std3d.cpp PROPERTIES COMPILE_FLAGS "/Ycstd3d.h")
  SET_TARGET_PROPERTIES(${NL3D_LIB} PROPERTIES LINK_FLAGS "/NODEFAULTLIB:libcmt")

  IF(JPEG_FOUND)
    ADD_DEFINITIONS(/DUSE_JPEG)
  ENDIF(JPEG_FOUND)
ELSE(WIN32)
  IF(JPEG_FOUND)
    ADD_DEFINITIONS(-DUSE_JPEG)
  ENDIF(JPEG_FOUND)
ENDIF(WIN32)

ADD_DEFINITIONS(${LIBXML2_DEFINITIONS})

INSTALL(TARGETS ${NL3D_LIB} LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)

ADD_SUBDIRECTORY(driver)
