# ---------------------------------------------------------------------
#
# Copyright (C) 2019-2020 by the SampleFlow authors.
#
# This file is part of the SampleFlow library.
#
# The SampleFlow library is free software; you can use it, redistribute
# it, and/or modify it under the terms of the GNU Lesser General
# Public License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# The full text of the license can be found in the file LICENSE.md at
# the top level directory of SampleFlow.
#
# ---------------------------------------------------------------------


CMAKE_MINIMUM_REQUIRED (VERSION 3.8)
PROJECT (SampleFlow
         CXX)

#########################################
### Set up compiler flags and input paths

# Compile everything with C++20 and with debug flags
SET (CMAKE_CXX_STANDARD 20)
SET (CMAKE_CXX_STANDARD_REQUIRED ON)
SET (CMAKE_CXX_EXTENSIONS OFF)

# Also make sure we link with the threads library in question
FIND_PACKAGE (Threads)

#########################################
# Set paths correctly to take BOOST from wherever it is
# installed on the current system
FIND_PACKAGE(Boost 1.39 REQUIRED)
INCLUDE(CheckIncludeFileCXX)

MESSAGE(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")

SET(CMAKE_REQUIRED_INCLUDES ${Boost_INCLUDE_DIRS})
CHECK_INCLUDE_FILE_CXX("boost/signals2.hpp" SF_HAVE_SIGNALS2)
IF (NOT SF_HAVE_SIGNALS2)
  MESSAGE(FATAL_ERROR "Could not find boost/signals2.hpp")
ENDIF()
CHECK_INCLUDE_FILE_CXX("boost/numeric/ublas/matrix.hpp" SF_HAVE_MATRIX)
IF (NOT SF_HAVE_MATRIX)
  MESSAGE(FATAL_ERROR "Could not find boost/numeric/ublas/matrix.hpp")
ENDIF()


#########################################
### Find the Eigen library
FIND_PATH(_eigen_include_dir
          NAMES eigen3/Eigen/Dense
          HINTS ${EIGEN_DIR}/include)
IF ("${_eigen_include_dir}" STREQUAL "_eigen_include_dir-NOTFOUND")
  MESSAGE(FATAL_ERROR
          "The Eigen library was not found. You have to specify a path "
          "to that library by setting the EIGEN_DIR environment variable, "
          "or passing '-DEIGEN_DIR=...' as an argument to 'cmake'.")
ELSE()
  MESSAGE(STATUS "Found EIGEN headers at ${_eigen_include_dir}")
  INCLUDE_DIRECTORIES(${_eigen_include_dir})
ENDIF()


############################################################
### Having done the set-up, let us describe what SampleFlow
### actually is. Specifically, it is a collection of header
### files that can be installed and that can be linked to.
###
### In cmake, this is expressed via an INTERFACE library.
### Take a look at the following blog post that describes
### how this works:
###   http://mariobadr.com/creating-a-header-only-library-with-cmake.html
############################################################

ADD_LIBRARY(${PROJECT_NAME} INTERFACE)
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}
                           INTERFACE
                           $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                           "${Boost_INCLUDE_DIRS}")
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
                      INTERFACE
                      Threads::Threads)
TARGET_COMPILE_FEATURES(${PROJECT_NAME} INTERFACE cxx_std_14)

FILE(GLOB_RECURSE _header_files
     LIST_DIRECTORIES false
     "${CMAKE_SOURCE_DIR}/include/*")
TARGET_SOURCES(${PROJECT_NAME}
               INTERFACE
               ${_header_files})

INSTALL(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}_Targets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
        DESTINATION include)

INCLUDE(CMakePackageConfigHelpers)
WRITE_BASIC_PACKAGE_VERSION_FILE(
    "${PROJECT_BINARY_DIR}/${PROJECT_NAME}config.cmake"
    VERSION 0.1
    COMPATIBILITY AnyNewerVersion
)
INSTALL(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}config.cmake"
        DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake)

#########################################
### Provide "indent" target for indenting all headers and source files
ADD_CUSTOM_TARGET(indent
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND ./doc/indent
  COMMENT "Indenting all ${PROJECT_NAME} header and source files..."
  )



#########################################
### Next, set up the testsuite, and the documentation generation machinery

ENABLE_TESTING()
ADD_SUBDIRECTORY(tests)

ADD_SUBDIRECTORY(doc)
