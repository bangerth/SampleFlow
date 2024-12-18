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


// Like the connections_02 test, where we have a chain
//   range_producer >> c2 >> c3 >> c5 >> c7 >> stream_output;
// Building connections is a transitive property, and this test checks
// that setting parentheses works as expected.
//
// The connections_02 checks that this works as expected if we capture
// the resulting chain object. Herein, we discard it, and that
// dissolves the intermediate objects, which then breaks the chain
// between producer and consumer. As a consequence, the producer
// should not produce anything at all, regardless of how we set
// parentheses. This test checks that this is so, and that we don't
// run into segmentation faults or other problems with the
// disappearing objects.


#include <sstream>
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

  auto not_a_multiple_of = [](const SampleType &s, const SampleType &t)
  {
    return ! ((s>t) && (s % t == 0));
  };

  auto make_filter = [not_a_multiple_of](const unsigned int p)
  {
    return
      SampleFlow::Filters::Condition<SampleType>
      (std::function<bool (SampleType)>(std::bind(not_a_multiple_of, std::placeholders::_1, p)));
  };

  // Generate reference output and output it:
  std::ostringstream reference;
  {
    SampleFlow::Producers::Range<SampleType> range_producer;
    SampleFlow::Filters::Condition<SampleType>
    c2 (std::function<bool (SampleType)>(std::bind(not_a_multiple_of, std::placeholders::_1, 2))),
    c3 (std::function<bool (SampleType)>(std::bind(not_a_multiple_of, std::placeholders::_1, 3))),
    c5 (std::function<bool (SampleType)>(std::bind(not_a_multiple_of, std::placeholders::_1, 5))),
    c7 (std::function<bool (SampleType)>(std::bind(not_a_multiple_of, std::placeholders::_1, 7)));
    SampleFlow::Consumers::StreamOutput<SampleType> stream_output(reference);
    range_producer >> c2 >> c3 >> c5 >> c7 >> stream_output;
    range_producer.sample (std::views::iota(1,100));

    std::cout << reference.str();
  }

  // Now try some variations with parentheses and ensure that the output is the same
  {
    std::ostringstream o;

    SampleFlow::Producers::Range<SampleType> range_producer;
    SampleFlow::Consumers::StreamOutput<SampleType> stream_output(o);
    ((((range_producer >> make_filter(2)) >> make_filter(3)) >> make_filter(5)) >> make_filter(7)) >> stream_output;
    range_producer.sample (std::views::iota(1,100));

    std::cout << (o.str() == "" ? "OK" : "FAIL!") << std::endl;
  }

  {
    std::ostringstream o;

    SampleFlow::Producers::Range<SampleType> range_producer;
    SampleFlow::Consumers::StreamOutput<SampleType> stream_output(o);
    range_producer >> (make_filter(2) >> (make_filter(3) >> (make_filter(5) >> (make_filter(7) >> stream_output))));
    range_producer.sample (std::views::iota(1,100));

    std::cout << (o.str() == "" ? "OK" : "FAIL!") << std::endl;
  }

  {
    std::ostringstream o;

    SampleFlow::Producers::Range<SampleType> range_producer;
    SampleFlow::Consumers::StreamOutput<SampleType> stream_output(o);
    (range_producer >> make_filter(2)) >> (make_filter(3) >> make_filter(5)) >> (make_filter(7) >> stream_output);
    range_producer.sample (std::views::iota(1,100));

    std::cout << (o.str() == "" ? "OK" : "FAIL!") << std::endl;
  }

  {
    std::ostringstream o;

    SampleFlow::Producers::Range<SampleType> range_producer;
    SampleFlow::Consumers::StreamOutput<SampleType> stream_output(o);
    range_producer >> (make_filter(2) >> make_filter(3)) >> (make_filter(5) >> make_filter(7)) >> stream_output;
    range_producer.sample (std::views::iota(1,100));

    std::cout << (o.str() == "" ? "OK" : "FAIL!") << std::endl;
  }
}

