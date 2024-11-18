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

#ifndef SAMPLEFLOW_CONNECTIONS_H
#define SAMPLEFLOW_CONNECTIONS_H

#include <sampleflow/config.h>

#include <sampleflow/concepts.h>
#include <sampleflow/consumer.h>
#include <sampleflow/producer.h>

#include <memory>
#include <type_traits>
#include <variant>


namespace SampleFlow
{
  /**
   * A class that describes a compound of two SampleFlow producers, filters,
   * or consumers. It is these kinds of objects that result from calling
   * constructs such as
   * @code
   *   producer >> consumer;
   * @endcode
   * or
   * @code
   *   producer >> filter >> consumer;
   * @endcode
   * Depending on whether the objects in question are passed in by-reference
   * (namely, when they are named objects as above) or as rvalue references
   * (namely, when they are unnamed objects created in place), the compound
   * either stores the object itself or a moved copy of it.
   *
   * Upon construction, the class also ensures that the two objects are
   * connected so that samples flow from the left to the right.
   *
   * This general template is used for the connection of two consecutive
   * filters where the left filter takes in samples of type `InputType`
   * and outputs `IntermediateType` samples, and the right filter takes
   * in `IntermediateType` and outputs `OutputType`.
   */
  template <typename InputType, typename IntermediateType, typename OutputType>
  class Chain : public Filter<InputType,OutputType>
  {
    public:
      /**
       * Constructor. Build an object out of the left and right producers,
       * filters, or consumers provided as argument.
       *
       * Template argument deduction by the compiler will result in `LeftType`
       * and `RightType` either being a class type (namely, if the object in
       * question was passed in via an rvalue reference), in which case we move
       * the object into our own storage space; or the type is a reference
       * type, in which case we store a reference to the object.
       */
      template <typename LeftType, typename RightType>
      Chain (LeftType &&left, RightType &&right)
        :
        left_object (nullptr),
        right_object (nullptr)
      {
        if constexpr (std::is_reference_v<LeftType>)
          left_object = left;
        else
          left_object = std::make_unique<LeftType>(std::move(left));

        if constexpr (std::is_reference_v<RightType>)
          right_object = right;
        else
          right_object = std::make_unique<RightType>(std::move(right));

        // Finally connect the right to the left object
        get_right_object().connect_to_producer (get_left_object());
      }

      /**
       * Move constructor.
       */
      Chain (Chain &&c) = default;


      /**
       * Destructor.
       */
      virtual
      ~Chain () override = default;


      /**
       * A function that overrides the one in the base class. In
       * particular, while the current object is derived from the
       * Filter base class, and consequently has output slots, what
       * we really want is to connect to the slots of the underlying
       * *right* subobject.
       */
      virtual
      std::pair<const Producer<OutputType> *,
          std::tuple<boost::signals2::connection,boost::signals2::connection,boost::signals2::connection>>
          connect_to_signals (const std::function<void (OutputType, AuxiliaryData)> &signal_slot,
                              const std::function<void ()> &flush_slot,
                              const std::function<void (const Producer<OutputType> &)> &disconnect_slot) override
      {
        return get_right_object().connect_to_signals(signal_slot, flush_slot, disconnect_slot);
      }

      /**
       * A function that overrides the one in the base class. In
       * particular, while the current object is derived from the
       * Consumer base class (by way of the Filter base class), and consequently
       * has the ability to connect to producers, what we really want is
       * for the *left* subobject to connect to the producer.
       */
      virtual
      void
      connect_to_producer (Producer<InputType> &producer) override
      {
        get_left_object().connect_to_producer(producer);
      }

      /**
       * An override of the main function of the Consumer class that this
       * class is derived from. We should never get here, but instead into
       * the `filter()` functions of the left and right sub-objects.
       */
      virtual
      std::optional<std::pair<OutputType, AuxiliaryData> >
      filter (InputType /*sample*/,
              AuxiliaryData /*aux_data*/) override final
      {
        std::abort();
      }

    private:
      /**
       * References or copies of the objects passed in.
       */
      std::variant<std::reference_wrapper<Filter<InputType,IntermediateType>>,
                                                                           std::unique_ptr<Filter<InputType,IntermediateType>>> left_object;
      std::variant<std::reference_wrapper<Filter<IntermediateType,OutputType>>,
          std::unique_ptr<Filter<IntermediateType,OutputType>>> right_object;

