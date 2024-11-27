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

#ifndef SAMPLEFLOW_TEST_WITH_MODULE
#  include <sampleflow/producers/metropolis_hastings.h>
#  include <sampleflow/consumers/mean_value.h>
#else
import SampleFlow;
#endif

class MyTriangle

{
  public:

    std::array<double, 3> side_lengths;

    MyTriangle()
      :
      side_lengths ({std::numeric_limits<double>::signaling_NaN(),
                   std::numeric_limits<double>::signaling_NaN(),
                   std::numeric_limits<double>::signaling_NaN()
    })
    {}

    MyTriangle(const std::array<double, 3> &lengths)
      :
      side_lengths (lengths)
    {}

};


std::ostream &operator<<(std::ostream &os, const MyTriangle &tri)
{
  os << "Triangle: " << tri.side_lengths[0] << ", " << tri.side_lengths[1] << ", " << tri.side_lengths[2];
  return os;
}


MyTriangle &operator+=(MyTriangle &tri1, const MyTriangle &tri2)
{
  tri1.side_lengths[0] += tri2.side_lengths[0];
  tri1.side_lengths[1] += tri2.side_lengths[1];
  tri1.side_lengths[2] += tri2.side_lengths[2];
  return tri1;
}


MyTriangle &operator-=(MyTriangle &tri1, const MyTriangle &tri2)
{
  tri1.side_lengths[0] -= tri2.side_lengths[0];
  tri1.side_lengths[1] -= tri2.side_lengths[1];
  tri1.side_lengths[2] -= tri2.side_lengths[2];
  return tri1;
}


MyTriangle &operator*=(MyTriangle &tri1, const double b)
{
  tri1.side_lengths[0] *= b;
  tri1.side_lengths[1] *= b;
  tri1.side_lengths[2] *= b;
  return tri1;
}


MyTriangle &operator/=(MyTriangle &tri1, const double b)
{
  tri1.side_lengths[0] /= b;
  tri1.side_lengths[1] /= b;
  tri1.side_lengths[2] /= b;
  return tri1;
}


MyTriangle operator*(const MyTriangle &tri1, const double b)
{
  MyTriangle tri = tri1;
  tri *= b;
  return tri;
}


MyTriangle operator*(const double b, const MyTriangle &tri1)
{
  return (tri1 * b);
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

  SampleFlow::Consumers::MeanValue<MyTriangle> mean_value;

  mean_value.connect_to_producer(mh_sampler);

  MyTriangle triangle_0({4, 6, 8});
  mh_sampler.sample (triangle_0, log_likelihood, perturb, 10);
  std::cout << "Mean = " << mean_value.get() << std::endl;
}
