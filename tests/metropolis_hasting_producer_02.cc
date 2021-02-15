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


// A simple test for the Metropolis-Hasting producer using the simple
// example built in the beginning of the code. After that, it uses
// MaximumProbabilitySample consumer and prints out sample with the
// biggest likelihood.


#include <iostream>
#include <sampleflow/producers/metropolis_hastings.h>
#include <sampleflow/consumers/maximum_probability_sample.h>
#include <random>
#include <cmath>

using SampleType = double;

// Simplified version of functions log_likelihood and perturb. These
// two don't have any random parts to initialize testing: We always
// move to the right, and the probability distribution also increases
// left to right -- so we will accept every trial sample.
double log_likelihood (const SampleType &x)
{
  return x+1;
}


std::pair<SampleType,double> perturb (const SampleType &x)
{
  // Return both the new sample and the ratio of proposal distribution
  // probabilities. We're moving the sample to the right, so that ratio
  // is actually infinity, but we can lie about it for the purposes of
  // this test.
  return {x+1, 1.0};
}

int main ()
{

  SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;

  SampleFlow::Consumers::MaximumProbabilitySample<SampleType> MAP_point;
  MAP_point.connect_to_producer (mh_sampler);

  // Sample, starting at zero. Because the probability distribution
  // increases left to right, and because trial samples always lie to
  // the right of the previous sample, the sampler will accept every
  // sample and should return numbers from 1 to 10000
  mh_sampler.sample ({0},
                     &log_likelihood,
                     &perturb,
                     10000);

  // The highest likelihood should have been attained at the last
  // (right-most) sample, which is what the MAP_point object should
  // output:
  std::cout << MAP_point.get().first << std::endl;
}
