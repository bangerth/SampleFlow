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
   * implementation: It is both a Producer and a Consumer, and everytime it
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
       * @param[in] f The function to be called whenever a new sample is produced.
       *   This may simply be a pointer to a function that takes two arguments
       *   (one of the type of the sample, and one of type AuxiliaryData), but
       *   more frequently it will be a lambda function that takes two
       *   arguments, or something that has been created using the std::bind
       *   functionalities.
       *
       * @return The returned object describes the connection made with
       *   the signal to which the caller wants to attach `f`. Callers
       *   may want to store this connection object and, if the calling
       *   object is destroyed, terminate the connection using
       *   `connection.disconnect()`. This ensures that whenever the signal
       *   is triggered, the function previously attached is no longer
       *   called.
       */
      boost::signals2::connection
      connect_to_signal (const std::function<void (OutputType, AuxiliaryData)> &f);

    protected:
      /**
       * The signal that is used to notify downstream objects of the
       * availability of a new sample. Implementations of derived
       * classes should call this signal whenever a new sample has
       * been produced.
       */
      boost::signals2::signal<void (OutputType, AuxiliaryData)> issue_sample;
  };



  template <typename OutputType>
  boost::signals2::connection
  Producer<OutputType>::
  connect_to_signal (const std::function<void (OutputType, AuxiliaryData)> &f)
  {
    // Connect with the signal and return the connection object.
    return issue_sample.connect (f);
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
