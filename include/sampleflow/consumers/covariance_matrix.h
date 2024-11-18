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

#ifndef SAMPLEFLOW_CONSUMERS_COVARIANCE_MATRIX_H
#define SAMPLEFLOW_CONSUMERS_COVARIANCE_MATRIX_H

#include <sampleflow/consumer.h>
#include <sampleflow/types.h>
#include <mutex>

#include <eigen3/Eigen/Dense>


namespace SampleFlow
{
  namespace Consumers
  {
    /**
     * A Consumer class that implements computing the running covariance matrix
     * $C_k = \frac{1}{k-1} \sum_{j=1}^k (x_j-{\bar x}_j)(x_j-{\bar x}_j)^T$
     * over all samples seen so far. Here, $x_k$ is the sample and
     * ${\bar x}_k=\frac{1}{k} \sum x_k$ the running mean up to sample $k$.
     * The last value $C_k$ so computed can be obtained by calling the get()
     * function.
     *
     * This class uses a formula to update the covariance matrix
     * in each step, rather than storing all samples and computing the matrix
     * from scratch in each step.
     * This formula is a symmetric variation of the one by Welford (1962), see
     * https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Welford's_online_algorithm)
     * and
     * https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online .
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
     *   order to compute covariances, the same kind of requirements
     *   have to hold as listed for the MeanValue class.
     */
    template <typename InputType>
    requires (Concepts::is_vector_space_type<InputType>)
    class CovarianceMatrix : public Consumer<InputType>
    {
      public:
        /**
         * The data type of the elements of the input type.
         */
        using scalar_type = types::ScalarType<InputType>;

        /**
         * The type of the information generated by this class, i.e., in which
         * the covariance matrix is computed.
         */
        using value_type = Eigen::Matrix<scalar_type,Eigen::Dynamic,Eigen::Dynamic>;

        /**
         * Constructor.
         *
         * This class does not care in which order samples are processed, and
         * consequently calls the base class constructor with
         * `ParallelMode::synchronous | ParallelMode::asynchronous` as argument.
         */
        CovarianceMatrix ();

        /**
         * Destructor. This function also makes sure that all samples this
         * object may have received have been fully processed. To this end,
         * it calls the Consumers::disconnect_and_flush() function of the
         * base class.
         */
        virtual ~CovarianceMatrix ();

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
         * A function that returns the covariance matrix computed from the
         * samples seen so far. If no samples have been processed so far, then
         * a default-constructed object of type InputType will be returned.
         *
         * @return The computed covariance matrix.
         */
        value_type
        get () const;

      private:
        /**
         * A mutex used to lock access to all member variables when running
         * on multiple threads.
         */
        mutable std::mutex mutex;

        /**
         * The current value of $\bar x_k$ as described in the introduction
         * of this class.
         */
        InputType           current_mean;

        /**
         * The current value of $\bar x_k$ as described in the introduction
         * of this class.
         */
        value_type current_covariance_matrix;

        /**
         * The number of samples processed so far.
         */
        types::sample_index n_samples;
    };



    template <typename InputType>
    requires (Concepts::is_vector_space_type<InputType>)
    CovarianceMatrix<InputType>::
    CovarianceMatrix ()
      :
      Consumer<InputType>(ParallelMode(static_cast<int>(ParallelMode::synchronous)
                                       |
                                       static_cast<int>(ParallelMode::asynchronous))),
      n_samples (0)
    {}



    template <typename InputType>
    requires (Concepts::is_vector_space_type<InputType>)
    CovarianceMatrix<InputType>::
    ~CovarianceMatrix ()
    {
      this->disconnect_and_flush();
    }



    template <typename InputType>
    requires (Concepts::is_vector_space_type<InputType>)
    void
    CovarianceMatrix<InputType>::
    consume (InputType sample, AuxiliaryData /*aux_data*/)
    {
      std::lock_guard<std::mutex> lock(mutex);

      // If this is the first sample we see, initialize the matrix with
      // this sample. After the first sample, the covariance matrix
      // is the zero matrix since a single sample has a zero variance.
      //
      // For the overall algorithm, we also have to keep track of the mean.
      // For this, we use the same algorithm as in the MeanValues class.
      if (n_samples == 0)
        {
          n_samples = 1;
          current_covariance_matrix.resize (Utilities::size(sample), Utilities::size(sample));
          current_mean = std::move(sample);
        }
      else
        {
          // Otherwise update the previously computed covariance by the current
          // sample; this also requires updating the current running mean.
          ++n_samples;

          InputType delta = sample;
          delta -= current_mean;
          for (unsigned int i=0; i<Utilities::size(sample); ++i)
            {
              const auto delta_i = Utilities::get_nth_element(delta, i);
              for (unsigned int j=0; j<Utilities::size(sample); ++j)
                {
                  const auto delta_j = Utilities::conj(Utilities::get_nth_element(delta, j));
                  current_covariance_matrix(i,j) += ((delta_i*delta_j)/(1.0*n_samples)) - current_covariance_matrix(i,j)/((1.0*n_samples)-1);
                }
            }
          InputType mean_update = sample;
          mean_update -= current_mean;
          mean_update /= n_samples;
          current_mean += mean_update;
        }
    }



    template <typename InputType>
    requires (Concepts::is_vector_space_type<InputType>)
    typename CovarianceMatrix<InputType>::value_type
    CovarianceMatrix<InputType>::
    get () const
    {
      std::lock_guard<std::mutex> lock(mutex);

      return current_covariance_matrix;
    }

  }
}

#endif
