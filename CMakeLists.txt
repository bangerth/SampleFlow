CMAKE_MINIMUM_REQUIRED (VERSION 2.8.7)
PROJECT (SampleFlow)

INCLUDE_DIRECTORIES(include ${CMAKE_BINARY_DIR}/include)
INCLUDE_DIRECTORIES(AFTER $ENV{HOME}/p/deal.II/1/dealii/bundled/boost-1.62.0/include)
ADD_DEFINITIONS("-std=c++11 -ggdb")
ADD_EXECUTABLE (mcmc_test mcmc_test.cc)

# Provide "indent" target for indenting all headers and source files
ADD_CUSTOM_TARGET(indent
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMAND ./doc/indent
  COMMENT "Indenting all ASPECT header and source files..."
  )
