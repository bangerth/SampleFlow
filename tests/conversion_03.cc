
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


// Test the conversion filter with a custom type to be converted using
// a custom conversion function. This test uses the MyTriangle class
// from the metropolis_hasting_10 test.


#include <iostream>

#include <sampleflow/producers/metropolis_hastings.h>
#include <sampleflow/consumers/stream_output.h>
#include <sampleflow/filters/conversion.h>
#include <valarray>
#include <random>


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


double triangle_area(const MyTriangle &sample)
{
  // calculate the area using Heron's formula
  double s = sample.side_lengths.sum() / 2.0;
  double area = std::sqrt(s * (s - sample.side_lengths[0]) *
                              (s - sample.side_lengths[1]) *
                              (s - sample.side_lengths[2]));
  return area;
}

int main()
{
  SampleFlow::Producers::MetropolisHastings<MyTriangle> mh_sampler;

  SampleFlow::Filters::Conversion<MyTriangle, double> convert_to_double(triangle_area);
  convert_to_double.connect_to_producer(mh_sampler);

  SampleFlow::Consumers::StreamOutput<double> stream_output(std::cout);
  stream_output.connect_to_producer(convert_to_double);

  MyTriangle triangle_0({4, 6, 8});
  // we are starting met-has with a triangle that has side lengths of 4, 6, and 8;
  // the area of this triangle is:
  // semiperimeter s = (4 + 6 + 8) / 2 = 9
  // area = (s * (s - 4) * (s - 6) * (s - 8))^(1/2) = 11.6
  // so, we should see area values around there, at least starting out
  mh_sampler.sample(triangle_0, log_likelihood, perturb, 10);
}

