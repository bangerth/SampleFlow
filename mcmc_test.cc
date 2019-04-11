#include <iostream>
#include <fstream>

#include <sampleflow/producers/metropolis_hastings.h>
#include <sampleflow/filters/take_every_nth.h>
#include <sampleflow/consumers/mean_value.h>
#include <sampleflow/consumers/histogram.h>



double log_likelihood (const double &x)
{
  return -(x-1)*(x-1);
}


double perturb (const double &x)
{
  static std::mt19937 rng;
  static std::uniform_real_distribution<> uniform_distribution(-0.1,0.1);
  return x + uniform_distribution(rng);
}



int main ()
{
  SampleFlow::Producers::MetropolisHastings<double> mh_sampler;

  SampleFlow::Filters::TakeEveryNth<double> take_every_nth (100);
  take_every_nth.connect_to_producer (mh_sampler);

  SampleFlow::Consumers::MeanValue<double> mean_value;
  mean_value.connect_to_producer (take_every_nth);

  SampleFlow::Consumers::Histogram<double> histogram (0.1, 5, 100,
      SampleFlow::Consumers::Histogram<double>::SubdivisionScheme::logarithmic);
  histogram.connect_to_producer (take_every_nth);

  mh_sampler.sample (0,
                     &log_likelihood,
                     &perturb,
                     1000000);

  std::cout << "Computed mean value: "
  << mean_value.get() << std::endl;

  histogram.write_gnuplot (std::ofstream("hist.txt"));
}
