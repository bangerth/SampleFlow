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

#ifndef SAMPLEFLOW_CONSUMERS_STREAM_OUTPUT_H
#define SAMPLEFLOW_CONSUMERS_STREAM_OUTPUT_H

#include <sampleflow/consumer.h>
#include <sampleflow/element_access.h>
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
     *   where `sample` is of type `InputType`. Alternatively, if one can
     *   call `sample.size()` and `sample[i]` with an integer index `i`,
     *   then this class only requires that one can write
     *   @code
     *      stream << sample[i];
     *   @endcode
     *   and writes all elements of the sample separated by spaces.
     *   In particular, this allows using this class for `InputType`
     *   equal to `std::vector<T>` and `std::valarray<T>`.
     */
    template <typename InputType>
    class StreamOutput : public Consumer<InputType>
    {
      public:
        /**
         * Constructor.
         *
         * This class does not support asynchronous processing of samples,
         * and consequently calls the base class constructor with
         * ParallelMode::synchronous as argument.
         *
         * @param[in] output_stream A reference to the stream to which output
         *   will be written for each sample. This class stores a reference
         *   to this stream object, so it needs to live at least as long
         *   as the current object.
         */
        StreamOutput (std::ostream &output_stream);

        /**
         * Destructor. This function also makes sure that all samples this
         * object may have received have been fully processed. To this end,
         * it calls the Consumers::disconnect_and_flush() function of the
         * base class.
         */
        virtual ~StreamOutput ();

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
        consume (InputType sample, AuxiliaryData aux_data) override;

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
      Consumer<InputType>(ParallelMode::synchronous),
      output_stream (output_stream)
    {}



    template <typename InputType>
    StreamOutput<InputType>::
    ~StreamOutput ()
    {
      this->disconnect_and_flush();
    }


    namespace internal
    {
      namespace StreamOutput
      {
        /**
         * Write a sample to the stream. This template is used for all
         * `SampleType` types that don't have an `operator[]`, i.e.,
         * for things where we can assume that they are scalar or,
         * if compound, have an appropriate `operator<<` for output.
         *
         * For class types that have an `operator[]`, we use the
         * specialization below to output each component separately.
         */
        template <typename SampleType>
        auto write (const SampleType &sample,
                    std::ostream &output_stream)
        -> typename std::enable_if<Utilities::internal::has_size_function<SampleType>::value == false
        ||
        Utilities::internal::has_subscript_operator<SampleType>::value == false,
                  void>::type
        {
          output_stream << sample;
        }


        /**
         * Write a sample to the stream. This template is used if the sample
         * type has an operator `operator[]`. In that case, just output
         * each component separated by spaces.
         */
        template <typename SampleType>
        auto write (const SampleType &sample,
                    std::ostream &output_stream)
        -> typename std::enable_if<Utilities::internal::has_size_function<SampleType>::value == true
        &&
        Utilities::internal::has_subscript_operator<SampleType>::value == true,
                  void>::type
        {
          for (unsigned int i=0; i<Utilities::size(sample); ++i)
            {
              write (Utilities::get_nth_element(sample, i), output_stream);
              output_stream << ' ';
            }
        }
      }
    }


    template <typename InputType>
    void
    StreamOutput<InputType>::
    consume (InputType sample, AuxiliaryData /*aux_data*/)
    {
      std::lock_guard<std::mutex> lock(mutex);

      internal::StreamOutput::write (sample, output_stream);
      output_stream << '\n';
    }
  }
}

#endif
