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
// This particular test checks that we can compute the covariance
// matrix for scalar sample types. The problem being that for
// scalar types, we can't call sample.size() nor sample[i], but
// have to come up with some work-around.


#include <iostream>
#include <fstream>

#include <sampleflow/producers/range.h>
#include <sampleflow/consumers/mean_value.h>
#include <sampleflow/consumers/covariance_matrix.h>


using SampleType = double;



int main ()
{
  SampleFlow::Producers::Range<SampleType> range_producer;

  SampleFlow::Consumers::MeanValue<SampleType> mean_value;
  mean_value.connect_to_producer(range_producer);

  SampleFlow::Consumers::CovarianceMatrix<SampleType> covariance_matrix;
  covariance_matrix.connect_to_producer(range_producer);

  const std::vector<SampleType> samples = { 0, 1, 2, 3, 4 };
  range_producer.sample (samples);

  // At this point, we have sampled the integers from 0 to 4. The mean
  // value should be 2
  std::cout << "Mean value: "
            << mean_value.get() << std::endl;

  // We can also compute the covariance matrix, which should here
  // simply be a scalar:
  //   C = 1/(5-1) \sum (x-x*)(x-x*)^T
  //     = 1/4 ( (0-2)^2 + (1-2)^2 + (2-2)^2 + (3-2)^2 + (4-2)^2 )
  //     = 1/4 ( 4+1+0+1+4 )
  //     = 1/4 ( 10 )
  //     = 2.5
  std::cout << "Covariance matrix: [["
            << covariance_matrix.get()(0,0) << "]]"
            << std::endl;
}
