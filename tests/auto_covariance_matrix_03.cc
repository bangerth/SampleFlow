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
// samples that is truly uncorrelated: We just pick numbers in the range
// [0,1] by random. For a sufficiently long such sequence, all
// autocorrelations (with lag>0) should be close to zero.


#include <iostream>
#include <valarray>
#include <vector>
#include <random>

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

  // Run the following workflow 3 times with increasing numbers of
  // samples
  for (auto N :
       {
         100, 1000, 10000
       })
    {
      SampleFlow::Producers::Range<SampleType> range_producer;

      const unsigned int max_lag = 10;
      SampleFlow::Consumers::AutoCovarianceMatrix<SampleType> autocovariance(max_lag);
      autocovariance.connect_to_producer (range_producer);

      std::vector<SampleType> samples(N, std::valarray<double>(1));
      std::mt19937 rng;
      std::uniform_real_distribution<> uniform_distribution(0,1);
      for (unsigned int i=0; i<samples.size(); ++i)
        samples[i][0] = uniform_distribution(rng);

      range_producer.sample (samples);

      std::cout << "Number of samples: " << N << std::endl;
      for (const auto v : autocovariance.get())
        std::cout << "  " << trace(v) << std::endl;

      // Now also compute the average of these numbers and output
      // that. Each of the samples are uncorrelated, so there is no
      // reason why the AC(1) should be any different from AC(10), and
      // we can just take the average. When computing the average,
      // skip AC(0) since that just measures the variance of samples
      double s = -trace(autocovariance.get()[0]);
      for (const auto v : autocovariance.get())
        s += trace(v);
      std::cout << "  average: " << s/max_lag << std::endl;
    }
}
