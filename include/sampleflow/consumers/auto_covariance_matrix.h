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

#ifndef SAMPLEFLOW_CONSUMERS_AUTOCOVARIANCEMATRIX_H
#define SAMPLEFLOW_CONSUMERS_AUTOCOVARIANCEMATRIX_H

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
     * This is a Consumer class that implements computing the
     * running sample auto-covariance matrix function:
     * @f{align*}{
     *   \gamma(l)
     *   &=
     *   \frac{1}{n-l-1} \sum_{t=1}^{n-l}{(x_{t+l}-\bar{x})(x_{t}-\bar{x})^T}
     * @f}
     * These matrices indicate how correlated samples $x_t$ and $x_{t+l}$
     * are for different time lags $l=1,\ldots,k$. It corresponds to the
     * [cross-covariance matrix](https://en.wikipedia.org/wiki/Cross-covariance_matrix)
     * for samples $x_t$ and $x_{t+l}$.
     * In other words, it calculates the trace of the (auto-)covariance matrix
     * $\gamma(l)$ of samples $x_{t+l}$ and $x_t$
     * with a lag between zero and $k$. The fraction in front is $n-l-1$ because
     * we are comparing $n-l$ pairs of samples, of which $n-l-1$ are statistically
     * independent. This is consistent with the definition of the covariance matrix
     * used by the CovarianceMatrix class.
     *
     * @note Computing matrix-valued covariances for different lags is quite
     *   expensive. In some cases, just computing the traces of these matrices
     *   may be good enough, and in those cases the AutoCovarianceTrace class
     *   offers a cheaper alternative.
     *
     * @note In the case of scalar data types, the (auto-)covariance matrices
     *   are also just numbers. In those cases, the results of the current
     *   AutoCovarianceMatrix and the AutoCovarianceTrace classes are the
     *   same, though the current class returns $1\times 1$ matrices whereas
     *   the latter class just returns numbers for each lag $l$.
     *
     *
     * <h3> Algorithm </h3>
     *
     * The approach to deriving an update formula for $\gamma(l)$ is the
     * same as for many other classes, and similar to the one used in the
     * MeanValue and AutoCovarianceTrace class.
     * In the following, let us only consider the case when we have already seen
     * $n\ge k$ samples, so that all of the $\gamma(l)$ can actually be computed.
     *
     * Let us expand the formula above and then denote some of its parts as $\alpha$
     * and $\beta$:
     * @f{align*}{
     *   \gamma(l)
     *   &=
     *   \frac{1}{n-l-1}\sum_{t=1}^{n-l}{(x_{t+l}-\bar{x}_n)(x_{t}-\bar{x}_n)^T}
     * \\&=
     *   \frac{1}{n-l-1}\sum_{t=1}^{n-l}{x_{t+l} x_{t}^T}
     *   -
     *   \bar{x}_n
     *   \left[ \frac{1}{n-l-1}\sum_{t=1}^{n-l}x_{t} \right]^T
     *   -
     *   \left[ \frac{1}{n-l-1}\sum_{t=1}^{n-l}x_{t+l} \right]
     *   \bar{x}_n^T
     *   +
     *   \frac{n-l}{n-l-1}\bar{x}_n^T \bar{x}_n
     * \\&=
     *   \underbrace{\frac{1}{n-l-1}\sum_{t=1}^{n-l}{x_{t+l} x_{t}^T}}_{\alpha_n(l)}
     *   -
     *   \bar{x}_n
     *   {\underbrace{\left[ \frac{1}{n-l-1}\sum_{t=1+l}^{n}x_{t-l} \right]}_{\eta_n(l)}}^T
     *   -
     *   \underbrace{\left[ \frac{1}{n-l-1}\sum_{t=1+l}^{n}x_{t} \right]}_{\beta_n(l)}
     *   \bar{x}_n^T
     *   +
     *   \left(1+ \frac{1}{n-l-1}\right) \bar{x}_n
     *   \bar{x}_n^T
     * @f}
     *
     * For each new sample, we then need to update the matrices $\alpha_{n+1}(l)$,
     * sample means $\bar{x}_{n-l+1}$. The
     * principle of updating $\bar x_{n-l+1}$ is equivalent to what the MeanValue
     * class does. For $\alpha$, we use that
     * @f{align*}{
     *  \alpha_{1}(l) = \cdots = \alpha_{l+1}(l)
     *  &= 0,
     *  \\
     *  \alpha_{l+2}(l)
     *  &=
     *  \sum_{t=1}^2 x_{t+l} x_t^T
     *  \\
     *  &=
     *  x_{1+l} x_1^T + x_{2+l} x_2^T,
     *  \\
     *  \alpha_{n+1}(l)
     *  &=
     *  \frac{1}{n-l-1+1}\sum_{t=1}^{n-l+1}{x_{t+l} x_{t}^T}
     *  \\
     *  &=
     *  \frac{1}{n-l}\left[\sum_{t=1}^{n-l}{x_{t+l} x_{t}^T} + x_{n+1} x_{n+1-l}^T \right]
     *  \\
     *  &=
     *  \frac{1}{n-l}\left[(n-l-1)\alpha_n(l) + x_{n+1} x_{n+1-l}^T \right]
     *  \\
     *  &=
     *  \frac{n-l-1}{n-l} \alpha_n(l) + \frac{1}{n-l} x_{n+1} x_{n+1-l}^T.
     *  \\
     *  &=
     *  \alpha_n(l) - \frac{1}{n-l} \alpha_n(l) + \frac{1}{n-l} x_{n+1} x_{n+1-l}^T.
     * @f}
     * Similarly, for $\beta$ we can use that
     * @f{align*}
     *  \beta_{1}(l) = \cdots = \beta_{l+1}(l)
     *  &= 0,
     *  \\
     *  \beta_{l+2}(l)
     *  &=
     *  \frac{1}{l+2-l-1}\sum_{t=1+l}^{l+2}x_t
     *  \\
     *  &=
     *  \sum_{t=1}^{2}x_{t+l}
     *  \\
     *  &=
     *  x_{l+1} + x_{l+2},
     *  \\
     *  \beta_{n+1}(l)
     *   &= \frac{1}{n-l}\sum_{t=1+l}^{n+1}x_{t}
     *   \qquad\qquad\qquad (\text{for}\, n\ge l+2)
     * \\
     *   &= \frac{1}{n-l} \left[\sum_{t=1+l}^{n}x_{t} + x_{n+1}\right]
     * \\
     *   &= \frac{1}{n-l} \left[(n-l-1)\beta_n(l) + x_{n+1}\right]
     * \\
     *   &= \beta_n(l) - \frac{1}{n-l}\beta_n(l) + \frac{1}{n-l} x_{n+1}.
     * @f}
     * Finally, for $\eta$ we have
     * @f{align*}
     *  \eta_{1}(l) = \cdots = \eta_{l+1}(l)
     *  &= 0,
     *  \\
     *  \eta_{l+2}(l)
     *  &=
     *  \frac{1}{l+2-l-1}\sum_{t=1+l}^{l+2}x_{t-l}
     *  \\
     *  &=
     *  x_1+x_2
     *  \\
     *  \eta_{n+1}(l)
     *   &=
     *   \frac{1}{n-l}\sum_{t=1+l}^{n+1}x_{t-l}
     *   \qquad\qquad\qquad (\text{for}\, n\ge l+2)
     * \\
     *   &= \frac{1}{n-l} \left[\sum_{t=1+l}^{n}x_{t-l} + x_{n+1-l}\right]
     * \\
     *   &= \frac{1}{n-l} \left[(n-l-1)\eta_n(l) + x_{n+1-l}\right]
     * \\
     *   &= \eta_n(l) - \frac{1}{n-l}\eta_n(l) + \frac{1}{n-l} x_{n+1-l}.
     * @f}
     *
     * It is instructive to compare these formulas to those used for the
     * AutoCovarianceTrace class. Specifically, the latter class computes the
     * traces $\hat\gamma(l)=\text{trace}\,\gamma(l)$. Using the last expression
     * above, we see that
     * @f{align*}{
     *   \text{trace}\,\gamma(l)
     *   &=
     *   \text{trace}\,\left\{
     *     \frac{1}{n-l-1}\sum_{t=1}^{n-l}{x_{t+l} x_{t}^T}
     *     -
     *     \frac{n-l}{n-l-1} \bar{x}_n
     *     \bar x_{n-l}^T
     *     +
     *     \frac{l}{n-l-1} \bar x_l
     *     \bar{x}_n^T
     *     -
     *     \frac{l}{n-l-1} \bar x_n
     *     \bar{x}_n^T
     *   \right\}
     *   \\
     *   &=
     *     \frac{1}{n-l-1}\sum_{t=1}^{n-l}{x_{t+l}^T x_{t}}
     *     -
     *     \frac{n-l}{n-l-1} \bar{x}_n^T
     *     \bar x_{n-l}
     *     +
     *     \frac{l}{n-l-1} \bar x_l^T
     *     \bar{x}_n
     *     -
     *     \frac{l}{n-l-1} \bar x_n^T
     *     \bar{x}_n
     *   \\
     *   &=
     *     \frac{1}{n-l-1}\sum_{t=1}^{n-l}{x_{t+l}^T x_{t}}
     *     -
     *     \frac{1}{n-l-1}
     *     \bar{x}_n^T
     *     \left[(n-l) \bar x_{n-l}
     *     -
     *     l \bar x_l^T
     *     +
     *     l
     *     \bar{x}_n
     *     \right]
     * @f}
     *
     * ### Making computing this operation less expensive ###
     *
     * The computations made by this class are quite expensive,
     * even more expensive than the ones made by the AutoCovarianceTrace
     * class. The techniques described there to make computations
     * cheaper also apply to the current class.
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
    class AutoCovarianceMatrix : public Consumer<InputType>
    {
      public:
        /**
         * The data type of the elements of the input type.
         */
        using scalar_type = types::ScalarType<InputType>;

        /**
         * The data type returned by the get() function.
         */
        using value_type = std::vector<Eigen::Matrix<scalar_type,Eigen::Dynamic,Eigen::Dynamic>>;

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
        AutoCovarianceMatrix(const unsigned int lag_length);

        /**
         * Destructor. This function also makes sure that all samples this
         * object may have received have been fully processed. To this end,
         * it calls the Consumers::disconnect_and_flush() function of the
         * base class.
         */
        virtual ~AutoCovarianceMatrix ();

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
        value_type alpha;
        std::vector<InputType> beta;
        std::vector<InputType> eta;

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
    AutoCovarianceMatrix<InputType>::
    AutoCovarianceMatrix (unsigned int lag_length)
      :
      Consumer<InputType>(ParallelMode::synchronous),
      max_lag(lag_length),
      n_samples (0)
    {}



    template <typename InputType>
    AutoCovarianceMatrix<InputType>::
    ~AutoCovarianceMatrix ()
    {
      this->disconnect_and_flush();
    }


    template <typename InputType>
    void
    AutoCovarianceMatrix<InputType>::
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
          alpha.resize(max_lag+1);
          for (auto &a : alpha)
            a.resize (Utilities::size(sample), Utilities::size(sample));
          beta.resize(max_lag+1);
          eta.resize(max_lag+1);

          for (unsigned int l=0; l<=max_lag; ++l)
            {
              // Initialize beta[i] to zero; first initialize it to 'sample'
              // so that it has the right size already. Do the same for eta.
              beta[l] = sample;
              eta[l] = sample;
              for (unsigned int j=0; j<Utilities::size(sample); ++j)
                {
                  Utilities::get_nth_element(beta[l], j) = 0;
                  Utilities::get_nth_element(eta[l], j) = 0;
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
                  for (unsigned int i=0; i<Utilities::size(sample); ++i)
                    for (unsigned int j=0; j<Utilities::size(sample); ++j)
                      alpha[l](i,j) += Utilities::get_nth_element (previous_samples[0], i) *
                                       Utilities::get_nth_element (previous_samples[l], j);
                  for (unsigned int i=0; i<Utilities::size(sample); ++i)
                    for (unsigned int j=0; j<Utilities::size(sample); ++j)
                      alpha[l](i,j) += Utilities::get_nth_element (previous_samples[1], i) *
                                       Utilities::get_nth_element (previous_samples[l+1], j);

                  beta[l] = previous_samples[0];
                  beta[l] += previous_samples[1];

                  eta[l] += previous_samples[l];
                  eta[l] += previous_samples[l+1];
                }
              else if (n_samples >= l+2)
                {
                  // Update alpha
                  Eigen::Matrix<scalar_type,Eigen::Dynamic,Eigen::Dynamic> alphaupd = -alpha[l];
                  for (unsigned int i=0; i<Utilities::size(sample); ++i)
                    for (unsigned int j=0; j<Utilities::size(sample); ++j)
                      {
                        alphaupd(i,j) += Utilities::get_nth_element (sample, i) *
                                         Utilities::get_nth_element (previous_samples[l], j);
                      }
                  alphaupd *= 1./(n_samples-l);
                  alpha[l] += alphaupd;

                  // Update beta. Start with the current sample and add up
                  // the updates.
                  InputType betaupd = sample;
                  betaupd -= beta[l];
                  betaupd *= 1./(n_samples-l);
                  beta[l] += betaupd;

                  // Finally also update eta
                  InputType etaupd = previous_samples[l];
                  etaupd -= eta[l];
                  etaupd *= 1./(n_samples-l);
                  eta[l] += etaupd;
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
    typename AutoCovarianceMatrix<InputType>::value_type
    AutoCovarianceMatrix<InputType>::
    get () const
    {
      std::lock_guard<std::mutex> lock(mutex);

      value_type current_autocovariation(max_lag+1);
      for (auto &a : current_autocovariation)
        a.resize (Utilities::size(current_mean), Utilities::size(current_mean));

      for (int l=0; l<=max_lag; ++l)
        {
          current_autocovariation[l] = alpha[l];

          for (unsigned int i=0; i<Utilities::size(current_mean); ++i)
            for (unsigned int j=0; j<Utilities::size(current_mean); ++j)
              current_autocovariation[l](i,j) -= Utilities::get_nth_element(current_mean,i) *
                                                 Utilities::get_nth_element(eta[l], j);

          for (unsigned int i=0; i<Utilities::size(current_mean); ++i)
            for (unsigned int j=0; j<Utilities::size(current_mean); ++j)
              current_autocovariation[l](i,j) -= Utilities::get_nth_element(beta[l],i) *
                                                 Utilities::get_nth_element(current_mean, j);

          if (n_samples > l+1 )
            for (unsigned int i=0; i<Utilities::size(current_mean); ++i)
              for (unsigned int j=0; j<Utilities::size(current_mean); ++j)
                current_autocovariation[l](i,j) += (1. + 1./(n_samples-l-1))
                                                   *
                                                   Utilities::get_nth_element(current_mean,i) *
                                                   Utilities::get_nth_element(current_mean,j);
        }

      return current_autocovariation;
    }
  }
}

#endif
