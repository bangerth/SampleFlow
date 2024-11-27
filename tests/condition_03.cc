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


// This is a variation of the metropolis_hastings_03 test in which we
// output the aux data produced by the MH sampler. We add another
// filter that only lets samples through that are not repeated. As a
// consequence, the output of the test is a subset of the output of
// the metropolis_hastings_03 sampler.

#include <any>
#include <iostream>
#include <random>
#include <mutex>

#ifndef SAMPLEFLOW_TEST_WITH_MODULE
#  include <sampleflow/producers/metropolis_hastings.h>
#  include <sampleflow/filters/condition.h>
#  include <sampleflow/consumer.h>
#  include <sampleflow/connections.h>
#else
import SampleFlow;
#endif

using SampleType = int;

// Choose a probability distribution that is essentially zero on the
// left side of the real line, highest at x=0, and then decreases to
// the right. This will bring samples back to zero, but they can't go
// into negative territory.
double log_likelihood (const SampleType &x)
{
  return (x>=0 ? -x/100. : -1e10);
}

std::pair<SampleType,double> perturb (const SampleType &x)
{
  static std::mt19937 rng;
  // give "true" 1/2 of the time and
  // give "false" 1/2 of the time
  std::bernoulli_distribution distribution(0.5);

  if (distribution(rng) == true)
    return {x-1, 1.0};
  else
    return {x+1, 1.0};
}


// Like the StreamOutput class, but not only output the sample itself
// but also its properties (as recorded by the AuxiliaryData object)
template <typename InputType>
class MyStreamOutput : public SampleFlow::Consumer<InputType>
{
  public:
    MyStreamOutput (std::ostream &output_stream)
      :
      output_stream (output_stream)
    {}

    ~MyStreamOutput ()
    {
      this->disconnect_and_flush();
    }


    virtual
    void
    consume (InputType sample, SampleFlow::AuxiliaryData aux_data) override
    {
      std::lock_guard<std::mutex> lock(mutex);

      output_stream << "Sample: " << sample << std::endl;
      for (const auto &data : aux_data)
        {
          // Output the key of each pair:
          output_stream << "   " << data.first;

          // Then see if we can interpret the value via a known type:
          if (const bool *p = std::any_cast<bool>(&data.second))
            output_stream << " -> " << (*p ? "true" : "false") << std::endl;
          else if (const double *p = std::any_cast<double>(&data.second))
            output_stream << " -> " << *p << std::endl;
          else
            output_stream << std::endl;
        }
    }


  private:
    mutable std::mutex mutex;
    std::ostream &output_stream;
};


int main ()
{

  SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;

  SampleFlow::Filters::Condition<SampleType> filter(
    [](const SampleType &, const SampleFlow::AuxiliaryData &aux)
  {
    return (std::any_cast<bool>(aux.find("sample is repeated")->second)
            == false);
  });
  MyStreamOutput<SampleType> stream_output(std::cout);

  mh_sampler >> filter >> stream_output;

  // Sample, starting at zero and perform a random walk to the left
  // and right using the proposal distribution. But, because the
  // probability distribution is essentially zero on the left, never
  // take a step to the left side.
  mh_sampler.sample ({0},
                     &log_likelihood,
                     &perturb,
                     20);
}
