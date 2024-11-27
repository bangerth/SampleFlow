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


// Like the covariance_matrix_{10,11} tests, but after the first 500
// samples, use the actual covariance matrix as the proposal
// distribution.


#include <iostream>
#include <fstream>
#include <random>

#include <eigen3/Eigen/Dense>

#ifndef SAMPLEFLOW_TEST_WITH_MODULE
#  include <sampleflow/producers/metropolis_hastings.h>
#  include <sampleflow/consumers/covariance_matrix.h>
#  include <sampleflow/consumers/mean_value.h>
#  include <sampleflow/consumers/count_samples.h>
#else
import SampleFlow;
#endif


using SampleType = Eigen::Vector2d;


double log_likelihood (const SampleType &x)
{
  const SampleType mu = {1,2};
  const SampleType y = x-mu;
  Eigen::Matrix2d C;
  C << 1, 0.1,
  0.1, 1;
  return -0.5 * (y.transpose()*(C.inverse()*y))(0,0);
}



// Use the non-adaptive proposal distribution from the _10 test for
// the first 500 samples
std::pair<SampleType,double> perturb_simple (const SampleType &x)
{
  static std::mt19937 rng;
  const double delta = 0.1;
  std::uniform_real_distribution<double> distribution(-delta,delta);

  SampleType y = x;
  for (unsigned int i=0; i<y.size(); ++i)
    y(i) += distribution(rng);

  return {y, 1.0};
}



// After a certain point, draw from something that considers the
// current covariance matrix.
std::pair<SampleType,double> perturb_adaptive (const SampleType &x,
                                               const Eigen::Matrix2d &C)
{
  const auto LLt = C.llt();

  static std::mt19937 rng;
  SampleType random_vector;
  for (unsigned int i=0; i<random_vector.size(); ++i)
    random_vector(i) = 2.4/std::sqrt(1.*x.size()) *
                       std::normal_distribution<double>(0,1)(rng);

  const SampleType y = x + LLt.matrixL() * random_vector;

  return {y, 1.0};
}



int main ()
{
  std::cout.precision(9);

  SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;

  SampleFlow::Consumers::MeanValue<SampleType> mean_value;
  mean_value.connect_to_producer(mh_sampler);

  SampleFlow::Consumers::CovarianceMatrix<SampleType> covariance_matrix;
  covariance_matrix.connect_to_producer(mh_sampler);

  SampleFlow::Consumers::CountSamples<SampleType> counter;
  counter.connect_to_producer(mh_sampler);

  mh_sampler.sample ({1,2},
                     &log_likelihood,
                     [&](const SampleType &x)
  {
    if (counter.get() < 1000)
      return perturb_simple(x);
    else
      return perturb_adaptive(x, covariance_matrix.get());
  },
  10000);

  std::cout << "Mean value:\n";
  std::cout << mean_value.get()(0) << std::endl;
  std::cout << mean_value.get()(1) << std::endl;

  std::cout << "Covariance matrix:\n";
  std::cout << covariance_matrix.get()(0,0) << std::endl;
  std::cout << covariance_matrix.get()(0,1) << std::endl;
  std::cout << covariance_matrix.get()(0,1) << std::endl;
  std::cout << covariance_matrix.get()(1,1) << std::endl;
}
