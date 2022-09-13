
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

#ifndef SAMPLEFLOW_PRODUCERS_DIFFERENTIAL_EVALUATION_MH_H
#define SAMPLEFLOW_PRODUCERS_DIFFERENTIAL_EVALUATION_MH_H

#include <sampleflow/producer.h>
#include <sampleflow/scope_exit.h>
#include <sampleflow/types.h>

#include <algorithm>
#include <future>
#include <random>
#include <cmath>

namespace SampleFlow
{
  namespace Producers
  {
    /**
     * An implementation of Differential Evaluation Metropolis Hastings from
     * "A Differential Evaluation Markov Chain Monte Carlo Algorithm for
     * Bayesian Model Updating," Sherri et. al.
     * This producer functions very similarly to the regular Metropolis Hastings
     * producer -- consult the documentation of that class for more details.
     * This class provides the same functionality as Metropolis Hastings but does
     * so over $N$ chains simultaneously, which can improve the rate of convergence. The
     * number of chains $N$ is given as an argument to the sample() function.
     */
    template <typename OutputType>
    class DifferentialEvaluationMetropolisHastings : public Producer<OutputType>
    {
      public:
        /**
         * The principal function of this class. Starting from the given
         * initial samples $x_{i, 0}$, it produces a multiple chains of
         * samples $x_{i, k}$ that are passed through the signal of the
         * base class to Consumer objects.
         *
         * @param[in] starting_points The initial samples of $x_{i, 0}$ for
         *   each chain $i$. The number of starting points also determines
         *   how many chains this algorithm will run.
         *   The algorithm needs to have at least three chains to make
         *   sense, and so the list of starting points must have at least
         *   3 elements.
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
         * @param[in] crossover A function that accepts the current sample of one chain, as well
         *   as two samples from other chains, as arguments and combines them to
         *   generate a new sample. This is the function that adds the
         *   "differential evaluation" or "differential evolution" to Metropolis
         *   Hastings. If the `OutputType` belongs to a vector space, the simple
         *   solution is
         *   ```
         *   return current_sample + gamma * (sample_a - sample_b);
         *   ```
         *   where `gamma` is a scaling parameter that is typically chosen as
         *   $\frac{2.38}{\sqrt{2d}}$, where $d$ is the dimension of the vector space.
         *   The reason for this factor is the same as in adaptive Metropolis
         *   methods: `sample_a` and `sample_b` are considered "typical"
         *   draws from the probability distribution we are sampling, so
         *   `sample_a - sample_b` is a quantity that can be expected to be
         *   about one or two standard deviation away from each other if we were in
         *   one space dimension. In higher dimensions, they are likely not
         *   going to be on opposite  sides of the mean of the distribution,
         *   but in quadrants (octants or, more generally, $n$-ants) that are
         *   further away -- more specifically, $\sqrt{d}$ times as far away.
         *   As a consequence, a random vector from the center of the distribution
         *   to an arbitrary sample can be expected to be of the form
         *   `(sample_a - sample_b) / sqrt{d}`; the 2.38 is then a further
         *   geometric factor that is explained in many papers on adaptive
         *   Metropolis algorithms.
         * @param[in] crossover_gap The number of iterations in between crossover
         *   iterations. In many implementations of the Differential Evaluation
         *   method, crossover is performed in each iteration. This corresponds to
         *   setting the `crossover_gap` argument to one.
         * @param[in] n_samples The number of (new) samples to be produced
         *   by this function. This is also the number of times the
         *   signal is called that notifies Consumer objects that a new
         *   sample is available.
         * @param[in] asynchronous_likelihood_execution If `true` (the default),
         *   then run the evaluation of the `log_likelihood` function asynchronously,
         *   that is on separate threads or tasks, for the different chains
         *   of the differential evaluation sampler. In each generation, this
         *   may then run as many log likelihood evaluations in parallel as there
         *   are chains. For this to work, the log likelihood function evaluators
         *   obviously have to be reentrant (that is, can be run multiple times
         *   in parallel).
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
         *
         * @note When `asynchronous_likelihood_execution` is set to `true`, the
         *   function evaluates the likelihoods for the current sample on all
         *   chains in parallel by calling `std::async` with `std::launch::async`
         *   as first argument. This may mean that the function evaluations are
         *   run on separate threads; you may therefore want to limit the number
         *   of chains to at most a small multiple of the number of processor
         *   cores available on the machine.
         */
        void
        sample (const std::vector<OutputType> starting_points,
                const std::function<double (const OutputType &)> &log_likelihood,
                const std::function<std::pair<OutputType,double> (const OutputType &)> &perturb,
                const std::function<OutputType (const OutputType &, const OutputType &, const OutputType &)> &crossover,
                const unsigned int crossover_gap,
                const types::sample_index n_samples,
                const bool asynchronous_likelihood_execution = true,
                const std::mt19937::result_type random_seed = {});
    };