      /**
       * Return a reference to the left object stored by the constructor.
       */
      Filter<InputType,IntermediateType> &
      get_left_object ()
      {
        if (std::holds_alternative<std::reference_wrapper<Filter<InputType,IntermediateType>>>(left_object))
          return std::get<std::reference_wrapper<Filter<InputType,IntermediateType>>>(left_object).get();
        else
          return *std::get<std::unique_ptr<Filter<InputType,IntermediateType>>>(left_object);
      }

      /**
       * Return a reference to the right object stored by the constructor.
       */
      Filter<IntermediateType,OutputType> &
      get_right_object ()
      {
        if (std::holds_alternative<std::reference_wrapper<Filter<IntermediateType,OutputType>>>(right_object))
          return std::get<std::reference_wrapper<Filter<IntermediateType,OutputType>>>(right_object).get();
        else
          return *std::get<std::unique_ptr<Filter<IntermediateType,OutputType>>>(right_object);
      }
  };


  /**
   * A class that describes a compound of two SampleFlow producers, filters,
   * or consumers. It is these kinds of objects that result from calling
   * constructs such as
   * @code
   *   producer >> consumer;
   * @endcode
   * or
   * @code
   *   producer >> filter >> consumer;
   * @endcode
   * Depending on whether the objects in question are passed in by-reference
   * (namely, when they are named objects as above) or as rvalue references
   * (namely, when they are unnamed objects created in place), the compound
   * either stores the object itself or a moved copy of it.
   *
   * Upon construction, the class also ensures that the two objects are
   * connected so that samples flow from the left to the right.
   *
   * This specialization of the general template is used for the connection
   * of a filter with a consumer, where the left object (the filter) takes in
   * samples of type `InputType` and outputs `IntermediateType` samples, and
   * the right object (the consumer) takes in `IntermediateType` and outputs
   * nothing (indicated by the `void` template argument in last position.)
   */
  template <typename InputType, typename IntermediateType>
  class Chain<InputType,IntermediateType,void> : public Consumer<InputType>
  {
    public:
      /**
       * Constructor. Build an object out of the left and right producers,
       * filters, or consumers provided as argument.
       *
       * Template argument deduction by the compiler will result in `LeftType`
       * and `RightType` either being a class type (namely, if the object in
       * question was passed in via an rvalue reference), in which case we move
       * the object into our own storage space; or the type is a reference
       * type, in which case we store a reference to the object.
       */
      template <typename LeftType, typename RightType>
      Chain (LeftType &&left, RightType &&right)
        :
        left_object (nullptr),
        right_object (nullptr)
      {
        if constexpr (std::is_reference_v<LeftType>)
          left_object = left;
        else
          left_object = std::make_unique<LeftType>(std::move(left));

        if constexpr (std::is_reference_v<RightType>)
          right_object = right;
        else
          right_object = std::make_unique<RightType>(std::move(right));

        // Finally connect the right to the left object
        get_right_object().connect_to_producer (get_left_object());
      }

      /**
       * Move constructor.
       */
      Chain (Chain &&c) = default;


      /**
       * Destructor.
       */
      virtual
      ~Chain () override = default;

      /**
       * A function that overrides the one in the base class. In
       * particular, while the current object is derived from the
       * Consumer base class, and consequently has
       * the ability to connect to producers, what we really want is
       * for the *left* subobject to connect to the producer.
       */
      virtual
      void
      connect_to_producer (Producer<InputType> &producer) override
      {
        get_left_object().connect_to_producer(producer);
      }

      /**
       * An override of the main function of the Consumer class that this
       * class is derived from. We should never get here, but instead into
       * the `consume()` function of the right sub-object.
       */
      virtual
      void
      consume (InputType /*sample*/,
               AuxiliaryData /*aux_data*/) override final
      {
        std::abort();
      }

    private:
      /**
       * References or copies of the objects passed in.
       */
      std::variant<std::reference_wrapper<Filter<InputType,IntermediateType>>,
          std::unique_ptr<Filter<InputType,IntermediateType>>> left_object;
      std::variant<std::reference_wrapper<Consumer<IntermediateType>>,
          std::unique_ptr<Consumer<IntermediateType>>> right_object;

