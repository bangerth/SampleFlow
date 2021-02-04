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
// distribution.

#include <iostream>
#include <sampleflow/producers/metropolis_hastings.h>
#include <sampleflow/filters/conversion.h>
#include <sampleflow/consumers/covariance_matrix.h>
#include <sampleflow/consumers/mean_value.h>
#include <valarray>
#include <random>
#include <cmath>


using SampleType = std::valarray<double>;
using MatrixType = boost::numeric::ublas::matrix<double>;
using VectorType = boost::numeric::ublas::vector<double>;


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


MatrixType cholesky(const MatrixType &A)
{
  double L[2][2];
  double D[2];
  for (int i = 0; i < 2; ++i)
    {
      for (int j = 0; j < 2; ++j)
        {
          double partial_sum = 0;
          for (int k = 0; k < j; ++k)
          {
            partial_sum += pow(L[j][k], 2) * D[k];
          }
          D[j] = A(i, j) - partial_sum;
          if (i > j)
            {
              for (int k = 0; k < j; ++k)
              {
                partial_sum += L[i][k] * L[j][k] * D[k];
              }
              L[i][j] = 1 / D[j] * (A(i, j) - partial_sum);
            }
          else if (i == j)
            L[i][j] = 1;
          else
            L[i][j] = 0;
        }
    }
  MatrixType sqrtD(1, 2);
  sqrtD(0, 0) = sqrt(D[0]);
  sqrtD(0, 1) = sqrt(D[1]);
  MatrixType Lmatrix(2, 2);
  Lmatrix(0, 0) = L[0][0];
  Lmatrix(0, 1) = L[0][1];
  Lmatrix(1, 0) = L[1][0];
  Lmatrix(1, 1) = L[1][1];
  MatrixType result(2, 2);
  boost::numeric::ublas::prod(Lmatrix, sqrtD, result);
  return result;
}

//
//
//int cholesky_decompose(const MatrixType& A)
//{
//  namespace ublas = ::boost::numeric::ublas;
//  
//  const MatrixType& A_c(A);
//
//  const size_t n = A.size1();
//  
//  for (size_t k=0 ; k < n; k++) {
//        
//    double qL_kk = A_c(k,k) - ublas::inner_prod( ublas::project( row(A_c, k), ublas::range(0, k) ),
//                                          ublas::project( ublas::row(A_c, k), ublas::range(0, k) ) );
//    
//    if (qL_kk <= 0) {
//      return 1 + k;
//    } else {
//      double L_kk = ::std::sqrt( qL_kk );
//      
//      ublas::matrix_column<MatrixType> cLk(A, k);
//      ublas::project( cLk, ublas::range(k+1, n) )
//        = ( ublas::project( ublas::column(A_c, k), ublas::range(k+1, n) )
//            - ublas::prod( ublas::project(A_c, ublas::range(k+1, n), ublas::range(0, k)), 
//                    ublas::project(ublas::row(A_c, k), ublas::range(0, k) ) ) ) / L_kk;
//      A(k,k) = L_kk;
//    }
//  }
//  return 0;
//}


std::pair<SampleType, double> perturb (const SampleType &x, const MatrixType &cov)
{
  std::cout << cov.size1() << std::endl;
  std::mt19937 rng;
  std::normal_distribution<double> dist(0, 1);
  VectorType delta(2);
  delta(0) = dist(rng);
  delta(1) = dist(rng);

  MatrixType L = cholesky(cov);
  VectorType perturbation(2);
  boost::numeric::ublas::prod(L, delta, perturbation);

  SampleType y = x;
  y[0] = x[0] + perturbation(0);
  y[1] = x[1] + perturbation(1);

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
  [&cov_matrix](const SampleType &x)
  {
    return perturb(x, cov_matrix.get());
  },
  10000
  );
  std::cout << "Mean value = " << mean_value.get()[0] << std::endl;
  std::cout << "Mean value = " << mean_value.get()[1] << std::endl;
}
