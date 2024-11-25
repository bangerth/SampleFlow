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
#include <sampleflow/consumers/covariance_matrix.h>
#include <sampleflow/consumers/mean_value.h>
#include <sampleflow/consumers/stream_output.h>
#include <valarray>
#include <random>
#include <cmath>


using SampleType = std::valarray<double>;


double log_likelihood (const SampleType &x)
{
  double mu[2] = {0, 0};
  // double cov[2][2] = {{10, 0}, {0, 1}};
  double cov_inv[2][2] = {{1.0/10.0, 0},{0, 1}};

  double phi = 3.1415 / 6;
  double rotation_matrix[2][2] = {{cos(phi), sin(phi)},
    {-sin(phi), cos(phi)}
  };
  double rotation_matrix_transpose[2][2] = {{cos(phi), -sin(phi)},
    {sin(phi), cos(phi)}
  };
  double RC[2][2] = {{
      rotation_matrix[0][0] *cov_inv[0][0] + rotation_matrix[0][1] *cov_inv[1][0],
      rotation_matrix[0][0] *cov_inv[0][1] + rotation_matrix[0][1] *cov_inv[1][1]
    },
    {
      rotation_matrix[1][0] *cov_inv[0][0] + rotation_matrix[1][1] *cov_inv[1][0],
      rotation_matrix[1][0] *cov_inv[0][1] + rotation_matrix[1][1] *cov_inv[1][1]
    }
  };
  double rotated_cov_inv[2][2] = {{
      RC[0][0] *rotation_matrix_transpose[0][0] + RC[0][1] *rotation_matrix_transpose[1][0],
      RC[0][0] *rotation_matrix_transpose[0][1] + RC[0][1] *rotation_matrix_transpose[1][1]
    },
    {
      RC[1][0] *rotation_matrix_transpose[0][0] + RC[1][1] *rotation_matrix_transpose[1][0],
      RC[1][0] *rotation_matrix_transpose[0][1] + RC[1][1] *rotation_matrix_transpose[1][1]
    }
  };

  double dev[2] = {x[0] - mu[0], x[1] - mu[1]};
  double dev_covinv[2] = {dev[0] *rotated_cov_inv[0][0] + dev[1] *rotated_cov_inv[0][1],
                          dev[0] *rotated_cov_inv[1][0] + dev[1] *rotated_cov_inv[1][1]
                         };
  double dev_covinv_dev = dev_covinv[0] * dev[0] + dev_covinv[1] * dev[1];
  return -0.5 * dev_covinv_dev;
}


std::pair<SampleType,double> perturb (const SampleType &x)
{
  static std::mt19937 rng;
  std::normal_distribution<double> distribution(0, 1);
  SampleType y = x;
  for (auto &el : y)
    el += distribution(rng);
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
  mh_sampler.sample({5, -1}, &log_likelihood, &perturb, 100000);
  std::cout << "Mean value = " << mean_value.get()[0] << std::endl;
  std::cout << "Mean value = " << mean_value.get()[1] << std::endl;
}
