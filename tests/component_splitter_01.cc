// ---------------------------------------------------------------------
//
// Copyright (C) 2020 by the SampleFlow authors.
//
// This file is part of the SampleFlow library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE.md at
// the top level directory of deal.II.
//
// ---------------------------------------------------------------------


// Test the component splitter filter


#include <iostream>
#include <valarray>

#include <sampleflow/producers/range.h>
#include <sampleflow/consumers/stream_output.h>
#include <sampleflow/filters/component_splitter.h>



int main ()
{
  using SampleType = std::valarray<double>;
  using ResultType = double;

  SampleFlow::Producers::Range<SampleType> range_producer;

  // add a ComponentSplitter filter to take only the second component of each sample
  SampleFlow::Filters::ComponentSplitter<SampleType> splitter(1);
  splitter.connect_to_producer(range_producer);

  SampleFlow::Consumers::StreamOutput<ResultType> stream_output(std::cout);
  stream_output.connect_to_producer(splitter);

  const std::vector<SampleType> samples = {{1,10}, {2,12}, {3,13}, {4,14}, {5,15}, {6,16}};
  range_producer.sample (samples);
}

