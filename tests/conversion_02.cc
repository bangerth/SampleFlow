
// ---------------------------------------------------------------------
//
// Copyright (C) 2020 by the SampleFlow authors.
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
#include <vector>

#ifndef SAMPLEFLOW_TEST_WITH_MODULE
#  include <sampleflow/consumers/stream_output.h>
#  include <sampleflow/filters/conversion.h>
#  include <sampleflow/producers/range.h>
#else
import SampleFlow;
#endif

/**
 * Example conversion function; converts a string to an integer
 * by getting the sum of the ASCII codes of each character.
 */
int string_ascii_sum(const std::string &sample)
{
  int sum = 0;
  for (const char &c : sample)
    {
      sum += c;
    }
  return sum;
}

int main()
{
  using SampleType = std::string;
  using ConvertedType = int;
  using ResultType = double;

  // Initialize a simple producer
  SampleFlow::Producers::Range<SampleType> producer;


  // Add a filter that converts the sample to an int, using
  // string_ascii_sum(sample).
  SampleFlow::Filters::Conversion<SampleType, ConvertedType> convert_to_int(string_ascii_sum);
  convert_to_int.connect_to_producer(producer);

  // Add a filter that converts to double.
  SampleFlow::Filters::Conversion<ConvertedType, ResultType> convert_to_double;
  convert_to_double.connect_to_producer(convert_to_int);

  // Add a simple consumer to view the output of the filter.
  SampleFlow::Consumers::StreamOutput<ResultType> consumer(std::cout);
  consumer.connect_to_producer(convert_to_double);

  // Generate samples
  const std::vector<SampleType> samples = {"Hello", "World"};
  producer.sample(samples);
}

