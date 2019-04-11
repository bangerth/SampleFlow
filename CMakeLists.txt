CMAKE_MINIMUM_REQUIRED (VERSION 2.8.7)
PROJECT (SampleFlow)

#########################################
### Set up compiler flags and input paths

# Compile everything with C++11 and with debug flags
ADD_DEFINITIONS("-std=c++11 -ggdb")

# Source files need to know where headers are:
INCLUDE_DIRECTORIES(include ${CMAKE_BINARY_DIR}/include)

# Set paths correctly to take BOOST from wherever it is
# installed on the current system
FIND_PACKAGE(Boost
             REQUIRED
             COMPONENTS signals)


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
