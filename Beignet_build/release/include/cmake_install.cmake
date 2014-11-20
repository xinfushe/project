# Install script for directory: /home/slilylsu/Desktop/project-repo/Beignet-0.9.3-Source/include

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "/usr/local")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

# Install shared libraries without execute permission?
IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  SET(CMAKE_INSTALL_SO_NO_EXE "1")
ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/CL" TYPE FILE FILES
    "/home/slilylsu/Desktop/project-repo/Beignet-0.9.3-Source/include/CL/cl_intel.h"
    "/home/slilylsu/Desktop/project-repo/Beignet-0.9.3-Source/include/CL/cl.h"
    "/home/slilylsu/Desktop/project-repo/Beignet-0.9.3-Source/include/CL/cl_egl.h"
    "/home/slilylsu/Desktop/project-repo/Beignet-0.9.3-Source/include/CL/opencl.h"
    "/home/slilylsu/Desktop/project-repo/Beignet-0.9.3-Source/include/CL/cl_d3d10.h"
    "/home/slilylsu/Desktop/project-repo/Beignet-0.9.3-Source/include/CL/cl_gl.h"
    "/home/slilylsu/Desktop/project-repo/Beignet-0.9.3-Source/include/CL/cl_d3d11.h"
    "/home/slilylsu/Desktop/project-repo/Beignet-0.9.3-Source/include/CL/cl_ext.h"
    "/home/slilylsu/Desktop/project-repo/Beignet-0.9.3-Source/include/CL/cl_gl_ext.h"
    "/home/slilylsu/Desktop/project-repo/Beignet-0.9.3-Source/include/CL/cl_dx9_media_sharing.h"
    "/home/slilylsu/Desktop/project-repo/Beignet-0.9.3-Source/include/CL/cl_platform.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/CL" TYPE FILE FILES "/home/slilylsu/Desktop/project-repo/Beignet-0.9.3-Source/include/CL/cl.hpp")
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

