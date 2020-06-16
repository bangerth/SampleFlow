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


// Check the AutoCovarianceTrace consumer. Do so with a sequence of
// samples that is truly uncorrelated: We just pick numbers in the range
// [0,1] by random. For a sufficiently long such sequence, all
// autocorrelations should be close to zero.


#include <iostream>
#include <valarray>
#include <random>

#include <sampleflow/producers/range.h>
#include <sampleflow/consumers/auto_covariance_trace.h>


int main ()
{
  using SampleType = std::valarray<double>;

  // Run the following workflow 3 times with increasing numbers of
  // samples
  for (auto N :
       {
         100, 1000, 10000
       })
    {
      SampleFlow::Producers::Range<SampleType> range_producer;

      const unsigned int AC_length = 10;
      SampleFlow::Consumers::AutoCovarianceTrace<SampleType> autocovariance(AC_length);
      autocovariance.connect_to_producer (range_producer);

      std::vector<SampleType> samples(N, std::valarray<double>(1));
      std::mt19937 rng;
      std::uniform_real_distribution<> uniform_distribution(0,1);
      for (unsigned int i=0; i<samples.size(); ++i)
        samples[i][0] = uniform_distribution(rng);

      range_producer.sample (samples);

      std::cout << "Number of samples: " << N << std::endl;
      for (const auto v : autocovariance.get())
        std::cout << "  " << v << std::endl;

      // Now also compute the average of these numbers and output
      // that. Each of the samples are uncorrelated, so there is no
      // reason why the AC(1) should be any different from AC(10), and
      // we can just take the average.
      double s=0;
      for (const auto v : autocovariance.get())
        s += v;
      std::cout << "  average: " << s/AC_length << std::endl;
    }
}
