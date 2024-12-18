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


// Check the Metropolis-Hastings sampler with the example of a
// dice. This is the first example shown in the documentation of the
// class.

#include <iostream>
#include <random>
#include <cmath>

#ifndef SAMPLEFLOW_TEST_WITH_MODULE
#  include <sampleflow/producers/metropolis_hastings.h>
#  include <sampleflow/filters/conversion.h>
#  include <sampleflow/consumers/stream_output.h>
#else
import SampleFlow;
#endif

using SampleType = int;


double log_likelihood (const SampleType &x)
{
  const double p_3 = 0.5;
  const double p_4 = 0.05;
  const double p_i = (1-p_3-p_4)/4;

  switch (x)
    {
      case 1:
        return std::log(p_i);
      case 2:
        return std::log(p_i);
      case 3:
        return std::log(p_3);
      case 4:
        return std::log(p_4);
      case 5:
        return std::log(p_i);
      case 6:
        return std::log(p_i);
      default:
        std::abort();
    }
}


std::pair<SampleType,double> perturb (const SampleType &x)
{
  static std::mt19937 rng;
  static std::bernoulli_distribution distribution(0.5);

  const bool coin = distribution(rng) == true;
  const SampleType x_tilde = (coin
                              ?
                              x+1
                              :
                              x-1);
  const SampleType min = 1;
  const SampleType max = 6;

  if (x_tilde < min)
    return {max, 1};
  else if (x_tilde > max)
    return {min, 1};
  else
    return {x_tilde, 1};
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
