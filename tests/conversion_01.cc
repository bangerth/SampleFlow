
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


// Check the conversion filter


#include <iostream>

#include <sampleflow/consumers/stream_output.h>
#include <sampleflow/filters/conversion.h>
#include <sampleflow/producers/range.h>

int main()
{
  using SampleType = int;
  using ResultType = double;

  // Initialize a simple producer
  SampleFlow::Producers::Range<SampleType> producer;

  // Add a filter that converts to double.
  // Here, the filter is treated as a consumer.
  SampleFlow::Filters::Conversion<SampleType, ResultType> filter;
  filter.connect_to_producer(producer);

  // Add a simple consumer to view the output of the filter.
  // Here, the filter is treated as a producer.
  SampleFlow::Consumers::StreamOutput<ResultType> consumer(std::cout);
  consumer.connect_to_producer(filter);

  // Generate samples
  const std::vector<SampleType> samples = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  producer.sample(samples);
}

