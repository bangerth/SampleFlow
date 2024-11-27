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


// Check the MeanValue consumer


#include <iostream>
#include <random>

#include "my_triangle.h"


#ifndef SAMPLEFLOW_TEST_WITH_MODULE
#  include <sampleflow/producers/metropolis_hastings.h>
#  include <sampleflow/consumers/mean_value.h>
#else
import SampleFlow;
#endif


double log_likelihood(const MyTriangle &sample)
{
  double a = std::abs(sample.side_lengths[0] - sample.side_lengths[1]);
  double b = std::abs(sample.side_lengths[0] - sample.side_lengths[2]);
  double c = std::abs(sample.side_lengths[1] - sample.side_lengths[2]);
  return -(a + b + c);
}


int main ()
{
  SampleFlow::Producers::MetropolisHastings<MyTriangle> mh_sampler;

  SampleFlow::Consumers::MeanValue<MyTriangle> mean_value;

  mean_value.connect_to_producer(mh_sampler);

  MyTriangle triangle_0({4, 6, 8});
  mh_sampler.sample (triangle_0, log_likelihood, perturb, 10);
  std::cout << "Mean = " << mean_value.get() << std::endl;
}
