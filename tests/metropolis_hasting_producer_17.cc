// ---------------------------------------------------------------------
//
// Copyright (C) 2021 by the SampleFlow authors.
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
// variable. Choose a case where one can get into areas of zero
// probability. We indicate this by returning
// -std::numeric_limits<double>::infinity(), in contrast to the _16
// test that returns ::max().
//
// Like the _16 test, we here start with a point that has zero
// probability.

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
  std::cout << "Evaluating at x=" << x << ": pi(x)=";
  if ((x>=0) && (x<=1))
    {
      std::cout << 1 << '\n';
      return 0;   // probability=1  =>  log(probability)=0
    }
  else
    {
      std::cout << 0 << '\n';
      return -std::numeric_limits<double>::max();
    }
}


std::pair<SampleType,double> perturb (const SampleType &x)
{
  static std::mt19937 rng;
  const double delta = 0.5;
  std::uniform_real_distribution<double> distribution(-delta,delta);

  return {x + distribution(rng), 1.0};
}


int main ()
{
  SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;

  SampleFlow::Consumers::StreamOutput<SampleType> stream_output(std::cout);
  stream_output.connect_to_producer(mh_sampler);


  // Start with an infeasible point. It's going to take a while to get
  // away from there, but once we find the feasible region, we stay there.
  mh_sampler.sample ({-1.5},
                     &log_likelihood,
                     &perturb,
                     100);
}
