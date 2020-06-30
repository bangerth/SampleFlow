// ---------------------------------------------------------------------
//
// Copyright (C) 2020 by the SampleFlow authors.
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

#ifndef SAMPLEFLOW_PARALLEL_MODE_H
#define SAMPLEFLOW_PARALLEL_MODE_H



namespace SampleFlow
{
  /**
   * An enumeration that designates how a Consumer (or Filter) object should
   * process a newly incoming sample. This is set through the
   * Consumer::set_parallel_mode() function.
   */
  enum class ParallelMode : int
  {
    /**
     * Process the sample synchronously, i.e., on the current thread
     * work on whatever the consumer does with the sample and, if this
     * consumer is in fact a filter, send the filtered sample downstream
     * to any connected other consumers. Only after all of these steps
     * have been executed does the program return to the place where the
     * sample was sent from.
     *
     * Note that this does *not* mean that a consumer or filter is only
     * processing one sample at a time. This is because it may be connected
     * to a filter or producer upstream that is itself working asynchronously,
     * or it may be connected to multiple consumers or filters upstream that
     * are working in parallel on separate threads.
     * These may all be sending samples in parallel, and they will have
     * to be processed in parallel. The `synchronous` flag here simply
     * determines what happens to each of these incoming samples: Will
     * it be processed right away on the current thread, or will it be
     * deferred to a later time and on a thread of the operating system's
     * choosing.
     */
    synchronous = 1,

    /**
     * Process the sample asynchronously by creating a new task that the
     * operating system can work on whenever it has available resources.
     * To make this possible, a Consumer or Filter object that uses this
     * mode copies the sample, and then creates a std::task object that
     * encapsulates what needs to be done (namely, processing the sample
     * and, if this consumer is in fact a filter, sending the processed
     * sample downstream to other consumers).
     *
     * Control flow then immediately returns to the place where the current
     * sample was sent from, with one caveat: when specifying
     * `asynchronous`, only a finite number of such tasks can exist at any
     * given time. If the number of tasks that were created for previous
     * samples and that have not been executed yet at the time a subsequent
     * sample is sent exceeds a certain limit, then the
     * current thread of execution will block until some of these still
     * pending tasks have been
     * completed, before creating a new task. This is to make sure that
     * if processing samples takes substantially longer than creating new
     * samples, we don't end up with an indefinite backlog of samples.
     *
     * The limit of currently pending tasks (the "queue size") can be
     * set through an argument to Consumer::set_parallel_mode().
     *
     * @note When a filter or consumer object uses this parallel mode,
     *   then it can be thought of copying every incoming sample into
     *   a separate location and describes to the operating system
     *   that that sample should be processed whenever computational
     *   resources are available. This also means that if the queue
     *   size is larger than one, then these tasks may be executed
     *   in a different order than the samples came in. This is
     *   because the operating system sees no need to execute
     *   the available tasks in any particular order. One could of course
     *   enforce this somehow, but when processing samples in parallel,
     *   it is often also the case that an upstream sample producer
     *   (or multiple producers feeding into a consumer or filter) already
     *   run in parallel and send their sample in a non-deterministic
     *   order -- making the effort to process them in a deterministic order
     *   pointless. In any case, if a user uses this asynchronous mode
     *   on a consumer of filter, then this is really only useful for
     *   consumers or filters for which processing in a random order makes
     *   sense. For example, the Consumers::MeanValue doesn't really care
     *   about the order in which samples are processed since the mean
     *   value computed after all samples have been processed is
     *   independent of their order. On the other hand, the
     *   Consumers::AcceptanceRatio class <i>does</i> care and one should
     *   probably not set this parallel mode for that class. Similarly,
     *   a program using Consumers::StreamOutput likely intends that
     *   samples are written to the stream in the order in which they were
     *   sent (though this may not in fact be the case).
     */
    asynchronous = 2
  };
}

#endif
