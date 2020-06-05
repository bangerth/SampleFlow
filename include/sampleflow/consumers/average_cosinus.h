// ---------------------------------------------------------------------

#ifndef SAMPLEFLOW_CONSUMERS_AVERAGE_COSINE_H
#define SAMPLEFLOW_CONSUMERS_AVERAGE_COSINE_H

#include <sampleflow/consumer.h>
#include <sampleflow/types.h>
#include <list>
#include <mutex>

#include <boost/numeric/ublas/matrix.hpp>


namespace SampleFlow
{
  namespace Consumers
  {
    /**
     * This is a Consumer class that implements computing the running average cosine of
     * angles between vectors value. This calculation, similarly to autocovariance, compares samples, that are
     * apart by specific index. In this sense, we get function where domain is bounded unsigned int and for
     * each value we calculate cosine average.
     * Let's say unsigned int is equal to l and n_samples=n and n is bigger than l. Then the formula can be written as:
     * $\hat{cos(\theta_n)(l)}=\frac{1}{n-l}\sum_{t=1}^{n-l}{(x_{t+l}^T\bar{x})(\|x_{t}\| \; \|\bar{x}\|)}$.
     *
     * This code for every new sample updates $\hat{cos(\theta_n)(l)}, l=1,2,3,\ldots,L$. The value of
     * $L$ is provided to the constructor of this class.
     *
     * Notice, that for each new sample $x_n$, we need to take a sample, that was $l$ earlier than new sample ($x_{n-l}$).
     * Working this way, the updating algorithm becomes very similar to one used in the MeanValue class.
     *
     * Every time, for updating we calculate corresponding fractions. There numerator is the dot product, while denominator -
     * multiplied those two vector norms.
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
    class AverageCosineBetweenSuccessiveSamples: public Consumer<InputType>
    {
      public:
        /**
         * The data type of the elements of the input type. This type is used to define type of members, used in
         * calculations.
         */
        using scalar_type = typename InputType::value_type;

        /**
         * Constructor.
         *
         * This class does not support asynchronous processing of samples,
         * and consequently calls the base class constructor with
         * ParallelMode::synchronous as argument.
         */
        AverageCosineBetweenSuccessiveSamples(const unsigned int length);

        /**
         * Destructor. This function also makes sure that all samples this
         * object may have received have been fully processed. To this end,
         * it calls the Consumers::disconnect_and_flush() function of the
         * base class.
         */
        virtual ~AverageCosineBetweenSuccessiveSamples ();

        /**
         * Process one sample by updating the previously computed average cosine
         * function using this one sample.
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
         * A function that returns the average cosine vector computed from the
         * samples seen so far. If no samples have been processed so far, then
         * a default-constructed object of type std::vector<scalar_type> will be returned.
         *
         * @return The computed average cosine vector.
         */
        std::vector<scalar_type> get() const;

      private:
        /**
         * A mutex used to lock access to all member variables when running
         * on multiple threads.
         */
        mutable std::mutex mutex;

        /**
         * The data type, where we save multiple set of previous samples.
         */
        using matrix_type = boost::numeric::ublas::matrix<scalar_type>;

        /**
         * Current average cosine. Description of this is given above
         */
        std::vector<scalar_type> current_avg_cosine;

        /**
         * Save previous sample value
         */
        matrix_type previous_sample;
        matrix_type previous_sample_replace;

        /**
         * The number of samples processed so far.
         */
        types::sample_index n_samples;

        /**
         * Describes how many values of average cosine function we calculate.
         */
        unsigned int history_length;
    };

    template <typename InputType>
    AverageCosineBetweenSuccessiveSamples<InputType>::
    AverageCosineBetweenSuccessiveSamples (const unsigned int length)
      :
      Consumer<InputType>(ParallelMode::synchronous),
      history_length(length),
      n_samples (0)

    {}



    template <typename InputType>
    AverageCosineBetweenSuccessiveSamples<InputType>::
    ~AverageCosineBetweenSuccessiveSamples ()
    {
      this->disconnect_and_flush();
    }



    template <typename InputType>
    void
    AverageCosineBetweenSuccessiveSamples<InputType>::
    consume (InputType sample, AuxiliaryData /*aux_data*/)
    {
      std::lock_guard<std::mutex> lock(mutex);

      // If this is the first sample we see, initialize all components
      // After the first sample, the average cosine vector.
      // is the zero vector since a single sample does not have any friends yet.


      if (n_samples == 0)
        {
          current_avg_cosine.resize(history_length);
          n_samples = 1;

          for (unsigned int i=0; i<history_length; ++i)
            {
              current_avg_cosine[i] = 0;
            }

          previous_sample.resize(history_length,sample.size());
          previous_sample_replace.resize(history_length,sample.size());

          for (unsigned int i=0; i<sample.size(); ++i) previous_sample(0,i) = sample[i];

        }

      else
        {
          unsigned int length1=std::min(static_cast<unsigned int>(n_samples),history_length);
          unsigned int length2=std::min(static_cast<unsigned int>(n_samples),history_length-1);

          ++n_samples;
          for (unsigned int i=0; i<length1; ++i)
            {

              //Update first dot product (alpha)
              double update=0,norm1=0, norm2=0;
              for (unsigned int j=0; j<sample.size(); ++j)
                {
                  update += sample[j]*previous_sample(i,j);
                  norm1 += sample[j]*sample[j];
                  norm2 += previous_sample(i,j)*previous_sample(i,j);
                }
              update = update/(std::sqrt(norm1*norm2));
              update -= current_avg_cosine[i];
              update /= static_cast<double>(n_samples-i-1);
              current_avg_cosine[i] += update;
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

        }
    }

//return value
    template <typename InputType>
    std::vector<typename InputType::value_type>
    AverageCosineBetweenSuccessiveSamples<InputType>::
    get () const
    {
      std::lock_guard<std::mutex> lock(mutex);

      return current_avg_cosine;
    }

  }
}

#endif
