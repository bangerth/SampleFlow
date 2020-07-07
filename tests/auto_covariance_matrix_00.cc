// ---------------------------------------------------------------------
//
// Copyright (C) 2019 by the SampleFlow authors.
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


// Check the AutoCovarianceMatrix consumer. Do so with a sequence of
// samples that consists of {-1, 1, -1, 1, ...}. For this case, we
// know what the long-term auto-covariances are: Since the average is
// zero, the auto-covariances are either +1 or -1. But for finite
// sequences, that's not quite true: first, the average up to sample k
// is not zero unless k is even; second, we divide by (n-l-1) instead
// of (n-l) when computing auto-covariances.
//
// Nonetheless, we can compute the exact values for each number of
// samples n and lag l. The formula for the autocovariance-trace is
//
//    1/(n-l-1)  \sum_{t=1}^{n-l}{(x_{t+l}-\bar{x}_n)^T(x_{t}-\bar{x}_n)}
//
// Here, \bar x_n = {0    if n=even
//                  {-1/n if n=odd
// and so
//       x_{t+l}  = { 1   if t+l=even
//                = {-1   if t+l=odd
// If now we only consider n=even, then
//       (x_{t+l}-\bar{x}_n)(x_{t}-\bar{x}_n)
//       = { 1  if sign(t+l)=sign(t)
//         { -1 if sign(t+l)!=sign(t)
//       = { 1  if l=even
//         { -1 if l=odd
// Then, this means that for n=even and l=even, we have that
//   AC(l) = (n-l)/(n-l-1)
// and if n=even and l=odd, we have
//   AC(l) = -(n-l)/(n-l-1)
//
// In the example below, we use n=20, so we should get for the auto-covariances:
//
//   AC(0) =  (20-0)/(20-0-1) =  20/19 = +1.05263
//   AC(1) = -(20-1)/(20-1-1) = -19/18
//   AC(2) =  (20-2)/(20-2-1) =  18/17
//   AC(3) = -(20-3)/(20-3-1) = -17/16
//   AC(4) =  (20-4)/(20-4-1) =  16/15
//   AC(5) = -(20-5)/(20-5-1) = -15/14
//   AC(6) =  (20-6)/(20-6-1) =  14/13
//   AC(7) = -(20-7)/(20-7-1) = -13/12
//   AC(8) =  (20-8)/(20-8-1) =  12/11
//   AC(9) = -(20-9)/(20-9-1) = -11/10 = -1.1
//
// The fact that AC(0) is also what the CovarianceMatrix computes is
// helpful here, since we can output that as well.

#include <iostream>
#include <valarray>

#include <sampleflow/producers/range.h>
#include <sampleflow/consumers/auto_covariance_matrix.h>
#include <sampleflow/consumers/covariance_matrix.h>
#include <sampleflow/consumers/stream_output.h>


template <typename T>
T trace (const boost::numeric::ublas::matrix<T> &A)
{
  T t = 0;
  for (unsigned int i=0; i<A.size1(); ++i)
    t += A(i,i);
  return t;
}



int main ()
{
  using SampleType = std::valarray<double>;

  SampleFlow::Producers::Range<SampleType> range_producer;

  const unsigned int max_lag = 10;
  SampleFlow::Consumers::AutoCovarianceMatrix<SampleType> autocovariance(max_lag);
  autocovariance.connect_to_producer (range_producer);

  SampleFlow::Consumers::CovarianceMatrix<SampleType> cov;
  cov.connect_to_producer (range_producer);

  SampleFlow::Consumers::StreamOutput<SampleType> stream_output(std::cout);
  stream_output.connect_to_producer(range_producer);

  std::vector<SampleType> samples(20, std::valarray<double>(1));
  for (unsigned int i=0; i<samples.size(); ++i)
    samples[i][0] = (i % 2 == 0 ? -1. : 1.);

  range_producer.sample (samples);

  std::cout.precision(16);
  std::cout << "Covariance matrix=" << cov.get()(0,0) << std::endl;

  std::cout << "Auto-covariances:" << std::endl;
  for (const auto v : autocovariance.get())
    std::cout << trace(v) << std::endl;
}
