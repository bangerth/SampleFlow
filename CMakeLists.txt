# ---------------------------------------------------------------------
#
# Copyright (C) 2019 by the SampleFlow authors.
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


CMAKE_MINIMUM_REQUIRED (VERSION 3.1)
PROJECT (SampleFlow CXX)

#########################################
### Set up compiler flags and input paths

# Compile everything with C++11 and with debug flags
SET (CMAKE_CXX_STANDARD 11)
SET (CMAKE_CXX_STANDARD_REQUIRED ON)
SET (CMAKE_CXX_EXTENSIONS OFF)
ADD_DEFINITIONS("-ggdb")

# Also make sure we link with the threads library in question
FIND_PACKAGE (Threads)

# Source files need to know where headers are:
INCLUDE_DIRECTORIES(include ${CMAKE_BINARY_DIR}/include)

# Set paths correctly to take BOOST from wherever it is
# installed on the current system
FIND_PACKAGE(Boost)
INCLUDE(CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX("boost/signals2.hpp" SF_HAVE_SIGNALS2)
IF (NOT SF_HAVE_SIGNALS2)
  MESSAGE(FATAL_ERROR "Could not find boost/signals2.hpp")
ENDIF()
CHECK_INCLUDE_FILE_CXX("boost/numeric/ublas/matrix.hpp" SF_HAVE_MATRIX)
IF (NOT SF_HAVE_MATRIX)
  MESSAGE(FATAL_ERROR "Could not find boost/numeric/ublas/matrix.hpp")
ENDIF()


#########################################
### Provide "indent" target for indenting all headers and source files
ADD_CUSTOM_TARGET(indent
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND ./doc/indent
  COMMENT "Indenting all SampleFlow header and source files..."
  )



#########################################
### Next, set up the testsuite, and the documentation generation machinery

ENABLE_TESTING()
ADD_SUBDIRECTORY(tests)

ADD_SUBDIRECTORY(doc)
