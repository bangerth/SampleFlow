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

#ifndef SAMPLEFLOW_CONCEPTS_H
#define SAMPLEFLOW_CONCEPTS_H

#include <concepts>

namespace SampleFlow
{
  /**
   * A namespace in which we define C++20 concepts that are used throughout
   * the library to describe properties of types.
   */
  namespace Concepts
  {
    /**
     * A concept that describes the minimal requirements we have of samples,
     * namely that they can be copied and that they can be moved/move-created.
     * Individual classes may of course have additional requirements -- for example,
     * to compute the mean value of a number of samples, one needs that they
     * can be added and divided by an integer. Individual classes then have to
     * require these additional concepts.
     */
    template <typename SampleType>
    concept is_valid_sampletype = (std::copyable<SampleType> &&
                                   std::movable<SampleType> &&
                                   std::move_constructible<SampleType>);


    /**
     * A concept that describes whether one can call `sample[index]` where
     * `sample` is of type `SampleType` and `index` is an integer.
     */
    template <typename SampleType>
    concept has_subscript_operator = requires (SampleType &sample, const std::size_t index)
    {
      {
        sample[index]
      };
    };


    /**
     * A concept that describes whether one can call `sample.size()` where
     * `sample` is of type `SampleType`.
     */
    template <typename SampleType>
    concept has_size_function = requires (SampleType &sample)
    {
      {
        sample.size()
      };
    };
  }
}

#endif
