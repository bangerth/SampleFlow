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


// Check the MeanValue consumer
// This test is similar to mean_01, but it uses an integer
// SampleType that is converted to float using the conversion
// filter.


#include <iostream>

#include <sampleflow/filters/conversion.h>
#include <sampleflow/producers/range.h>
#include <sampleflow/consumers/mean_value.h>


int main ()
{
  using SampleType = int;
  using ResultType = double;

  SampleFlow::Producers::Range<SampleType> range_producer;

  // Add filter to convert to double
  SampleFlow::Filters::Conversion<SampleType, ResultType> double_converter;
  double_converter.connect_to_producer(range_producer);

  // Add mean value consumer
  SampleFlow::Consumers::MeanValue<ResultType> mean_consumer;
  mean_consumer.connect_to_producer(double_converter);

  const auto samples = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  range_producer.sample(samples);

  // At this point, we have sampled all numbers between 1 and 9. Their
  // mean is clearly going to be 5 (or so we hope). Output whatever we
  // got:
  std::cout << "Mean = " << mean_consumer.get() << std::endl;
}