      /**
       * Return a reference to the left object stored by the constructor.
       */
      Filter<InputType,IntermediateType> &
      get_left_object ()
      {
        if (std::holds_alternative<std::reference_wrapper<Filter<InputType,IntermediateType>>>(left_object))
          return std::get<std::reference_wrapper<Filter<InputType,IntermediateType>>>(left_object).get();
        else
          return *std::get<std::unique_ptr<Filter<InputType,IntermediateType>>>(left_object);
      }

      /**
       * Return a reference to the right object stored by the constructor.
       */
      Consumer<IntermediateType> &
      get_right_object ()
      {
        if (std::holds_alternative<std::reference_wrapper<Consumer<IntermediateType>>>(right_object))
          return std::get<std::reference_wrapper<Consumer<IntermediateType>>>(right_object).get();
        else
          return *std::get<std::unique_ptr<Consumer<IntermediateType>>>(right_object);
      }
  };


  /**
   * A class that describes a compound of two SampleFlow producers, filters,
   * or consumers. It is these kinds of objects that result from calling
   * constructs such as
   * @code
   *   producer >> consumer;
   * @endcode
   * or
   * @code
   *   producer >> filter >> consumer;
   * @endcode
   * Depending on whether the objects in question are passed in by-reference
   * (namely, when they are named objects as above) or as rvalue references
   * (namely, when they are unnamed objects created in place), the compound
   * either stores the object itself or a moved copy of it.
   *
   * Upon construction, the class also ensures that the two objects are
   * connected so that samples flow from the left to the right.
   *
   * This specialization of the general template is used for the connection
   * of a producer with a filter, where the left object (the producer) takes in
   * nothing (indicated by the `void` template argument on the left) and
   * outputs `IntermediateType` samples, and
   * the right object (the filter) takes in `IntermediateType` and outputs
   * `OutputType` samples.
   */
  template <typename IntermediateType, typename OutputType>
  class Chain<void,IntermediateType,OutputType> : public Producer<OutputType>
  {
    public:
      /**
       * Constructor. Build an object out of the left and right producers,
       * filters, or consumers provided as argument.
       *
       * Template argument deduction by the compiler will result in `LeftType`
       * and `RightType` either being a class type (namely, if the object in
       * question was passed in via an rvalue reference), in which case we move
       * the object into our own storage space; or the type is a reference
       * type, in which case we store a reference to the object.
       */
      template <typename LeftType, typename RightType>
      Chain (LeftType &&left, RightType &&right)
        :
        left_object (nullptr),
        right_object (nullptr)
      {
        if constexpr (std::is_reference_v<LeftType>)
          left_object = left;
        else
          left_object = std::make_unique<LeftType>(std::move(left));

        if constexpr (std::is_reference_v<RightType>)
          right_object = right;
        else
          right_object = std::make_unique<RightType>(std::move(right));

        // Finally connect the right to the left object
        get_right_object().connect_to_producer (get_left_object());
      }

      /**
       * Move constructor.
       */
      Chain (Chain &&c) = default;


      /**
       * Destructor.
       */
      virtual
      ~Chain () override = default;

      /**
       * A function that overrides the one in the base class. In
       * particular, while the current object is derived from the
       * Filter base class, and consequently has output slots, what
       * we really want is to connect to the slots of the underlying
       * objects.
       */
      virtual
      std::pair<const Producer<OutputType> *,
          std::tuple<boost::signals2::connection,boost::signals2::connection,boost::signals2::connection>>
          connect_to_signals (const std::function<void (OutputType, AuxiliaryData)> &signal_slot,
                              const std::function<void ()> &flush_slot,
                              const std::function<void (const Producer<OutputType> &)> &disconnect_slot) override
      {
        return get_right_object().connect_to_signals(signal_slot, flush_slot, disconnect_slot);
      }

    private:
      /**
       * References or copies of the objects passed in.
       */
      std::variant<std::reference_wrapper<Producer<IntermediateType>>,
                                                                   std::unique_ptr<Producer<IntermediateType>>> left_object;
      std::variant<std::reference_wrapper<Filter<IntermediateType,OutputType>>,
          std::unique_ptr<Filter<IntermediateType,OutputType>>> right_object;

