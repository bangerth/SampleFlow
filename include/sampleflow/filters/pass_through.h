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

#ifndef SAMPLEFLOW_FILTERS_PASS_THROUGH_H
#define SAMPLEFLOW_FILTERS_PASS_THROUGH_H

#include <sampleflow/filter.h>

namespace SampleFlow
{
  namespace Filters
  {
    /**
     * This class acts as a filter in that it takes samples from
     * upstream producers and simply passes them down to downstream
     * consumers without change.
     *
     * This seems like a not so useful operation, but it has its
     * place. A typical use case is if one has many producers (say,
     * several sampling algorithms) whose samples should all be fed
     * into a number of downstream consumers that assess the solution
     * (say, something like Consumers::MeanValue,
     * Consumers::CountSamples, or Consumers::CovarianceMatrix). With
     * $N$ producers and $M$ consumers, one would have to write out in
     * code all $NM$ connections. Instead, with the current class, one
     * can simply connect all upstream producers to a single object of
     * the current PassThrough type, and then all downstream consumers
     * to this one pass through object -- requiring to spell out only
     * $N+M$ connections.
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
     *   For the current class, the output type of samples is the `value_type`
     *   of the `InputType`, i.e., `typename InputType::value_type`, as this
     *   indicates the type of individual components of the `InputType`.
     */
    template <typename InputType>
    class PassThrough : public SampleFlow::Filter<InputType, InputType>
    {
      public:
        /**
         * Destructor. This function also makes sure that all samples this
         * object may have received have been fully processed. To this end,
         * it calls the Consumers::disconnect_and_flush() function of the
         * base class.
         */
        virtual ~PassThrough ();

        /**
         * Process one sample by simply passing it on
         *
         * @param[in] sample The sample to process.
         * @param[in] aux_data Auxiliary data about this sample. The current
         *   class does not know what to do with any such data and consequently
         *   simply passes it on.
         *
         * @return The input sample and the auxiliary data
         *   originally associated with the sample.
         */
        virtual
        boost::optional<std::pair<InputType, SampleFlow::AuxiliaryData> >
        filter (InputType sample,
                SampleFlow::AuxiliaryData aux_data) override;
    };



    template <typename InputType>
    PassThrough<InputType>::
    ~PassThrough ()
    {
      this->disconnect_and_flush();
    }



    template <typename InputType>
    boost::optional<std::pair<InputType, SampleFlow::AuxiliaryData> >
    PassThrough<InputType>::
    filter (InputType sample,
            SampleFlow::AuxiliaryData aux_data)
    {
      return std::pair<InputType, SampleFlow::AuxiliaryData>
      { sample, aux_data };
    }
  }
}

#endif
