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


// A simple test for the DEMC producer. This test
// samples integers between 1 and 100, from a probability
// distribution that has pi(x)=2^(-x). This happens to be an almost
// normalized probability distribution because
//
//    sum_{x=1}^{100} pi(x)
//    \approx sum_{x=1}^{\infty} pi(x)
//    =       sum_{x=1}^{\infty} 2^(-x)
//    =       1
//
// and we neglect the difference.
//
// For this case, we can compute the mean value to pretty accuracy:
//
//   \bar x \approx sum_{x=1}^{\infty} x pi(x)
//          =       2
//
// We use the Consumers::MeanValue to compute an approximation from
// 100,000 samples of this value. To do this, we need to convert the
// integer SampleType to double, so that we can compute mean values
// with floating point arithmetic.

#include <iostream>
#include <sampleflow/producers/differential_evaluation_mh.h>
#include <sampleflow/filters/conversion.h>
#include <sampleflow/consumers/mean_value.h>
#include <random>
#include <cmath>

using SampleType = int;


// Use the probability distribution mentioned above
double log_likelihood (const SampleType &x)
{
  return -1. * x * std::log(2.);
}

// Go to the left or right with equal probability. Wrap around if we
// get below 1 or beyond 100.
std::pair<SampleType,double> perturb (const SampleType &x)
{
  static std::mt19937 rng;
  // give "true" 1/2 of the time and
  // give "false" 1/2 of the time
  std::bernoulli_distribution distribution(0.5);

  const SampleType x_tilde = (distribution(rng) == true
                              ?
                              x-1
                              :
                              x+1);
  const SampleType min = 1;
  const SampleType max = 100;

  if (x_tilde < min)
    return {max, 1.0};
  else if (x_tilde > max)
    return {min, 1.0};
  else
    return {x_tilde, 1.0};
}

SampleType crossover(const SampleType &current_sample, const SampleType &sample_a, const SampleType &sample_b)
{
  return current_sample + (2.38 * sqrt(2)) * (sample_a - sample_b);
}

int main ()
{
  SampleFlow::Producers::DifferentialEvaluationMetropolisHastings<SampleType> mh_sampler;

  SampleFlow::Filters::Conversion<SampleType,double> conversion;
  conversion.connect_to_producer (mh_sampler);

  SampleFlow::Consumers::MeanValue<double> mean_value;
  mean_value.connect_to_producer (conversion);

  // Sample, starting at 1, and creating
  mh_sampler.sample ({1, 5, 10, 15, 25},
                     &log_likelihood,
                     &perturb,
                     &crossover,
                     10,
                     100000);

  std::cout << "Mean value = " << mean_value.get() << std::endl;
}
