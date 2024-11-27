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


// A simple test for the Metropolis-Hasting producer using the first
// example syntax used in the documentation of the
// SampleFlow::Producers::MetropolisHastings class.


#include <iostream>
#include <random>
#include <cmath>

#ifndef SAMPLEFLOW_TEST_WITH_MODULE
#  include <sampleflow/producers/metropolis_hastings.h>
#  include <sampleflow/consumers/stream_output.h>
#else
import SampleFlow;
#endif

using SampleType = double;


// Use a (non-normalized) probability distribution that increases left
// to right.
double log_likelihood (const SampleType &x)
{
  return x+1;
}


// Always move to the right when trying to find a new trial sample.
std::pair<SampleType,double> perturb (const SampleType &x)
{
  // Return both the new sample and the ratio of proposal distribution
  // probabilities. We're moving the sample to the right, so that ratio
  // is actually infinity, but we can lie about it for the purposes of
  // this test.
  return {x+1, 1.};
}

int main ()
{

  SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;

  SampleFlow::Consumers::StreamOutput<SampleType> stream_output(std::cout);

  stream_output.connect_to_producer(mh_sampler);

  // Sample, starting at zero. Because the probability distribution
  // increases left to right, and because trial samples always lie to
  // the right of the previous sample, the sampler will accept every
  // sample and should return numbers from 1 to 10
  mh_sampler.sample ({0},
                     &log_likelihood,
                     &perturb,
                     10);
}
