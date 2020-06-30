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
// of type double


#include <iostream>

#include <sampleflow/producers/range.h>
#include <sampleflow/consumers/stream_output.h>



int main ()
{
  using SampleType = double;

  SampleFlow::Producers::Range<SampleType> range_producer;

  SampleFlow::Consumers::StreamOutput<SampleType> stream_output(std::cout);
  stream_output.connect_to_producer(range_producer);

  const std::vector<SampleType> samples = {1, 2, 3, 4, 5, 6};
  range_producer.sample (samples);
}


