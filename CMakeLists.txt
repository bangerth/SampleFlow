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


CMAKE_MINIMUM_REQUIRED (VERSION 3.28)
PROJECT (SampleFlow
         CXX)

#########################################
### Set up compiler flags and input paths

#########################################
# Also make sure we link with the threads library in question.
# We need this when linking C++ stuff that uses threads.
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
###
### SampleFlow uses C++20.
############################################################

ADD_LIBRARY(${PROJECT_NAME} INTERFACE)
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
SET_PROPERTY(TARGET ${PROJECT_NAME} PROPERTY CXX_EXTENSIONS OFF)

TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME}
                           INTERFACE
                           $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                           "${Boost_INCLUDE_DIRS}")
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
                      INTERFACE
                      Threads::Threads)

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


# Now also define a library that consists of the C++20 modules,
# assuming the compiler supports this:
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  set(SAMPLEFLOW_BUILD_MODULE "ON")
endif()

if (SAMPLEFLOW_BUILD_MODULE)
  set(PROJECT_MODULE ${PROJECT_NAME}.module)
  add_library(${PROJECT_MODULE} SHARED)
  set_target_properties(${PROJECT_MODULE} PROPERTIES LINKER_LANGUAGE CXX)
  SET_PROPERTY(TARGET ${PROJECT_MODULE} PROPERTY CXX_STANDARD 20)
  SET_PROPERTY(TARGET ${PROJECT_MODULE} PROPERTY CXX_EXTENSIONS OFF)
  target_sources(${PROJECT_MODULE}
                 PUBLIC
                 ${_header_files})
  target_sources(${PROJECT_MODULE}
                 PUBLIC
                 FILE_SET CXX_MODULES FILES
                 source/core.cc
                 source/producers.cc
                 source/filters.cc
                 source/consumers.cc
                 source/sampleflow.cc)

  target_compile_options(${PROJECT_MODULE} PRIVATE "-Wno-include-angled-in-module-purview")
  TARGET_INCLUDE_DIRECTORIES(${PROJECT_MODULE}
                             PUBLIC
                             $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
                             $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                             "${Boost_INCLUDE_DIRS}")
  TARGET_LINK_LIBRARIES(${PROJECT_MODULE}
                        INTERFACE
                        Threads::Threads)
endif()


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
