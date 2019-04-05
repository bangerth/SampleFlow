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
    /**
     * A Consumer class that outputs each sample it receives to a
     * `std::ostream` object. This can be used to write all samples into
     * a file, for example.
     *
     *
     * ### Threading model ###
     *
     * The implementation of this class is thread-safe, i.e., its
     * consume() member function can be called concurrently and from multiple
     * threads.
     *
     *
     * @tparam InputType The C++ type used for the samples $x_k$. In
     *   order to output it through an object of type `std::ostream`,
     *   it must be possible to write
     *   @code
     *      stream << sample;
     *   @endcode
     *   where `sample` is of type `InputType`.
     */
    template <typename InputType>
    class StreamOutput : public Consumer<InputType>
    {
      public:
        /**
         * Constructor.
         *
         * @param[in] output_stream A reference to the stream to which output
         *   will be written for each sample. This class stores a reference
         *   to this stream object, so it needs to live at least as long
         *   as the current object.
         */
        StreamOutput (std::ostream &output_stream);

        /**
         * Process one sample by outputting it to the stream set in the
         * constructor.
         *
         * @param[in] sample The sample to process.
         * @param[in] aux_data Auxiliary data about this sample. The current
         *   class does not know what to do with any such data and consequently
         *   simply ignores it.
         */
        virtual
        void
        consume (InputType sample, AuxiliaryData /*aux_data*/) override;

      private:
        /**
         * A mutex used to lock access to all member variables when running
         * on multiple threads.
         */
        mutable std::mutex mutex;

        /**
         * A reference to the stream to which output will be written for each
         * sample.
         */
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
