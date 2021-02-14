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

#ifndef SAMPLEFLOW_CONSUMERS_AUTOCOVARIANCETRACE_H
#define SAMPLEFLOW_CONSUMERS_AUTOCOVARIANCETRACE_H

#include <sampleflow/consumer.h>
#include <sampleflow/types.h>
#include <sampleflow/element_access.h>
#include <mutex>
#include <deque>

#include <eigen3/Eigen/Dense>


namespace SampleFlow
{
  namespace Consumers
  {
    /**
     * This is a Consumer class that implements computing the trace of the
     * running sample auto-covariance matrix function:
     * @f{align*}{
     *   \hat\gamma(l)
     *   &=
     *   \frac{1}{n-l-1} \sum_{t=1}^{n-l}{(x_{t+l}-\bar{x})^T(x_{t}-\bar{x})}
     *   \\
     *   &=
     *   \frac{1}{n-l-1} \text{trace}\left[
     *      \sum_{t=1}^{n-l}{(x_{t+l}-\bar{x}) (x_{t}-\bar{x})^T}
     *   \right]
     *   \\
     *   &=
     *   \text{trace}\; \gamma(l).
     * @f}
     * In other words, it calculates the trace of the (auto-)covariance matrix
     * $\gamma(l)$ of samples $x_{t+l}$ and $x_t$
     * with a lag between zero and $k$. The fraction in front is $n-l-1$ because
     * we are comparing $n-l$ pairs of samples, of which $n-l-1$ are statistically
     * independent. This is consistent with the definition of the covariance matrix
     * used by the CovarianceMatrix class.
     *
     * This class updates $\hat\gamma(l), l=0,1,2,3,\ldots,k$ for each new, incoming
     * sample. The value of the maximum lag $k$ is set in the constructor. The
     * auto-covariances $\hat\gamma(l)$ are only defined if the class has seen
     * $n>k+1$ samples.
     *
     * @note This class only calculates a quantity derived from the actual
     *   auto-covariance of the samples (which is a matrix), namely the
     *   trace. The trace is an indication for the size of a matrix and in the
     *   case of the auto-covariance, is used to see how quickly the
     *   correlation of samples in a Markov chain decreases -- in other words,
     *   how quickly $\hat \gamma(l)$ goes to zero as $l\to\infty$. As such,
     *   the quantity computed by this class is a cheap way to assess the decay
     *   of the correlation of successive samples. At
     *   the same time, the trace is not a *measure* on the auto-covariance
     *   matrices since matrices may be large even if their trace is small.
     *   For example, one could imagine processes in which the diagonal
     *   elements of $\gamma(l)$ are small but the off-diagonal entries
     *   are large. In those cases, $\hat\gamma(l)=\text{trace}\;\hat\gamma(l)$
     *   will be small even though the actual auto-covariance matrices are
     *   large. A more rigorous approach is therefore to use the
     *   AutoCovarianceMatrix class to compute the matrices $\gamma(l)$
     *   and to compute some kind of norm of these matrices; the downside is,
     *   of course, that computing $\gamma(l)$ is substantially more expensive
     *   than computing the quantity $\hat\gamma(l)$ used here.
     *
     * @note In the case of scalar data types, the (auto-)covariance matrices
     *   are also just numbers. In those cases, the results of the current
     *   AutoCovarianceTrace and the AutoCovarianceMatrix classes are the
     *   same, though the latter returns $1\times 1$ matrices whereas the
     *   current class just returns numbers for each lag $l$.
     *
     *
     * <h3> Algorithm </h3>
     *
     * The approach to deriving an update formula for $\gamma(l)$ is the
     * same as for many other classes, and similar to the one used in the
     * MeanValue class.
     * In the following, let us only consider the case when we have already seen
     * $n\ge k$ samples, so that all of the $\hat\gamma(l)$ can actually be computed.
     *
     * Let us expand the formula above and then denote some of its parts as $\hat\alpha$
     * and $\hat\beta$:
     * @f{align*}{
     *   \hat\gamma(l)
     *   &=
     *   \frac{1}{n-l-1}\sum_{t=1}^{n-l}{(x_{t+l}-\bar{x}_n)^T(x_{t}-\bar{x}_n)}
     * \\&=
     *   \underbrace{\frac{1}{n-l-1}\sum_{t=1}^{n-l}{x_{t+l}^T x_{t}}}_{\hat\alpha_n(l)}
     *   -
     *   \bar{x}_n^T
     *   \underbrace{\left[ \frac{1}{n-l-1}\sum_{t=1}^{n-l}(x_{t+l}+x_{t}) \right]}_{\hat\beta_n(l)}
     *   +
     *   \frac{n-l}{n-l-1}\bar{x}_n^T \bar{x}_n
     * \\&=
     *   \hat\alpha_n(l)-\bar{x}_n^T \hat \beta_n(l)+\frac{n-l}{n-l-1} \bar{x}_n^T \bar{x}_n
     * \\&=
     *   \hat\alpha_n(l)-\bar{x}_n^T \hat \beta_n(l)+\left(1+\frac{1}{n-l-1}\right) \bar{x}_n^T \bar{x}_n.
     * @f}
     *
     * For each new sample, we then need to update the scalars $\hat\alpha_{n+1}(l)$,
     * vectors $\hat\beta_{n+1}(l)$, and sample mean $\bar{x}_{n+1}$. The
     * principle of updating $\bar x_{n+1}$ is equivalent to what the MeanValue
     * class does. For $\hat\alpha$, we use that
     * @f{align*}{
     *  \hat\alpha_{1}(l) = \cdots = \hat\alpha_{l+1}(l)
     *  &= 0,
     *  \\
     *  \hat\alpha_{l+2}(l)
     *  &=
     *  \sum_{t=1}^2 x_{t+l}^T x_t
     *  \\
     *  &=
     *  x_{1+l}^T x_1 + x_{2+l}^T x_2,
     *  \\
     *  \hat\alpha_{n+1}(l)
     *  &=
     *  \frac{1}{n-l}\sum_{t=1}^{n+1-l}{x_{t+l}^T x_{t}}
     *  \qquad\qquad\qquad (\text{for}\, n\ge l+2)
     *  \\
     *  &=
     *  \frac{1}{n-l}\left[\sum_{t=1}^{n-l}{x_{t+l}^T x_{t}} + x_{n+1}^T x_{n+1-l} \right]
     *  \\
     *  &=
     *  \frac{1}{n-l}\left[(n-l-1)\hat\alpha_n(l) + x_{n+1}^T x_{n+1-l} \right]
     *  \\
     *  &=
     *  \frac{n-l-1}{n-l} \alpha_n(l) + \frac{1}{n-l} x_{n+1}^T x_{n+1-l}.
     *  \\
     *  &=
     *  \hat\alpha_n(l) - \frac{1}{n-l} \hat\alpha_n(l) + \frac{1}{n-l} x_{n+1}^T x_{n+1-l}.
     * @f}
     * Similarly, for $\hat\beta$ we can use that
     * @f{align*}
     *  \hat\beta_{1}(l) = \cdots = \hat\beta_{l+1}(l)
     *  &= 0,
     *  \\
     *  \hat\beta_{l+2}(l)
     *  &=
     *  \frac{1}{l+2-l-1}\sum_{t=1}^{l+2-l}(x_{t+l}+x_{t})
     *  \\
     *  &=
     *  \sum_{t=1}^{2}(x_{t+l}+x_{t})
     *  \\
     *  &=
     *  (x_{l+1}+x_{1}) + (x_{l+2}+x_{2}),
     *  \\
     *   \hat\beta_{n+1}(l)
     *   &= \frac{1}{n-l}\sum_{t=1}^{n+1-l}(x_{t+l}+x_{t})
     *   \qquad\qquad\qquad (\text{for}\, n\ge l+2)
     * \\
     *   &= \frac{1}{n-l} \left[\sum_{t=1}^{n-l}(x_{t+l}+x_{t}) + (x_{n+1}+x_{n+1-l})\right]
     * \\
     *   &= \frac{1}{n-l} \left[(n-l-1)\beta_n(l) + (x_{n+1}+x_{n+1-l})\right]
     * \\
     *   &= \beta_n(l) - \frac{1}{n-l}\beta_n(l) + \frac{1}{n-l} (x_{n+1}+x_{n+1-l}).
     * @f}
     *
     *
     * ### Making computing this operation less expensive ###
     *
     * In many situations, samples are quite highly correlated. An example is
     * Metropolis-Hastings sampling (e.g., using the Producers::MetropolisHastings
     * class) in high dimensional spaces: In those cases, it often takes many
     * hundreds or thousands of steps to obtain another sample that is
     * statistically uncorrelated to the first one. Unfortunately, to find
     * out how long this "correlation length" is, one has to compute
     * autocorrelations with rather large lags -- say, a lag of 10,000.
     * In these cases, it becomes very very expensive to actually compute the
     * autocorrelation: For every new sample, this class has to compute
     * data against each of the previous 10,000 samples. This can quite easily
     * be as expensive as it was to compute the new sample itself, or maybe even
     * more expensive.
     *
     * But, one often doesn't actually need the autocorrelation for every lag
     * between 1 and 10,000. Rather, it would be enough to really know the
     * correlation between samples $x_k$ and $x_{k-10}$, $x_{k-20}$, etc.
     * That's because we generally aren't really interested in the
     * autocorrelation for every lag $l$ between 1 and 10,000 -- rather,
     * what one typically wants to know for which lag samples become
     * uncorrelated and statistically it isn't going to make much difference
     * to us whether we know that the autocorrelation first is approximately
     * zero after a lag of 5,434 or 5,440. In other words, instead of computing
     * 10,000 pieces of data for each new sample, one would only compute
     * 1,000 pieces of data for each new sample. But this may still be very
     * expensive, and it, too, is not necessary: That's because in those
     * cases where we care to compute autocorrelations out to a lag of
     * 10,000, the contributions of samples 54,130, 54,131, ..., 54,139
     * to the autocorrelations are going to be almost identical because
     * these samples are high correlated with each other. In other
     * words, we can compute a very good approximation to the autocorrelation
     * if we take only every tenth sample of the original stream and compute
     * the autocorrelation with those samples back to have a lag of 10,
     * 20, 30, ... -- i.e., we really only need to compute the autocorrelation
     * back to a lag of 1,000 for every tenth sample.
     *
     * We can implement this in the following way: Let's say we have a
     * `sampler` object that produces samples (e.g., a
     * Producers::MetropolisHastings object), then the original and
     * very expensive way to compute the autocorrelation back to a lag
     * of 10,000 is to use the current class like this:
     * @code
     *   SampleFlow::Consumers::AutoCovarianceTrace<SampleType> covariance(10000);
     *   covariance.connect_to_producer (sampler);
     * @endcode
     * On the other hand, a cheaper way is as follows:
     * @code
     *   SampleFlow::Filters::TakeEveryNth<SampleType> every_10th(10);
     *   every_10th.connect_to_producer (sampler);
     *
     *   SampleFlow::Consumers::AutoCovarianceTrace<SampleType> covariance(1000);
     *   covariance.connect_to_producer (every_10th);
     * @endcode
     * And an even cheaper way would be to use this, if we
     * expect that autocorrelations in the sample stream persist
     * substantially beyond a lag of several hundred:
     * @code
     *   SampleFlow::Filters::TakeEveryNth<SampleType> every_100th(100);
     *   every_100th.connect_to_producer (sampler);
     *
     *   SampleFlow::Consumers::AutoCovarianceTrace<SampleType> covariance(100);
     *   covariance.connect_to_producer (every_100th);
     * @endcode
     *
     * In order to illustrate this with an example, we have taken the first
     * 100,000 samples from a Metropolis-Hastings chain of a 64-dimensional
     * problem to find out how long the autocorrelation length scale is.
     * For this problem, computing the autocorrelation to a lag of 10,000
     * using every sample takes 198 minutes. Using only every tenth sample
     * using the two additional lines above reduces this time to 2 minutes
     * 54 seconds -- not surprisingly a reduction of almost a factor of 100,
     * given that we only consider one tenth the samples and for each need
     * to compute only one tenth of the information. A further reduction
     * to every 100th sample further reduces the time to 35 seconds, which
     * is also the time it takes if we take only every 1000th sample; both
     * of these times reflect the time to run those parts of the program
     * that have nothing to do with computing the autocorrelation.
     *
     * The question of course is how good this approximation with a reduced
     * number of samples then is. The following picture shows the
     * autocovariances computed with all of these subsets of data:
     *
     * @image html spurious_autocovariance_01.png
     *
     * It is clear from the image that the information we get from using
     * only every tenth sample (at one hundredth the computational cost)
     * is as good as when using every sample: The two curves cover each
     * other. When only using every hundredth sample, one can see differences,
     * and the information is substantially different when only using every
     * thousandth sample. What *is* interesting, however, is that all of
     * these data subsets give us the same idea that only samples at least
     * around 5000 apart in the chain are statistically uncorrelated. In
     * other words, if we care about finding out every how manyth sample
     * we should take from the chain to have the same information content
     * as when considering every single sample, then the answer around 5000
     * no matter how we compute it. In other words, reducing the sampling
     * rate when computing the autocovariance to a point where it no longer
     * is prohibitively expensive works.
     *
     *
     * ### Threading model ###
     *
     * The implementation of this class is thread-safe, i.e., its
     * consume() member function can be called concurrently and from multiple
     * threads.
     *
     *
     * @tparam InputType The C++ type used for the samples $x_k$. In
     *   order to compute auto covariances, the same kind of requirements
     *   have to hold as listed for the MeanValue class (we need to be able to
     *   calculate mean, divide by positive integers, etc.).
     */

