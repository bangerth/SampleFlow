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


// Check the LastSample consumer


#include <iostream>

#ifndef SAMPLEFLOW_TEST_WITH_MODULE
#  include <sampleflow/producers/range.h>
#  include <sampleflow/consumers/last_sample.h>
#else
import SampleFlow;
#endif


int main ()
{
  using SampleType = double;

  SampleFlow::Producers::Range<SampleType> range_producer;

  SampleFlow::Consumers::LastSample<SampleType> last;
  last.connect_to_producer(range_producer);

  const auto samples = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  range_producer.sample (samples);

  // At this point, we have sampled all numbers between 1 and 9. The
  // last one should have been '9'. Output whatever we got:
  std::cout << "Last sample = " << last.get() << std::endl;
}

