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


// Test the Metropolis-Hastings producer with a diagonally elongated
// distribution.

#include <iostream>
#include <sampleflow/producers/metropolis_hastings.h>
#include <sampleflow/filters/conversion.h>
#include <sampleflow/consumers/mean_value.h>
#include <valarray>
#include <random>
#include <cmath>


using SampleType = std::valarray<double>;


double log_likelihood (const SampleType &x)
{
  std::valarray<double> mu = {0, 0};
  std::valarray<double> cov = {{1, 1.5}, {1.5, 3}};
  // FIXME: what's the correct way to do these matrix operations?
  return -0.5 * (ln(|cov|) + (x - mu)^T @ cov^-1 @ (x - mu) + k * ln(2 * pi));
}


std::pair<SampleType,double> perturb (const SampleType &x)
{
  // FIXME: how to sample MVN(x, cov)?
  static std::mt19937 rng;
  std::normal_distribution<double> distribution(0, 1);
  SampleType y = x;
  for (auto &el : y)
    y += distribution(rng);
  return {y, 1.0};
}


int main ()
{
  SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;
  SampleFlow::Producers::CovarianceMatrix<SampleType> cov_matrix;
  SampleFlow::Consumers::MeanValue<SampleType> mean_value;
  cov_matrix.connect_to_producer(mh_sampler);
  mean_value.connect_to_producer(mh_sampler);
  // Sample, starting at an asymmetric point, and creating 100,000 samples
  mh_sampler.sample({5, -1}, &log_likelihood, &perturb, 100000);
  std::cout << "Mean value = " << mean_value.get() << std::endl;
}