    template <typename InputType>
    class AutoCovarianceTrace: public Consumer<InputType>
    {
      public:
        /**
         * The data type of the elements of the input type.
         */
        using scalar_type = types::ScalarType<InputType>;

        /**
         * The data type returned by the get() function.
         */
        using value_type = std::vector<scalar_type>;

        /**
         * Constructor.
         *
         * This class does not support asynchronous processing of samples,
         * and consequently calls the base class constructor with
         * ParallelMode::synchronous as argument.
         *
         * @param[in] lag_length A number that indicates how many autocovariance
         *   values we want to calculate, i.e., how far back in the past we
         *   want to check how correlated each sample is.
         */
        AutoCovarianceTrace(const unsigned int lag_length);

        /**
         * Destructor. This function also makes sure that all samples this
         * object may have received have been fully processed. To this end,
         * it calls the Consumers::disconnect_and_flush() function of the
         * base class.
         */
        virtual ~AutoCovarianceTrace ();

        /**
         * Process one sample by updating the previously computed covariance
         * values using this one sample.
         *
         * @param[in] sample The sample to process.
         * @param[in] aux_data Auxiliary data about this sample. The current
         *   class does not know what to do with any such data and consequently
         *   simply ignores it.
         */
        virtual
        void
        consume (InputType     sample,
                 AuxiliaryData aux_data) override;

