// ---------------------------------------------------------------------

#ifndef SAMPLEFLOW_CONSUMERS_AVERAGE_COSINE_H
#define SAMPLEFLOW_CONSUMERS_AVERAGE_COSINE_H

#include <sampleflow/consumer.h>
#include <sampleflow/types.h>
#include <list>
#include <mutex>
#include <deque>


#include <boost/numeric/ublas/matrix.hpp>


namespace SampleFlow
{
  namespace Consumers
  {
    /**
     * This is a Consumer class that implements computing the running average cosine of
     * angles between vectors value. This calculation, similarly to autocovariance, compares samples, that are
     * apart by specific index. In this sense, we get function where domain is bounded unsigned int and for
     * each value we calculate average cosine of angles.
     *
     * Let's say unsigned int is equal to l, n_samples=n, n is bigger than l and \hat{cos(\theta_n)(l) is estimated average
     * cosinus of angles between vectors that are apart by l. Then the formula can be written as:
     * $\hat{cos(\theta_n)(l)}=\frac{1}{n-l}\sum_{t=1}^{n-l}{(x_{t+l}^T\bar{x})(\|x_{t}\| \; \|\bar{x}\|)}$.
     *
     * This code for every new sample updates $\hat{cos(\theta_n)(l)}, l=1,2,3,\ldots,L$. The value of
     * $L$ is provided to the constructor of this class.
     *
     * Notice, that for each new sample $x_n$, we need to take a sample, that was $l$ earlier than new sample ($x_{n-l}$).
     *
     * ### Threading model ###
     *
     * The implementation of this class is thread-safe, i.e., its
     * consume() member function can be called concurrently and from multiple
     * threads.
     **/
    template <typename InputType>
    class AverageCosineBetweenSuccessiveSamples: public Consumer<InputType>
    {
    public:
      /**
       * The data type of the elements of the input type. This type is used to define type of members, used in
       * calculations.
       */
      using scalar_type = types::ScalarType<InputType>;

      /**
       * The data type, where we save multiple set of previous samples.
       */
      using value_type = std::vector<scalar_type>;

      /**
       * Constructor.
       *
       * @param[in] lag_length A number that indicates how many average_cosinus
       *   values we want to calculate, i.e., how far back in the past we
       *   want to check how correlated (how big cosinus value) each sample is.
       */
      AverageCosineBetweenSuccessiveSamples(const unsigned int length);

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
       * a default-constructed object of value_type will be returned.
       *
       * @return The computed average cosine vector.
       */
      value_type get() const;

    private:
      /**
       * A mutex used to lock access to all member variables when running
       * on multiple threads.
       */
      mutable std::mutex mutex;

      /**
       * Current average cosine. Description of this is given above
       */
      value_type current_avg_cosine;


      /**
       * A data type used to store the past few samples.
       */
      using PreviousSamples = std::deque<InputType>;

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

      /**
       * Describes how many values of average cosine function we calculate.
       */
      unsigned int history_length;
    };

    template <typename InputType>
    AverageCosineBetweenSuccessiveSamples<InputType>::
    AverageCosineBetweenSuccessiveSamples (const unsigned int length)
    :
    history_length(length),
    n_samples (0)

    {}


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

	  // Push the first sample to the front of the list of samples:
	  previous_samples.push_front (sample);

	}
      else
	{
	  ++n_samples;
	  for (unsigned int l=0; l<previous_samples.size(); ++l)
	    {
	      //Calculate each member of fractions separately
	      double update=0,norm1=0, norm2=0;
	      for (unsigned int j=0; j<sample.size(); ++j)
		{
		  update += sample[j]*previous_samples[l][j];
		  norm1 += sample[j]*sample[j];
		  norm2 += previous_samples[l][j]*previous_samples[l][j];
		}
	      //Put calculations together
	      update = update/(std::sqrt(norm1*norm2));
	      update -= current_avg_cosine[l];
	      update /= static_cast<double>(n_samples-l-1);
	      current_avg_cosine[l] += update;
	    }

	  // Now save the sample. If the list is becoming longer than the lag
	  // length, drop the oldest sample.
	  previous_samples.push_front (sample);
	  if (previous_samples.size() > history_length)
	    previous_samples.pop_back ();

	}
    }

    //return value
    template <typename InputType>
    std::vector<typename types::ScalarType<InputType>>
    AverageCosineBetweenSuccessiveSamples<InputType>::
    get () const
    {
      std::lock_guard<std::mutex> lock(mutex);

      return current_avg_cosine;
    }

  }
}

#endif
