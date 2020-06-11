// ---------------------------------------------------------------------
//
// Copyright (C) 2019 by the SampleFlow authors.
//
// This file is part of the SampleFlow library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE.md at
// the top level directory of deal.II.
//
// ---------------------------------------------------------------------


// A clone of the metropolis_hastings_producer_01 test
// that uses double complex numbers instead of doubles.


#include <iostream>
#include <sampleflow/producers/metropolis_hastings.h>
#include <sampleflow/consumers/stream_output.h>
#include <valarray>
#include <random>
#include <cmath>
#include <complex>


using SampleType = std::complex<double>;


// Thinking of the complex numbers as samples from a 2D distribution,
// define a non-normalized distribution that increases to the right and up.
double log_likelihood (const SampleType &x)
{
  return std::norm(x) + 1;
}


// Thinking of the complex numbers as points in the plane,
// always move to the right and up when trying to find a new trial sample.
std::pair<SampleType,double> perturb (const SampleType &x)
{
  SampleType perturbation(1.0, 1.0);
  // Return both the new sample and the ratio of proposal distribution
  // probabilities. We're moving the sample to the right and up, so that ratio
  // is actually infinity, but we can lie about it for the purposes of
  // this test.
  return {x + perturbation, 1.};
}


int main ()
{

  SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;

  SampleFlow::Consumers::StreamOutput<SampleType> stream_output(std::cout);

  stream_output.connect_to_producer(mh_sampler);

  // Sample, starting at the origin. Because the probability distribution
  // increases increases into the first quadrant, and because trial samples
  // always lie to the right of and above the previous sample, the sampler
  // will accept every sample
  SampleType initial(0.0, 0.0);
  mh_sampler.sample ({initial},
                     &log_likelihood,
                     &perturb,
                     10);
}
