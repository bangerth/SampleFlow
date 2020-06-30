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


// Check the Histogram consumer. Do so with a sequence of samples that
// consists of {0, 1, 0, 1, ...}. For this test, we put the histogram
// intervals at [-1,0], [0,1], [1,2], i.e., we do have to worry about
// hitting the end points and it will matter which bin a sample is
// counted for.


#include <iostream>
#include <valarray>

#include <sampleflow/producers/range.h>
#include <sampleflow/consumers/histogram.h>


int main ()
{
  using SampleType = double;

  SampleFlow::Producers::Range<SampleType> range_producer;

  SampleFlow::Consumers::Histogram<SampleType> histogram(-1, 2, 3);
  histogram.connect_to_producer (range_producer);

  std::vector<SampleType> samples(1000);
  for (unsigned int i=0; i<1000; ++i)
    samples[i] = (i % 2 == 0 ? 0. : 1.);

  range_producer.sample (samples);

  for (const auto v : histogram.get())
    std::cout << std::get<0>(v) << ' '
              << std::get<1>(v) << " -> "
              << std::get<2>(v)
              << std::endl;
}
