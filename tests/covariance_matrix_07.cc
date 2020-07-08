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
// is a complex-typed, real number:
// 1st dimension: (1 + 0i, 2 + 0i, 3 + 0i, 4 + 0i, 5 + 0i, 6 + 0i, 7 + 0i, 8 + 0i)
// 2nd dimension: (2 + 0i, 3 + 0i, 4 + 0i, 5 + 0i, 6 + 0i, 8 + 0i, 8 + 0i, 9 + 0i)
// This test should be essentiall equivalent to covariance_matrix_01,
// where the covariance matrix is
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
      el += SampleType(1, 0);

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

  mh_sampler.sample ({ComponentType(1, 0), ComponentType(2, 0)},
                     &log_likelihood,
                     &perturb,
                     8);

  std::cout << covariance_matrix.get()(0,0) << std::endl;
  std::cout << covariance_matrix.get()(0,1) << std::endl;
  std::cout << covariance_matrix.get()(1,0) << std::endl;
  std::cout << covariance_matrix.get()(1,1) << std::endl;
}
