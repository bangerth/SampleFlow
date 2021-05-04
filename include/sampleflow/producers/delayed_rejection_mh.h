
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

#ifndef SAMPLEFLOW_PRODUCERS_DELAYED_REJECTION_MH_H
#define SAMPLEFLOW_PRODUCERS_DELAYED_REJECTION_MH_H

#include <sampleflow/producer.h>
#include <sampleflow/scope_exit.h>
#include <sampleflow/types.h>

#include <random>
#include <cmath>

namespace SampleFlow
{
  namespace Producers
  {
    /**
     * An implementation of Delayed Rejection Metropolis Hastings, or DRAM.
     * This producer functions very similarly to the regular Metropolis Hastings
     * producer - consult the documentation of that class for more details.
     * This class provides the same functionality as Metropolis Hastings but with
     * the additional capability to perform delayed rejection. This requries a slightly
     * different perturb function that accepts both the current sample and a vector
     * of rejected samples as arguments. These rejected samples can be used for a more
     * sophisticated perturbation strategy.
     */
    template <typename OutputType>
    class DelayedRejectionMetropolisHastings : public Producer<OutputType>
    {
      public:
        /**
         * The principal function of this class. Starting from the given
         * initial sample $x_0$, it produces a sequence of samples $x_k$
         * that are passed through the signal of the base class to
         * Consumer objects.
         *
         * @param[in] starting_point The initial sample $x_0$.
         * @param[in] log_likelihood A function object that, when called
         *   with a sample $x$, returns $\log(\pi(x))$, i.e., the natural
         *   logarithm of the likelihood function evaluated at the sample.
         * @param[in] perturb A function object that, when given a sample
         *   $x$ as well as a (possibly empty) vector of rejected samples
         *   $\{y_1,\ldots,y_n\}$, returns a pair of values containing the
         *   following:
         *   <ol>
         *   <li> A different sample $\tilde x$ that is perturbed
         *     in some way from the given sample $x$.
         *   <li> The relative probability of the transition $x\to\tilde x$
         *     divided by the probability of the transition $\tilde x\to x$.
         *     Specifically, if the proposal distribution is given by
         *     $\pi_\text{proposal}(\tilde x|x)$, then the second element
         *     of the pair returned by the `perturb` argument is
         *     $\frac{\pi_\text{proposal}(\tilde x|x)}
         *           {\pi_\text{proposal}(x|\tilde x)}$.
         *   </ol>
         *   If samples are from some continuous space, say
         *   ${\mathbb R}^n$, then the perturbation function is often
         *   implemented by choosing $\tilde x$ from a neighborhood
         *   of $x$. If, for example, $\tilde x$ is chosen with a probability
         *   that only depends on the distance $\|\tilde x-x\|$ as is often
         *   done (e.g., uniformly in a disk of a certain radius around
         *   $x$, or using a Gaussian probability distribution centered at
         *   $x$), then the ratio returned as second argument is
         *   $\frac{\pi_\text{proposal}(\tilde x|x)}
         *           {\pi_\text{proposal}(x|\tilde x)}=1$.
         *   On the other hand, the whole point of *delayed rejection*
         *   algorithms is to choose the proposed sample $\tilde x$
         *   not only based on the last accepted sample $x$, but also taking
         *   into account that the samples $y_i$, if that is a non-empty set,
         *   have already been rejected; this might suggest that going from
         *   $x$ in the direction of the $y_i$ is not a profitable idea
         *   and that going in the *opposite* direction is what one should do.
         *   In this case, the second component of the returned object is
         *   more difficult to compute since
         *   @f[
         *     \pi_\text{proposal}(\tilde x|x) =
         *     \pi_\text{proposal}(\tilde x|x, \{y_1,\ldots,y_n\})
         *   @f]
         *   and care has to be taken to compute the ratio with
         *   $\pi_\text{proposal}(x|\tilde x)$. The paper @cite trias2009delayed
         *   provides a nice introduction.
         * @param[in] max_delays The maximum number of delayed rejection stages.
         *   if `max_delays==0`, then this class functions the same as a regular
         *   Metropolis-Hastings producer.
         * @param[in] n_samples The number of (new) samples to be produced
         *   by this function. This is also the number of times the
         *   signal is called that notifies Consumer objects that a new
         *   sample is available.
         * @param[in] random_seed If not equal to the default value, this optional
         *   argument is used to "seed" the random number generator. Using the
         *   default, or passing the same seed every time this function is called
         *   will then result in a reproducible sequence of samples. On the other
         *   hand, if you want to generate a different sequence of samples every
         *   time this function is called, then you want to pass this function
         *   a different seed every time it is called, for example by using the
         *   output of
         *   [std::random_device()](https://en.cppreference.com/w/cpp/numeric/random/random_device)
         *   as argument.
         */
        void
        sample (const OutputType &starting_point,
                const std::function<double (const OutputType &)> &log_likelihood,
                const std::function<std::pair<OutputType,double> (const OutputType &, const std::vector<OutputType> &)> &perturb,
                const unsigned int max_delays,
                const types::sample_index n_samples,
                const std::mt19937::result_type random_seed = {});
      private:
        /**
         * Recursively compute the acceptance ratio given the previous sample and
         * a vector of proposed samples (all but one that have already been rejected).
         *
         * @param[in] x The previous accepted sample and its associated log likelihood.
         * @param[in] y Vector of proposed samples and their corresponding log
         *   likelihoods; all but the last element have already been rejected.
         */
        double
        alpha_fn (const std::pair<OutputType,double> &x,
                  const std::vector<std::pair<OutputType,double>> &y);
    };



