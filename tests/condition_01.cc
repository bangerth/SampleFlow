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


// Test the Filters::Condition class with a predicate that ignores
// the aux_data.


#include <iostream>

#include <sampleflow/producers/range.h>
#include <sampleflow/filters/condition.h>
#include <sampleflow/consumers/stream_output.h>
#include <sampleflow/connections.h>


int main ()
{
  using SampleType = int;

  SampleFlow::Producers::Range<SampleType> range_producer;
  auto evens_only = [](const SampleType &s)
  {
    return (s % 2 == 0);
  };
  SampleFlow::Filters::Condition<SampleType> condition (evens_only);
  SampleFlow::Consumers::StreamOutput<SampleType> stream_output(std::cout);
  range_producer >> condition >> stream_output;

  range_producer.sample (std::views::iota(1,7));
}

