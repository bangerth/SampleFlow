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

namespace SampleFlow
{
  namespace Producers
  {
    /**
     * A class that produces samples from an iterator range, such as the
     * range that corresponds to a
     * std::vector or std::list, or simply a C-style array. This class is
     * often useful to test filters and consumers because the exact sequence
     * of samples is known.
     *
     * Here is an example of how to use this class:
     * @code
     *   SampleFlow::Producers::Range<double> range_producer;
     *
     *   ...connect consumers and filters to this producer...
     *
     *   const std::vector<double> samples = {1, 2, 3, 4, 5, 6};
     *   range_producer.sample (samples);
     * @endcode
     * This code snippet will produce six samples (as listed in the initializer
     * of the `samples` variable. The code can be simplified to read as
     * follows:
     * @code
     *   SampleFlow::Producers::Range<double> range_producer;
     *
     *   ...connect consumers and filters to this producer...
     *
     *   const auto samples = {1, 2, 3, 4, 5, 6};
     *   range_producer.sample (samples);
     * @endcode
     * In this case, `samples` has type `std::initializer_list<int>`, which can
     * also serve the source of samples of type `double`, letting the compiler
     * do the conversion.
     *
     * This class is also useful if the input range does not actually store the
     * elements. An example is
     * [std::ranges::iota_view](https://en.cppreference.com/w/cpp/ranges/iota_view)
     * which is the range of values provided by a begin and end element. For
     * example, the previous example could also have been written as
     * @code
     *   SampleFlow::Producers::Range<double> range_producer;
     *
     *   ...connect consumers and filters to this producer...
     *
     *   range_producer.sample (std::ranges::iota_view(1,7));
     * @endcode
     * or equivalently
     * @code
     *   SampleFlow::Producers::Range<double> range_producer;
     *
     *   ...connect consumers and filters to this producer...
     *
     *   range_producer.sample (std::views::iota(1,7));
     * @endcode
     *
     * @tparam OutputType The type the samples sent downstream should have.
     *   This need not necessarily be the same type as the one of the objects
     *   provided to the sample() member function, but these objects must be
     *   convertible to the `OutputType` of this class.
     */
    template <typename OutputType>
    class Range : public Producer<OutputType>
    {
      public:
        /**
         * The principal function of this class. It produces samples
         * (that are then sent to consumers and filters connected to this
         * class) by using the given range as the source in a range-based
         * `for` loop of the form
         * @code
         *   for (auto sample : range)
         *     ...send the sample to filters and consumers...
         * @endcode
         * In other words, the *type* of the given range needs to satisfy
         * the requirement that it can be used in the right hand side of
         * a range-based for loop. Furthermore, the values produced in
         * the loop (i.e., the `sample` objects above) need to be convertible
         * to `OutputType`.
         */
        template <typename RangeType>
        requires (std::ranges::input_range<RangeType> &&
                  std::convertible_to<std::ranges::range_value_t<RangeType>,OutputType>)
        void
        sample (const RangeType &range);
    };


    template <typename OutputType>
    template <typename RangeType>
    requires (std::ranges::input_range<RangeType> &&
              std::convertible_to<std::ranges::range_value_t<RangeType>,OutputType>)
    void
    Range<OutputType>::
    sample (const RangeType &range)
    {
      // Make sure the flush_consumers() function is called at any point
      // where we exit the current function.
      Utilities::ScopeExit scope_exit ([this]()
      {
        this->flush_consumers();
      });

      // Loop over all elements of the given range and issue a sample for
      // each of them.
      for (auto sample : range)
        this->issue_sample (sample, {});
    }

  }
}
