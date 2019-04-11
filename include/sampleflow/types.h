// ---------------------------------------------------------------------
//
// Copyright (C) 2019 by the SampleFlow authors.
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

#ifndef SAMPLEFLOW_TYPES_H
#define SAMPLEFLOW_TYPES_H

#include <cstddef>

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
  }
}

#endif