      /**
       * Return a reference to the left object stored by the constructor.
       */
      Producer<IntermediateType> &
      get_left_object ()
      {
        if (std::holds_alternative<std::reference_wrapper<Producer<IntermediateType>>>(left_object))
          return std::get<std::reference_wrapper<Producer<IntermediateType>>>(left_object).get();
        else
          return *std::get<std::unique_ptr<Producer<IntermediateType>>>(left_object);
      }

      /**
       * Return a reference to the right object stored by the constructor.
       */
      Filter<IntermediateType,OutputType> &
      get_right_object ()
      {
        if (std::holds_alternative<std::reference_wrapper<Filter<IntermediateType,OutputType>>>(right_object))
          return std::get<std::reference_wrapper<Filter<IntermediateType,OutputType>>>(right_object).get();
        else
          return *std::get<std::unique_ptr<Filter<IntermediateType,OutputType>>>(right_object);
      }
  };


  /**
   * A class that describes a compound of two SampleFlow producers, filters,
   * or consumers. It is these kinds of objects that result from calling
   * constructs such as
   * @code
   *   producer >> consumer;
   * @endcode
   * or
   * @code
   *   producer >> filter >> consumer;
   * @endcode
   * Depending on whether the objects in question are passed in by-reference
   * (namely, when they are named objects as above) or as rvalue references
   * (namely, when they are unnamed objects created in place), the compound
   * either stores the object itself or a moved copy of it.
   *
   * Upon construction, the class also ensures that the two objects are
   * connected so that samples flow from the left to the right.
   *
   * This specialization of the general template is used for the connection
   * of a producer with a consumer, where the left object (the producer) takes in
   * nothing (indicated by the `void` template argument on the left) and
   * outputs `IntermediateType` samples, and
   * the right object (the consumer) takes in `IntermediateType` and outputs
   * nothing (indicated by the `void` right template argument).
   */
  template <typename IntermediateType>
  class Chain<void,IntermediateType,void>
  {
    public:
      /**
       * Constructor. Build an object out of the left and right producers,
       * filters, or consumers provided as argument.
       *
       * Template argument deduction by the compiler will result in `LeftType`
       * and `RightType` either being a class type (namely, if the object in
       * question was passed in via an rvalue reference), in which case we move
       * the object into our own storage space; or the type is a reference
       * type, in which case we store a reference to the object.
       */
      template <typename LeftType, typename RightType>
      Chain (LeftType &&left, RightType &&right)
        :
        left_object (nullptr),
        right_object (nullptr)
      {
        if constexpr (std::is_reference_v<LeftType>)
          left_object = left;
        else
          left_object = std::make_unique<LeftType>(std::move(left));

        if constexpr (std::is_reference_v<RightType>)
          right_object = right;
        else
          right_object = std::make_unique<RightType>(std::move(right));

        // Finally connect the right to the left object
        get_right_object().connect_to_producer (get_left_object());
      }

      /**
       * Move constructor.
       */
      Chain (Chain &&c) = default;


      /**
       * Destructor.
       */
      virtual
      ~Chain () = default;

    private:
      /**
       * References or copies of the objects passed in.
       */
      std::variant<std::reference_wrapper<Producer<IntermediateType>>,
          std::unique_ptr<Producer<IntermediateType>>> left_object;
      std::variant<std::reference_wrapper<Consumer<IntermediateType>>,
          std::unique_ptr<Consumer<IntermediateType>>> right_object;

      /**
       * Return a reference to the left object stored by the constructor.
       */
      Producer<IntermediateType> &
      get_left_object ()
      {
        if (std::holds_alternative<std::reference_wrapper<Producer<IntermediateType>>>(left_object))
          return std::get<std::reference_wrapper<Producer<IntermediateType>>>(left_object).get();
        else
          return *std::get<std::unique_ptr<Producer<IntermediateType>>>(left_object);
      }

      /**
       * Return a reference to the right object stored by the constructor.
       */
      Consumer<IntermediateType> &
      get_right_object ()
      {
        if (std::holds_alternative<std::reference_wrapper<Consumer<IntermediateType>>>(right_object))
          return std::get<std::reference_wrapper<Consumer<IntermediateType>>>(right_object).get();
        else
          return *std::get<std::unique_ptr<Consumer<IntermediateType>>>(right_object);
      }
  };


