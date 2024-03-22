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


// If a producer object is destroyed before a consumer it is connected
// to, then we need to tell the consumer to disconnect
// itself. Otherwise, when the consumer itself is later destroyed, it
// tries to disconnect its connection to the producer, but the
// producer no longer exists.


#include <iostream>

#include <sampleflow/producers/range.h>
#include <sampleflow/consumers/stream_output.h>



int main ()
{
  using SampleType = double;

  // First create consumer, then producer
  SampleFlow::Consumers::StreamOutput<SampleType> stream_output(std::cout);
  SampleFlow::Producers::Range<SampleType> range_producer;

  stream_output.connect_to_producer(range_producer);

  const std::vector<double> samples = {1, 2, 3, 4, 5, 6};
  range_producer.sample (samples);

  // Here, we first destroy the producer and then the consumer. This
  // must not lead to a memory error.
}

