// ---------------------------------------------------------------------
//
// Copyright (C) 2019 by the SampleFlow authors.
//
// This file is part of the SampleFlow library.
//
// The SampleFlow library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE.md at
// the top level directory of SampleFlow.
//
// ---------------------------------------------------------------------


// Check the Utilities::size() function for a variety of types


#include <iostream>
#include <valarray>
#include <vector>

#ifndef SAMPLEFLOW_TEST_WITH_MODULE
#  include <sampleflow/element_access.h>
#else
import SampleFlow;
#endif


int main ()
{
  using namespace SampleFlow;

  {
    using SampleType = double;
    SampleType sample;
    std::cout << "double: "
              << Utilities::size(sample)
              << std::endl;
  }

  {
    using SampleType = double[8];
    SampleType sample;
    std::cout << "double[8]: "
              << Utilities::size(sample)
              << std::endl;
  }

  {
    using SampleType = std::vector<double>;
    SampleType sample(32);
    std::cout << "std::vector<double>(32): "
              << Utilities::size(sample)
              << std::endl;
  }

  {
    using SampleType = std::valarray<double>;
    SampleType sample(64);
    std::cout << "std::valarray<double>(64): "
              << Utilities::size(sample)
              << std::endl;
  }
}

