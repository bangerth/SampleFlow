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


// Check the CovarianceMatrix consumer with complex numbers.
// This test generates a sequence of 2D samples where each dimension
// is a complex-typed, imaginary number:
// 1st dimension: (0 + 1i, 0 + 2i, 0 + 3i, 0 + 4i, 0 + 5i, 0 + 6i, 0 + 7i, 0 + 8i)
// 2nd dimension: (0 + 2i, 0 + 3i, 0 + 4i, 0 + 5i, 0 + 6i, 0 + 7i, 0 + 8i, 0 + 9i)
// This test should produce the same results as covariance_matrix_01 
// (since the variance and covariance of imaginary numbers are equivalent
//  to that of real numbers), which are:
// [6, 6
//  6, 6]


#include <iostream>
#include <fstream>
#include <valarray>
#include <complex>

#include <sampleflow/producers/metropolis_hastings.h>
#include <sampleflow/consumers/covariance_matrix.h>

using ComponentType = std::complex<double>;
using SampleType = std::valarray<std::complex<double>>;

double log_likelihood (const SampleType &x)
{
  return 1;
}

std::pair<SampleType,double> perturb (const SampleType &x)
{
  SampleType y = x;

  for (auto &el : y)
    el += ComponentType(0, 1);

  // Return both the new sample and the ratio of proposal distribution
  // probabilities. We've moved the sample to the right, so that ratio
  // is actually infinity, but we can lie about it for the purposes of
  // this test.
  return {y, 1.0};
}

int main ()
{

  SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;

  SampleFlow::Consumers::CovarianceMatrix<SampleType> covariance_matrix;
  covariance_matrix.connect_to_producer(mh_sampler);

  mh_sampler.sample ({ComponentType(0, 1), ComponentType(0, 2)},
                     &log_likelihood,
                     &perturb,
                     8);

  std::cout << covariance_matrix.get()(0,0) << std::endl;
  std::cout << covariance_matrix.get()(0,1) << std::endl;
  std::cout << covariance_matrix.get()(1,0) << std::endl;
  std::cout << covariance_matrix.get()(1,1) << std::endl;
}
