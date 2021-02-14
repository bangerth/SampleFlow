// ---------------------------------------------------------------------
//
// Copyright (C) 2020 by the SampleFlow authors.
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


// A simple test for the DelayedRejection Metropolis Hastings producer,
// copied from delayed_rejection_mh_02.  This test samples from a normal
// distribution and calculates the mean.

#include <iostream>
#include <sampleflow/producers/delayed_rejection_mh.h>
#include <sampleflow/filters/conversion.h>
#include <sampleflow/consumers/mean_value.h>
#include <random>
#include <cmath>

using SampleType = double;


// Log likelihood function of the normal(5, 10)
double log_likelihood (const SampleType &x)
{
  return -3.222 + (-0.005 * std::pow(x - 5, 2));
}

// Perturb by adding realizations of standard normal.
std::pair<SampleType,double> perturb (const SampleType &x, const std::vector<SampleType> &)
{
  static std::mt19937 rng;
  std::normal_distribution<double> distribution(0, 1);
  const double perturbation = distribution(rng);
  const SampleType x_tilde = x + perturbation;
  return {x_tilde, 1.0};
}


int main ()
{
  SampleFlow::Producers::DelayedRejectionMetropolisHastings<SampleType> drmh_sampler;

  SampleFlow::Filters::Conversion<SampleType,double> conversion;
  conversion.connect_to_producer (drmh_sampler);

  SampleFlow::Consumers::MeanValue<double> mean_value;
  mean_value.connect_to_producer (conversion);

  // Sample, starting at 1, and creating 100,000 samples
  drmh_sampler.sample ({10},
                       &log_likelihood,
                       &perturb,
                       5,
                       100000);

  std::cout << "Mean value = " << mean_value.get() << std::endl;
}
