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


#ifndef SAMPLEFLOW_TESTS_MY_TRIANGLE_H
#define SAMPLEFLOW_TESTS_MY_TRIANGLE_H

#include <limits>
#include <valarray>


/**
 * A class that serves as a custom sample class that is not a vector
 * of some sort.
 */
class MyTriangle
{
  public:
    std::valarray<double> side_lengths;

    MyTriangle()
      :
      side_lengths ({std::numeric_limits<double>::signaling_NaN(),
                   std::numeric_limits<double>::signaling_NaN(),
                   std::numeric_limits<double>::signaling_NaN()
    })
    {}

    MyTriangle(const std::valarray<double> lengths)
    {
      side_lengths = lengths;
    }

};


inline
std::ostream &operator<<(std::ostream &os, const MyTriangle &tri)
{
  os << "Triangle: " << tri.side_lengths[0] << ", " << tri.side_lengths[1] << ", " << tri.side_lengths[2];
  return os;
}

namespace MyRand
{
  template<typename _RealType = double>
  class normal_distribution
  {
      static_assert(std::is_floating_point<_RealType>::value,
                    "result_type must be a floating point type");

    public:
      /** The type of the range of the distribution. */
      typedef _RealType result_type;

      /** Parameter type. */
      struct param_type
      {
          typedef normal_distribution<_RealType> distribution_type;

          param_type() : param_type(0.0) { }

          explicit
          param_type(_RealType __mean, _RealType __stddev = _RealType(1))
            : _M_mean(__mean), _M_stddev(__stddev)
          {
          }

          _RealType
          mean() const
          {
            return _M_mean;
          }

          _RealType
          stddev() const
          {
            return _M_stddev;
          }

          friend bool
          operator==(const param_type &__p1, const param_type &__p2)
          {
            return (__p1._M_mean == __p2._M_mean
                    && __p1._M_stddev == __p2._M_stddev);
          }

#if __cpp_impl_three_way_comparison < 201907L
          friend bool
          operator!=(const param_type &__p1, const param_type &__p2)
          {
            return !(__p1 == __p2);
          }
#endif

        private:
          _RealType _M_mean;
          _RealType _M_stddev;
      };

    public:
      normal_distribution() : normal_distribution(0.0) { }

      /**
       * Constructs a normal distribution with parameters @f$mean@f$ and
       * standard deviation.
       */
      explicit
      normal_distribution(result_type __mean,
                          result_type __stddev = result_type(1))
        : _M_param(__mean, __stddev)
      { }

      explicit
      normal_distribution(const param_type &__p)
        : _M_param(__p)
      { }

      /**
       * @brief Resets the distribution state.
       */
      void
      reset()
      {
        _M_saved_available = false;
      }

      /**
       * @brief Returns the mean of the distribution.
       */
      _RealType
      mean() const
      {
        return _M_param.mean();
      }

      /**
       * @brief Returns the standard deviation of the distribution.
       */
      _RealType
      stddev() const
      {
        return _M_param.stddev();
      }

      /**
       * @brief Returns the parameter set of the distribution.
       */
      param_type
      param() const
      {
        return _M_param;
      }

      /**
       * @brief Sets the parameter set of the distribution.
       * @param __param The new parameter set of the distribution.
       */
      void
      param(const param_type &__param)
      {
        _M_param = __param;
      }

      /**
       * @brief Returns the greatest lower bound value of the distribution.
       */
      result_type
      min() const
      {
        return std::numeric_limits<result_type>::lowest();
      }

      /**
       * @brief Returns the least upper bound value of the distribution.
       */
      result_type
      max() const
      {
        return std::numeric_limits<result_type>::max();
      }

      /**
       * @brief Generating functions.
       */
      template<typename _UniformRandomNumberGenerator>
      result_type
      operator()(_UniformRandomNumberGenerator &__urng)
      {
        return this->operator()(__urng, _M_param);
      }

      template<typename _UniformRandomNumberGenerator>
      result_type
      operator()(_UniformRandomNumberGenerator &__urng,
                 const param_type &__p);

      template<typename _ForwardIterator,
               typename _UniformRandomNumberGenerator>
      void
      __generate(_ForwardIterator __f, _ForwardIterator __t,
                 _UniformRandomNumberGenerator &__urng)
      {
        this->__generate(__f, __t, __urng, _M_param);
      }

      template<typename _ForwardIterator,
               typename _UniformRandomNumberGenerator>
      void
      __generate(_ForwardIterator __f, _ForwardIterator __t,
                 _UniformRandomNumberGenerator &__urng,
                 const param_type &__p)
      {
        this->__generate_impl(__f, __t, __urng, __p);
      }

      template<typename _UniformRandomNumberGenerator>
      void
      __generate(result_type *__f, result_type *__t,
                 _UniformRandomNumberGenerator &__urng,
                 const param_type &__p)
      {
        this->__generate_impl(__f, __t, __urng, __p);
      }

      /**
       * @brief Return true if two normal distributions have
       *        the same parameters and the sequences that would
       *        be generated are equal.
       */
      template<typename _RealType1>
      friend bool
      operator==(const std::normal_distribution<_RealType1> &__d1,
                 const std::normal_distribution<_RealType1> &__d2);

