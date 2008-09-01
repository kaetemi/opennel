#-----------------------------------------------------------------------------
#
# YourProjectName
#    Author: YourName
#
# Notes:
#   * Changing install location: add -DCMAKE_INSTALL_PREFIX:PATH=/my/new/path
#   * Changing NeL prefix: add -DNEL_PREFIX_PATH:PATH=/my/nel/install
#
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# Load some macros.
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")
INCLUDE(CMakeModules/opennel.cmake)

#-----------------------------------------------------------------------------
# Force out of source builds.
CHECK_OUT_OF_SOURCE()

IF(WIN32)
  SET(CMAKE_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/../3rdParty/include;${CMAKE_INCLUDE_PATH}")
  SET(CMAKE_LIBRARY_PATH "${CMAKE_SOURCE_DIR}/../3rdParty/lib;${CMAKE_INCLUDE_PATH}")
ENDIF(WIN32)

IF(DEFINED NEL_PREFIX_PATH)
  SET(CMAKE_INCLUDE_PATH "${NEL_PREFIX_PATH}/include;${CMAKE_INCLUDE_PATH}")
  SET(CMAKE_LIBRARY_PATH "${NEL_PREFIX_PATH}/lib;${CMAKE_INCLUDE_PATH}")
ENDIF(DEFINED NEL_PREFIX_PATH)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4)

PROJECT(YourProjectName CXX C)
SET(NL_VERSION "0.6.0")

#-----------------------------------------------------------------------------
# Redirect output files
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH  ${CMAKE_BINARY_DIR}/lib)

#-----------------------------------------------------------------------------
# Set default config options
#
NL_SETUP_DEFAULT_OPTIONS()

#-----------------------------------------------------------------------------
# Override default options
#SET(WITH_COVERAGE       ON )
SET(WITH_3D	ON )

#-----------------------------------------------------------------------------
#Platform specifics

FIND_PACKAGE(Threads REQUIRED)
FIND_PACKAGE(LibXml2 REQUIRED)

NL_SETUP_BUILD()

FIND_PACKAGE(NeLMISC)

###
# An example of finding NeL 3D and CEGUI Renderer.
###
IF(WITH_3D)
  FIND_PACKAGE(FreeType)
  FIND_PACKAGE(NeL3D)
  FIND_PACKAGE(Jpeg)
#  
#  IF(WITH_DRIVER_OPENGL)
#    FIND_PACKAGE(OpenGL)
#    FIND_PACKAGE(NeLDriverGL)
#  ENDIF(WITH_DRIVER_OPENGL)
#
#  IF(WITH_CEGUI)
#    FIND_PACKAGE(CEGUI)
#    FIND_PACKAGE(NeLCEGUIRenderer)
#  ENDIF(WITH_CEGUI)
#
#  SET(JPEG_LIB jpeg)
ENDIF(WITH_3D)

###
# An example of finding sound and a driver.
###
#IF(WITH_SOUND)
#  FIND_PACKAGE(NeLSound)
#  IF(WITH_DRIVER_OPENAL)
#    FIND_PACKAGE(OpenAL)
#    FIND_PACKAGE(NeLDriverAL)
#  ENDIF(WITH_DRIVER_OPENAL)
#ENDIF(WITH_SOUND)

IF(WIN32)
  INCLUDE(${CMAKE_ROOT}/Modules/Platform/Windows-cl.cmake)
  SET(WINSOCK2_LIB ws2_32.lib)
ENDIF(WIN32)

NL_SETUP_BUILD_FLAGS()

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(include)

