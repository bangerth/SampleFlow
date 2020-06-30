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

#ifndef SAMPLEFLOW_CONSUMERS_ACTION_H
#define SAMPLEFLOW_CONSUMERS_ACTION_H

#include <sampleflow/consumer.h>
#include <mutex>


namespace SampleFlow
{
  namespace Consumers
  {
    /**
     * A Consumer class that executes an action every time a sample comes
     * in, passing the sample to the action. The action is simply a function
     * object that takes the sample and the corresponding auxiliary data
     * as its only arguments; whatever the action object returns is discarded.
     *
     * A typical way to use this class is as follows:
     * @code
     *   using SampleType = ...;
     *   Producers::SomeProducerClass<SampleType> producer(...);
     *
     *   const auto action_function =
     *     [](SampleType sample,
     *        AuxiliaryData aux_data) {  std::cout << sample << std::endl;  };
     *   Consumers::Action<SampleType> action (action_function);
     *   action.connect_to_producer (producer);
     * @endcode
     * In this scenario, every sample that the `producer` generates is sent to
     * the Action object that then calls the lambda function `action_function`
     * on it, which in turn outputs the sample to the screen. (The
     * `action_function` in this example simply ignores the `aux_data` argument
     * it receives.
     *
     * A slightly more interesting example would be the following:
     * @code
     *   using SampleType = ...;
     *   Producers::SomeProducerClass<SampleType> producer(...);
     *
     *   std::ofstream output_file ("samples.txt");
     *   const auto action_function =
     *     [&output_file](SampleType sample,
     *                    AuxiliaryData aux_data)
     *     { output_file << sample << std::endl; };
     *   Consumers::Action<SampleType> action (action_function);
     *   action.connect_to_producer (producer);
     * @endcode
     * Here, the sample is output to a file stream that writes into a file
     * named `samples.txt`. The lambda function "captures" the `output_file`
     * variable declared in the previous line.
     *
     *
     * ### Differences to other consumers ###
     *
     * In some sense, all classes derived from Consumer perform some kind of
     * action when they receive a sample. The difference is that almost all
     * of these classes update some state variables. For example, the
     * Consumers::MeanValue class takes a sample and uses it to update
     * its current estimate of the mean of all samples. On the other hand,
     * the action object passed to the current class can not update any
     * internal state because it is, in essence, a global function -- it
     * has no state other than some global variable. In the first example
     * above, the only state it updates is the global `std::cout` variable,
     * in the second one the `output_file` variable declared in the scope
     * in which the Action object is declared.
     *
     * As a consequence of these considerations, the Action class is
     * typically used as a "trigger": Do something concrete every time
     * a sample comes in, but do this in a stateless way. (If one considers
     * writing to a file or to screen as "stateless" -- it is, from a
     * pragmatic perspective, not an "update" operation on a variable whose
     * state is of relevance at a later time in the program.)
     *
     * This class is often used together with the Filters::TakeEveryNth class
     * to execute some action every $n$th sample.
     *
     *
     * ### Threading model ###
     *
     * The implementation of this class is thread-safe, i.e., its
     * consume() member function can be called concurrently and from multiple
     * threads. However, it ensures that the action is executed only once
     * at a time.
     *
     *
     * @tparam InputType The C++ type used for the samples $x_k$.
     */
    template <typename InputType>
    class Action : public Consumer<InputType>
    {
      public:
        /**
         * Constructor. Take the action (a function object) as argument.
         *
         * The second argument (defaulted to ParallelMode::synchronous) indicates
         * whether incoming samples should be processed immediately, on the current
         * thread, or can be deferred to (possibly out of order) processing on
         * a separate thread. See ParallelMode for more information. Whether this
         * is possible or not depends on what the `action` function does.
         */
        Action (const std::function<void (InputType, AuxiliaryData)> &action,
                const ParallelMode supported_parallel_modes = ParallelMode::synchronous);

        /**
         * Destructor. This function also makes sure that all samples this
         * object may have received have been fully processed. To this end,
         * it calls the Consumers::disconnect_and_flush() function of the
         * base class.
         */
        virtual ~Action ();

        /**
         * Process one sample by forgetting about the previously last sample
         * and instead storing this one for later access using the get()
         * function.
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

        const std::function<void (InputType, AuxiliaryData)> action_function;
    };


    template <typename InputType>
    Action<InputType>::
    Action (const std::function<void (InputType, AuxiliaryData)> &action,
            const ParallelMode supported_parallel_modes)
      :
      Consumer<InputType>(supported_parallel_modes),
      action_function (action)
    {}



    template <typename InputType>
    Action<InputType>::
    ~Action ()
    {
      this->disconnect_and_flush();
    }


    template <typename InputType>
    void
    Action<InputType>::
    consume (InputType sample, AuxiliaryData aux_data)
    {
      std::lock_guard<std::mutex> lock(mutex);

      action_function (std::move (sample), std::move(aux_data));
    }
  }
}

#endif
