#ifndef MCMC_H
#define MCMC_H

#include <boost/signals2.hpp>

#include <random>
#include <functional>
#include <iostream>


namespace Samplers
{
  template <typename T>
  class Base
  {
  public:
    void register_listener (const std::function<void (const T &)> &f)
    {
      sample_signal.connect (f);
    }

  private:
    boost::signals2::signal<void (const T &)> sample_signal;
  };


  template <typename T>
  class MetropolisHastings : public Base<T>
  {
  public:
    void sample (const T &starting_point,
                 const std::function<double (const T &)> &log_likelihood,
                 const std::function<T (const T &)> &perturb,
                 const unsigned int n_samples)
    {
      std::mt19937 rng;
      std::uniform_real_distribution<> uniform_distribution(0,1);

      T      current_sample         = starting_point;
      double current_log_likelihood = log_likelihood (current_sample);

      sample_signal(current_sample);

      for (unsigned int i=0; i<n_samples; ++i)
        {
          const T      trial_sample         = perturb (current_sample);
          const double trial_log_likelihood = log_likelihood (trial_sample);

          if ((trial_log_likelihood > current_log_likelihood)
              ||
              (exp(trial_log_likelihood)/exp(current_log_likelihood) >= uniform_distribution(rng)))
            {
              current_sample         = trial_sample;
              current_log_likelihood = trial_log_likelihood;
            }

          sample_signal (current_sample);
        }
    }
  };
}



namespace Observers
{
  template <typename T>
  class Base
  {
  public:
    virtual void receive_sample (const T &t) = 0;
  };


  template <typename T>
  class MeanValue : public Base<T>
  {
  public:
    MeanValue()
      :
      terms_in_sum (0)
    {}


    virtual void receive_sample (const T &t)
    {
      if (terms_in_sum == 0)
        sample_signal(0, sum);

      sum += t;
      ++terms_in_sum;

      sample_signal(terms_in_sum, sum/terms_in_sum);
    }

    void register_listener (const std::function<void (const unsigned int &sample_number,
                                                      const T &current_mean)>  &f)
    {
      sample_signal.connect (f);
    }

  private:
    T            sum;
    unsigned int terms_in_sum;

    boost::signals2::signal<void (const unsigned int &, const T &)> sample_signal;
  };


  template <typename T>
  class StreamOutput : public Base<T>
  {
  public:
    StreamOutput (std::ostream &out)
      :
      out (out)
    {}

    virtual void receive_sample (const T &t)
    {
      out << t << std::endl;
    }

  private:
    std::ostream &out;
  };


  template <typename T>
  class SampleStore : public Base<T>
  {
  public:
    virtual void receive_sample (const T &t)
    {
      samples.push_back (t);
    }

    typename std::vector<T>::size_type
    n_samples () const
    {
      return samples.size();
    }

    const T &
    operator[] (const typename std::vector<T>::size_type i) const
    {
      return samples[i];
    }

  private:
    std::vector<T> samples;
  };



  template <typename T>
  class LastSample : public Base<T>
  {
  public:
    virtual void receive_sample (const T &t)
    {
      sample = t;
    }

    const T &
    get () const
    {
      return sample;
    }

  private:
    T sample;
  };

}


#endif
