CMAKE_MINIMUM_REQUIRED (VERSION 3.1)
PROJECT (SampleFlow CXX)

#########################################
### Set up compiler flags and input paths

# Compile everything with C++11 and with debug flags
SET (CMAKE_CXX_STANDARD 11)
SET (CMAKE_CXX_STANDARD_REQUIRED ON)
SET (CMAKE_CXX_EXTENSIONS OFF)
ADD_DEFINITIONS("-ggdb")

# Source files need to know where headers are:
INCLUDE_DIRECTORIES(include "${CMAKE_BINARY_DIR}/include")

# Set paths correctly to take BOOST from wherever it is
# installed on the current system
FIND_PACKAGE(Boost
             REQUIRED
             COMPONENTS signals)

# Check that the header files we need really are there.
INCLUDE(CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX("boost/signals2.hpp" SF_HAVE_SIGNALS2)
IF (NOT SF_HAVE_SIGNALS2)
  MESSAGE(FATAL_ERROR "Could not find boost/signals2.hpp")
ENDIF()

CHECK_INCLUDE_FILE_CXX("boost/numeric/ublas/matrix.hpp" SF_HAVE_MATRIX)
IF (NOT SF_HAVE_MATRIX)
  MESSAGE(FATAL_ERROR "Could not find boost/numeric/ublas/matrix.hpp")
ENDIF()

# And then there is the issue of the BOOST histogram library. It was
# only added in version 1.70, so may not be available on some systems
# for now. Therefore, there is a copy of it in contrib/boost-histogram
# of the pre-inclusion version of that library. We fall back on it if
# the BOOST version we have is too old.
CHECK_INCLUDE_FILE_CXX("boost/histogram.hpp" SF_HAVE_HISTOGRAM)
IF (NOT SF_HAVE_HISTOGRAM)
  MESSAGE("--   Falling back to the version of boost::histogram in contrib/.")
  INCLUDE_DIRECTORIES(AFTER "${CMAKE_SOURCE_DIR}/contrib/boost-histogram/include")
ENDIF()


#########################################
### Describe what is currently the only executable
ADD_EXECUTABLE (mcmc_test mcmc_test.cc)


#########################################
### Provide "indent" target for indenting all headers and source files
ADD_CUSTOM_TARGET(indent
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND ./doc/indent
  COMMENT "Indenting all SampleFlow header and source files..."
  )


