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


// Check the AutoCovarianceMatrix consumer. Do so with a sequence of
// samples that consists of {0, 1, 0, 1, ...}


#include <iostream>
#include <valarray>
#include <vector>

#include <eigen3/Eigen/Dense>

#ifndef SAMPLEFLOW_TEST_WITH_MODULE
#  include <sampleflow/producers/range.h>
#  include <sampleflow/consumers/auto_covariance_matrix.h>
#else
import SampleFlow;
#endif


template <typename T>
T trace (const Eigen::Matrix<T,Eigen::Dynamic,Eigen::Dynamic> &A)
{
  T t = 0;
  for (unsigned int i=0; i<A.rows(); ++i)
    t += A(i,i);
  return t;
}



int main ()
{
  using SampleType = std::valarray<double>;

  SampleFlow::Producers::Range<SampleType> range_producer;

  const unsigned int max_lag = 10;
  SampleFlow::Consumers::AutoCovarianceMatrix<SampleType> autocovariance(max_lag);
  autocovariance.connect_to_producer (range_producer);

  std::vector<SampleType> samples(1000, std::valarray<double>(1));
  for (unsigned int i=0; i<1000; ++i)
    samples[i][0] = (i % 2 == 0 ? 0. : 1.);

  range_producer.sample (samples);

  for (const auto v : autocovariance.get())
    std::cout << trace(v) << std::endl;
}
