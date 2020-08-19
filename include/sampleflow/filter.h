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

#ifndef SAMPLEFLOW_FILTER_H
#define SAMPLEFLOW_FILTER_H

#include <sampleflow/producer.h>
#include <sampleflow/consumer.h>

#include <boost/optional.hpp>
#include <mutex>

namespace SampleFlow
{
  /**
   * This is the base class for all filter classes. Filters implement both
   * the functionality of Consumer and Producer objects as they take samples
   * from producing objects upstream (i.e., the filter is a Consumer), process
   * them in some way, and then send them downstream to other consumers (i.e.,
   * the filter is also a Producer). The idea of filters is that they
   * *transform* samples in some way, or *select* samples.
   *
   * Examples for transformations are filters that *subset* information, such
   * as the Filters::ComponentSplitter class that takes a vector-valued
   * sample and picks an individual (scalar) component of it. One might,
   * for example, want to use such as filter to generate a histogram for the
   * selected component (using the Consumers::Histogram) class, or to compute
   * the standard deviation of just this one component.
   *
   * Examples for selections are filters that only forward samples that
   * satisfy certain criteria. One case of this is the Filters::TakeEveryNth
   * class that picks every $n$th samples and discards all others; only the
   * picked samples are then sent downstream to further consumers.
   *
   *
   * ### Threading model ###
   *
   * Like Consumer classes, Filter classes can be attached to multiple
   * producers. As a consequence, implementations of classes derived from the
   * Filter base class need to expect that their member functions can be called
   * from different threads, and, more importantly, concurrently. Thus, it
   * is important that all member functions of derived classes use
   * appropriate strategies for dealing with concurrency. Principally,
   * this implies that all functions that access the current state of
   * their object need to use `std::mutex` and `std::lock_guard` objects
   * appropriately.
   *
   *
   * @tparam InputType The C++ type used to describe the incoming samples.
   *   For example, if one samples from a distribution over a continuous,
   *   vector-valued vector space, then an appropriate type may be
   *   `std::vector<double>`.
   * @tparam OutputType The C++ type used to describe the outgoing samples,
   *   i.e., the type of the objects generated after processing or selection.
   */
  template <typename InputType, typename OutputType>
  class Filter : public Consumer<InputType>, public Producer<OutputType>
  {
    public:
      /**
       * Constructor. The only meaningful action of this constructor is to
       * set the parallel mode of this object to its default,
       * ParallelMode::synchronous, when calling the constructor of its
       * underlying Consumer base class.
       *
       * @param supported_parallel_modes An optional argument indicating
       *   which possible parallel modes (concatenated by `operator|`)
       *   a derived class supports. By default, this is only
       *   `ParallelMode::synchronous`, implying that the derived class
       *   can only process one sample at a time.
       *
       * @note Generally, filters should only run in synchronous mode.
       *   That's because they hand these samples to downstream consumers
       *   that may require synchronous processing of samples themselves.
       *   An asynchronous filter may pass on samples in a different order
       *   than they were received, and consequently downstream consumers
       *   may receive them in a different order than they were generated.
       *   In other words, an upstream filter running in asynchronous
       *   mode may force a downstream consumer to also work in asynchronous
       *   mode even if they declare that they want to must work in
       *   synchronous mode.
       */
      Filter (const ParallelMode supported_parallel_modes = ParallelMode::synchronous);

      /**
       * An implementation of the Consumer::consume() function. In the
       * current context, what this function does is to call the
       * filter() function with the sample and auxiliary data, and then
       * evaluate if that function returned a new sample. If so, then that
       * new sample is sent to all consumers connected to this filter.
       *
       * @param[in] sample A sample $x_k$.
       * @param[in] aux_data Additional information the producer that
       *   generated the sample may have wanted to convey along with
       *   the same value itself.
       */
      virtual
      void
      consume (InputType sample,
               AuxiliaryData aux_data) override final;

      /**
       * Ensure that all samples currently being worked on by this object
       * are finished up. In a parallel context, there may still be new samples
       * that are coming in even while this function is working, and as a
       * consequence, to *really* make sure that no samples are worked on
       * as this function returns, one needs to ensure one of two things:
       * - Shut down all connections to upstream producers and filters.
       *   This is what the disconnect_and_flush() function does.
       * - Ensure that flush() has been called before on all upstream
       *   producers and filters.
       *
       * The operations described above are the same as for any other
       * kind of Consumer object. But, since the current class also represents
       * a Producer, this function -- after finishing the operations above
       * by calling the Consumer::flush() function -- also triggers the
       * `flush` signal so that all downstream Consumers connected to this
       * Filter also flush their queues.
       */
      virtual
      void
      flush () override;

      /**
       * The main function of this class, which needs to be implemented by
       * derived classes. This function takes a sample of type `InputType`
       * along with auxiliary data, processes it, and may return the
       * processed sample for further distribution to downstream
       * consumers or other filters.
       *
       * Implementations of this function in derived classes do not
       * *have* to produce a new sample. If the returned object is an
       * empty `boost::optional` object, then this is meant to indicate that
       * the sample may have been processed but was simply swallowed up without
       * producing another sample for further distribution.
       *
       * @param[in] sample A sample $x_k$.
       * @param[in] aux_data Additional information the producer that
       *   generated the sample may have wanted to convey along with
       *   the same value itself.
       *
       * @return If the returned `boost::optional` object is empty, then
       * nothing further happens. If it is non-empty, then the pair that the
       * `boost::optional` stores is interpreted as a new sample (of type
       * OutputType) and auxiliary data that is then sent to all consumers
       * connected to this filter.
       */
      virtual
      boost::optional<std::pair<OutputType, AuxiliaryData> >
      filter (InputType sample,
              AuxiliaryData aux_data) = 0;
  };



  template <typename InputType, typename OutputType>
  Filter<InputType,OutputType>::
  Filter (const ParallelMode supported_parallel_modes)
    :
    Consumer<InputType>(supported_parallel_modes)
  {}



  template <typename InputType, typename OutputType>
  void
  Filter<InputType,OutputType>::
  consume (InputType sample,
           AuxiliaryData aux_data)
  {
    // Call the virtual function that needs to be implemented by derived
    // classes and store the result in a local variable.
    boost::optional<std::pair<OutputType, AuxiliaryData> >
    maybe_sample =
      filter (std::move (sample), std::move (aux_data));

    // Then see whether the derived class actually produced anything,
    // and if so, send it downstream.
    if (maybe_sample)
      this->issue_sample (std::move (maybe_sample->first),
                          std::move (maybe_sample->second));
  }



  template <typename InputType, typename OutputType>
  void
  Filter<InputType,OutputType>::
  flush()
  {
    // First flush all of the samples that are currently still queued
    // up by the current Filter object
    Consumer<InputType>::flush();

    // Then also trigger a flush operation on all downstream Consumer
    // objects connected to this Filter
    this->flush_consumers();
  }

}

#endif
