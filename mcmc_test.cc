#include <iostream>
#include <fstream>
#include <valarray>

#include <sampleflow/producers/metropolis_hastings.h>
#include <sampleflow/filters/take_every_nth.h>
#include <sampleflow/consumers/mean_value.h>
#include <sampleflow/consumers/covariance_matrix.h>
#include <sampleflow/consumers/spurious_autocovariance_dim_n.h>//Spuriuos autocovariance for not one dimensional sample
#include <sampleflow/consumers/acceptance_ratio.h>
#include <sampleflow/consumers/average_cosinus.h>
#include <sampleflow/consumers/histogram.h>
#include <sampleflow/consumers/maximum_probability_sample.h>
#include <sampleflow/consumers/stream_output.h>

namespace Test1
{
  using SampleType = double;


  double log_likelihood (const SampleType &x)
  {
    return std::log ( std::exp(-(x)*(x)/2));
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

    SampleFlow::Filters::TakeEveryNth<SampleType> take_every_nth (100);
    take_every_nth.connect_to_producer (mh_sampler);

    SampleFlow::Consumers::MeanValue<SampleType> mean_value;
    mean_value.connect_to_producer (take_every_nth);

   /*SampleFlow::Consumers::AutocovarianceDim1<SampleType> autocovariance_dim1(15);
    		autocovariance_dim1.connect_to_producer (take_every_nth);*/

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
        500000);

    std::cout << "Computed mean value: "
        << mean_value.get() << std::endl;

   /*std::cout << "Computed autocovariance matrix: "
               << autocovariance_dim1.get()(0,2) << ' '
               << autocovariance_dim1.get()(1,2) << ' '
               << autocovariance_dim1.get()(2,2) << ' '
               << autocovariance_dim1.get()(3,2) << ' '
               << autocovariance_dim1.get()(4,2) << ' '
               << autocovariance_dim1.get()(5,2) << ' '
               << autocovariance_dim1.get()(6,2) << ' '
               << autocovariance_dim1.get()(7,2) << ' '
               << autocovariance_dim1.get()(8,2) << ' '
               << autocovariance_dim1.get()(9,2)
               << std::endl;*/


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

//    for (auto el : x)
//      likelihood += std::log ( std::exp(-(el)*(el)/2));

    for (unsigned int i=0; i<x.size(); ++i)
    	likelihood += -(x[i])*(x[i])/(2);

    return likelihood;
  }


  SampleType perturb (const SampleType &x)
  {
    static std::mt19937 rng;
    static std::uniform_real_distribution<> uniform_distribution(-2,2);
    static std::uniform_real_distribution<> tst_distribution(-0.00005,0.00005);

    SampleType y = x;

    for (unsigned int i=0; i<y.size(); ++i){
      if(i!=1) {
      y[i] += uniform_distribution(rng);
      } else {
      y[i] += tst_distribution(rng);
      }
    }
    return y;
  }


  void test ()
  {
    SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;

    SampleFlow::Filters::TakeEveryNth<SampleType> take_every_nth (5);
    take_every_nth.connect_to_producer (mh_sampler);

    SampleFlow::Consumers::MeanValue<SampleType> mean_value;
    mean_value.connect_to_producer (take_every_nth);

    SampleFlow::Consumers::CovarianceMatrix<SampleType> covariance_matrix;
    covariance_matrix.connect_to_producer (take_every_nth);

    const unsigned int AC_length = 10;
    SampleFlow::Consumers::Spurious_Autocovariance<SampleType> autocovariance(AC_length);
    autocovariance.connect_to_producer (take_every_nth);

    SampleFlow::Consumers::AverageCosinus<SampleType> average_cosinus(AC_length);
    average_cosinus.connect_to_producer (take_every_nth);

    SampleFlow::Consumers::AcceptanceRatio<SampleType> acceptance_ratio;
    acceptance_ratio.connect_to_producer (mh_sampler);

//    SampleFlow::Consumers::Histogram<SampleType> histogram (0.1, 5, 100,
//        SampleFlow::Consumers::Histogram<SampleType>::SubdivisionScheme::logarithmic);
//    histogram.connect_to_producer (take_every_nth);

    SampleFlow::Consumers::MaximumProbabilitySample<SampleType> MAP_point;
    MAP_point.connect_to_producer (mh_sampler);

    std::ofstream samples ("samples.txt");
    SampleFlow::Consumers::StreamOutput<SampleType> stream_output(samples);
    stream_output.connect_to_producer(take_every_nth);

    mh_sampler.sample ({0,1},
        &log_likelihood,
        &perturb,
        100000);

    std::cout << "Computed mean value: "
        << mean_value.get()[0] << ' ' << mean_value.get()[1] << std::endl;

    std::cout << "Computed covariance matrix: "
        << covariance_matrix.get()(0,0) << ' '
        << covariance_matrix.get()(1,0) << ' '
        << covariance_matrix.get()(0,1) << ' '
        << covariance_matrix.get()(1,1)
        << std::endl;

    std::cout << "Computed spurious autocovariance vector: ";
    for (unsigned int k=0; k<std::min(AC_length, 10u); ++k)
    	std::cout << autocovariance.get()[k] << ' ';
    if (AC_length > 10)
    	std::cout << "...";
    std::cout << std::endl;

    std::cout << "Computed average cosinus vector: ";
    for (unsigned int k=0; k<std::min(AC_length, 10u); ++k)
    	std::cout << average_cosinus.get()[k] << ' ';
    if (AC_length > 10)
    	std::cout << "...";
    std::cout << std::endl;

    std::ofstream AC_file ("AC.txt");
    for (unsigned int k=0; k<AC_length; ++k)
    	AC_file << autocovariance.get()[k] << '\n';

    std::ofstream AvgCos_file ("AvgCos.txt");
        for (unsigned int k=0; k<AC_length; ++k)
        	AvgCos_file << average_cosinus.get()[k] << '\n';

    std::cout << "Computed MAP point: "
        << MAP_point.get()[0] << ' ' << MAP_point.get()[1] << std::endl;

    std::cout << "Computed Acceptance ratio: "
            << acceptance_ratio.get() << std::endl;

//    histogram.write_gnuplot (std::ofstream("hist.txt"));
  }
}


int main ()
{
  Test2::test ();
}





