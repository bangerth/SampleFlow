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
//
// This test checks that with a filter that is created and then
// immediately goes away. This will disconnect the producer from the
// consumer and so lead to no output, but it should not result in a
// crash or similar.


#include <iostream>

#include <sampleflow/producers/range.h>
#include <sampleflow/filters/pass_through.h>
#include <sampleflow/consumers/stream_output.h>
#include <sampleflow/connections.h>



int main ()
{
  using SampleType = double;

  {
    // First create consumer, then producer
    SampleFlow::Consumers::StreamOutput<SampleType> stream_output(std::cout);
    SampleFlow::Producers::Range<SampleType> range_producer;

    // Create a filter as a temporary object. It is captured in a
    // Compound object, but since we don't capture the compound
    // object, the filter goes away again immediately and so no
    // connection remains between producer and consumer.
    {
      range_producer >> SampleFlow::Filters::PassThrough<SampleType>()
                     >> stream_output;
    }

    const std::vector<double> samples = {1, 2, 3, 4, 5, 6};
    range_producer.sample (samples);
  }

  std::cout << "OK" << std::endl;
}

