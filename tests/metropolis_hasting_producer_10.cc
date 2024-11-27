// ---------------------------------------------------------------------
//
// Copyright (C) 2020 by the SampleFlow authors.
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


// Test the Metropolis-Hastings producer with a custom sample class.


#include <iostream>
#include <valarray>
#include <random>
#include <cmath>

#ifndef SAMPLEFLOW_TEST_WITH_MODULE
#  include <sampleflow/producers/metropolis_hastings.h>
#  include <sampleflow/consumers/stream_output.h>
#else
import SampleFlow;
#endif

class MyTriangle

{
  public:

    std::valarray<double> side_lengths;

    MyTriangle(const std::valarray<double> lengths)
    {
      side_lengths = lengths;
    }

};


std::ostream &operator<<(std::ostream &os, const MyTriangle &tri)
{
  os << "Triangle: " << tri.side_lengths[0] << ", " << tri.side_lengths[1] << ", " << tri.side_lengths[2];
  return os;
}


std::pair<MyTriangle, double> perturb(const MyTriangle &sample)
{
  static std::mt19937 gen;
  std::normal_distribution<> d {0, 1};
  double side_a = sample.side_lengths[0] + d(gen);
  double side_b = sample.side_lengths[1] + d(gen);
  double side_c = sample.side_lengths[2] + d(gen);
  if (side_c > (side_a + side_b))
    side_c = side_a + side_b;
  if (side_c < std::abs(side_a - side_b))
    side_c = std::abs(side_a - side_b);
  MyTriangle result({side_a, side_b, side_c});
  // We are going to pretend that the change we are making to the triangle
  // is equally likely to a change going in the opposite direction, even
  // though this is not true because of the clipping that is happening
  // in the if statements. This is okay for the purposes of this test.
  return {result, 1.0};
}


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

  SampleFlow::Consumers::StreamOutput<MyTriangle> stream_output(std::cout);

  stream_output.connect_to_producer(mh_sampler);

  MyTriangle triangle_0({4, 6, 8});
  mh_sampler.sample (triangle_0, log_likelihood, perturb, 10);
}
