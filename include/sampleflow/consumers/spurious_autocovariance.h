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

#ifndef SAMPLEFLOW_CONSUMERS_SPURIOUS_AUTOCOVARIANCE_H
#define SAMPLEFLOW_CONSUMERS_SPURIOUS_AUTOCOVARIANCE_H

#include <sampleflow/consumer.h>
#include <sampleflow/types.h>
#include <sampleflow/element_access.h>
#include <mutex>
#include <deque>

#include <boost/numeric/ublas/matrix.hpp>


namespace SampleFlow
{
  namespace Consumers
  {
    /**
     * @note This class only calculates a "spurious autocovariance" since the actual
     *   definition of "autocovariance" is more complex than what we do here (except
     *   if we work scalar samples types). That said, below we will use the word
     *   "autocovariance" even when refering to this spurious autocovariance.
     *
     * This is a Consumer class that implements computing the running sample autocovariance function:
     * @f{align*}{
     *   \hat\gamma(l)
     *   =
     *   \frac{1}{n} \sum_{t=1}^{n-l}{(x_{t+l}-\bar{x})(x_{t}-\bar{x})}.
     * @f}
     * In other words, it calculates the covariance of samples $x_{t+l}$ and $x_t$
     * with a lag between zero and $k$
     *
     * This class updates $\hat\gamma(l), l=0,1,2,3,\ldots,k$ for each new, incoming
     * sample. The value of $k$ is set in the constructor.
     *
     *
     * <h3> Algorithm </h3>
     *
     * In the following, let us only consider the case when we have already seen
     * $n\ge k$ samples, so that all of the $\hat\gamma(l)$ can actually be computed.
     *
     * Let us expand formula above and then denote some of its parts as $\alpha$ and $\beta$:
     * @f{align*}{
     *   \hat\gamma(l)
     *   &=
     *   \frac{1}{n}\sum_{t=1}^{n-l}{(x_{t+l}-\bar{x}_n)^T(x_{t}-\bar{x}_n)}
     * \\&=
     *   \underbrace{\frac{1}{n}\sum_{t=1}^{n-l}{x_{t+l}^T x_{t}}}_{\alpha_n(l)}
     *   -
     *   \bar{x}_n^T
     *   \underbrace{\left[ \frac{1}{n}\sum_{t=1}^{n-l}(x_{t+l}+x_{t}) \right]}_{\beta_n(l)}
     *   +
     *   \frac{n-l}{n}\bar{x}_n^T \bar{x}_n
     * \\&=
     *   \alpha_n(l)-\bar{x}_n^T \beta_n(l)+\frac{n-l}{n} \bar{x}_n^T \bar{x}_n.
     * @f}
     *
     * For each new sample, we then need to update the scalars $\alpha_{n+1}(l)$,
     * vectors $\beta_{n+1}(l)$, and sample mean $\bar{x}_{n+1}$. The
     * principle of this updating algorithm is equivalent to what the MeanValue
     * class does.
     *
     *
     * ### Making computing the spurious autocovariance less expensive ###
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
     *   SampleFlow::Consumers::SpuriousAutocovariance<SampleType> covariance(10000);
     *   covariance.connect_to_producer (sampler);
     * @endcode
     * On the other hand, a cheaper way is as follows:
     * @code
     *   SampleFlow::Filters::TakeEveryNth<SampleType> every_10th(10);
     *   every_10th.connect_to_producer (sampler);
     *
     *   SampleFlow::Consumers::SpuriousAutocovariance<SampleType> covariance(1000);
     *   covariance.connect_to_producer (every_10th);
     * @endcode
     * And an even cheaper way would be to use this, if we
     * expect that autocorrelations in the sample stream persist
     * substantially beyond a lag of several hundred:
     * @code
     *   SampleFlow::Filters::TakeEveryNth<SampleType> every_100th(100);
     *   every_100th.connect_to_producer (sampler);
     *
     *   SampleFlow::Consumers::SpuriousAutocovariance<SampleType> covariance(100);
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
    class SpuriousAutocovariance: public Consumer<InputType>
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
         * @param[in] lag_length A number that indicates how many autocovariance
         *   values we want to calculate, i.e., how far back in the past we
         *   want to check how correlated each sample is.
         */
        SpuriousAutocovariance(const unsigned int lag_length);

        /**
         * Destructor. This function also makes sure that all samples this
         * object may have received have been fully processed. To this end,
         * it calls the Consumers::disconnect_and_flush() function of the
         * base class.
         */
        virtual ~SpuriousAutocovariance ();

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
         * @return The computed autocovariance vector of length `lag_length`
         *   as provided to the constructor.
         */
        value_type get() const;

