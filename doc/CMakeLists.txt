# ---------------------------------------------------------------------
#
# Copyright (C) 2020 by the SampleFlow authors.
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

CMAKE_MINIMUM_REQUIRED (VERSION 3.9)


MESSAGE(STATUS "Setting up documentation")

FIND_PROGRAM(DOXYGEN_EXECUTABLE
  NAMES doxygen
  HINTS ${DOXYGEN_DIR}
  PATH_SUFFIXES bin
)
MARK_AS_ADVANCED(DOXYGEN_EXECUTABLE)

IF( "${DOXYGEN_EXECUTABLE}" MATCHES "DOXYGEN_EXECUTABLE-NOTFOUND")
  MESSAGE(FATAL_ERROR
    "Could not find 'doxygen', which is required for running the testsuite.\n"
    "Please specify DOXYGEN_DIR to a location containing the binary, or\n"
    "install 'doxygen' via a package manage.")
ENDIF()


# First gather all of the input files that are to be used when creating
# the documentation.
FILE(GLOB_RECURSE _doxygen_inputs "${CMAKE_SOURCE_DIR}/include/sampleflow/*.h")

# Then set up the doxygen infrastructure
FIND_PACKAGE(Doxygen
  REQUIRED dot)

SET(DOXYGEN_PROJECT_NAME "SampleFlow")
SET(DOXYGEN_PROJECT_BRIEF
    "A C++ library for the type-safe generation and processing of statistical samples.")
SET(DOXYGEN_EXTRACT_ALL "YES")
SET(DOXYGEN_EXTRACT_PRIVATE "YES")
SET(DOXYGEN_EXTRACT_STATIC "YES")
SET(DOXYGEN_CITE_BIB_FILES "${CMAKE_CURRENT_SOURCE_DIR}/references.bib")
SET(DOXYGEN_IMAGE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/images")
SET(DOXYGEN_INPUT_FILTER "${CMAKE_CURRENT_SOURCE_DIR}/filter")
SET(DOXYGEN_FILTER_SOURCE_FILES "YES")
SET(DOXYGEN_USE_MATHJAX "YES")
SET(DOXYGEN_QUIET "YES")

DOXYGEN_ADD_DOCS(doxygen
  ${_doxygen_inputs}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR/doxygen}
  COMMENT "Running doxygen...")

# Create a target that we can use for 'make doc'.
ADD_CUSTOM_TARGET(doc
  DEPENDS doxygen
  COMMENT "Documentation was built successfully"
  COMMAND echo '-- '
  COMMAND echo '-- You can now find the documentation in 'doc/html/index.html'.'
  COMMAND echo '-- '
)

