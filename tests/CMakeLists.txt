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
# the top level directory of deal.II.
#
# ---------------------------------------------------------------------

CMAKE_MINIMUM_REQUIRED (VERSION 3.28)


MESSAGE(STATUS "Setting up tests")

FIND_PROGRAM(NUMDIFF_EXECUTABLE
  NAMES numdiff
  HINTS ${NUMDIFF_DIR}
  PATH_SUFFIXES bin
)
MARK_AS_ADVANCED(NUMDIFF_EXECUTABLE)

IF( "${NUMDIFF_EXECUTABLE}" MATCHES "NUMDIFF_EXECUTABLE-NOTFOUND")
  MESSAGE(FATAL_ERROR
    "Could not find 'numdiff', which is required for running the testsuite.\n"
    "Please specify NUMDIFF_DIR to a location containing the binary, or\n"
    "install 'numdiff' via a package manage.")
ENDIF()


macro(sampleflow_add_test _testfile _use_cxx20_modules)
  STRING(REPLACE ".cc" "" _testname_base ${_testfile})
  STRING(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" _testname_base ${_testname_base})
  set(_testname "${_testname_base}")

  if(${_use_cxx20_modules} STREQUAL "TRUE")
    set(_testname "${_testname}.module")
  endif()

  MESSAGE(STATUS "  ${_testname}")

  # For each test, add the corresponding executable and the commands
  # necessary to run the test. Compile the executable with -Werror
  # to ensure that there are no warnings in either the tests, or the
  # header files these tests #include.
  ADD_EXECUTABLE(${_testname} EXCLUDE_FROM_ALL ${_testfile})

  SET_PROPERTY(TARGET ${_testname} PROPERTY CXX_STANDARD 20)
  SET_PROPERTY(TARGET ${_testname} PROPERTY CXX_EXTENSIONS OFF)

  set_target_properties(${_testname} PROPERTIES COMPILE_WARNING_AS_ERROR ON)
  TARGET_LINK_LIBRARIES (${_testname} ${PROJECT_NAME})

  if(${_use_cxx20_modules} STREQUAL "TRUE")
    target_compile_definitions(${_testname} PRIVATE "SAMPLEFLOW_TEST_WITH_MODULE")
    TARGET_LINK_LIBRARIES (${_testname} ${PROJECT_MODULE})
  endif()

  # Then specify what it means to run a test:
  #
  # Step 1 is to:
  # - remove the previous .result file just in case the executable aborts before
  #   it produces any output (which leaves the previous output file in place)
  # - execute the test and write the output to a .result file
  ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_testname}.result
    COMMAND rm -f ${CMAKE_CURRENT_BINARY_DIR}/${_testname}.result
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${_testname} > ${CMAKE_CURRENT_BINARY_DIR}/${_testname}.result
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${_testname}
    COMMENT "Running test <${_testname}>...")

  # Step 2 is to:
  # - remove the .ok file
  # - compare the .result file to the .output file
  # - if this succeeds, write a .ok file
  ADD_CUSTOM_COMMAND(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_testname}.ok
    COMMAND rm -f ${_testname}.ok
    COMMAND ${NUMDIFF_EXECUTABLE} -a 1e-6 -r 1e-8 -s "' \\t\\n\\r=,:;<>[](){}^'" ${CMAKE_CURRENT_SOURCE_DIR}/${_testname_base}.output ${CMAKE_CURRENT_BINARY_DIR}/${_testname}.result
    COMMAND touch ${CMAKE_CURRENT_BINARY_DIR}/${_testname}.ok
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${_testname}.result ${_testname_base}.output
    COMMENT "Comparing test <${_testname}>...")

  # Now make success of the test be dependent on there being an .ok file:
  ADD_CUSTOM_TARGET(run_${_testname}
                    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${_testname}.ok)

  # Define running the test as executing the cmake program
  # with the given target.
  ADD_TEST(NAME ${_testname} 
           COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target run_${_testname})

  # Now also make sure that the test is part of the 'make check' command.
  ADD_DEPENDENCIES(check ${_testname})
endmacro()


# Create a target that we can use for 'make check'.
ADD_CUSTOM_TARGET(check COMMAND ${CMAKE_CTEST_COMMAND})

# Loop over all .cc files in this directory and make tests out of them.
FILE(GLOB _testfiles "*cc")
FOREACH(_testfile ${_testfiles})
  sampleflow_add_test(${_testfile} "FALSE")
  if (SAMPLEFLOW_BUILD_MODULE)
    sampleflow_add_test(${_testfile} "TRUE")
  endif()
ENDFOREACH()