      private:
        /**
         * A mutex used to lock access to all member variables when running
         * on multiple threads.
         */
        mutable std::mutex mutex;

        /**
         * Describes how many values of autocovariance function we calculate.
         */
        const unsigned int autocovariance_length;

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
         * Parts for running autocovariation calculations.
         * Description of these parts is given at this class description above.
         * We should notice, that alpha and current_autocovariation is vectors, while beta is matrix.
         * That happens, because if we want to update beta for each new sample, we need to know mean
         * summed vector values (sum numbers depends from lag parameter). There might be other ways how
         * to update this member, but this one appears the most stable.
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
    SpuriousAutocovariance<InputType>::
    SpuriousAutocovariance (unsigned int lag_length)
      :
      autocovariance_length(lag_length),
      n_samples (0)
    {}



    template <typename InputType>
    SpuriousAutocovariance<InputType>::
    ~SpuriousAutocovariance ()
    {
      this->disconnect_and_flush();
    }


    template <typename InputType>
    void
    SpuriousAutocovariance<InputType>::
    consume (InputType sample, AuxiliaryData /*aux_data*/)
    {
      std::lock_guard<std::mutex> lock(mutex);

      // If this is the first sample we see, initialize all components
      // After the first sample, the autocovariance vector
      // is the zero vector since a single sample does not have any friends yet.

      if (n_samples == 0)
        {
          n_samples = 1;
          alpha.resize(autocovariance_length);
          beta.resize(autocovariance_length);

          for (unsigned int i=0; i<autocovariance_length; ++i)
            {
              alpha[i] = 0;

              // Initialize beta[i] to zero; first initialize it to 'sample'
              // so that it has the right size already
              beta[i] = sample;
              for (unsigned int j=0; j<Utilities::size(sample); ++j)
                {
                  Utilities::get_nth_element(beta[i], j) = 0;
                }
            }
          current_mean = sample;

          // Push the first sample to the front of the list of samples:
          previous_samples.push_front (sample);
        }
      else
        {
          /*
           * In the start of updating algorithm, there is no enough samples already seen, to be able to
           * calculate autocovariance function values for argument bigger than sample size. In order to avoid errors, we need
           * to use minimum function. However, we need slightly different minimums for calculation of autocovariance parts and for saving
           * previous values.
           */
          ++n_samples;
          for (unsigned int l=0; l<previous_samples.size(); ++l)
            {
              // Update alpha
              double alphaupd = 0;
              for (unsigned int j=0; j<Utilities::size(sample); ++j)
                {
                  alphaupd += Utilities::get_nth_element (sample, j) *
                              Utilities::get_nth_element (previous_samples[l], j);
                }
              alphaupd -= alpha[l];
              alphaupd /= n_samples-(l+1);
              alpha[l] += alphaupd;

              // Update beta
              InputType betaupd = sample;
              for (unsigned int j=0; j<Utilities::size(sample); ++j)
                {
                  Utilities::get_nth_element(betaupd, j)
                  += Utilities::get_nth_element (previous_samples[l], j);

                  Utilities::get_nth_element(betaupd, j)
                  -= Utilities::get_nth_element (beta[l], j);

                  Utilities::get_nth_element(betaupd, j)
                  /= n_samples-(l+1);
                }

              beta[l] += betaupd;
            }

          // Now save the sample. If the list is becoming longer than the lag
          // length, drop the oldest sample.
          previous_samples.push_front (sample);
          if (previous_samples.size() > autocovariance_length)
            previous_samples.pop_back ();

          // Then also update the running mean:
          InputType update = sample;
          update -= current_mean;
          update /= n_samples;
          current_mean += update;
        }
    }



    template <typename InputType>
    typename SpuriousAutocovariance<InputType>::value_type
    SpuriousAutocovariance<InputType>::
    get () const
    {
      std::lock_guard<std::mutex> lock(mutex);

      std::vector<scalar_type> current_autocovariation(autocovariance_length,
                                                       scalar_type(0));

      if (n_samples !=0 )
        {
          for (int i=0; i<previous_samples.size(); ++i)
            {
              current_autocovariation[i] = alpha[i];

              for (unsigned int j=0; j<Utilities::size(current_mean); ++j)
                current_autocovariation[i] -= Utilities::get_nth_element(current_mean,j) *
                                              Utilities::get_nth_element(beta[i], j);

              for (unsigned int j=0; j<Utilities::size(current_mean); ++j)
                current_autocovariation[i] += Utilities::get_nth_element(current_mean,j) *
                                              Utilities::get_nth_element(current_mean,j);
            }
        }

      return current_autocovariation;
    }
  }
}

#endif
