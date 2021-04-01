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

#ifndef SAMPLEFLOW_PRODUCERS_METROPOLIS_HASTINGS_H
#define SAMPLEFLOW_PRODUCERS_METROPOLIS_HASTINGS_H

#include <sampleflow/producer.h>
#include <sampleflow/scope_exit.h>
#include <sampleflow/types.h>

#include <random>
#include <functional>
#include <cmath>
#include <limits>

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
     * <h3>Choosing the perturb() function for continuous variables</h3>
     *
     * A difficult aspect of sampling is how to choose the proposal distribution
     * from which the `perturb()` function draws a proposed new sample that will
     * then either be accepted or rejected.
     *
     * In the examples above, we have chosen the new sample $\tilde x$ within
     * a certain expected distance from the previous sample $x$ by drawing a
     * random number between `-delta` and `delta`. An alternative would be
     * to draw a Gaussian-distributed perturbation of expected size `delta`:
     * @code
     * std::pair<SampleType,double>
     * perturb (const SampleType &x)
     * {
     *   static std::mt19937 rng;
     *   const double delta = 0.1;
     *   std::normal_distribution<double> perturbation(0, delta);
     *
     *   return {x + perturbation(rng), 1.0};
     * }
     * @endcode
     * In code such as this, the question is how to choose `delta`. Clearly, if
     * the probability distribution we want to draw from is very broad, we
     * should also choose `delta` to be large so that we can traverse the
     * distribution rapidly; if `delta` is too small, it will take a large
     * number of samples to visit all sides of the probability distribution
     * repeatedly so that we can expect to have reasonable approximations of
     * mean value and standard deviations of the distribution we are sampling
     * from.
     *
     * A rule of thumb is that one wants to choose `delta` in such a way that
     * the acceptance ratio of the Metropolis-Hastings sampler is approximately
     * equal to 0.234, see @cite Gelman97 and @cite Sherlock2009 for example.
     * The acceptance ratio of the sampler can be determined using the
     * Consumers::AcceptanceRatio class.
     *
     * The issue becomes more complicated if the sample type is a vector. In
     * this case, a typical `perturb()` function would look like this:
     * @code
     * std::pair<SampleType,double>
     * perturb (const SampleType &x)
     * {
     *   static std::mt19937 rng;
     *   const double delta = 0.1;
     *   std::normal_distribution<double> perturbation(0,delta);
     *
     *   SampleType x_tilde = x;
     *   for (unsigned int i=0; i<x.size(); ++i)
     *     x_tilde[i] += perturbation(rng);
     *
     *   return {x_tilde, 1.0};
     * }
     * @endcode
     * The problem here is that we have used the same `delta` for each component
     * of the vector, and this only makes sense if the standard deviation of
     * all components of the samples is approximately the same. In particular,
     * if the different components of the `SampleType` vector have different
     * physical units, this choice clearly will not make any sense. In those
     * cases, one needs to use different values for `delta` for each component
     * $i$ of the sample vector.
     *
     * That said, the recommendation to choose these `delta` values in such
     * a way to achieve an overall acceptance ratio of 0.234 remains in place.
     *
     *
     * <h3>Adaptive Metropolis for continuous variables</h3>
     *
     * One issue of the strategy described in the previous section arises
     * from the fact that it is not clear how we should choose the
     * multitude of `delta` values. It is conceivable that there are many
     * choices of the array of `delta`s that yield the same optimal acceptance
     * ratio of 0.234, but which one of those is best suited to explore the
     * target distribution is not a priori clear.
     *
     * A similar problem to the one mentioned above arises if the target
     * distribution we are trying to sample from is not aligned with
     * the coordinate axes. In this case, one would want to choose a proposal
     * distribution in the `perturb()` function that is aligned with the
     * target distribution, whereas the examples above all led to proposal
     * distributions that are axis-aligned.
     *
     * Both of these issues are addressed by "Adaptive Metropolis" samplers.
     * In these methods, one chooses a proposed sample based on a Gaussian
     * distribution centered at the current sample, and with a covariance
     * matrix equal to the covariance matrix of the samples so far
     * computed. There is an issue in the fact that in the beginning,
     * we do not yet have a good approximation of the true covariance
     * matrix, and as a consequence a typical strategy looks like this:
     * @code
     * // Use a non-adaptive proposal distribution for
     * // the first several samples.
     * std::pair<SampleType,double> perturb_simple (const SampleType &x)
     * {
     *   static std::mt19937 rng;
     *   const double delta = 0.1;
     *   std::uniform_real_distribution<double> distribution(-delta,delta);
     *
     *   SampleType y = x;
     *   for (unsigned int i=0; i<x.size(); ++i)
     *     y[i] = distribution(rng);
     *
     *   return {y, 1.0};
     * }
     *
     *
     *
     * // After a certain point, draw from something that considers the
     * // current covariance matrix.
     * std::pair<SampleType,double> perturb_adaptive (const SampleType &x,
     *                                                const Eigen::Matrix2d &C)
     * {
     *   const auto LLt = C.llt();
     *
     *   static std::mt19937 rng;
     *   SampleType random_vector;
     *   for (unsigned int i=0; i<random_vector.size(); ++i)
     *     random_vector[i] = 2.4/std::sqrt(1.*x.size()) *
     *                        std::normal_distribution<double>(0,1)(rng);
     *
     *   const SampleType y = x + LLt.matrixL() * random_vector;
     *
     *   return {y, 1.0};
     * }
     *
     * @endcode
     * The second of these functions draws a random variable from the
     * distribution $y \sim N\left(x,\frac{2.4^2}{d} C\right)$ where
     * $d$ is the dimensionality of the space of samples, and
     * $C$ needs to be (some sort of approximation of) the covariance
     * matrix of the target distribution. The method is explained
     * in substantial detail in @cite HST01. The factor $\frac{2.4^2}{d}$
     * with which the covariance matrix is multiplied is empirical and
     * discussed in detail in @cite GRG95.
     *
     * The implementation above makes use of the fact that to draw
     * a sample from the distribution $N\left(x,\frac{2.4^2}{d} C\right)$
     * is equivalent to the following steps:
     * - Decompose the covariance matrix $C=LL^T$ (i.e., into a Cholesky
     *   factorization).
     * - Create a vector $s \sim N(0,I_d)$ with independently distributed
     *   Gaussian entries of mean value zero and unit standard deviation.
     * - Multiply $t=Ls$ to obtain a vector $t\sim N(0,C)$.
     * - Multiply everything by $\frac{2.4^2}{d}$.
     * - Add it to the previous sample $x$.
     * The code above combines some of these steps, but the general idea
     * remains visible.
     *
     * Adaptive Metropolis thus has the advantage that we no longer have to
     * think about how to scale each of the components of the `delta`
     * array, nor how to deal with proposal distributions that are not
     * aligned with the axes -- the covariance matrix $C$ takes care of
     * all of this. (As explained in @cite HST01, one has to be careful
     * in cases where $C$ might be singular since then, the $LL^T$
     * decomposition may not exist. In those cases, one adds a small
     * multiple of the identity matrix to $C$.)
     *
     * There remains the question of where to obtain (an approximation of)
     * the covariance matrix. In general, we don't know it before we start
     * sampling. The Adaptive Metropolis method therefore takes the
     * covariance matrix obtained from the previous samples. Because this
     * is not available during the first few samples, the method
     * first runs a few samples with the `perturb_simple()` function
     * above, and then switches to the `perturb_adaptive()` function
     * that uses the current estimate of the covariance matrix. This
     * can be implemented as follows:
     * @code
     *   SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;
     *
     *   SampleFlow::Consumers::CovarianceMatrix<SampleType> covariance_matrix;
     *   covariance_matrix.connect_to_producer(mh_sampler);
     *
     *   SampleFlow::Consumers::CountSamples<SampleType> counter;
     *   counter.connect_to_producer(mh_sampler);
     *
     *   mh_sampler.sample ({1,2},
     *                      &log_likelihood,
     *                      [&](const SampleType &x) {
     *                         if (counter.get() < 1000)
     *                           return perturb_simple(x);
     *                         else
     *                           return perturb_adaptive(x, covariance_matrix.get());
     *                      },
     *                      10000);
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
         *   that only depends on the distance $\|\tilde x-x\|$ as is often
         *   done (e.g., uniformly in a disk of a certain radius around
         *   $x$, or using a Gaussian probability distribution centered at
         *   $x$), then the ratio returned as second argument is
         *   $\frac{\pi_\text{proposal}(\tilde x|x)}
         *           {\pi_\text{proposal}(x|\tilde x)}=1$.
         *   See the documentation of this class for examples of the
         *   @p perturb function.
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
         *
         * @note There are cases where the likelihood of a proposal sample,
         *   $\pi(\tilde x)$, is zero. This can happen if the `perturb()` function
         *   provided to this function proposes a sample location that is in an
         *   area for which the likelihood is zero. The question is what the
         *   `log_likelihood()` function should return in this case given that the
         *   logarithm of zero is not defined. In such cases, the function should
         *   either return `-std::numeric_limits<double>::max()` or
         *   `-std::numeric_limits<double>::infinity()` to indicate this case. The
         *   sampler will, if it encounters either of these two cases, reject
         *   the sample, *unless* the previous sample also had a zero probability
         *   as indicated by these special values. Of course, the previous sample
         *   had to come from somewhere, and because samples with zero probability
         *   are never accepted, the only case where this could have happened is if
         *   the *initial* sample already had a zero probability. In this situation,
         *   the code accepts the trial sample with a probability proportional to
         *   the proposal distribution ratio; this leads to a random walk that
         *   hopefully will eventually reach an area where samples have nonzero
         *   probabilities.
         */
        void
        sample (const OutputType &starting_point,
                const std::function<double (const OutputType &)> &log_likelihood,
                const std::function<std::pair<OutputType,double> (const OutputType &)> &perturb,
                const types::sample_index n_samples,
                const std::mt19937::result_type random_seed = {});
    };


    template <typename OutputType>
    void
    MetropolisHastings<OutputType>::
    sample (const OutputType &starting_point,
            const std::function<double (const OutputType &)> &log_likelihood,
            const std::function<std::pair<OutputType,double> (const OutputType &)> &perturb,
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
        rng.seed (random_seed);

      std::uniform_real_distribution<> uniform_distribution(0,1);

      OutputType current_sample         = starting_point;
      double     current_log_likelihood = log_likelihood (current_sample);

      // Loop over the desired number of samples
      for (types::sample_index i=0; i<n_samples; ++i)
        {
          // Obtain a new sample by perturbation and evaluate the
          // log likelihood for it
          std::pair<OutputType,double> trial_sample_and_ratio = perturb (current_sample);
          OutputType trial_sample = std::move(trial_sample_and_ratio.first);
          const double proposal_distribution_ratio = trial_sample_and_ratio.second;

          const double trial_log_likelihood = log_likelihood (trial_sample);

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
          //
          // There are two special cases to consider. If the probability of the
          // new sample is zero (i.e., the log likelihood is either -infinity or
          // -numeric_limits<double>::max()), then we never want to accept the
          // sample and there is no need to do any arithmetic on it. If, on
          // the other hand, the sample has a zero probability *and* the
          // previous probability was *also* zero, then we always want to accept
          // it so that we can do a random walk that hopefully at some point leads
          // to an area of nonzero probabilities.
          const bool trial_sample_has_zero_probability
            = ((trial_log_likelihood == -std::numeric_limits<double>::max())
               ||
               (trial_log_likelihood == -std::numeric_limits<double>::infinity()));
          const bool current_sample_has_zero_probability
            = ((current_log_likelihood == -std::numeric_limits<double>::max())
               ||
               (current_log_likelihood == -std::numeric_limits<double>::infinity()));

          bool repeated_sample;
          if (!(trial_sample_has_zero_probability && !current_sample_has_zero_probability)
              &&
              ((trial_sample_has_zero_probability && current_sample_has_zero_probability
                && (1. / proposal_distribution_ratio >= uniform_distribution(rng)))
               ||
               (trial_log_likelihood - std::log(proposal_distribution_ratio) > current_log_likelihood)
               ||
               (std::exp(trial_log_likelihood - current_log_likelihood) / proposal_distribution_ratio >= uniform_distribution(rng))))
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
