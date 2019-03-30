#include "mcmc.h"
#include <random>

/*
namespace Samplers
{
  template <typename T>
  class MCPMC : public Base<T>
  {
  public:
    void sample (const Observers::SampleStore<T> &samples,
                 const std::function<double (const T &)> &log_likelihood,
                 const std::function<T (const T &)> &perturb,
                 const unsigned int               side_chain_length,
                 const unsigned int               n_samples)
    {
      std::mt19937 rng;

      for (unsigned int i=0; i<n_samples; ++i)
        {
          std::uniform_int_distribution<typename std::vector<T>::size_type>
          uniform_distribution(0,samples.n_samples()-1);
          const T &starting_point = samples[uniform_distribution(rng)];

          Observers::LastSample<T>        last_sample;
          Samplers::MetropolisHastings<T> side_chain_sampler;
          side_chain_sampler.register_listener (std::bind (&Observers::Base<double>::receive_sample,
                                                           std::ref(last_sample),
                                                           std::placeholders::_1));

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

*/
