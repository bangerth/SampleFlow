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


// Check the AutoCovarianceMatrix consumer.
//
// This test is the like the _01 and _02 tests, but now finally sets
// both components of the sample. This leads to a covariance matrix in
// which all entries are the same

#include <iostream>
#include <valarray>
#include <vector>

#ifndef SAMPLEFLOW_TEST_WITH_MODULE
#  include <sampleflow/producers/range.h>
#  include <sampleflow/consumers/auto_covariance_matrix.h>
#  include <sampleflow/consumers/covariance_matrix.h>
#  include <sampleflow/consumers/stream_output.h>
#else
import SampleFlow;
#endif


int main ()
{
  using SampleType = std::valarray<double>;

  SampleFlow::Producers::Range<SampleType> range_producer;

  const unsigned int max_lag = 10;
  SampleFlow::Consumers::AutoCovarianceMatrix<SampleType> autocovariance(max_lag);
  autocovariance.connect_to_producer (range_producer);

  SampleFlow::Consumers::CovarianceMatrix<SampleType> cov;
  cov.connect_to_producer (range_producer);

  SampleFlow::Consumers::StreamOutput<SampleType> stream_output(std::cout);
  stream_output.connect_to_producer(range_producer);

  std::vector<SampleType> samples(20, std::valarray<double>(2));
  for (unsigned int i=0; i<samples.size(); ++i)
    samples[i][0] = samples[i][1] = (i % 2 == 0 ? -1. : 1.);

  range_producer.sample (samples);

  std::cout.precision(16);
  std::cout << "Covariance matrix="
            << cov.get()(0,0) << ' '
            << cov.get()(0,1) << ' '
            << cov.get()(1,0) << ' '
            << cov.get()(1,1)
            << std::endl;

  std::cout << "Auto-covariances:" << std::endl;
  for (const auto v : autocovariance.get())
    std::cout << v(0,0) << ' ' << v(0,1) << ' ' << v(1,0) << ' ' << v(1,1) << std::endl;
}
