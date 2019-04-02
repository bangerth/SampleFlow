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

#ifndef SAMPLEFLOW_CONSUMERS_MEAN_VALUE_H
#define SAMPLEFLOW_CONSUMERS_MEAN_VALUE_H

#include <sampleflow/consumer.h>
#include <mutex>


namespace SampleFlow
{
  namespace Consumers
  {
    template <typename InputType>
    class MeanValue : public Consumer<InputType>
    {
      public:
        using value_type = InputType;

        MeanValue ();

        virtual
        void
        consume (InputType sample, AuxiliaryData /*aux_data*/) override;

        value_type
        get () const;

      private:
        mutable std::mutex mutex;
        InputType sum;
        std::size_t n_samples;
    };


    template <typename InputType>
    MeanValue<InputType>::
    MeanValue ()
      :
      n_samples (0)
    {}



    template <typename InputType>
    void
    MeanValue<InputType>::
    consume (InputType sample, AuxiliaryData /*aux_data*/)
    {
      std::lock_guard<std::mutex> lock(mutex);

      if (n_samples == 0)
        {
          n_samples = 1;
          sum = std::move(sample);
        }
      else
        {
          ++n_samples;
          sum += sample;
        }
    }



    template <typename InputType>
    typename MeanValue<InputType>::value_type
    MeanValue<InputType>::
    get () const
    {
      std::lock_guard<std::mutex> lock(mutex);

      value_type mean = sum;
      mean /= n_samples;
      return std::move (mean);
    }

  }
}

#endif
