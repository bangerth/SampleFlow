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


// Check the AutoCovarianceMatrix consumer. This test is a bit like
// the _02 test, but instead of a sequence of samples that consists of
// {0, 1, 0, 1, ...}, we use the sequence {{1,1}, {-1,1}, {-1,-1}, {1,-1}, ...}.
// For this sequence, we know that the mean value is {0,0} and we can
// again compute all covariances by hand.


#include <iostream>
#include <valarray>

#include <sampleflow/producers/range.h>
#include <sampleflow/consumers/auto_covariance_matrix.h>


int main ()
{
  using SampleType = std::valarray<double>;

  SampleFlow::Producers::Range<SampleType> range_producer;

  const unsigned int max_lag = 10;
  SampleFlow::Consumers::AutoCovarianceMatrix<SampleType> autocovariance(max_lag);
  autocovariance.connect_to_producer (range_producer);

  const std::valarray<double> base_samples[4]
  = {{1,1}, {-1,1}, {-1,-1}, {1,-1}};

  std::vector<SampleType> samples(1000);
  for (unsigned int i=0; i<1000; ++i)
    samples[i] = base_samples[i % 4];

  range_producer.sample (samples);

  for (const auto v : autocovariance.get())
    std::cout << v << std::endl;
}
