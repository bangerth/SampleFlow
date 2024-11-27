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


// Check that the StreamOutput class can work for SampleType objects
// of type std::vector


#include <iostream>
#include <vector>

#ifndef SAMPLEFLOW_TEST_WITH_MODULE
#  include <sampleflow/producers/range.h>
#  include <sampleflow/consumers/stream_output.h>
#else
import SampleFlow;
#endif


int main ()
{
  using SampleType = std::vector<double>;

  SampleFlow::Producers::Range<SampleType> range_producer;

  SampleFlow::Consumers::StreamOutput<SampleType> stream_output(std::cout);
  stream_output.connect_to_producer(range_producer);

  const std::vector<SampleType> samples = {{1,10}, {2,12}, {3,13}, {4,14}, {5,15}, {6,16}};
  range_producer.sample (samples);
}

