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


// Check the Metropolis-Hastings sampler using a continuous
// variable. This is the third example shown in the documentation of
// the class.

#include <iostream>
#include <sampleflow/producers/metropolis_hastings.h>
#include <sampleflow/filters/conversion.h>
#include <sampleflow/consumers/stream_output.h>
#include <valarray>
#include <random>
#include <cmath>

using SampleType = double;


double log_likelihood (const SampleType &x)
{
  return -(x-1)*(x-1);
}


std::pair<SampleType,double> perturb (const SampleType &x)
{
  static std::mt19937 rng;
  const double delta = 0.1;
  std::uniform_real_distribution<double> distribution(-delta,delta);

  return {x + distribution(rng), 1.0};
}


int main ()
{
  SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;

  SampleFlow::Consumers::StreamOutput<SampleType> stream_output(std::cout);
  stream_output.connect_to_producer(mh_sampler);

  mh_sampler.sample ({3},
                     &log_likelihood,
                     &perturb,
                     10000);
}
