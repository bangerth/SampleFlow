// ---------------------------------------------------------------------

#ifndef SAMPLEFLOW_CONSUMERS_AVERAGE_COSINUS_H
#define SAMPLEFLOW_CONSUMERS_AVERAGE_COSINUS_H

#include <sampleflow/consumer.h>
#include <sampleflow/types.h>
#include <mutex>

#include <boost/numeric/ublas/matrix.hpp>


namespace SampleFlow
{
  namespace Consumers
  {
  /**
  * This is a Consumer class that implements computing the running average cosinus of
  * angles between vectors value. This calculation, similarly to autocovariance, compares samples, that are
  * apart by specific index. In this sense, we get function where domain is bounded unsigned int and for
  * each value we calculate cosinus average.
  * Let's say unsigned int is equal to l and n_samples=n and n is bigger than l. Then the formula can be written as:
 * $\hat{cos(\theta_n)(l)}=\frac{1}{n-l}\sum_{t=1}^{n-l}{(\bm{x}_{t+l}^T\bar\bm{x})(||bm{x}_{t}||*||\bar\bm{x}||)}.
 *
 * This code for every new sample updates $\hat{cos(\theta_n)(l)}, l=1,2,3...,L$. The value of
 * cosinus length can done by setting it in mcmc_test.cc.
 *
 * Notice, that for each new sample $x_n$, we need to take a sample, that was l earlier than new sample ($x_{n-l}).
 * Working this way, the updating algorithm becomes very similar to one used in the MeanValue class.
 *
 * Every time, for updating we calculate corresponding fractions. There numerator is the dot product, while denominator -
 * multiplied those two vector norms
 *
 * ### Threading model ###
 *
 * The implementation of this class is thread-safe, i.e., its
 * consume() member function can be called concurrently and from multiple
 * threads.
   **/

    template <typename InputType>
    class AverageCosinus: public Consumer<InputType>
    {
      public:
        /**
         * The data type of the elements of the input type.
         */

       using scalar_type = typename InputType::value_type;

       /**
   	 * The data type, where we save multiple set of past samples.
   	 */

       using value_type = boost::numeric::ublas::matrix<scalar_type>;

        /**
         * Constructor.
         */

        AverageCosinus(unsigned int length);

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
         * A function that returns the average cosinus vector computed from the
         * samples seen so far. If no samples have been processed so far, then
         * a default-constructed object of type InputType will be returned.
         *
         * @return The computed average cosinus vector.
         */
        std::vector<scalar_type> get() const;

      private:
        /**
         * A mutex used to lock access to all member variables when running
         * on multiple threads.
         */

      mutable std::mutex mutex;


        /**
         * Parts for running average_cosinus
         * Description of these parts is given above
         */

      std::vector<scalar_type> current_avg_cosinus;//Current average cosinus

        /**
        * Save previous sample value
        * %%%%%%%%%%%%Manty's comment: I didn't find (I'm 100% there are) better way to save last k sample values
        * as just using one more object and assign i'th row of past_sample to i+1'th row of past_sample_replace
        *
        */

      value_type past_sample;
      value_type past_sample_replace;

        /**
         * The number of samples processed so far.
         */

        types::sample_index n_samples;

        unsigned int avg_cosinus_length;
    };

    template <typename InputType>
    AverageCosinus<InputType>::
	AverageCosinus (unsigned int length)
      :
		avg_cosinus_length(length),
      n_samples (0)

    {}


    template <typename InputType>
    void
	AverageCosinus<InputType>::
    consume (InputType sample, AuxiliaryData /*aux_data*/)
    {
      std::lock_guard<std::mutex> lock(mutex);

      // If this is the first sample we see, initialize all components
      // After the first sample, the average cosinus vector.
      // is the zero vector since a single sample does not have any friends yet.


      if (n_samples == 0)
        {
          current_avg_cosinus.resize(avg_cosinus_length);
          n_samples = 1;

          for (unsigned int i=0; i<avg_cosinus_length; ++i){
        	  current_avg_cosinus[i] = 0;
          	  }

          past_sample.resize(avg_cosinus_length,sample.size());
          past_sample_replace.resize(avg_cosinus_length,sample.size());

          for (unsigned int j=0; j<sample.size(); ++j) past_sample(0,j) = sample[j];
        }

      else
		{
     	unsigned int length1=std::min(static_cast<unsigned int>(n_samples),avg_cosinus_length);
     	unsigned int length2=std::min(static_cast<unsigned int>(n_samples),avg_cosinus_length-1);

    	 ++n_samples;
    	for (unsigned int i=0; i<length1; ++i){

    		//Update first dot product (alpha)
    		    double update=0,norm1=0, norm2=0;
    		    for (unsigned int j=0; j<sample.size(); ++j){
    		    	update += sample[j]*past_sample(i,j);
    		    	norm1 += sample[j]*sample[j];
    		    	norm2 += past_sample(i,j)*past_sample(i,j);
    		    	}
    		    update = update/(std::sqrt(norm1*norm2));
    		    update -= current_avg_cosinus[i];
    		    update /= static_cast<double>(n_samples-i);
    		    current_avg_cosinus[i] += update;
    		}
    		//Save needed past values
    	for (unsigned int i=0; i<length2; ++i){
    		for (unsigned int j=0; j<sample.size(); ++j){
    			past_sample_replace(i+1,j)=past_sample(i,j);
    			}
    	}

    	for (unsigned int j=0; j<sample.size(); ++j) past_sample_replace(0,j) = sample[j];
    	past_sample = past_sample_replace;

	}
   }

//return value
    template <typename InputType>
    std::vector<typename InputType::value_type>
	AverageCosinus<InputType>::
    get () const
    {
      std::lock_guard<std::mutex> lock(mutex);

      return current_avg_cosinus;
    }

  }
}

#endif
