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


// Check the AutoCovarianceTrace consumer. Do so with a sequence of
// samples that consists of {0, 1, 0, 1, ...}


#include <iostream>
#include <valarray>

#include <sampleflow/producers/range.h>
#include <sampleflow/consumers/auto_covariance_trace.h>


int main ()
{
  using SampleType = std::valarray<double>;

  SampleFlow::Producers::Range<SampleType> range_producer;

  const unsigned int AC_length = 10;
  SampleFlow::Consumers::AutoCovarianceTrace<SampleType> autocovariance(AC_length);
  autocovariance.connect_to_producer (range_producer);

  std::vector<SampleType> samples(1000, std::valarray<double>(1));
  for (unsigned int i=0; i<1000; ++i)
    samples[i][0] = (i % 2 == 0 ? 0. : 1.);

  range_producer.sample (samples);

  for (const auto v : autocovariance.get())
    std::cout << v << std::endl;
}
