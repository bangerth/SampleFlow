// ---------------------------------------------------------------------
//
// Copyright (C) 2019 by the SampleFlow authors.
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


// Check the MeanValue consumer


#include <iostream>

#include <sampleflow/producers/range.h>
#include <sampleflow/consumers/mean_value.h>



int main ()
{
  using SampleType = double;

  SampleFlow::Producers::Range<SampleType> range_producer;

  SampleFlow::Consumers::MeanValue<SampleType> mean;
  mean.connect_to_producer(range_producer);

  const auto samples = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  range_producer.sample (samples);

  // At this point, we have sampled all numbers between 1 and 9. Their
  // mean is clearly going to be 5 (or so we hope). Output whatever we
  // got:
  std::cout << "Mean = " << mean.get() << std::endl;
}