        /**
         * A function that returns the autocovariance vector computed from the
         * samples seen so far. If no samples have been processed so far, then
         * a default-constructed object of type InputType will be returned.
         *
         * @return The computed autocovariance vector of length `lag_length+1`
         *   as provided to the constructor. The $l$th element of this vector,
         *   starting from $l=0$ and going to $l=$`lag_length` (both inclusive)
         *   corresponds to the auto-covariance of lag $l$. As a consequence,
         *   the first entry ($l=0$) is the trace of the covariance matrix
         *   that would have been returned by the CovarianceMatrix class.
         */
        value_type get() const;

      private:
        /**
         * A mutex used to lock access to all member variables when running
         * on multiple threads.
         */
        mutable std::mutex mutex;

        /**
         * Describes the maximal lag up to which we calculate auto-covariances.
         */
        const unsigned int max_lag;

        /**
         * The current value of $\bar{x}_k$ as described in the introduction
         * of this class. For more detailed description of calculation, check mean_value.h
         */
        InputType current_mean;

        /**
         * A data type used to store the past few samples.
         */
        using PreviousSamples = std::deque<InputType>;

        /**
         * Update variables necessary to compute the autocovariation. See their
         * definition in the documentation of this class.
         */
        std::vector<scalar_type> alpha;
        std::vector<InputType> beta;

