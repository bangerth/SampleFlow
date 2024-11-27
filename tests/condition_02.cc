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
//
// This test is in essence an implementation of the Sieve of
// Eratosthenes for computing primes.


#include <iostream>
#include <functional>
#include <ranges>

#ifndef SAMPLEFLOW_TEST_WITH_MODULE
#  include <sampleflow/producers/range.h>
#  include <sampleflow/filters/condition.h>
#  include <sampleflow/consumers/stream_output.h>
#  include <sampleflow/connections.h>
#else
import SampleFlow;
#endif

int main ()
{
  using SampleType = int;

  SampleFlow::Producers::Range<SampleType> range_producer;
  auto not_a_multiple_of = [](const SampleType &s, const SampleType &t)
  {
    return ! ((s>t) && (s % t == 0));
  };

  SampleFlow::Filters::Condition<SampleType>
  c2 (std::function<bool (SampleType)>(std::bind(not_a_multiple_of, std::placeholders::_1, 2))),
  c3 (std::function<bool (SampleType)>(std::bind(not_a_multiple_of, std::placeholders::_1, 3))),
  c5 (std::function<bool (SampleType)>(std::bind(not_a_multiple_of, std::placeholders::_1, 5))),
  c7 (std::function<bool (SampleType)>(std::bind(not_a_multiple_of, std::placeholders::_1, 7)));

  SampleFlow::Consumers::StreamOutput<SampleType> stream_output(std::cout);
  range_producer >> c2 >> c3 >> c5 >> c7 >> stream_output;

  range_producer.sample (std::views::iota(1,100));
}

