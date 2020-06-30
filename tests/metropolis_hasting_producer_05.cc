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


// This test is just like the _04 test, but we use a non-symmetric
// proposal distribution where with probability p we go from
// x->x'=x+1 and probability (1-p) from x->x'=x-1.
//
// To make this work, the perturb() function needs to return the ratio
// pi_proposal(x'|x)/pi_proposal(x|x'). Thus, if x'=x+1, then this
// ratio is p/(1-p) and if x'=x-1 it is (p-1)/p.
//
// The proposal distribution should not make a difference (in the
// limit of infinitely many samples) of the probability distribution
// of the samples drawn. Consequently, there should also be no
// difference in the mean value of all samples, which should thus
// continue to be around 2 as in the _04 test.

#include <iostream>
#include <sampleflow/producers/metropolis_hastings.h>
#include <sampleflow/filters/conversion.h>
#include <sampleflow/consumers/mean_value.h>
#include <valarray>
#include <random>
#include <cmath>

using SampleType = int;


// Use the probability distribution mentioned above
double log_likelihood (const SampleType &x)
{
  return -1. * x * std::log(2.);
}

// Go to the left or right with unequal probability. Wrap around if we
// get below 1 or beyond 100.
//
// For the computation of the second part of the return type, see the
// comment at the top of this file.
std::pair<SampleType,double> perturb (const SampleType &x)
{
  static std::mt19937 rng;
  // give "true" with probability p
  // give "false" with probability (1-p)
  const double p = 0.9;
  std::bernoulli_distribution distribution(p);

  const bool coin = distribution(rng) == true;
  const SampleType x_tilde = (coin
                              ?
                              x+1
                              :
                              x-1);
  const double proposal_probability_ratio = (coin ?
                                             p / (1-p) :
                                             (1-p) / p);

  const SampleType min = 1;
  const SampleType max = 100;

  if (x_tilde < min)
    return {max, proposal_probability_ratio};
  else if (x_tilde > max)
    return {min, proposal_probability_ratio};
  else
    return {x_tilde, proposal_probability_ratio};
}


int main ()
{
  SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;

  SampleFlow::Filters::Conversion<SampleType,double> conversion;
  conversion.connect_to_producer (mh_sampler);

  SampleFlow::Consumers::MeanValue<double> mean_value;
  mean_value.connect_to_producer (conversion);

  // Sample, starting at 1, and creating
  mh_sampler.sample ({10},
                     &log_likelihood,
                     &perturb,
                     100000);

  std::cout << "Mean value = " << mean_value.get() << std::endl;
}
