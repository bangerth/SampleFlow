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


// Check the DiscardFirstN filter


#include <iostream>

#ifndef SAMPLEFLOW_TEST_WITH_MODULE
#  include <sampleflow/producers/range.h>
#  include <sampleflow/filters/discard_first_n.h>
#  include <sampleflow/consumers/stream_output.h>
#else
import SampleFlow;
#endif


int main ()
{
  using SampleType = double;

  SampleFlow::Producers::Range<SampleType> range_producer;

  SampleFlow::Filters::DiscardFirstN<SampleType> discard_first_n(5);
  discard_first_n.connect_to_producer(range_producer);

  SampleFlow::Consumers::StreamOutput<SampleType> stream_output(std::cout);
  stream_output.connect_to_producer(discard_first_n);

  // Producer gives us 9 samples, but this filter should drop first 5 samples and just return 6,7,8 and 9.
  const auto samples = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  range_producer.sample (samples);
}
