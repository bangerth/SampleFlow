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

#ifndef SAMPLEFLOW_PRODUCER_H
#define SAMPLEFLOW_PRODUCER_H

#include <sampleflow/auxiliary_data.h>
#include <boost/signals2.hpp>
#include <functional>


namespace SampleFlow
{
  /**
   * This is the base class for classes that *produce* samples. Principally,
   * it provides a way for Consumer objects to attach themselves to a signal
   * through which they will then be informed whenever a new sample has been
   * produced. Consumer objects can do this by calling the connect_to_signal()
   * function with a function object that will be called whenever a new
   * sample has been produced. As many consumers as desired can be connected
   * to this signal.
   *
   * Derived classes are free to implement whatever algorithm they want in
   * generating these samples. (The Filter class is one example of an
   * implementation: It is both a Producer and a Consumer, and every time it
   * "consumes" a sample, i.e., it called from another producer with a
   * new sample, it decides whether it wants to convert this input into
   * an output sample of its own.) In general, implementations of derived
   * classes signal the availability of a new sample by triggering the
   * `issue_sample` member variable of this class, which then passes on the
   * sample (and any auxiliary data that may be available along with the
   * sample) to all consumers that have connected to the sample.
   *
   * @tparam OutputType The C++ type used to describe samples. For example,
   *   if one samples from a continuous, one-dimensional distribution, then
   *   an appropriate type may be `double`. If one samples from the two
   *   sides of a coin, then `bool` may be the appropriate choice.
   */
  template <typename OutputType>
  class Producer
  {
    public:
      /**
       * Connect the function passed as argument to the signal that is
       * triggered whenever a new sample is produced. All function
       * objects attached by calling the current function are then called
       * every time a new sample becomes available.
       *
       * @param[in] signal_slot The function to be called whenever a new sample is produced.
       *   This may simply be a pointer to a function that takes two arguments
       *   (one of the type of the sample, and one of type AuxiliaryData), but
       *   more frequently it will be a lambda function that takes two
       *   arguments, or something that has been created using the std::bind
       *   functionalities.
       *
       * @param[in] flush_slot The function to be called whenever a this
       *   producer decides that it is, at least for the moment, done with
       *   producing samples. Downstream listeners to this signal are
       *   supposed to finish all processing of samples that may have
       *   been deferred to separate threads or tasks. If a downstream
       *   listener is a Filter, rather than just a Consumer, then the
       *   filter is supposed to recursively send the same signal to the
       *   filters and consumers connected to its own slots. (By default,
       *   what the Consumer class does when this signal is triggered, is
       *   call the Consumer::flush() function. The Filter class overloads
       *   this function in Filter::flush().)
       *
       * @return The returned object describes the connection made with
       *   the signal to which the caller wants to attach `f`. Callers
       *   may want to store this connection object and, if the calling
       *   object is destroyed, terminate the connection using
       *   `connection.disconnect()`. This ensures that whenever the signal
       *   is triggered, the function previously attached is no longer
       *   called.
       */
      std::pair<boost::signals2::connection,boost::signals2::connection>
      connect_to_signals (const std::function<void (OutputType, AuxiliaryData)> &signal_slot,
                          const std::function<void ()> &flush_slot);

    protected:
      /**
       * The signal that is used to notify downstream objects of the
       * availability of a new sample. Implementations of derived
       * classes should call this signal whenever a new sample has
       * been produced.
       */
      boost::signals2::signal<void (OutputType, AuxiliaryData)> issue_sample;

      /**
       * The signal that is used to notify downstream objects of the
       * end of the stream of samples. This signal is intended to signal
       * to consumers to wait for all samples whose processing has been
       * queued (see the ParallelMode options) to finish processing.
       *
       * In other words, triggering this signal by calling
       * `flush_consumers()` is supposed to only return once all consumers
       * attached to this producer are done with all sample processing.
       * If a Consumer object is in fact a Filter exit, it recursively
       * calls the `flush_consumer` signal on those consumers connected
       * to it.
       *
       * Every Producer function should call this object at the end of the
       * function call that produces a set of samples to ensure that
       * that function only returns when the samples have not only been
       * produced, but have in fact also been completely consumed. This
       * can be done by explicitly calling this signal or, better, by
       * having code such as the following that ensures that the flush
       * operation also happens if the function exits via an exception
       * or, if later modifications are made that introduce a `return`
       * statement in the middle of the function:
       * @code
       * void
       * Range<OutputType>::
       * sample (const RangeType &range)
       * {
       *   // Make sure the flush_consumers() function is called at any point
       *   // where we exit the current function.
       *   Utilities::ScopeExit scope_exit ([this]() {this->flush_consumers();});
       *
       *   // Loop over all elements of the given range and issue a sample for
       *   // each of them.
       *   for (auto sample : range)
       *     this->issue_sample (sample, {});
       * }
       * @endcode
       * This code, taken from the Producers::Range class, sets up the
       * `scope_exit` object that, in essence, stores a function to be executed
       * whenever the surrounding `sample()` function exits. This may happen
       * either by just falling off the end of the function itself, through
       * an explicit `return` statement, an explicit `throw` statement, or if
       * any of the functions being called here throw an exception themselves
       * (which, because it isn't caught here, automatically leads to the
       * current function exiting as well).
       */
      boost::signals2::signal<void ()> flush_consumers;
  };



  template <typename OutputType>
  std::pair<boost::signals2::connection,boost::signals2::connection>
  Producer<OutputType>::
  connect_to_signals (const std::function<void (OutputType, AuxiliaryData)> &new_sample_slot,
                      const std::function<void ()> &flush_slot)
  {
    // Connect with the signal and return the connection object.
    return { issue_sample.connect (new_sample_slot),
             flush_consumers.connect (flush_slot)
           };
  }


  /**
   * A namespace for the implementation of producers, i.e., classes
   * derived from the Producer class.
   *
   * Strictly speaking, this should also include filters (i.e., classes
   * derived from the Filter class), but since these are in addition
   * derived from the Consumer class, they are in their own namespace
   * Filters.
   */
  namespace Producers
  {}
}


#endif
