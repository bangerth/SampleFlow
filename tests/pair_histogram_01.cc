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


// Check the PairHistogram consumer. Do so with a sequence of samples
// that consists of {{0,0}, {1,1}, {0,0}, {1,1}, ...}. For this test,
// we put the histogram intervals at [-0.5,0.5], [0.5,1.5], [1.5,2.5]
// in x-direction, and [-0.5,0.5], [0.5,1.5] in y-direction, i.e., we
// do not have to worry about hitting any of the end points.


#include <iostream>
#include <valarray>

#include <sampleflow/producers/range.h>
#include <sampleflow/consumers/pair_histogram.h>


int main ()
{
  using SampleType = std::valarray<double>;

  SampleFlow::Producers::Range<SampleType> range_producer;

  SampleFlow::Consumers::PairHistogram<SampleType> histogram(-0.5, 2.5, 3,
                                                             -0.5, 1.5, 2);
  histogram.connect_to_producer (range_producer);

  std::vector<SampleType> samples(1000);
  for (unsigned int i=0; i<1000; ++i)
    samples[i] = SampleType(i % 2 == 0 ? 0. : 1., 2);

  range_producer.sample (samples);

  const auto x = histogram.get();

  for (const auto v : x)
    std::cout << std::get<0>(v)[0] << ',' << std::get<0>(v)[1] << ' '
              << std::get<1>(v)[1] << ',' << std::get<1>(v)[1] << " -> "
              << std::get<2>(v)
              << std::endl;
}
