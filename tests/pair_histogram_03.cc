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


// Check the PairHistogram consumer. Do so with a sequence of samples
// that lie on the unit circle.
//
// Like _02, but use the write_gnuplot() function.


#include <iostream>
#include <valarray>

#include <sampleflow/producers/range.h>
#include <sampleflow/consumers/pair_histogram.h>


int main ()
{
  using SampleType = std::valarray<double>;

  SampleFlow::Producers::Range<SampleType> range_producer;

  SampleFlow::Consumers::PairHistogram<SampleType> histogram(-1.5, 1.5, 20,
                                                             -1.5, 1.5, 30);
  histogram.connect_to_producer (range_producer);

  std::vector<SampleType> samples(1000);
  for (unsigned int i=0; i<1000; ++i)
    samples[i] = { std::cos(100.*3.1415926+i), std::sin(100.*3.1415926+i) };

  range_producer.sample (samples);

  const auto x = histogram.get();

  histogram.write_gnuplot(std::move(std::cout));
}
