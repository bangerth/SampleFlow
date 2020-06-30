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

CMAKE_MINIMUM_REQUIRED (VERSION 3.1)


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
FILE(GLOB _doxygen_input "../include/sampleflow/*.h" "../include/sampleflow/*/*.h")
MESSAGE (STATUS "doc input ${_doxygen_input}")

ADD_CUSTOM_COMMAND(
  OUTPUT SampleFlow.dox
  DEPENDS CMakeLists.txt ${_doxygen_input}
  COMMAND echo '@INCLUDE               = options.dox'        > SampleFlow.dox
  COMMAND echo 'INPUT                  = ${_doxygen_input}' >> SampleFlow.dox
  COMMENT "Generating SampleFlow.dox")

ADD_CUSTOM_TARGET(doxygen
  DEPENDS SampleFlow.dox
  BYPRODUCTS SampleFlow.tag
  COMMAND doxygen SampleFlow.dox
  COMMENT "Running doxygen...")

# Create a target that we can use for 'make doc'.
ADD_CUSTOM_TARGET(doc
  DEPENDS doxygen)

