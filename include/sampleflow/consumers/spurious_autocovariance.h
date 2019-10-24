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
#include <mutex>

#include <boost/numeric/ublas/matrix.hpp>


namespace SampleFlow
{
  namespace Consumers
  {
    /**
     * NOTICE: We can only say, that this algorithm calculates "spurious autocovariance", because by definition
     * autocovariance function is much more complex it is not (except if we work samples of one dimension). Anyway, later in this code we use word autocovariance as
     * we refer to spurious autocovariance.
     *
     * This is a Consumer class that implements computing the running sample autocovariance function:
     * $\hat\gamma(l)=\frac{1}{n}\sum_{t=1}^{n-l}{(\bm{x}_{t+l}-\bar\bm{x})(bm{x}_{t}-\bar\bm{x})}$
     *
     * This code for every new sample updates $\hat\gamma(k), l=1,2,3...,k$. The value of $n$ is set
     * in the constructor.
     *
     * Algorithm:
     * There are three parts: 1) When amount of samples (sample_n) is equal 0 2) When k>sample_n 3) Otherwise
     * Second and third parts are almost identical, just for "l" bigger than sample_n it is not possible to
     * get \gamma(l) estimation. Further description focus on part 3) (case with big enough sample_n)
     *
     * Let expand formula above and then denote some of its parts as $\alpha$ and $\beta$:
     * \hat\gamma(l)=\frac{1}{n}\sum_{t=1}^{n-l}{(\bm{x}_{t+l}-\bar\bm{x_n})^T(bm{x}_{t}-\bar\bm{x_n})}=
     * =\frac{1}{n}\sum_{t=1}^{n-l}{(\bm{x}_{t+l})^T(bm{x}_{t})}-
     * -(\bar\bm{x_n}^T)\frac{1}{n}\sum_{t=1}^{n-l}{\bm{x}_{t+l}+(bm{x}_{t}}+
     * +\frac{n-l}{n}(\bar\bm{x_n}^T)(\bar\bm{x_n})=
     * =\alpha_n(l)-(\bar\bm{x_n}^T)\bm{beta_n(l)}+\frac{n-l}{n}(\bar\bm{x_n}^T)(\bar\bm{x_n}).$
     *
     * During calculation, we need to update $\alpha_{n+1}(l) (scalar),
     * \bm{beta_{n+1}(l)}$ (same dimension as sample) and sample mean $\bar\bm{x_{n+1}$
     *
     * Notice, that for each l, $\alpha_{n}(l)$ and $\bm{beta_{n}(l)}$ is different. So to save $\alpha$ values
     * we need to have vector, while for $\bm{\beta}$ - matrix.
     *
     * Principle of this updating algorithm is equivalent as in mean_value.h
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
     *   have to hold as listed for the Covariance class (we need to be able to
     *   calculate mean, divide by positive integer ant etc)
     */

    template <typename InputType>
    class SpuriousAutocovariance: public Consumer<InputType>
    {
      public:
        /**
         * The data type of the elements of the input type.
         * MANTYS COMMENT: IF InputType is scalar by itself, this part gives error and I couldnt find the way to solve this.
         */
        using scalar_type = typename InputType::value_type;

        /**
         * The data type returned by the get() function.
         */
        using value_type = std::vector<scalar_type>;

        /**
         * Constructor
         *
         * @param[in] parameter lag_lenght refers to how many autocovariance function values we want
         * to calculate
         */
        SpuriousAutocovariance(const unsigned int lag_length);

        /**
         * Process one sample by updating the previously computed covariance
         * matrix using this one sample.
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
         * @return The computed autocovariance vector of length leg_length.
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
         * The current value of $\bar x_k$ as described in the introduction
         * of this class. For more detailed description of calculation, check mean_value.h
         */
        InputType current_mean;

        /**
         * A data type used to store the past few samples.
         */
        using PreviousSamples = boost::numeric::ublas::matrix<scalar_type>;

