#include <iostream>
#include <fstream>
#include <valarray>

#include <sampleflow/producers/metropolis_hastings.h>
#include <sampleflow/filters/take_every_nth.h>
#include <sampleflow/consumers/mean_value.h>
#include <sampleflow/consumers/histogram.h>
#include <sampleflow/consumers/maximum_probability_sample.h>
#include <sampleflow/consumers/stream_output.h>


namespace Test1
{
  using SampleType = double;


  double log_likelihood (const SampleType &x)
  {
    return std::log ( std::exp(-(x-1.5)*(x-1.5)*10)
    +
    std::exp(-(x-0.5)*(x-0.5)*10));
  }


  SampleType perturb (const SampleType &x)
  {
    static std::mt19937 rng;
    static std::uniform_real_distribution<> uniform_distribution(-0.1,0.1);
    return x + uniform_distribution(rng);
  }



  void test ()
  {
    SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;

    SampleFlow::Filters::TakeEveryNth<SampleType> take_every_nth (25);
    take_every_nth.connect_to_producer (mh_sampler);

    SampleFlow::Consumers::MeanValue<SampleType> mean_value;
    mean_value.connect_to_producer (take_every_nth);

    SampleFlow::Consumers::Histogram<SampleType> histogram (0.1, 5, 100,
        SampleFlow::Consumers::Histogram<SampleType>::SubdivisionScheme::logarithmic);
    histogram.connect_to_producer (take_every_nth);

    SampleFlow::Consumers::MaximumProbabilitySample<SampleType> MAP_point;
    MAP_point.connect_to_producer (mh_sampler);

    std::ofstream samples ("samples.txt");
    SampleFlow::Consumers::StreamOutput<SampleType> stream_output(samples);
    stream_output.connect_to_producer(take_every_nth);


    mh_sampler.sample (0,
        &log_likelihood,
        &perturb,
        100000);

    std::cout << "Computed mean value: "
        << mean_value.get() << std::endl;

    std::cout << "Computed MAP point: "
        << MAP_point.get() << std::endl;

    histogram.write_gnuplot (std::ofstream("hist.txt"));
  }
}



namespace Test2
{
  using SampleType = std::valarray<double>;

  double log_likelihood (const SampleType &x)
  {
    double likelihood = 0;

    const double sigma = 0.1;

    for (auto el : x)
      likelihood += (std::log ( std::exp(-(el-1.5)*(el-1.5)/(2*sigma*sigma))
                     +
                     std::exp(-(el-0.5)*(el-0.5)/(2*sigma*sigma))));

    return likelihood;
  }


  SampleType perturb (const SampleType &x)
  {
    static std::mt19937 rng;
    static std::uniform_real_distribution<> uniform_distribution(-0.1,0.1);

    SampleType y = x;

    for (auto &el : y)
      el += uniform_distribution(rng);

    return y;
  }



  void test ()
  {
    SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;

    SampleFlow::Filters::TakeEveryNth<SampleType> take_every_nth (25);
    take_every_nth.connect_to_producer (mh_sampler);

    SampleFlow::Consumers::MeanValue<SampleType> mean_value;
    mean_value.connect_to_producer (take_every_nth);

//    SampleFlow::Consumers::Histogram<SampleType> histogram (0.1, 5, 100,
//        SampleFlow::Consumers::Histogram<SampleType>::SubdivisionScheme::logarithmic);
//    histogram.connect_to_producer (take_every_nth);

    SampleFlow::Consumers::MaximumProbabilitySample<SampleType> MAP_point;
    MAP_point.connect_to_producer (mh_sampler);

    std::ofstream samples ("samples.txt");
    SampleFlow::Consumers::StreamOutput<SampleType> stream_output(samples);
    stream_output.connect_to_producer(take_every_nth);


    mh_sampler.sample ({1,0},
        &log_likelihood,
        &perturb,
        5000000);

    std::cout << "Computed mean value: "
        << mean_value.get()[0] << ' ' << mean_value.get()[1] << std::endl;

    std::cout << "Computed MAP point: "
        << MAP_point.get()[0] << ' ' << MAP_point.get()[1] << std::endl;

//    histogram.write_gnuplot (std::ofstream("hist.txt"));
  }
}


int main ()
{
  Test2::test ();
}

