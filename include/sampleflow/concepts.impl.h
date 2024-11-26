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
     * A concept that describes whether a given `SampleType` models an object
     * in a vector space. In particular, one needs to be able to add objects
     * in vector spaces, and one needs to be able to multiply them by a scalar
     * (real) number. This concept is used, for example, in order to guarantee
     * that one can implement computing a mean value of samples.
     */
    template <typename SampleType>
    concept is_vector_space_type = requires (SampleType       &a,
                                             const SampleType &b,
                                             const std::size_t i,
                                             const double      d)
    {
      {
        a += b
      } -> std::convertible_to<SampleType>;
      {
        a -= b
      } -> std::convertible_to<SampleType>;
      {
        d *a
      } -> std::convertible_to<SampleType>;
      {
        a *d
      } -> std::convertible_to<SampleType>;
      a /= i;
    };
  }


  // Forward declarations of Consumer and Producer, as well as Filter:
  template <typename InputType>
  requires (Concepts::is_valid_sampletype<InputType>)
  class Consumer;

  template <typename OutputType>
  requires (Concepts::is_valid_sampletype<OutputType>)
  class Producer;

  template <typename InputType, typename OutputType>
  requires (Concepts::is_valid_sampletype<InputType>  &&Concepts::is_valid_sampletype<OutputType>)
  class Filter;


  namespace Concepts
  {
    /**
     * A concept that describes whether a class `C` is derived
     * from `Consumer<T>` for some `T`.
     */
    template <typename C>
    concept is_consumer = std::derived_from<C, Consumer<typename C::input_type>>;

    /**
     * A concept that describes whether a class `C` is derived
     * from `Producer<T>` for some `T`.
     */
    template <typename C>
    concept is_producer = std::derived_from<C, Producer<typename C::output_type>>;


    /**
     * A concept that describes whether a class `C` is derived
     * from `Filter<T,U>` for some `T` and `U`.
     */
    template <typename C>
    concept is_filter = std::derived_from<C, Filter<typename C::input_type, typename C::output_type>>;

    /**
     * A concept that tests whether a class has a member type `value_type`.
     */
    template <typename SampleType>
    concept has_value_type = requires ()
    {
      typename SampleType::value_type;
    };


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

