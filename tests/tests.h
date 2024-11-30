// ---------------------------------------------------------------------
//
// Copyright (C) 2020 by the SampleFlow authors.
//
// This file is part of the SampleFlow library.
//
// The deal.II library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE.md at
// the top level directory of deal.II.
//
// ---------------------------------------------------------------------


#ifndef SAMPLEFLOW_TESTS_H
#define SAMPLEFLOW_TESTS_H


namespace SampleFlow
{
  namespace Testing
  {
    /**
     * Whereas the random number generators are portable between
     * compilers, the random number distributions are not. This class
     * makes sure we remain compatible at least between GCC and Clang
     * by creating a class that, given a random number generator,
     * returns normally distributed random floating point numbers.
     *
     * The class is a drop-in replacement for std::normal_distribution.
     */
    template<typename RealType = double>
    class NormalDistribution
    {
        static_assert(std::is_floating_point_v<RealType>,
                      "result_type must be a floating point type");

      public:
        using result_type = RealType;


      public:
        explicit
        NormalDistribution(result_type mu,
                           result_type sigma)
          : mean(mu), stddev(sigma)
        { }

        template<typename RNG>
        result_type
        operator()(RNG &rng);

      private:
        result_type mean, stddev;
        result_type saved_value = 0;
        bool        saved_value_available = false;
    };



    template<typename RealType>
    template<typename RNG>
    typename NormalDistribution<RealType>::result_type
    NormalDistribution<RealType>::
    operator()(RNG &rng)
    {
      result_type ret;

      auto get_real = [&rng]()
      {
        return std::generate_canonical<RealType,
               std::numeric_limits<RealType>::digits,
               RNG>(rng);
      };

      if (saved_value_available)
        {
          saved_value_available = false;
          ret = saved_value;
        }
      else
        {
          result_type x, y, r2;
          do
            {
              x = result_type(2.0) * get_real() - 1.0;
              y = result_type(2.0) * get_real() - 1.0;
              r2 = x * x + y * y;
            }
          while (r2 > 1.0 || r2 == 0.0);

          const result_type mult = std::sqrt(-2 * std::log(r2) / r2);
          ret = y * mult;
          saved_value = x * mult;
          saved_value_available = true;

        }
      return ret * stddev + mean;
    }
  }
}


#endif
