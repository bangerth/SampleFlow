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
    /**
     * An implementation of the Metropolis-Hastings algorithms sampling
     * from a probability distribution $\pi(x)$ defined on objects $x$ of type
     * `OutputType`. This class is therefore an implementation of the
     * "Producer" idiom in SampleFlow. For a discussion of the
     * Metropolis-Hastings method, see
     * [this wikipedia article](https://en.wikipedia.org/wiki/Metropolis%E2%80%93Hastings_algorithm).
     *
     * The Metropolis-Hastings algorithms, often abbreviates as the
     * "MH sampler" requires three inputs: a starting sample, a way to
     * evaluate the value of the (non-normalized) probability density
     * function for some value of the sample space, and a way to
     * "perturb" a given sample to get a new sample. In the context
     * of sampling algorithms, the non-normalized probability
     * density $\pi(x)$ is typically called the "likelihood";
     * for implementational stability, this class requires users
     * to provide a function that returns $\log(\pi(x))$ instead
     * of $\pi(x)$.
     *
     * The result of calling this class's sample() function is a sequence of
     * samples $x_k$ approximating $\pi(x)$ and that are passed to all
     * Consumer objects connected to the corresponding signal (by calling
     * Producer::connect_to_signal() or using Consumer::connect_to_producer())
     * one at a time. The AuxiliaryData object associated with each sample
     * $x_k$ stores two entries:
     * - An entry with name "relative log likelihood" of type
     *   `double` that stores $\log(\pi(x_k))$;
     * - An entry with name "sample is repeated" that stores a `bool`
     *   indicating whether the algorithm has chosen the current
     *   sample as an accepted trial sample (if `false`) or whether
     *   it is a repeated sample because the trial sample has been
     *   rejected (if `true`).
     */
    template <typename OutputType>
    class MetropolisHastings : public Producer<OutputType>
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
         *   $x$, returns a pair of values containing the following:
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
         *   that only depends on the distance $\|\tilde x-\x\|$ as is often
         *   done (e.g., uniformly in a disk of a certain radius around
         *   $x$, or using a Gaussian probability distribution centered at
         *   $x$), then the ratio returned as second argument is
         *   $\frac{\pi_\text{proposal}(\tilde x|x)}
         *           {\pi_\text{proposal}(x|\tilde x)}=1$.
         * @param[in] n_samples The number of (new) samples to be produced
         *   by this function. This is also the number of times the
         *   signal is called that notifies Consumer objects that a new
         *   sample is available.
         */
        void
        sample (const OutputType &starting_point,
                const std::function<double (const OutputType &)> &log_likelihood,
                const std::function<std::pair<OutputType,double> (const OutputType &)> &perturb,
                const unsigned int n_samples);
    };


    template <typename OutputType>
    void
    MetropolisHastings<OutputType>::
    sample (const OutputType &starting_point,
            const std::function<double (const OutputType &)> &log_likelihood,
            const std::function<std::pair<OutputType,double> (const OutputType &)> &perturb,
            const unsigned int n_samples)
    {
      std::mt19937 rng;
      std::uniform_real_distribution<> uniform_distribution(0,1);

      OutputType current_sample         = starting_point;
      double     current_log_likelihood = log_likelihood (current_sample);

      // Loop over the desired number of samples
      for (unsigned int i=0; i<n_samples; ++i)
        {
          // Obtain a new sample by perturbation and evaluate the
          // log likelihood for it
          std::pair<OutputType,double> trial_sample_and_ratio = perturb (current_sample);
          OutputType trial_sample = std::move(trial_sample_and_ratio.first);
          const double proposal_distribution_ratio = trial_sample_and_ratio.second;

          const double     trial_log_likelihood = log_likelihood (trial_sample);

          // Then see if we want to accept the sample. This happens if either
          // the new sample has a higher likelihood (which happens if and
          // only if the log likelihood of the new sample is larger than the
          // log likelihood of the old sample), or if the ratio of likelihoods
          // is larger than a randomly drawn number between zero and one. The
          // ratio of likelihoods equals the exp of the difference of
          // log likelihoods.
          //
          // If the sample is not accepted, then we simply stick with (i.e.,
          // repeat) the previous sample.
          bool repeated_sample;
          if ((trial_log_likelihood - std::log(proposal_distribution_ratio) > current_log_likelihood)
              ||
              (std::exp(trial_log_likelihood - current_log_likelihood) / proposal_distribution_ratio >= uniform_distribution(rng)))
            {
              current_sample         = trial_sample;
              current_log_likelihood = trial_log_likelihood;

              repeated_sample = false;
            }
          else
            repeated_sample = true;

          // Output the new sample (which may be equal to the old sample).
          this->issue_sample (current_sample,
          {
            {"relative log likelihood", boost::any(current_log_likelihood)},
            {"sample is repeated", boost::any(repeated_sample)}
          });
        }
    }

  }
}


#endif
