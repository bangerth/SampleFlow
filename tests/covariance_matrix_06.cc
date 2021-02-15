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
// This test generates a sequence of complex numbers
// (1 + 1i, 2 + 2i, 3 + 3i, 4 + 4i, 5 + 5i, 6 + 6i, 7 + 7i, 8 + 8i)
// as samples. Because the samples only have a single complex component,
// the covariance matrix only has one element, which is the variance
// of the sequence. The variance of a complex number is equal to the sum
// of the variances of the real and imaginary components. The variance
// of the real component in this sequence is 6, and the variance of the
// imaginary component in this sequence is 6, so the variance of the sequence
// overall is 6+6=12.


#include <iostream>
#include <fstream>
#include <complex>

#include <sampleflow/producers/metropolis_hastings.h>
#include <sampleflow/consumers/covariance_matrix.h>

using SampleType = std::complex<double>;

double log_likelihood (const SampleType &x)
{
  return 1;
}

std::pair<SampleType,double> perturb (const SampleType &x)
{
  SampleType y = x;

  y += SampleType(1, 1);

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

  mh_sampler.sample (SampleType(0, 0),
                     &log_likelihood,
                     &perturb,
                     8);

  // The samples are the numbers [1 + 1i, 2 + 2i, 3 + 3i, 4 + 4i , 5 + 5i, 6 + 6i, 7 + 7i, 8 + 8i] so
  // the variance should be 6 + 6 = 12
  std::cout << covariance_matrix.get()(0,0) << std::endl;
}
