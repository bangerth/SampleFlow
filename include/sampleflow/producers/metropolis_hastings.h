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

#ifndef SAMPLEFLOW_PRODUCERS_METROPOLIS_HASTINGS_H
#define SAMPLEFLOW_PRODUCERS_METROPOLIS_HASTINGS_H

#include <sampleflow/producer.h>

#include <random>
#include <cmath>

namespace SampleFlow
{
  namespace Producers
  {
    template <typename OutputType>
    class MetropolisHastings : public Producer<OutputType>
    {
      public:
        void sample (const OutputType &starting_point,
                     const std::function<double (const OutputType &)> &log_likelihood,
                     const std::function<OutputType (const OutputType &)> &perturb,
                     const unsigned int n_samples);
    };


    template <typename OutputType>
    void
    MetropolisHastings<OutputType>::
    sample (const OutputType &starting_point,
            const std::function<double (const OutputType &)> &log_likelihood,
            const std::function<OutputType (const OutputType &)> &perturb,
            const unsigned int n_samples)
    {
      std::mt19937 rng;
      std::uniform_real_distribution<> uniform_distribution(0,1);

      OutputType current_sample         = starting_point;
      double     current_log_likelihood = log_likelihood (current_sample);

      this->issue_sample(current_sample,
      {{"relative likelihood", boost::any(current_log_likelihood)}});

      for (unsigned int i=0; i<n_samples; ++i)
        {
          const OutputType trial_sample         = perturb (current_sample);
          const double     trial_log_likelihood = log_likelihood (trial_sample);

          if ((trial_log_likelihood > current_log_likelihood)
              ||
              (std::exp(trial_log_likelihood - current_log_likelihood) >= uniform_distribution(rng)))
            {
              current_sample         = trial_sample;
              current_log_likelihood = trial_log_likelihood;
            }

          this->issue_sample (current_sample,
          {{"relative likelihood", boost::any(current_log_likelihood)}});
        }
    }

  }
}


#endif
