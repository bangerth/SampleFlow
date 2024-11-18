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

#include <sampleflow/config.h>

#include <sampleflow/concepts.h>

#include <cstddef>
#include <cassert>
#include <type_traits>

namespace SampleFlow
{
  namespace Utilities
  {
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
    requires (Concepts::has_size_function<SampleType>)
    auto size (const SampleType &sample)
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
    requires (!Concepts::has_size_function<SampleType>  &&!std::is_array_v<SampleType>)
    auto size (const SampleType &sample)
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
    requires (!Concepts::has_size_function<SampleType>  &&std::is_array_v<SampleType>)
    auto size (const SampleType &sample)
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
    requires (Concepts::has_subscript_operator<SampleType>)
    auto get_nth_element (const SampleType &sample,
                          const std::size_t index)
    -> std::remove_cv_t<std::remove_reference_t<decltype(std::declval<SampleType>()[index])>>
    {
      assert (index < Utilities::size(sample));
      return sample[index];
    }


    /**
     * Like the previous function, but for non-`const` objects for which the
     * returned object is a reference to the elements of the `sample` object.
     */
    template <typename SampleType>
    requires (Concepts::has_subscript_operator<SampleType>)
    auto get_nth_element (SampleType &sample,
                          const std::size_t index)
    -> std::remove_cv_t<std::remove_reference_t<decltype(std::declval<SampleType>()[index])>> &
    {
      assert (index < Utilities::size(sample));
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
    requires (!Concepts::has_subscript_operator<SampleType>)
    auto get_nth_element (const SampleType &sample,
                          const std::size_t index)
    -> SampleType
    {
      assert (index == 0);
      return sample;
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
    requires (!Concepts::has_subscript_operator<SampleType>)
    auto get_nth_element (SampleType &sample,
                          const std::size_t index)
    -> SampleType &
    {
      assert (index == 0);
      return sample;
    }
  }
}

#endif
