// ---------------------------------------------------------------------
//
// Copyright (C) 2019 by the SampleFlow authors.
//
// This file is part of the SampleFlow library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE.md at
// the top level directory of deal.II.
//
// ---------------------------------------------------------------------


// Check the Utilities::get_nth_element() function for a variety of
// types


#include <iostream>
#include <valarray>
#include <vector>

#include <sampleflow/element_access.h>



int main ()
{
  using namespace SampleFlow;

  {
    using SampleType = double;
    SampleType sample = 3.1415;
    std::cout << "double: "
              << Utilities::get_nth_element(sample, 0)
              << std::endl;
  }

  {
    using SampleType = double[8];
    SampleType sample = { 1, 2, 3, 4, 5, 6, 7, 8 };
    std::cout << "double[8]: "
              << Utilities::get_nth_element(sample, 0) << ' '
              << Utilities::get_nth_element(sample, 4)
              << std::endl;
  }

  {
    using SampleType = std::vector<double>;
    SampleType sample = { 1, 2, 3, 4 };
    std::cout << "std::vector<double>(4): "
              << Utilities::get_nth_element(sample, 0) << ' '
              << Utilities::get_nth_element(sample, 2)
              << std::endl;
  }

  {
    using SampleType = std::valarray<double>;
    SampleType sample = { 1, 2, 3, 4 };
    std::cout << "std::valarray<double>(4): "
              << Utilities::get_nth_element(sample, 0) << ' '
              << Utilities::get_nth_element(sample, 2)
              << std::endl;
  }
}

