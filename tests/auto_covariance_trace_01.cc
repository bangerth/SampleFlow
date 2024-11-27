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


// Check the AutoCovarianceTrace consumer


#include <iostream>
#include <fstream>
#include <valarray>
#include <vector>
#include <iomanip>

#include <eigen3/Eigen/Dense>

#ifndef SAMPLEFLOW_TEST_WITH_MODULE
#  include <sampleflow/producers/metropolis_hastings.h>
#  include <sampleflow/consumers/auto_covariance_trace.h>
#else
import SampleFlow;
#endif


using SampleType = std::valarray<double>;

double log_likelihood (const SampleType &x)
{
  return 1;
}

std::pair<SampleType,double> perturb (const SampleType &x)
{
  SampleType y = x;

  for (auto &el : y)
    el += 1;

  // Return both the new sample and the ratio of proposal distribution
  // probabilities. We've moved the sample to the right, so that ratio
  // is actually infinity, but we can lie about it for the purposes of
  // this test.
  return {y, 1.0};
}

int main ()
{

  SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;

  const unsigned int max_lag = 10;
  SampleFlow::Consumers::AutoCovarianceTrace<SampleType> autocovariance(max_lag);
  autocovariance.connect_to_producer (mh_sampler);

  mh_sampler.sample ({0,1},
                     &log_likelihood,
                     &perturb,
                     20);

  // Due to division by not very nice numbers, we might expect answers
  // with long decimal numbers, in which we are not that much
  // interested in. So set precision 3 would help us to avoid
  // unnecessary problems.
  std::cout << std::fixed;
  std::cout << std::setprecision(3);

  // At this point, we have sampled two dimensional vectors - (1,2),
  // (2,3), ..., (20,21).  After doing calculations in statistical
  // software R, we expect result to be as it is at output file.
  // Whatever we got, we print it out
  for (const auto v : autocovariance.get())
    std::cout << v << std::endl;
}