  /**
   * Connect a consumer to a producer of samples, by writing chains such as
   * @code
   *   producer >> consumer;
   * @endcode
   * or
   * @code
   *   producer >> filter >> consumer;
   * @endcode
   * Both consumer and producer may themselves be "filters", i.e.,
   * derived from the Filter class. (Filters are both consumers and
   * producers, and so qualify for both the left and right hand side
   * of `operator>>`.) The result of calling `operator>>` is a Chain
   * object, and in chains such as the second code example, the second
   * call to `operator>>` will get a Chain object as its left argument.
   *
   * There are four cases this function has to differentiate (where in the
   * following we use `ProducerType = std::remove_reference_t<LeftType>`
   * and `ConsumerType = std::remove_reference_t<RightType>`, i.e., the
   * types of the left and right arguments without possible reference
   * qualification):
   *
   * 1. Creating the connection between a producer and consumer as in
   *    the first code example above, where neither of the two objects are
   *    filters. (This case also applies to the second call to
   *    `operator>>` in the second example above: The left two arguments have
   *    already been combined into a compound producer, and they are not
   *    combined with a consumer that is not a filter.)
   *
   *    In this case, the return value is an object of type
   *    `Chain<void, ProducerType::output_type, void>`.
   *
   * 2. Creating the connection between a producer and filter as in
   *    the left half of the second code example above, where the leftmost
   *    object is not a filter.
   *
   *    In this case, the return value is an object of type
   *    `Chain<void, ProducerType::output_type, ConsumerType::output_type>`.
   *
   * 3. Creating the connection a filter and a consumer. This does not happen
   *    in either of the two examples above, but would happen if you wrote
   *    in the parentheses of the code
   *    @code
   *      producer >> (filter >> consumer);
   *    @endcode
   *    which functionally is identical to writing
   *    @code
   *      producer >> filter >> consumer;
   *    @endcode
   *
   *    In this case, the return value is an object of type
   *    `Chain<ProducerType::input_type, ProducerType::output_type, void>`.
   *
   * 4. Creating the connection between two filters. This would happen in
   *    the parenthesized part of code such as
   *    @code
   *      producer >> (filter1 >> filter2) >> consumer;
   *    @endcode
   *    which functionally is identical to writing
   *    @code
   *      producer >> filter1 >> filter2 >> consumer;
   *    @endcode
   *
   *    In this case, the return value is an object of type
   *    `Chain<ProducerType::input_type, ProducerType::output_type, ConsumerType::output_type>`.
   */
  template <typename LeftType, typename RightType>
  requires (Concepts::is_producer<std::remove_reference_t<LeftType>>  &&
            Concepts::is_consumer<std::remove_reference_t<RightType>> &&
            std::same_as<typename std::remove_reference_t<LeftType>::output_type,
            typename std::remove_reference_t<RightType>::input_type>)
  auto
  operator>> (LeftType &&producer, RightType &&consumer)
  {
    using ProducerType = std::remove_reference_t<LeftType>;
    using ConsumerType = std::remove_reference_t<RightType>;

    if constexpr (!Concepts::is_filter<ProducerType>  &&
                  !Concepts::is_filter<ConsumerType>)
      return Chain<void, typename ProducerType::output_type, void>
             (std::forward<LeftType>(producer), std::forward<RightType>(consumer));
    else if constexpr (!Concepts::is_filter<ProducerType>  &&
                       Concepts::is_filter<ConsumerType>)
      return Chain<void,
             typename ProducerType::output_type,
             typename ConsumerType::output_type>
             (std::forward<LeftType>(producer), std::forward<RightType>(consumer));
    else if constexpr (Concepts::is_filter<ProducerType>  &&
                       !Concepts::is_filter<ConsumerType>)
      return
        Chain<typename ProducerType::input_type,
        typename ProducerType::output_type,
        void>
        (std::forward<LeftType>(producer), std::forward<RightType>(consumer));
    else if constexpr (Concepts::is_filter<ProducerType>  &&
                       Concepts::is_filter<ConsumerType>)
      return
        Chain<typename ProducerType::input_type,
        typename ProducerType::output_type,
        typename ConsumerType::output_type>
        (std::forward<LeftType>(producer), std::forward<RightType>(consumer));
    else
      assert (false);
  }

}

#endif /* SAMPLEFLOW_CONNECTIONS_H */
