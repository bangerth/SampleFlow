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

#ifndef SAMPLEFLOW_FILTERS_DISCARD_FIRST_N_H
#define SAMPLEFLOW_FILTERS_DISCARD_FIRST_N_H

#include <sampleflow/filter.h>
#include <sampleflow/types.h>

#include <mutex>

namespace SampleFlow
{
  namespace Filters
  {
    /**
     * An implementation of the Filter interface which discards the first
     * $n$ samples that this filter sees. This is often useful in Markov Chain
     * algorithms that have a certain "burn in" period during which it is known
     * that the samples are not a reliable reflection of the underlying
     * probability distribution
     *
     *
     * ### Threading model ###
     *
     * The implementation of this class is thread-safe, i.e., its
     * filter() member function can be called concurrently and from multiple
     * threads.
     *
     *
     * @tparam InputType The C++ type used to describe the incoming samples.
     *   For the current class, this is of course also the type used for
     *   the outgoing samples. Consequently, this class is a model of the
     *   Filter base class where both input and output type use this
     *   template type as template arguments.
     */
    template <typename InputType>
    class DiscardFirstN : public Filter<InputType, InputType>
    {
      public:
        /**
         * Constructor.
         *
         * @param[in] initial_n_samples The distance between samples that are to be
         *  forwarded to downstream consumers of this filter.
         */
        DiscardFirstN (const types::sample_index initial_n_samples);

        /**
         * Destructor. This function also makes sure that all samples this
         * object may have received have been fully processed. To this end,
         * it calls the Consumers::disconnect_and_flush() function of the
         * base class.
         */
        virtual ~DiscardFirstN ();

        /**
         * Process one sample by checking whether it is after the initial $n$
         * samples and if so, pass it on to downstream consumers. If it isn't,
         * return an empty object which the caller of this function in the
         * base class will interpret as the instruction to discard the
         * sample from further processing.
         *
         * @param[in] sample The sample $x_k$ to process.
         * @param[in] aux_data Auxiliary data about this sample. The current
         *   class does not know what to do with any such data and consequently
         *   simply passed it on.
         *
         * @return The sample $x_k$ and its auxiliary data if $k>n$ where
         *   $n$ is the argument passed to the constructor.
         */
        virtual
        boost::optional<std::pair<InputType, AuxiliaryData> >
        filter (InputType sample,
                AuxiliaryData aux_data) override;

      private:
        /**
         * A mutex used to lock access to all member variables when running
         * on multiple threads.
         */
        std::mutex mutex;

        /**
         * A counter counting how many samples we have seen so far.
         */
        types::sample_index counter;

        /**
         * The variable storing how many samples to discard initially.
         */
        const types::sample_index initial_n_samples;
    };



    template <typename InputType>
    DiscardFirstN<InputType>::
    DiscardFirstN (const types::sample_index initial_n_samples)
      : counter (0),
        initial_n_samples (initial_n_samples)
    {}



    template <typename InputType>
    DiscardFirstN<InputType>::
    ~DiscardFirstN ()
    {
      this->disconnect_and_flush();
    }


    template <typename InputType>
    boost::optional<std::pair<InputType, AuxiliaryData> >
    DiscardFirstN<InputType>::
    filter (InputType sample,
            AuxiliaryData aux_data)
    {
      std::lock_guard<std::mutex> lock(mutex);

      ++counter;
      if (counter > initial_n_samples)
        {
          return
          {{ std::move(sample), std::move(aux_data)}};
        }
      else
        return
          {};
    }

  }
}

#endif
