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
     *
     *
     * <h3>Example 1: A discrete sample space</h3>
     *
     * Let's say you want to sample the throws of a dice that is weighted
     * so that the 3 happens with probability $p_3>\frac 16$, the opposite
     * side with the 4 with a probability $p_4<\frac 16$, and the
     * remaining four sides of the dice happen with probability
     * $p_i=\frac 14(1-p_3-p_4)$.
     *
     * You don't actually need a Metropolis-Hastings sampler for this
     * kind of problem. This is principally the case because the state
     * space is so small here (just 6 possible outcomes, and they are
     * easy to enumerate) and because we have an explicit formula for the
     * absolute probability with which each of these outcomes happens. But
     * we *can* use a Metropolis-Hastings sampler nonetheless. This is
     * how we would set this up:
     *
     * First, we need to state the data type we want to use for the samples.
     * We could presumably invent some `enum` for this, but we might as well
     * use `int` for this.
     *
     * Second, we need a function that describes a probability
     * for each possible outcome -- in fact, the natural log of these
     * probabilities. In practice, all we need is a non-normalized
     * probability, but here we have the normalized one (i.e, where the
     * probabilities all add up to one), and this is of course also fine
     * for our purposes. This is easy here:
     * @code
     * double
     * log_likelihood (const SampleType &x)
     * {
     *   const double p_3 = 0.5;
     *   const double p_4 = 0.05;
     *   const double p_i = (1-p_3-p_4)/4;
     *
     *   switch (x)
     *     {
     *     case 1: return std::log(p_i);
     *     case 2: return std::log(p_i);
     *     case 3: return std::log(p_3);
     *     case 4: return std::log(p_4);
     *     case 5: return std::log(p_i);
     *     case 6: return std::log(p_i);
     *     default:
     *           std::abort();
     *     }
     * }
     * @endcode
     * Note that in this case, we can simply enumerate the values of the
     * function. We shouldn't get to see any inputs other than those in
     * the enumerated range from 1 to 6, but if we should, that's clearly
     * a bug worth investigating -- so we abort the program.
     *
     * Third, we need a function that given a sample $x$ returns a
     * "trial sample" $\tilde x$ and the ratio of probabilities as
     * outlined in the documentation of the sample() function below.
     * Here, let us simply flip a coin that with equal
     * probabilities chooses $\tilde x=x+1$ or $\tilde x=x-1$, wrapping
     * around to stay in the range $1\ldots 6$. In this
     * case, the probability of landing at $\tilde x$ when starting
     * with $x$ is the same as the probability of landing at $x$ when
     * starting at $\tilde x$, and consequently
     * $\frac{\pi_\text{proposal}(\tilde x|x)}
     *       {\pi_\text{proposal}(x|\tilde x)} = 1$. This can
     * be encoded in the following function:
     * @code
     * std::pair<SampleType,double>
     * perturb (const SampleType &x)
     * {
     *   static std::mt19937 rng;
     *   static std::bernoulli_distribution distribution(0.5);
     *
     *   const bool coin = distribution(rng) == true;
     *   const SampleType x_tilde = (coin
     *                               ?
     *                               x+1
     *                               :
     *                               x-1);
     *   const SampleType min = 1;
     *   const SampleType max = 6;
     *
     *   if (x_tilde < min)
     *     return {max, 1};
     *   else if (x_tilde > max)
     *     return {min, 1};
     *   else
     *     return {x_tilde, 1};
     * }
     * @endcode
     *
     * With all of this, we can create 10,000 samples using the following snippet
     * of code:
     * @code
     *   SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;
     *
     *   SampleFlow::Consumers::StreamOutput<SampleType> stream_output(std::cout);
     *   stream_output.connect_to_producer(mh_sampler);
     *
     *   mh_sampler.sample ({3},
     *                      &log_likelihood,
     *                      &perturb,
     *                      100);
     * @endcode
     * The output for this (written to `std::cout`) will contain the value "3"
     * approximately 50 per cent of the time.
     *
     *
     * <h3>Example 2: Using a non-symmetric proposal distribution</h3>
     *
     * There are circumstances where it is useful to a proposal distribution
     * that is "non-symmetric", i.e., where
     * $\pi_\text{proposal}(\tilde x|x) \neq
     *  \pi_\text{proposal}(x|\tilde x)$. We won't go into the reasons
     * for this, but examples include continuous variables that must stay
     * positive (so we can't use symmetric intervals of fixed size
     * around $x$ to find $\tilde x$). In those cases, the ratio
     * $\frac{\pi_\text{proposal}(\tilde x|x)}
     *       {\pi_\text{proposal}(x|\tilde x)} \neq 1$. An example
     * would be if we replaced the proposal distribution above by one where
     * we choose $\tilde x=x+1$ with a probability $p$ that is different
     * from the probability $1-p$ with which we choose $\tilde x=x-1$.
     *
     * To account for this, we can use the following `perturb()` function:
     * @code
     * std::pair<SampleType,double>
     * perturb (const SampleType &x)
     * {
     *   static std::mt19937 rng;
     *   const double p = 0.9;
     *   std::bernoulli_distribution distribution(p);
     *
     *   const bool coin = distribution(rng) == true;
     *   const SampleType x_tilde = (coin
     *                               ?
     *                               x+1
     *                               :
     *                               x-1);
     *   const double proposal_probability_ratio = (coin ?
     *                                              p / (1-p) :
     *                                              (1-p) / p);
     *
     *   const SampleType min = 1;
     *   const SampleType max = 6;
     *
     *   if (x_tilde < min)
     *     return {max, proposal_probability_ratio};
     *   else if (x_tilde > max)
     *     return {min, proposal_probability_ratio};
     *   else
     *     return {x_tilde, proposal_probability_ratio};
     * }
     * @endcode
     * In this example, if `coin` is `true` (with probability $p$), then we go
     * from $x$ to $\tilde x=x+1$, i.e., $\pi_\text{proposal}(x+1|x)=p$. On
     * the other hand, going from one location one step to the left happens
     * with probability $1-p$, so
     * $\pi_\text{proposal}(x|x+1)=1-p$, and so if `coin` is `true`, then
     * $\frac{\pi_\text{proposal}(\tilde x|x)}
     *       {\pi_\text{proposal}(x|\tilde x)} = \frac{p}{1-p}$. On the
     * other hand, if `coin` is `false`, then we choose $\tilde x$ to the left
     * of $x$ (with probability $1-p$), and in that case
     * $\pi_\text{proposal}(x|x+1)=1-p$, and so if `coin` is `true`, then
     * $\frac{\pi_\text{proposal}(\tilde x|x)}
     *       {\pi_\text{proposal}(x|\tilde x)} = \frac{1-p}{p}$, explaining
     * the return values of the function.
     *
     * Using the same code snippets to create the sampler, we will again
     * generate roughly the same number of each of the possible outcomes
     * of the dice throw, even though we have used a non-symmetric proposal
     * distribution.
     *
     *
     * <h3>Example 3: A continuous sample variable</h3>
     *
     * Finally, let us draw real-valued samples from a Gaussian
     * distribution $\pi(x)=\frac 1C e^{-(x-1)^2}$, where $C$ is
     * a normalization factor to ensure that $\int \pi(x) \, dx = 1$.
     * Because the samples $x$ are real numbers, we now use
     * @code
     *   using SampleType = double;
     * @endcode
     *
     * For this, we can describe the probability to draw from using
     * the following function:
     * @code
     * double
     * log_likelihood (const SampleType &x)
     * {
     *   return -(x-1)*(x-1);
     * }
     * @endcode
     * Note that we don't care about the constant factor that normalizes
     * the probability distribution: The Metropolis-Hastings method only
     * needs ratios of likelihoods, and consequently does not care about
     * the normalization.
     *
     * The trial sample generation then happens using the following function
     * that draws $\tilde x$ randomly in the interval $[x-\delta,x+\delta]$:
     * @code
     * std::pair<SampleType,double>
     * perturb (const SampleType &x)
     * {
     *   static std::mt19937 rng;
     *   const double delta = 0.1;
     *   std::uniform_real_distribution<double> distribution(-delta,delta);
     *
     *   return {x + distribution(rng), 1.0};
     * }
     * @endcode
     *
     * Using the same code as before, we can generate samples from this
     * probability distribution. Specifically, this code generates 100
     * samples, starting with $x_0=3$:
     * @code
     *   SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;
     *
     *   SampleFlow::Consumers::StreamOutput<SampleType> stream_output(std::cout);
     *   stream_output.connect_to_producer(mh_sampler);
     *
     *   mh_sampler.sample ({3},
     *                      &log_likelihood,
     *                      &perturb,
     *                      100);
     * @endcode
     *
     *
     * <h3>Other sample types</h3>
     *
     * The examples above have used either `SampleType=int` or
     * `SampleType=double`. But the current class doesn't actually care what
     * type the samples have as long as the given `log_likelihood` function
     * returns a probability value for a given sample, and as long as the
     * `perturb` function generates a new sample and returns it along with
     * the ratio of proposal distribution likelihoods. Common types for samples
     * are vectors (e.g., represented by `std::valarray`), but one can also
     * draw samples from `std::complex` numbers, quaternions, graphs, or,
     * in essence, any other data type for which one can define the necessary
     * operations mentioned above.
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

      this->flush_consumers();
    }

  }
}


#endif
