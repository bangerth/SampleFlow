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

#ifndef SAMPLEFLOW_TYPES_H
#define SAMPLEFLOW_TYPES_H

#include <cstddef>
#include <complex>

#include <sampleflow/element_access.h>


namespace SampleFlow
{
  /**
   * A namespace for types define for use in the SampleFlow library.
   */
  namespace types
  {
    /**
     * The data type used throughout the SampleFlow library to indicate
     * the index of a sample.
     *
     * This type aliases `std::size_t`, which on most systems is an unsigned
     * 64-bit integer type. This should be enough to represent any number of
     * samples you're likely going to generate using this library.
     *
     * @note We use this data type throughout SampleFlow not because it
     *   represents anything other than just `std::size_t`, but because by
     *   using a separate name, we clearly indicate that a variable is used
     *   to index individual samples, rather just be a generic index for
     *   *something*. In other words, the use of this type does not provide
     *   different semantics to a variable, but it makes code *easier to read*.
     */
    using sample_index = std::size_t;


    /**
     * Declaration of a type that represents the scalar type underlying
     * a given `SampleType`. The way this is determined is that if it
     * is an array type of some kind, i.e., if one can form the
     * expression `sample[0]` where `sample` is an (rvalue) object of
     * type `SampleType`, then the type of `sample[0]` is used as the
     * one to be used for `ScalarType`.
     *
     * On the other hand, for types for which `sample[0]` does not
     * make sense, the type `SampleType` itself is used under the assumption
     * that `SampleType` is then considered the scalar itself.
     */
    template <typename SampleType>
    using ScalarType = decltype(Utilities::get_nth_element(std::declval<SampleType>(), 0));
  }


  namespace Utilities
  {
    /**
     * Form the complex-conjugate of the argument. This function
     * template is chosen when the argument is not, in fact, complex-valued
     * and consequently simply returns the argument itself.
     */
    template <typename T>
    T conj (const T &value)
    {
      return value;
    }



    /**
     * Form the complex-conjugate of the argument. This function
     * template is chosen when the argument is complex-valued.
     */
    template <typename T>
    std::complex<T> conj (const std::complex<T> &value)
    {
      return std::conj(value);
    }
  }
}

#endif
