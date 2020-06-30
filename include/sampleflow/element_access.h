// ---------------------------------------------------------------------
//
// Copyright (C) 2019 by the SampleFlow authors.
//
// This file is part of the SampleFlow library.
//
// The SampleFlow library is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE.md at
// the top level directory of SampleFlow.
//
// ---------------------------------------------------------------------

#ifndef SAMPLEFLOW_UTILITIES_H
#define SAMPLEFLOW_UTILITIES_H

#include <cstddef>
#include <cassert>
#include <type_traits>

namespace SampleFlow
{
  /**
   * A namespace for utility functions in the SampleFlow library.
   */
  namespace Utilities
  {
    namespace internal
    {
      /**
       * A class that defines a member variable `value` that represents
       * whether the class `SampleType` given as template argument allows
       * forming subscripts, i.e., whether it has an `operator[]` that can
       * be used in expressions of the form `sample[i]` where `sample` is
       * an object of type `SampleType` and where `i` is an integer type.
       */
      template <typename SampleType>
      struct has_subscript_operator
      {
        private:
          /**
           * A function that can always be called with any argument.
           */
          static void
          detect(...);

          /**
           * A detection function that can only be called with an object of
           * a type that allows subscripting. Importantly, it returns something
           * other than the `void` return type of the function above.
           */
          template <typename U>
          static decltype(std::declval<U>()[0])
          detect(const U &);

        public:
          /**
           * A member variable that indicates whether the template type
           * of the class can be subscripted.
           */
          static constexpr bool value =
            !std::is_same<void, decltype(detect(std::declval<SampleType>()))>::value;
      };


      /**
       * A class that defines a member variable `value` that represents
       * whether objects of type `SampleType` can be passed as expressions
       * of the form `sample.size()`.
       */
      template <typename SampleType>
      struct has_size_function
      {
        private:
          /**
           * A function that can always be called with any argument.
           */
          static void
          detect(...);

          /**
           * A detection function that can only be called with an object of
           * a type that allows calling `sample.size()`. Importantly, it
           * returns something other than the `void` return type of the
           * function above.
           */
          template <typename U>
          static decltype(std::declval<U>().size())
          detect(const U &);

        public:
          /**
           * A member variable that indicates whether the template type
           * of the class can be used in expressions of the form
           * `sample.size()`.
           */
          static constexpr bool value =
            !std::is_same<void, decltype(detect(std::declval<SampleType>()))>::value;
      };
    }

    /**
     * A function that, for types `SampleType` for which one can build
     * expressions of the form `sample.size()`, returns the size of the
     * object. This is used in classes such as
     * Consumers::CovarianceMatrix that need to size a matrix corresponding
     * to the number of elements in a sample type.
     * In case a `SampleType` represents a scalar that can not be decomposed
     * into smaller pieces, there is a separate function that allows
     * the expression `size(sample)` simply returning 1. There is also
     * a specialization of the function for the case that `SampleType`
     * is an array type.
     */
    template <typename SampleType>
    auto size (const SampleType &sample)
    -> typename std::enable_if<internal::has_size_function<SampleType>::value == true,
    decltype(std::declval<SampleType>().size())>::type
    {
      return sample.size();
    }


    /**
     * A function that, for types `SampleType` for which one can not build
     * expressions of the form `std::size(sample)`, simply returns 1
     * This is meant to make it possible to access scalar `SampleType` objects
     * in the same way as one does with arrays of objects (or
     * `std::vector<T>` or `std::valarray<T>`, or similar things).
     */
    template <typename SampleType>
    auto size (const SampleType &sample)
    -> typename std::enable_if<internal::has_size_function<SampleType>::value == false
    &&
    std::is_array<SampleType>::value == false,
        std::size_t>::type
    {
      return 1;
    }


    /**
     * A function that, for types `SampleType` for which one can not build
     * expressions of the form `std::size(sample)`, simply returns 1
     * This is meant to make it possible to access scalar `SampleType` objects
     * in the same way as one does with arrays of objects (or
     * `std::vector<T>` or `std::valarray<T>`, or similar things).
     */
    template <typename SampleType>
    auto size (const SampleType &sample)
    -> typename std::enable_if<internal::has_size_function<SampleType>::value == false
    &&
    std::is_array<SampleType>::value == true,
        std::size_t>::type
    {
      // We now know that SampleType is an array type of the form
      // `scalar[size]`. We just need to return `size`.
      return std::extent<SampleType>::value;
    }


    /**
     * A function that, for types `SampleType` for which one can build
     * expressions of the form `sample[i]`, returns the `index`th
     * element of the sample. This is used in classes such as
     * Consumers::CovarianceMatrix to compute expressions that require
     * indexing into the sample type -- assuming this is possible.
     * In case a `SampleType` represents a scalar that can not be decomposed
     * into smaller pieces, there is a separate function that allows
     * the expression `get_nth_element(sample,0)` returning the
     * entire sample itself.
     */
    template <typename SampleType>
    auto get_nth_element (const SampleType &sample,
                          const std::size_t index)
    -> typename std::enable_if<internal::has_subscript_operator<SampleType>::value == true,
    typename std::remove_cv<
    typename std::remove_reference<
    decltype(std::declval<SampleType>()[index])>::type>::type>::type
    {
      assert (index < size(sample));
      return sample[index];
    }



    /**
     * Like the previous function, but for non-`const` objects for which the
     * returned object is a reference to the elements of the `sample` object.
     */
    template <typename SampleType>
    auto get_nth_element (SampleType       &sample,
                          const std::size_t index)
    -> typename std::enable_if<internal::has_subscript_operator<SampleType>::value == true,
    typename std::remove_cv<
    typename std::remove_reference<
    decltype(std::declval<SampleType>()[index])>::type>::type>::type &
    {
      assert (index < size(sample));
      return sample[index];
    }


    /**
     * A function that, for types `SampleType` for which one can not build
     * expressions of the form `sample[i]`, simply returns the object itself.
     * This is meant to make it possible to access scalar `SampleType` objects
     * in the same way as one does with arrays of objects (or
     * `std::vector<T>` or `std::valarray<T>`, or similar things).
     *
     * If an object does not allow accessing array elements, then clearly
     * the only valid value for the `index` argument to this function is zero.
     */
    template <typename SampleType>
    auto get_nth_element (const SampleType &sample,
                          const std::size_t index)
    -> typename std::enable_if<internal::has_subscript_operator<SampleType>::value == false,
    SampleType>::type
    {
      assert (index == 0);
      return sample;
    }


    /**
     * Like the previous function, but for non-`const` objects for which the
     * returned object is a reference to the (single) element
     * of the `sample` object.
     */
    template <typename SampleType>
    auto get_nth_element (SampleType       &sample,
                          const std::size_t index)
    -> typename std::enable_if<internal::has_subscript_operator<SampleType>::value == false,
    SampleType>::type &
    {
      assert (index == 0);
      return sample;
    }
  }
}

#endif