        /**
         * Save previous samples needed to do calculations when a new sample
         * comes in.
         *
         * These samples are stored in a double-ended queue (`std::deque`) so
         * that it is efficient to push a new sample to the front of the list
         * as well as to remove one from the end of the list.
         */
        PreviousSamples previous_samples;

        /**
         * The number of samples processed so far.
         */
        types::sample_index n_samples;
    };



    template <typename InputType>
    AutoCovarianceTrace<InputType>::
    AutoCovarianceTrace (unsigned int lag_length)
      :
      Consumer<InputType>(ParallelMode::synchronous),
      max_lag(lag_length),
      n_samples (0)
    {}



    template <typename InputType>
    AutoCovarianceTrace<InputType>::
    ~AutoCovarianceTrace ()
    {
      this->disconnect_and_flush();
    }


    template <typename InputType>
    void
    AutoCovarianceTrace<InputType>::
    consume (InputType sample, AuxiliaryData /*aux_data*/)
    {
      std::lock_guard<std::mutex> lock(mutex);

      // If this is the first sample we see, initialize all components
      // After the first sample, the autocovariance vector
      // is the zero vector since a single sample does not have any friends yet.
      if (n_samples == 0)
        {
          // Initialize the alpha and beta vectors to a length of lag_length+1
          // (so that we can compute the variance plus lag_length auto-variances)
          alpha = std::vector<double>(max_lag+1, 0.);
          beta.resize(max_lag+1);

          for (unsigned int l=0; l<=max_lag; ++l)
            {
              // Initialize beta[i] to zero; first initialize it to 'sample'
              // so that it has the right size already
              beta[l] = sample;
              for (unsigned int j=0; j<Utilities::size(sample); ++j)
                {
                  Utilities::get_nth_element(beta[l], j) = 0;
                }
            }
          current_mean = sample;

          // Push the first sample to the front of the list of samples:
          previous_samples.push_front (sample);
          n_samples = 1;
        }
      else
        {
          // Now save the sample. If the list is becoming longer than the lag
          // length (plus one, for l=0), drop the oldest sample at the end of this
          // block. This makes sure that we always have all samples up to a lag of l+1
          // available, which we need for the initialization step
          previous_samples.push_front (sample);

          for (unsigned int l=0; l<=max_lag; ++l)
            {
              if (n_samples == l+1)
                {
                  // We need to initialize alpha via the formula
                  // alpha_{l+2}(l) = sum_{t=1}^2 x_{t+l} x_t
                  alpha[l] = 0;
                  for (unsigned int j=0; j<Utilities::size(sample); ++j)
                    alpha[l] += Utilities::get_nth_element (previous_samples[0], j) *
                                Utilities::get_nth_element (previous_samples[l], j);
                  for (unsigned int j=0; j<Utilities::size(sample); ++j)
                    alpha[l] += Utilities::get_nth_element (previous_samples[1], j) *
                                Utilities::get_nth_element (previous_samples[l+1], j);

                  beta[l] = previous_samples[0];
                  beta[l] += previous_samples[1];
                  beta[l] += previous_samples[l];
                  beta[l] += previous_samples[l+1];
                }
              else if (n_samples >= l+2)
                {
                  // Update alpha
                  double alphaupd = -alpha[l];
                  for (unsigned int j=0; j<Utilities::size(sample); ++j)
                    {
                      alphaupd += Utilities::get_nth_element (sample, j) *
                                  Utilities::get_nth_element (previous_samples[l], j);
                    }
                  alphaupd *= 1./(n_samples-l);
                  alpha[l] += alphaupd;

                  // Update beta. Start with the current sample and add up
                  // the updates.
                  InputType betaupd = sample;
                  for (unsigned int j=0; j<Utilities::size(sample); ++j)
                    {
                      Utilities::get_nth_element(betaupd, j)
                      += Utilities::get_nth_element (previous_samples[l], j);

                      Utilities::get_nth_element(betaupd, j)
                      -= Utilities::get_nth_element (beta[l], j);

                      Utilities::get_nth_element(betaupd, j)
                      *= 1./(n_samples-l);
                    }
                  beta[l] += betaupd;
                }
            }

          if (previous_samples.size() > max_lag+1)
            previous_samples.pop_back ();
          ++n_samples;

          // Then also update the running mean:
          InputType update = sample;
          update -= current_mean;
          update /= n_samples;
          current_mean += update;
        }
    }



    template <typename InputType>
    typename AutoCovarianceTrace<InputType>::value_type
    AutoCovarianceTrace<InputType>::
    get () const
    {
      std::lock_guard<std::mutex> lock(mutex);

      std::vector<scalar_type> current_autocovariation(max_lag+1,
                                                       scalar_type(0));

      for (int l=0; l<=max_lag; ++l)
        {
          current_autocovariation[l] = alpha[l];

          for (unsigned int j=0; j<Utilities::size(current_mean); ++j)
            current_autocovariation[l] -= Utilities::get_nth_element(current_mean,j) *
                                          Utilities::get_nth_element(beta[l], j);

          if (n_samples > l+1 )
            for (unsigned int j=0; j<Utilities::size(current_mean); ++j)
              current_autocovariation[l] += (1. + 1./(n_samples-l-1))
                                            *
                                            Utilities::get_nth_element(current_mean,j) *
                                            Utilities::get_nth_element(current_mean,j);
        }

      return current_autocovariation;
    }
  }
}

#endif