    template <typename OutputType>
    double
    DelayedRejectionMetropolisHastings<OutputType>::
    alpha_fn (const std::pair<OutputType,double> &x,
              const std::vector<std::pair<OutputType,double>> &y)
    {
      // Define the current delay stage as the number of rejected samples
      // (number of proposed samples minus 1)
      const unsigned int num_rejected_samples = y.size() - 1;
      // Get the current proposed sample from the list of all proposed samples
      const auto yi = y[num_rejected_samples];

      // Get the likelihood ratio of the previous sample to the current proposed sample
      const double likelihood_ratio = std::exp(yi.second - x.second);

      // In the case where no samples have been rejected yet, the acceptance ratio is calculated
      // the same as regular MH; we assume that the proposal is symmetric, so the acceptance
      // ratio is simply the likelihood ratio
      if (num_rejected_samples == 0)
        return likelihood_ratio;

      // Otherwise, recursively compute acceptance ratio
      double alpha = likelihood_ratio;
      for (int j = 1; j <= num_rejected_samples; ++j)
        {
          const std::vector<std::pair<OutputType,double>> num_yvec_rev(y.begin() + (num_rejected_samples - j),
                                                                       y.begin() + num_rejected_samples);
          const std::vector<std::pair<OutputType,double>> num_yvec(num_yvec_rev.rbegin(), num_yvec_rev.rend() + 1);
          const std::vector<std::pair<OutputType,double>> den_yvec(y.begin(), y.begin() + j);
          alpha *= (1 - alpha_fn(yi, num_yvec_rev)) /
                   (1 - alpha_fn(x, den_yvec));
        }
      return alpha;
    }


    template <typename OutputType>
    void
    DelayedRejectionMetropolisHastings<OutputType>::
    sample (const OutputType &starting_point,
            const std::function<double (const OutputType &)> &log_likelihood,
            const std::function<std::pair<OutputType,double> (const OutputType &, const std::vector<OutputType> &)> &perturb,
            const unsigned int max_delays,
            const types::sample_index n_samples,
            const std::mt19937::result_type random_seed)
    {
      // Make sure the flush_consumers() function is called at any point
      // where we exit the current function.
      Utilities::ScopeExit scope_exit ([this]()
      {
        this->flush_consumers();
      });

      std::mt19937 rng;
      if (random_seed != std::mt19937::result_type {})
        rng.seed(random_seed);

      std::uniform_real_distribution<> uniform_distribution(0,1);

      OutputType current_sample         = starting_point;
      double     current_log_likelihood = log_likelihood (current_sample);

      // Loop over the desired number of samples
      for (types::sample_index i=0; i<n_samples; ++i)
        {
          // Initialize a vector to store rejected samples, along with their
          // log likelihoods
          std::vector<std::pair<OutputType,double>> proposed_samples;
          // Initialize a bool to store whether a sample is accepted
          bool accepted_sample = false;
          // Delayed rejection loop
          for (unsigned int delay_stage = 0; delay_stage <= max_delays; ++delay_stage)
            {
              // Obtain a new sample by perturbation of the previous samples
              // (the previously last accepted one, along with the rejected ones)
              // and then evaluate its log likelihood.
              //
              // TODO: The current implementation discards the second part of the
              // information returned by the 'perturb' function. This is based on
              // the assumption that the proposal distributions used by 'perturb'
              // are symmetric, and that the second number equals 1.0. We should
              // generalize this.
              std::vector<OutputType> proposed_samples_only (proposed_samples.size());
              for (unsigned int i=0; i<proposed_samples.size(); ++i)
                proposed_samples_only[i] = proposed_samples[i].first;
              std::pair<OutputType,double> trial_sample_and_ratio = perturb(current_sample,
                                                                            proposed_samples_only);
              const OutputType trial_sample = std::move(trial_sample_and_ratio.first);
              const double trial_log_likelihood = log_likelihood(trial_sample);
              proposed_samples.push_back({trial_sample, trial_log_likelihood});

              const double acceptance_ratio = alpha_fn({current_sample, current_log_likelihood},
                                                       proposed_samples);
              if (acceptance_ratio > 1 || acceptance_ratio >= uniform_distribution(rng))
                accepted_sample = true;
              if (accepted_sample)
                {
                  current_sample         = trial_sample;
                  current_log_likelihood = trial_log_likelihood;
                  break;
                }
            }

          // Output the new sample (which may be equal to the old sample).
          this->issue_sample (current_sample,
          {
            {"relative log likelihood", boost::any(current_log_likelihood)},
            {"sample is repeated", boost::any(!accepted_sample)}
          });
        }

      this->flush_consumers();
    }

  }
}


#endif