      /**
       * @brief Inserts a %normal_distribution random number distribution
       * @p __x into the output stream @p __os.
       *
       * @param __os An output stream.
       * @param __x  A %normal_distribution random number distribution.
       *
       * @returns The output stream with the state of @p __x inserted or in
       * an error state.
       */
      template<typename _RealType1, typename _CharT, typename _Traits>
      friend std::basic_ostream<_CharT, _Traits> &
      operator<<(std::basic_ostream<_CharT, _Traits> &__os,
                 const std::normal_distribution<_RealType1> &__x);

      /**
       * @brief Extracts a %normal_distribution random number distribution
       * @p __x from the input stream @p __is.
       *
       * @param __is An input stream.
       * @param __x  A %normal_distribution random number generator engine.
       *
       * @returns The input stream with @p __x extracted or in an error
       *          state.
       */
      template<typename _RealType1, typename _CharT, typename _Traits>
      friend std::basic_istream<_CharT, _Traits> &
      operator>>(std::basic_istream<_CharT, _Traits> &__is,
                 std::normal_distribution<_RealType1> &__x);

    private:
      template<typename _ForwardIterator,
               typename _UniformRandomNumberGenerator>
      void
      __generate_impl(_ForwardIterator __f, _ForwardIterator __t,
                      _UniformRandomNumberGenerator &__urng,
                      const param_type &__p);

      param_type  _M_param;
      result_type _M_saved = 0;
      bool        _M_saved_available = false;
  };

  namespace __detail
  {
    /*
     * An adaptor class for converting the output of any Generator into
     * the input for a specific Distribution.
     */
    template<typename _Engine, typename _DInputType>
    struct _Adaptor
    {
        static_assert(std::is_floating_point<_DInputType>::value,
                      "template argument must be a floating point type");

      public:
        _Adaptor(_Engine &__g)
          : _M_g(__g) { }

        _DInputType
        min() const
        {
          return _DInputType(0);
        }

        _DInputType
        max() const
        {
          return _DInputType(1);
        }

        /*
         * Converts a value generated by the adapted random number generator
         * into a value in the input domain for the dependent random number
         * distribution.
         */
        _DInputType
        operator()()
        {
          return std::generate_canonical<_DInputType,
                 std::numeric_limits<_DInputType>::digits,
                 _Engine>(_M_g);
        }

      private:
        _Engine &_M_g;
    };
  }


  template<typename _RealType>
  template<typename _UniformRandomNumberGenerator>
  typename normal_distribution<_RealType>::result_type
  normal_distribution<_RealType>::
  operator()(_UniformRandomNumberGenerator &__urng,
             const param_type &__param)
  {
    result_type __ret;
    __detail::_Adaptor<_UniformRandomNumberGenerator, result_type>
    __aurng(__urng);

    if (_M_saved_available)
      {
        _M_saved_available = false;
        __ret = _M_saved;
      }
    else
      {
        result_type __x, __y, __r2;
        do
          {
            __x = result_type(2.0) * __aurng() - 1.0;
            __y = result_type(2.0) * __aurng() - 1.0;
            __r2 = __x * __x + __y * __y;
          }
        while (__r2 > 1.0 || __r2 == 0.0);

        const result_type __mult = std::sqrt(-2 * std::log(__r2) / __r2);
        _M_saved = __x * __mult;
        _M_saved_available = true;
        __ret = __y * __mult;
      }

    __ret = __ret * __param.stddev() + __param.mean();
    return __ret;
  }
}


inline
std::pair<MyTriangle, double> perturb(const MyTriangle &sample)
{
  static std::mt19937 gen;
  MyRand::normal_distribution<> d {0, 1};
  double side_a = sample.side_lengths[0] + d(gen);
  double side_b = sample.side_lengths[1] + d(gen);
  double side_c = sample.side_lengths[2] + d(gen);
  if (side_c > (side_a + side_b))
    side_c = side_a + side_b;
  if (side_c < std::abs(side_a - side_b))
    side_c = std::abs(side_a - side_b);
  MyTriangle result({side_a, side_b, side_c});
  // We are going to pretend that the change we are making to the triangle
  // is equally likely to a change going in the opposite direction, even
  // though this is not true because of the clipping that is happening
  // in the if statements. This is okay for the purposes of this test.
  return {result, 1.0};
}


inline
MyTriangle &operator+=(MyTriangle &tri1, const MyTriangle &tri2)
{
  tri1.side_lengths[0] += tri2.side_lengths[0];
  tri1.side_lengths[1] += tri2.side_lengths[1];
  tri1.side_lengths[2] += tri2.side_lengths[2];
  return tri1;
}


inline
MyTriangle &operator-=(MyTriangle &tri1, const MyTriangle &tri2)
{
  tri1.side_lengths[0] -= tri2.side_lengths[0];
  tri1.side_lengths[1] -= tri2.side_lengths[1];
  tri1.side_lengths[2] -= tri2.side_lengths[2];
  return tri1;
}


inline
MyTriangle &operator*=(MyTriangle &tri1, const double b)
{
  tri1.side_lengths[0] *= b;
  tri1.side_lengths[1] *= b;
  tri1.side_lengths[2] *= b;
  return tri1;
}


inline
MyTriangle &operator/=(MyTriangle &tri1, const double b)
{
  tri1.side_lengths[0] /= b;
  tri1.side_lengths[1] /= b;
  tri1.side_lengths[2] /= b;
  return tri1;
}


inline
MyTriangle operator*(const MyTriangle &tri1, const double b)
{
  MyTriangle tri = tri1;
  tri *= b;
  return tri;
}


inline
MyTriangle operator*(const double b, const MyTriangle &tri1)
{
  return (tri1 * b);
}


#endif
