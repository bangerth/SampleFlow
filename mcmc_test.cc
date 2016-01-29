#include <iostream>
#include <fstream>
#include "mcmc.h"
#include "mcpmc.h"
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>

double log_likelihood (const double &x)
{
  return -(x-1)*(x-1);
}


double perturb (const double &x)
{
  static boost::random::mt19937 rng;
  static boost::random::uniform_real_distribution<> uniform_distribution(-0.01,0.01);
  return x + uniform_distribution(rng);
}


template <typename T>
void write_every_nth_sample (const unsigned int &sample_number,
                             const T &sample,
                             const unsigned int &every_n_samples,
                             std::ostream &out)
{
  if (sample_number % every_n_samples == 0)
    out << sample << std::endl;
}



int main ()
{
  Observers::SampleStore<double> mh_sample_store;
  Samplers::MetropolisHastings<double> mh_sampler;
  mh_sampler.register_listener (boost::bind(&Observers::Base<double>::receive_sample,
                                            boost::ref(mh_sample_store),
                                            _1));
  mh_sampler.sample (0,
                  &log_likelihood,
                  &perturb,
                  100000);

  
  Samplers::MCPMC<double> sampler;
  
  std::ofstream o ("output");
  Observers::StreamOutput<double> s(o);
  sampler.register_listener (boost::bind(&Observers::Base<double>::receive_sample,
                                         boost::ref(s),
                                         _1));

  std::ofstream o_m ("output.mean");
  Observers::MeanValue<double> m;
  sampler.register_listener (boost::bind(&Observers::Base<double>::receive_sample,
                                         boost::ref(m),
                                         _1));
  m.register_listener (boost::bind (&write_every_nth_sample<double>,
                                    _1,
                                    _2,
                                    1000,
                                    boost::ref(o_m)));
  
  sampler.sample (mh_sample_store,
                  &log_likelihood,
                  &perturb,
                  10,
                  10000);

}
