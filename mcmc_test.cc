#include <iostream>
#include <fstream>
#include <valarray>

#include <sampleflow/producers/metropolis_hastings.h>
#include <sampleflow/filters/take_every_nth.h>
#include <sampleflow/filters/component_splitter.h>
#include <sampleflow/consumers/mean_value.h>
//#include <sampleflow/consumers/count_samples.h>

#include <sampleflow/consumers/histogram.h>
#include <sampleflow/consumers/maximum_probability_sample.h>
#include <sampleflow/consumers/stream_output.h>

#include <sampleflow/consumers/covariance_matrix.h>
#include <sampleflow/consumers/spurious_autocovariance_dim_n.h>
#include <sampleflow/consumers/acceptance_ratio.h>
#include <sampleflow/consumers/average_cosinus.h>

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

    SampleFlow::Consumers::Histogram<SampleType> histogram (0.1, 5, 100);
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
        << MAP_point.get().first << std::endl;

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

    SampleFlow::Consumers::CovarianceMatrix<SampleType> covariance_matrix;
    covariance_matrix.connect_to_producer (take_every_nth);

    const unsigned int AC_length = 10;
    SampleFlow::Consumers::SpuriousAutocovariance<SampleType> autocovariance(AC_length);
    autocovariance.connect_to_producer (take_every_nth);

    SampleFlow::Consumers::AverageCosineBetweenSuccessiveSamples<SampleType> average_cosinus(AC_length);
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

    mh_sampler.sample ({1,0},
        &log_likelihood,
        &perturb,
        500000);

    std::cout << "Computed mean value: "
        << mean_value.get()[0] << ' ' << mean_value.get()[1] << std::endl;

    std::cout << "Computed covariance matrix: "
        << covariance_matrix.get()(0,0) << ' '
        << covariance_matrix.get()(1,0) << ' '
        << covariance_matrix.get()(0,1) << ' '
        << covariance_matrix.get()(1,1)
        << std::endl;

    std::cout << "Computed MAP point: "
        << MAP_point.get().first[0] << ' ' << MAP_point.get().first[1] << std::endl;

    
    std::cout << "Computed Acceptance ratio: "
        << acceptance_ratio.get() << std::endl;

//    histogram.write_gnuplot (std::ofstream("hist.txt"));
  }
}

namespace Test3
{
  using SampleType = std::valarray<double>;

  class ReadFromFile : public SampleFlow::Producer<SampleType>
  {
    public:
      void
      read_from (std::istream &input);
  };


  void
  ReadFromFile::
  read_from (std::istream &input)
  {
    while (input)
      {
        double log_likelihood;
        unsigned independent_sample;
        std::valarray<double> sample(64);

        input >> log_likelihood >> independent_sample;
        for (auto &el : sample)
          input >> el;


        if (!input)
          return;

        this->issue_sample (sample,
        {{"relative log likelihood", boost::any(log_likelihood)}});
      }
  }


  void test (const std::string &base_name)
  {
    ReadFromFile reader;

    SampleFlow::Consumers::MeanValue<SampleType> mean_value;
    mean_value.connect_to_producer (reader);

    SampleFlow::Consumers::MaximumProbabilitySample<SampleType> MAP_point;
    MAP_point.connect_to_producer (reader);

//    SampleFlow::Consumers::CountSamples<SampleType> sample_count;
//    sample_count.connect_to_producer (reader);

    std::vector<SampleFlow::Filters::ComponentSplitter<SampleType>> component_splitters;
    std::vector<SampleFlow::Consumers::Histogram<SampleType::value_type>> histograms;
    component_splitters.reserve(64);
    histograms.reserve(64);
    for (unsigned int c=0; c<64; ++c)
      {
        component_splitters.emplace_back (c);
        component_splitters.back().connect_to_producer (reader);

        histograms.emplace_back(-3, 3, 1000, &exp10);
        histograms.back().connect_to_producer (component_splitters[c]);
      }

    std::cout << "Reading from <" << base_name << ".txt>" << std::endl;
    {
      std::ifstream input(base_name + ".txt");
      reader.read_from(input);
    }

//    std::cout << "Number of samples: " << sample_count.get() << std::endl;

    {
      std::ofstream output (base_name + ".mean");
      for (const auto &el : mean_value.get())
        output << el << '\n';
    }

    {
      std::ofstream output (base_name + ".MAP");
      for (const auto &el : MAP_point.get().first)
        output << el << '\n';
    }

    for (unsigned int c=0; c<64; ++c)
      histograms[c].write_gnuplot (std::ofstream(base_name + ".histogram." + std::to_string(c)));
  }
}


int main (int argc, char **argv)
{
  Test2::test ();
  //  Test3::test (argv[1]);
}
