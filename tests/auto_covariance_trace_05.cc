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


// Sample a Gaussian distribution using a MH sampler and compute the
// autocorrelation of samples. We don't know the exact values of the
// autocorrelation in this case, but we know that it should decay with
// distance between samples.


#include <iostream>
#include <fstream>
#include <valarray>
#include <vector>
#include <iomanip>
#include <random>

#include <eigen3/Eigen/Dense>

#include "tests.h"

#ifndef SAMPLEFLOW_TEST_WITH_MODULE
#  include <sampleflow/producers/metropolis_hastings.h>
#  include <sampleflow/consumers/acceptance_ratio.h>
#  include <sampleflow/consumers/auto_covariance_trace.h>
#else
import SampleFlow;
#endif

using SampleType = std::valarray<double>;

double log_likelihood (const SampleType &x)
{
  double sum = 0;
  for (unsigned int i=0; i<x.size(); ++i)
    sum += x[i] * x[i];

  return -sum;
}

std::pair<SampleType,double> perturb (const SampleType &x)
{
  // Perturb the current sample using a Gaussian distribution
  // around the current point with standard deviation 1.5.
  static std::mt19937 rng;
  SampleFlow::Testing::NormalDistribution<double> distribution(0., 1.5);

  SampleType y = x;
  for (auto &el : y)
    el += distribution(rng);

  return {y,1};
}

int main ()
{

  SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;

  SampleFlow::Consumers::AcceptanceRatio<SampleType> acceptance_ratio;
  acceptance_ratio.connect_to_producer(mh_sampler);;

  const unsigned int max_lag = 30;
  SampleFlow::Consumers::AutoCovarianceTrace<SampleType> autocovariance(max_lag);
  autocovariance.connect_to_producer (mh_sampler);

  mh_sampler.sample ({0,1},
                     &log_likelihood,
                     &perturb,
                     2000);

  // Due to division by not very nice numbers, we might expect answers
  // with long decimal numbers, in which we are not that much
  // interested in. So set precision 3 would help us to avoid
  // unnecessary problems.
  std::cout << std::fixed;
  std::cout << std::setprecision(3);


  // Now output the acceptance ratio and the autocorrelations
  std::cout << "# Acceptance ratio: "
            << acceptance_ratio.get() << std::endl;
  for (const auto v : autocovariance.get())
    std::cout << v << std::endl;
}
