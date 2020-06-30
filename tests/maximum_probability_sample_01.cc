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


// Test the maximum probability sample consumer by observing the
// auxiliary data from the MH producer.


#include <iostream>
#include <sampleflow/producers/metropolis_hastings.h>
#include <sampleflow/consumers/maximum_probability_sample.h>
#include <random>

using SampleType = int;

// Choose a probability distribution that is essentially zero on the
// left side of the real line, highest at x=0, and then decreases to
// the right. This will bring samples back to zero, but they can't go
// into negative territory.
double log_likelihood (const SampleType &x)
{
  return (x>=0 ? -x/100. : -1e10);
}



std::pair<SampleType,double> perturb (const SampleType &x)
{
  static std::mt19937 rng;
  // give "true" 1/2 of the time and
  // give "false" 1/2 of the time
  std::bernoulli_distribution distribution(0.5);

  if (distribution(rng) == true)
    return {x-1, 1.0};
  else
    return {x+1, 1.0};
}


int main ()
{

  SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;

  SampleFlow::Consumers::MaximumProbabilitySample<SampleType> map;
  map.connect_to_producer(mh_sampler);

  // Sample, starting at zero and perform a random walk to the left
  // and right using the proposal distribution. But, because the
  // probability distribution is essentially zero on the left, never
  // take a step to the left side.
  mh_sampler.sample ({0},
                     &log_likelihood,
                     &perturb,
                     20);
  std::cout << "MAP = " << map.get().first << std::endl;
}
