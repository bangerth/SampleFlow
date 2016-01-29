#include "mcmc.h"
#include <boost/random/uniform_int_distribution.hpp>


namespace Samplers
{
  template <typename T>
  class MCPMC : public Base<T>
  {
  public:
    void sample (const Observers::SampleStore<T> &samples,
                 const boost::function<double (const T &)> &log_likelihood,
                 const boost::function<T (const T &)> &perturb,
                 const unsigned int               side_chain_length,
                 const unsigned int               n_samples)
      {
        boost::random::mt19937 rng;
          
        for (unsigned int i=0; i<n_samples; ++i)
          {  
            boost::random::uniform_int_distribution<typename std::vector<T>::size_type>
              uniform_distribution(0,samples.n_samples()-1);
            const T &starting_point = samples[uniform_distribution(rng)];

            Observers::LastSample<T>        last_sample;
            Samplers::MetropolisHastings<T> side_chain_sampler;
            side_chain_sampler.register_listener (boost::bind (&Observers::Base<double>::receive_sample,
                                                               boost::ref(last_sample),
                                                               _1));
            
            side_chain_sampler.sample (starting_point,
                                       log_likelihood,
                                       perturb,
                                       side_chain_length);

            const T &current_sample = last_sample.get();
            
            sample_signal (current_sample);
          }
      }
  };
}
