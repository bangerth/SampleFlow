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


#include <iostream>
#include <fstream>
#include <valarray>

#include <sampleflow/producers/range.h>
#include <sampleflow/consumers/mean_value.h>
#include <sampleflow/consumers/covariance_matrix.h>

using SampleType = std::valarray<double>;



int main ()
{
  SampleFlow::Producers::Range<SampleType> range_producer;

  SampleFlow::Consumers::MeanValue<SampleType> mean_value;
  mean_value.connect_to_producer(range_producer);

  SampleFlow::Consumers::CovarianceMatrix<SampleType> covariance_matrix;
  covariance_matrix.connect_to_producer(range_producer);

  const std::vector<SampleType> samples = { {0,0}, {1,0}, {1,1}, {0,1}};
  range_producer.sample (samples);

  // At this point, we have sampled the corners of a square. The mean
  // value should be the point (0.5,0.5)
  std::cout << "Mean value: "
            << mean_value.get()[0] << ' '
            << mean_value.get()[1] << std::endl;

  // We can also compute the covariance matrix:
  //   C = 1/(4-1) \sum (x-x*)(x-x*)^T
  //     = 1/3 { (-0.5,-0.5)(-0.5,-0.5)^T
  //            +( 0.5,-0.5)( 0.5,-0.5)^T
  //            +( 0.5, 0.5)( 0.5, 0.5)^T
  //            +(-0.5, 0.5)(-0.5, 0.5)^T }
  //     = 2/3 { ( 0.5,-0.5)( 0.5,-0.5)^T
  //            +( 0.5, 0.5)( 0.5, 0.5)^T }
  //     = 2/3 { [[1/4, -1/4], [-1/4, 1/4]]
  //            +[[1/4,  1/4], [ 1/4, 1/4]] }
  //     = 2/3 [[1/2, 0], [0, 1/2]]
  //     = [[1/3, 0], [0, 1/3]]
  std::cout << "Covariance matrix: [["
            << covariance_matrix.get()(0,0) << ','
            << covariance_matrix.get()(0,1) << "], ["
            << covariance_matrix.get()(0,1) << ','
            << covariance_matrix.get()(1,1) << "]]"
            << std::endl;
}
