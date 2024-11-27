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


#ifndef SAMPLEFLOW_TESTS_MY_TRIANGLE_H
#define SAMPLEFLOW_TESTS_MY_TRIANGLE_H

#include <limits>
#include <valarray>


/**
 * A class that serves as a custom sample class that is not a vector
 * of some sort.
 */
class MyTriangle
{
  public:
    std::valarray<double> side_lengths;

    MyTriangle()
      :
      side_lengths ({std::numeric_limits<double>::signaling_NaN(),
                   std::numeric_limits<double>::signaling_NaN(),
                   std::numeric_limits<double>::signaling_NaN()
    })
    {}

    MyTriangle(const std::valarray<double> lengths)
    {
      side_lengths = lengths;
    }

};


inline
std::ostream &operator<<(std::ostream &os, const MyTriangle &tri)
{
  os << "Triangle: " << tri.side_lengths[0] << ", " << tri.side_lengths[1] << ", " << tri.side_lengths[2];
  return os;
}


inline
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


inline
MyTriangle &operator+=(MyTriangle &tri1, const MyTriangle &tri2)
{
  tri1.side_lengths[0] += tri2.side_lengths[0];
  tri1.side_lengths[1] += tri2.side_lengths[1];
  tri1.side_lengths[2] += tri2.side_lengths[2];
  return tri1;
}


inline
MyTriangle &operator-=(MyTriangle &tri1, const MyTriangle &tri2)
{
  tri1.side_lengths[0] -= tri2.side_lengths[0];
  tri1.side_lengths[1] -= tri2.side_lengths[1];
  tri1.side_lengths[2] -= tri2.side_lengths[2];
  return tri1;
}


inline
MyTriangle &operator*=(MyTriangle &tri1, const double b)
{
  tri1.side_lengths[0] *= b;
  tri1.side_lengths[1] *= b;
  tri1.side_lengths[2] *= b;
  return tri1;
}


inline
MyTriangle &operator/=(MyTriangle &tri1, const double b)
{
  tri1.side_lengths[0] /= b;
  tri1.side_lengths[1] /= b;
  tri1.side_lengths[2] /= b;
  return tri1;
}


inline
MyTriangle operator*(const MyTriangle &tri1, const double b)
{
  MyTriangle tri = tri1;
  tri *= b;
  return tri;
}


inline
MyTriangle operator*(const double b, const MyTriangle &tri1)
{
  return (tri1 * b);
}


#endif
