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


// Copy of metropolis_hasting_producer_12, but with adaptive sampling.
// Test the Metropolis-Hastings producer with a diagonally elongated
// distribution; here, we use a MVN with mean [0, 0] and covariance
// [[10, 0], [0, 1]].

#include <iostream>
#include <sampleflow/producers/metropolis_hastings.h>
#include <sampleflow/filters/conversion.h>
#include <sampleflow/consumers/covariance_matrix.h>
#include <sampleflow/consumers/mean_value.h>
#include <valarray>
#include <random>
#include <cmath>


using SampleType = std::valarray<double>;


double log_likelihood (const SampleType &x)
{
  double mu[2] = {0, 0};
  double cov[2][2] = {{10, 0}, {0, 1}};
  // return -0.5 * (ln(|cov|) + (x - mu)^T @ cov^-1 @ (x - mu) + k * ln(2 * pi));
  return -0.5 * ((x[0]-mu[0])*cov[0][0]*(x[0]-mu[0]) +
                 (x[0]-mu[0])*cov[0][1]*(x[1]-mu[1]) +
                 (x[1]-mu[1])*cov[1][0]*(x[0]-mu[0]) +
                 (x[1]-mu[1])*cov[1][1]*(x[1]-mu[1]));
}


std::pair<double[][], double[]> cholesky(A)
{
  double L[A.size()][A[0].size()];
  double D[A[0].size()];
  for(i = 0; i < A.size(); ++i)
    {
      for (j = 0; j < A[0].size(); ++j)
        {
          double partial_sum = 0;
          for (k = 0; k < (j - 1); ++k)
            partial_sum += L[j][k] ** 2 * D[k];
          D[j] = A[j][j] - partial_sum;
          if (i > j)
            {
              for (k = 0; k < (j - 1); ++k)
                partial_sum += L[i][k] * L[j][k] * D[k];
              L[i][j] = 1 / D[j] * (A[i][j] - partial_sum);
            }
          else if (i == j)
            L[i][j] = 1;
          else
            L[i][j] = 0;
        }
    }
  return {L, D};
}


std::pair<SampleType,double> perturb (const SampleType &x, const double[] &cov)
{
  std::pair<double[], double[]> decomp = cholesky(cov);
  double L[][] = decomp.first;
  double D[] = decomp.second;
  static std::mt19937 rng;
  SampleType y = x;
  for (i = 0; i < y.size(); ++i)
    {
      std::normal_distribution<double> distribution(x[i], D[i]);
      y[i] = L[i][j] * distribution(rng) * L[j][i];
    }
  return {y, 1.0};
}


int main ()
{
  SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;
  SampleFlow::Consumers::CovarianceMatrix<SampleType> cov_matrix;
  SampleFlow::Consumers::MeanValue<SampleType> mean_value;
  cov_matrix.connect_to_producer(mh_sampler);
  mean_value.connect_to_producer(mh_sampler);
  // Sample, starting at an asymmetric point, and creating 100,000 samples
  mh_sampler.sample(
    {5, -1},
    &log_likelihood,
    [](const SampleType &x){return perturb(x, cov_matrix.get())},
    100000
  );
  std::cout << "Mean value = " << mean_value.get()[0] << std::endl;
  std::cout << "Mean value = " << mean_value.get()[1] << std::endl;
}
