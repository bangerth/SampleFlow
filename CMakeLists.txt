cmake_minimum_required (VERSION 2.8.7)
project (mcmc)

include_directories(AFTER $ENV{HOME}/p/deal.II/1/dealii/bundled/boost-1.56.0/include)
add_definitions("-std=c++0x")
add_executable (mcmc_test mcmc_test.cc)