    template <typename OutputType>
    void
    DifferentialEvaluationMetropolisHastings<OutputType>::
    sample (const std::vector<OutputType> starting_points,
            const std::function<double (const OutputType &)> &log_likelihood,
            const std::function<std::pair<OutputType,double> (const OutputType &)> &perturb,
            const std::function<OutputType (const OutputType &, const OutputType &, const OutputType &)> &crossover,
            const unsigned int crossover_gap,
            const types::sample_index n_samples,
            const bool asynchronous_likelihood_execution,
            const std::mt19937::result_type random_seed)
    {
      const typename std::vector<OutputType>::size_type n_chains = starting_points.size();
      assert (n_chains >= 3);
      // Make sure the flush_consumers() function is called at any point
      // where we exit the current function.
      Utilities::ScopeExit scope_exit ([this]()
      {
        this->flush_consumers();
      });

      std::mt19937 rng;
      if (random_seed != std::mt19937::result_type {})
        rng.seed (random_seed);

      // Initialize distribution for comparing to acceptance ratio
      std::uniform_real_distribution<> uniform_distribution(0,1);

      std::vector<OutputType> current_samples = starting_points;
      std::vector<double> current_log_likelihoods(n_chains);
      // Include another array to store new values so that all crossovers
      // can be performed with the previous set of samples
      std::vector<OutputType> next_samples = starting_points;

      // Loop over the desired number of samples, using an outer loop over
      // "generations" and an inner loop over the individual chains. We
      // exit from the inner loop when we have reached the desired number of
      // samples.
      for (types::sample_index generation=0; true; ++generation)
        {
          // A vector that will contain the std::future objects used to
          // hold results of either synchronous or asynchronous
          // evaluation.
          std::vector<std::future<bool>> chain_evaluation_results;
          chain_evaluation_results.reserve(n_chains);

          // Loop over the desired number of chains and set up the computation
          // of log likelihoods for new samples:
          for (typename std::vector<OutputType>::size_type chain = 0; chain < n_chains; ++chain)
            {
              // Skip if we have enough samples already:
              if (generation * n_chains + chain >= n_samples)
                break;

              // Determine trial sample and likelihood ratio; either from
              // crossover operation or regular perturbation
              std::pair<OutputType, double> trial_sample_and_ratio;

              // Perform crossover every crossover_gap iterations
              if ((generation % crossover_gap) == 0 && generation > 0)
                {
                  // Pick one of the other chains from which we want to draw from:
                  std::uniform_int_distribution<typename std::vector<OutputType>::size_type>
                  a_dist(0, n_chains - 2);

                  typename std::vector<OutputType>::size_type a = a_dist(rng);
                  if (a >= chain)
                    a += 1;
                  const OutputType trial_a = current_samples[a];

                  // Then the other chain to draw from:
                  std::uniform_int_distribution<typename std::vector<OutputType>::size_type>
                  b_dist(0, n_chains - 3);

                  typename std::vector<OutputType>::size_type b = b_dist(rng);
                  if (b >= std::max(a, chain))
                    b += 2;
                  else if (b >= std::min<typename std::vector<OutputType>::size_type>(a, chain))
                    b += 1;
                  const OutputType trial_b = current_samples[b];

                  // Combine trial a and trial b
                  const OutputType crossover_result = crossover(current_samples[chain], trial_a, trial_b);
                  trial_sample_and_ratio = perturb(crossover_result);
                }
              else
                trial_sample_and_ratio = perturb(current_samples[chain]);

              // Now that we have a trial sample, we need to evaluate the likelihood
              // on it. We can do this sequentially or in parallel, and for this, we
              // enclose everything into a lambda function that we can then execute
              // either right away, or via a std::async. The only thing we need to pay
              // attention to is that we create random numbers in a fixed order.
              // We do this by evaluating the random number generator upon creating
              // the lambda function.
              const auto task
              = [trial_sample = std::move(trial_sample_and_ratio.first),
                 proposal_distribution_ratio = trial_sample_and_ratio.second,
                 log_likelihood,
                 uniform_random_number = uniform_distribution(rng),
                 chain,
                 &current_samples,
                 &next_samples,
                 &current_log_likelihoods]()
              -> bool
              {
                const double trial_log_likelihood = log_likelihood (trial_sample);
                // Accept trial sample with probability equal to ratio of likelihoods;
                // (always accept if > 1)
                double acceptance_ratio = (std::exp(trial_log_likelihood - current_log_likelihoods[chain]) /
                proposal_distribution_ratio);
                bool accepted_sample = false;
                if (acceptance_ratio >= uniform_random_number)
                  accepted_sample = true;
                if (accepted_sample)
                  {
                    next_samples[chain] = trial_sample;
                    current_log_likelihoods[chain] = log_likelihood(trial_sample);
                  }
                else
                  next_samples[chain] = current_samples[chain];

                return accepted_sample;
              };


              // Now either execute the evaluator asynchronously or synchronously.
              // In the former case, get a future from the async call that we can
              // later use to wait for the evaluator. In the latter case,
              // create a packaged task that we can ask for a future, then
              // just execute the task synchronously.
              if (asynchronous_likelihood_execution)
                chain_evaluation_results.emplace_back (std::async(std::launch::async, task));
              else
                {
                  std::packaged_task<bool()> t(task);
                  chain_evaluation_results.emplace_back (t.get_future());
                  t();
                }
            }

          // We have either executed the log likelihood function for
          // all chains right away, or at least set up the asynchronous
          // execution. Now it is time to let the cows come in for
          // milking.
          //
          // In the following, we are in essence only issuing the new
          // samples. We could have done that in the lambda function
          // above, but that would have issued samples in unpredictable
          // orders. Doing this relatively cheap step here sequentially
          // guarantees a stable order.
          for (typename std::vector<OutputType>::size_type chain = 0; chain < n_chains; ++chain)
            {
              // Return if we have already generated the desired number of
              // samples. The ScopeExit object above also makes sure that
              // we flush the downstream consumers.
              if (generation * n_chains + chain >= n_samples)
                return;

              // Wait for the futures to be ready. Then output the new sample
              // (which may of course be equal to the old sample).
              assert (chain < chain_evaluation_results.size());
              const bool accepted_sample = chain_evaluation_results[chain].get();
              this->issue_sample (next_samples[chain],
              {
                {"relative log likelihood", boost::any(current_log_likelihoods[chain])},
                {"sample is repeated", boost::any(!accepted_sample)}
              });
            }

          current_samples = next_samples;
        }
    }

  }
}


#endif
