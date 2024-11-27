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


// Check the CovarianceMatrix consumer with a specific list of input
// samples for which we can explicitly compute the covariance matrix
//
// This test is just like the _02 test except:
// * It uses the same samples 1000 times so that we actually have a
//   large number of samples.
// * Processes them in parallel


#include <iostream>
#include <fstream>
#include <valarray>
#include <vector>

#ifndef SAMPLEFLOW_TEST_WITH_MODULE
#  include <sampleflow/producers/range.h>
#  include <sampleflow/consumers/mean_value.h>
#  include <sampleflow/consumers/covariance_matrix.h>
#else
import SampleFlow;
#endif

using SampleType = std::valarray<double>;



int main ()
{
  SampleFlow::Producers::Range<SampleType> range_producer;

  SampleFlow::Consumers::MeanValue<SampleType> mean_value;
  mean_value.connect_to_producer(range_producer);

  SampleFlow::Consumers::CovarianceMatrix<SampleType> covariance_matrix;
  covariance_matrix.set_parallel_mode (SampleFlow::ParallelMode::asynchronous,
                                       8);
  covariance_matrix.connect_to_producer(range_producer);

  std::vector<SampleType> samples;
  for (unsigned int i=0; i<1000; ++i)
    for (const SampleType &s :
    {
      SampleType {0,0}, SampleType {1,0},
                 SampleType {1,1}, SampleType {0,1}
    })
  samples.push_back (s);

  // Now run the samples
  range_producer.sample (samples);

  // At this point, we have sampled the corners of a square. The mean
  // value should be the point (0.5,0.5)
  std::cout << "Mean value: "
            << mean_value.get()[0] << ' '
            << mean_value.get()[1] << std::endl;

  // We can also compute the covariance matrix:
  //   C = 1/(4000-1) \sum (x-x*)(x-x*)^T
  //     = 1/3999 { (-0.5,-0.5)(-0.5,-0.5)^T
  //               +( 0.5,-0.5)( 0.5,-0.5)^T
  //               +( 0.5, 0.5)( 0.5, 0.5)^T
  //               +(-0.5, 0.5)(-0.5, 0.5)^T }*1000
  //     = 2000/3999 { ( 0.5,-0.5)( 0.5,-0.5)^T
  //                  +( 0.5, 0.5)( 0.5, 0.5)^T }
  //     = 2000/3999 { [[1/4, -1/4], [-1/4, 1/4]]
  //                  +[[1/4,  1/4], [ 1/4, 1/4]] }
  //     = 2/3.999 [[1/2, 0], [0, 1/2]]
  //     = [[1/3.999, 0], [0, 1/3.999]]
  //     = [[0.2500625, 0], [0, 0.25006251]]
  std::cout << "Covariance matrix: [["
            << covariance_matrix.get()(0,0) << ", "
            << covariance_matrix.get()(0,1) << "], ["
            << covariance_matrix.get()(0,1) << ", "
            << covariance_matrix.get()(1,1) << "]]"
            << std::endl;
}
