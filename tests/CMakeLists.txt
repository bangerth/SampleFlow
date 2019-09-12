# ---------------------------------------------------------------------
#
# Copyright (C) 2019 by the SampleFlow authors.
#
# This file is part of the SampleFlow library.
#
# The deal.II library is free software; you can use it, redistribute
# it, and/or modify it under the terms of the GNU Lesser General
# Public License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
# The full text of the license can be found in the file LICENSE.md at
# the top level directory of deal.II.
#
# ---------------------------------------------------------------------

CMAKE_MINIMUM_REQUIRED (VERSION 3.1)


MESSAGE(STATUS "Setting up tests")

# Create a target that we can use for 'make check'.
ADD_CUSTOM_TARGET(check COMMAND ${CMAKE_CTEST_COMMAND})

# Loop over all .cc files in this directory and make tests out of them.
FILE(GLOB _testfiles "*cc")
FOREACH(_testfile ${_testfiles})
  STRING(REPLACE ".cc" "" _testname ${_testfile})
  STRING(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" _testname ${_testname})
  MESSAGE(STATUS "  ${_testname}")

  # For each test, add the corresponding executable and the commands
  # necessary to run the test
  ADD_EXECUTABLE(${_testname} EXCLUDE_FROM_ALL ${_testfile})

  # Then specify what it means to run a test:
  # - execute the test and write the output to a .result file
  # - compare the .result file to the .output file
  ADD_CUSTOM_COMMAND(
    OUTPUT ${_testname}.result
    COMMAND ${_testname} > ${_testname}.result
    COMMAND diff -q ${_testname}.output ${_testname}.result
    DEPENDS ${_testname} ${_testname}.output
    COMMENT "Running test <${_testname}>...")
  ADD_CUSTOM_TARGET(run_${_testname}
                    DEPENDS ${_testname}.result)

  # Define running the test as executing the cmake program
  # with the given target.
  ADD_TEST(NAME ${_testname} 
           COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target run_${_testname})

  # Now also make sure that the test is part of the 'make check' command.
  ADD_DEPENDENCIES(check ${_testname})
ENDFOREACH()
