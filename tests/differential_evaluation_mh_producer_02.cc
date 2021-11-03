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


// A simple test for the DEMH producer. Check that we get the right
// number of samples if the requested number of samples is not a
// multiple of the number of chains.

#include <iostream>
#include <sampleflow/producers/differential_evaluation_mh.h>
#include <sampleflow/consumers/count_samples.h>
#include <random>
#include <cmath>

using SampleType = int;


// Use a constant probability equal to one.
double log_likelihood (const SampleType &x)
{
  return 0;
}

// Go to the left or right with equal probability. The details
std::pair<SampleType,double> perturb (const SampleType &x)
{
  static std::mt19937 rng;
  // give "true" 1/2 of the time and
  // give "false" 1/2 of the time
  std::bernoulli_distribution distribution(0.5);

  if (distribution(rng) == true)
    return {x-1, 1.};
  else
    return {x+1, 1. };
}


// Concoct some crossover function. It works on integers, so the
// following doesn't make much sense, but that doesn't matter either.
SampleType crossover(const SampleType &current_sample,
                     const SampleType &sample_a,
                     const SampleType &sample_b)
{
  return current_sample + (2.38 * sqrt(2)) * (sample_a - sample_b);
}



int main ()
{
  SampleFlow::Producers::DifferentialEvaluationMetropolisHastings<SampleType> mh_sampler;

  SampleFlow::Consumers::CountSamples<SampleType> counter;
  counter.connect_to_producer (mh_sampler);

  // Get 15 samples on four chains with a number of cross-overs that
  // never triggers. The point is that we need to stop half-way
  // through a full cycle through all chains
  mh_sampler.sample ({1, 5, 10, 12},
                     &log_likelihood,
                     &perturb,
                     &crossover,
                     1000,
                     15);

  std::cout << "Number of samples = " << counter.get() << std::endl;
}
