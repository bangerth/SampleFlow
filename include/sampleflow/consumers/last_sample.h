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

#ifndef SAMPLEFLOW_CONSUMERS_LAST_SAMPLE_H
#define SAMPLEFLOW_CONSUMERS_LAST_SAMPLE_H

#include <sampleflow/consumer.h>
#include <mutex>


namespace SampleFlow
{
  namespace Consumers
  {
    template <typename InputType>
    class LastSample : public Consumer<InputType>
    {
      public:
        using value_type = InputType;

        LastSample () = default;

        virtual
        void
        consume (InputType sample, AuxiliaryData /*aux_data*/) override;

        value_type
        get () const;

      private:
        mutable std::mutex mutex;
        InputType last_sample;
    };



    template <typename InputType>
    void
    MeanValue<InputType>::
    consume (InputType sample, AuxiliaryData /*aux_data*/)
    {
      std::lock_guard<std::mutex> lock(mutex);

      last_sample = std::move (sample);
    }



    template <typename InputType>
    typename MeanValue<InputType>::value_type
    MeanValue<InputType>::
    get () const
    {
      std::lock_guard<std::mutex> lock(mutex);

      return last_sample;
    }

  }
}

#endif