        /**
         * Parts for running autocovariation calculations.
         * Description of these parts is given at this class description above.
         * We should notice, that alpha and current_autocovariation is vectors, while beta is matrix.
         * That happens, because if we want to update beta for each new sample, we need to know mean
         * summed vector values (sum numbers depends from lag parameter). There might be other ways how
         * to update this member, but this one appears the most stable.
         */
        std::vector<scalar_type> alpha;//First dot product of autocovariation
        PreviousSamples beta; //Sum of vectors for innerproduct

        /**
         * Save previous sample value needed to do calculations then new sample comes.
         *
         */
        PreviousSamples previous_sample;
        PreviousSamples previous_sample_replace;

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
          beta.resize(autocovariance_length,sample.size());

          for (unsigned int i=0; i<autocovariance_length; ++i)
            {
              alpha[i] = 0;
              for (unsigned int j=0; j<sample.size(); ++j)
                {
                  beta(i,j) = 0;
                }
            }
          current_mean = sample;
          previous_sample.resize(autocovariance_length,sample.size());
          previous_sample_replace.resize(autocovariance_length,sample.size());

          for (unsigned int i=0; i<sample.size(); ++i) previous_sample(0,i) = sample[i];
        }

      else
        {
          /*
           * In the start of updating algorithm, there is no enough samples already seen, to be able to
           * calculate autocovariance function values for argument bigger than sample size. In order to avoid errors, we need
           * to use minimum function. However, we need slightly different minimums for calculation of autocovariance parts and for saving
           * previous values.
           * length1 refers to how many samples were already seen before a new one and if it is more
           * than our desired autocovariance function length, it sets to that value. In this sense, it restricts how "long" our
           * calculations should or can be.
           * length2 refers to saving previous values. It refers to, that at most we can save autocovariance_length-1 values(making space
           * to save the newest one.
           */

          unsigned int length1=std::min(static_cast<unsigned int>(n_samples),autocovariance_length);
          unsigned int length2=std::min(static_cast<unsigned int>(n_samples),autocovariance_length-1);

          ++n_samples;
          for (unsigned int i=0; i<length1; ++i)
            {

              //Update first dot product (alpha)
              double alphaupd = 0;
              for (unsigned int j=0; j<sample.size(); ++j)
                {
                  alphaupd += sample[j]*previous_sample(i,j);
                }
              alphaupd -= alpha[i];
              alphaupd /= n_samples-(i+1);
              alpha[i] += alphaupd;

              //Update second value (beta)
              InputType betaupd = sample;
              for (unsigned int j=0; j<sample.size(); ++j)
                {
                  betaupd[j] += previous_sample(i,j);
                  betaupd[j] -= beta(i,j);
                  betaupd[j] /= n_samples-(i+1);
                  beta(i,j) += betaupd[j];
                }
            }

          //Save needed previous values
          for (unsigned int i=0; i<length2; ++i)
            {
              for (unsigned int j=0; j<sample.size(); ++j)
                {
                  previous_sample_replace(i+1,j)=previous_sample(i,j);
                }
            }

          for (unsigned int j=0; j<sample.size(); ++j) previous_sample_replace(0,j) = sample[j];
          previous_sample = previous_sample_replace;

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

      if (n_samples!=0)
        {
          const types::sample_index length1 = std::min(n_samples,
                                                       static_cast<types::sample_index>(autocovariance_length));

          for (int i=0; i<length1; ++i)
            {
              current_autocovariation[i] = alpha[i];

              // current_mean.size() is equal to sample.size(). While we don't get samples here, alpha.size helps to
              for (int j=0; j<current_mean.size(); ++j)
                current_autocovariation[i] -= current_mean[j] * beta(i,j);

              current_autocovariation[i] += (current_mean*current_mean).sum();
            }
        }

      return current_autocovariation;
    }
  }
}

#endif
