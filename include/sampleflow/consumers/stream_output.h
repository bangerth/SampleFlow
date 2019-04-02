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

#ifndef SAMPLEFLOW_CONSUMERS_STREAM_OUTPUT_H
#define SAMPLEFLOW_CONSUMERS_STREAM_OUTPUT_H

#include <sampleflow/consumer.h>
#include <mutex>
#include <ostream>


namespace SampleFlow
{
  namespace Consumers
  {
    template <typename InputType>
    class StreamOutput : public Consumer<InputType>
    {
      public:
        StreamOutput (std::ostream &output_stream);

        virtual
        void
        consume (InputType sample, AuxiliaryData /*aux_data*/) override;

      private:
        mutable std::mutex mutex;
        std::ostream &output_stream;
    };


    template <typename InputType>
    StreamOutput<InputType>::
    StreamOutput (std::ostream &output_stream)
      :
      output_stream (output_stream)
    {}



    template <typename InputType>
    void
    MeanValue<InputType>::
    consume (InputType sample, AuxiliaryData /*aux_data*/)
    {
      std::lock_guard<std::mutex> lock(mutex);

      output_stream << sample << std::endl;
    }
  }
}

#endif
