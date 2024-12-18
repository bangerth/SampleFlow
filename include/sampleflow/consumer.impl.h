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

namespace SampleFlow
{
  /**
   * This is the base class for classes that *consume* samples, i.e. react
   * in some way to a sample produced by a Producer object. Examples of
   * consumers are classes that compute the average $\left<x\right>$ over
   * the samples $x_k$ they are sent (implemented in Consumers::MeanValue),
   * the standard deviation or covariance matrix (implemented in
   * Consumers::StandardDeviation and Consumers::Covariance), or that
   * simply output each sample to a `std::ostream` (implemented in
   * Consumers::StreamOutput). A special type of consumers are classes
   * derived from the Filter class: These generally don't carry any significant
   * state around (such as the mean value of previously received samples) but
   * instead transform each sample $x_k$ into a different $y_k=f(x_k)$.
   * The Filter class has more examples on this.
   *
   * The current class has a rather minimalist interface: Its
   * connect_to_producer() member function is used to attach a Consumer
   * object to a Producer object; whenever the producer then generates
   * a sample, it indicates the value of the sample to all connected
   * consumers through a signal that in connect_to_producer() is set to
   * call the consumer() member function of this class. The consume()
   * member function is `virtual` and abstract and needs to be implemented
   * in derived classes to do whatever that derive class wants to do with
   * each sample.
   *
   *
   * ### Threading model ###
   *
   * Consumers can be attached to multiple producers (see the example in
   * the documentation of the connect_to_producer() function), and these
   * producers may be running on separate threads. As a consequence,
   * implementations of classes derived from the Consumer base class need
   * to expect that their member functions can be called from different
   * threads, and, more importantly, concurrently. Thus, it
   * is important that all member functions of derived classes use
   * appropriate strategies for dealing with concurrency. Principally,
   * this implies that all functions that access the current state of
   * their object need to use `std::mutex` and `std::lock_guard` objects
   * appropriately.
   *
   *
   * @tparam InputType The C++ type used to describe samples. For example,
   *   if one samples from a continuous, one-dimensional distribution, then
   *   an appropriate type may be `double`. If one samples from the two
   *   sides of a coin, then `bool` may be the appropriate choice.
   */
  template <typename InputType>
  requires (Concepts::is_valid_sampletype<InputType>)
  class Consumer
  {
    public:
      /**
       * A type alias for the input type.
       */
      using input_type = InputType;

      /**
       * Constructor. The only meaningful action of this constructor is to
       * set the parallel mode of this object to its default,
       * ParallelMode::synchronous.
       *
       * @param supported_parallel_modes An optional argument indicating
       *   which possible parallel modes (concatenated by `operator|`)
       *   a derived class supports. By default, this is only
       *   `ParallelMode::synchronous`, implying that the derived class
       *   can only process one sample at a time
       */
      Consumer (const ParallelMode supported_parallel_modes = ParallelMode::synchronous);

      /**
       * Copy constructor. Consumer objects can not be copied, and so
       * this operator is deleted.
       */
      Consumer (const Consumer &consumer) = delete;

      /**
       * Move constructor. This constructor can only be called if no
       * connections from producers to the moved-from consumer already
       * exist. This is because if you move a consumer that does already
       * have connections to it, you will create dangling connections
       * where samples will be sent to an object that no longer exists.
       * This is not likely what you intended.
       */
      Consumer (Consumer &&consumer);

      /*
       * The destructor.
       *
       * Derived classes have to, in their destructors, call the
       * disconnect_and_flush() function to ensure that all samples
       * have been processed before the derived class object goes out
       * of scope. If a derived class does not call disconnect_and_flush()
       * in its destructor, it may happen that a sample is still in
       * the process of being processed, or maybe still only scheduled
       * for processing, while the member variables of the derived class
       * are already destroyed -- nothing good will typically come out
       * of such situations.
       */
      virtual
      ~Consumer ();

      /**
       * A member function typically called from user code to connect
       * this consumer object to a producer object. As a consequence,
       * every time the producer generates a sample, it will call an internal
       * function of the current object which in turn then calls the
       * consumer() member function that needs to be implemented in derived
       * classes.
       *
       * Note that this function can be called for multiple producers. This
       * would connect the same consumer to multiple producers -- an
       * application of this facility would be if a program were to run
       * multiple sampling algorithms in parallel on separate threads. In
       * such cases, it might still be useful to compute the mean value
       * over all samples produced by all of the sampling objects. One
       * would do this by connecting the same mean value consumer object
       * to all samplers by calling this function several times with
       * different arguments.
       *
       * @param[in] producer A reference to the producer object whose
       *   samples we want to consumer in the current object.
       */
      virtual
      void
      connect_to_producer (Producer<InputType> &producer);

      /**
       * The main function of this class. It is the only function a
       * derived class needs to implement. It receives both the sample
       * and some additional information about this sample as argument.
       *
       * @param[in] sample A sample $x_k$.
       * @param[in] aux_data Additional information the producer that
       *   generated the sample may have wanted to convey along with
       *   the same value itself.
       */
      virtual
      void
      consume (InputType sample,
               AuxiliaryData aux_data) = 0;


      /**
       * Set how this consumer or filter should process newly incoming samples.
       * In particular, the arguments to this function determine whether
       * new samples should be processed on the current thread, or
       * whether processing should be deferred to a separate task that
       * may run at a later time or on a different processor core -- at
       * the operating system's preference, whenever resources are
       * available.
       *
       * See the description of the ParallelMode `enum` for more information.
       *
       * @param[in] parallel_mode Determines how new samples should be
       *   processed.
       * @param[in] queue_size The maximum number of samples whose processing
       *   has been deferred at any given time and whose processing has not
       *   finished yet. If, for example, `queue_size` is one and a previous
       *   sample has not completed processing, then a newly incoming sample
       *   will be held up (and the current process will block) until the
       *   previous sample has completed processing.
       *
       * @note This function needs to be be called *before* this consumer or
       *   filter is connected to any upstream producer (or other filter), and
       *   in particular before any samples are actually sent to the current
       *   object.
       */
      void
      set_parallel_mode (const ParallelMode parallel_mode,
                         const unsigned int queue_size = 1);

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
       */
      virtual
      void
      flush ();

      /**
       * Shut down the connections to upstream producers and filters,
       * ensuring that no further samples will be sent to the current object.
       * Then call the flush() function that makes sure that all samples that
       * are currently still being processed are finished up.
       *
       * @note Derived classes can only allow destruction to proceed if they
       *   can make sure that no further samples will be sent to them. This
       *   function ensures exactly this, and as a consequence all Filter
       *   and Consumer implementations must call this function in their
       *   destructor before destroying any other data structures.
       */
      void
      disconnect_and_flush ();

    private:

      /**
       * A list of connections created by calling connect_to_producer().
       * We store this list so that we can terminate the connections once
       * the current object is destroyed, in order to avoid triggering
       * a slot that no longer exists if the originally connected
       * producer decides to generate a sample after the current object
       * has been destroyed.
       *
       * The connections are indexed by the producer object we are
       * connected to. This is a multimap because a consumer may be connected
       * to the same producer more than once.
       */
      std::multimap<const Producer<InputType> *,
          std::tuple<boost::signals2::connection,boost::signals2::connection,boost::signals2::connection>>
          connections_to_producers;

      /**
       * How newly incoming samples should be processed.
       *
       * This variable can be read from/written to in an atomic
       * fashion to ensure that different threads don't tread on
       * each other.
       */
      std::atomic<int> parallel_mode;

      /**
       * A bit field that describes the parallel modes supported by the
       * derived class.
       */
      const ParallelMode supported_parallel_modes;

      /**
       * How many tasks can be in flight at any given time.
       *
       * This variable can be read from/written to in an atomic
       * fashion to ensure that different threads don't tread on
       * each other.
       */
      std::atomic<unsigned int> queue_size;

      /**
       * A mutex that controls access to all of the data structures involved
       * in parallel processing of samples in asynchronous mode. In particular,
       * this includes the task queue, but also shutting down the process
       * of accepting samples.
       */
      std::mutex asynchronous_mode_mutex;

      /**
       * A mutex that controls access to all of the data structures involved
       * in parallel processing of samples in synchronous mode. In synchronous
       * mode, we can consume multiple items at the same time on different
       * threads (and so a shared mutex is appropriate) but when shutting things
       * down, we really need to have unique access to it. This is facilitated
       * by using shared_lock for the former operation, and unique_lock for the
       * latter.
       */
      std::shared_mutex synchronous_mode_mutex;

      /**
       * A queue of std::future objects that correspond to tasks that
       * process samples.
       */
      std::deque<std::future<void>> background_tasks;


      /**
       * Ensure that the queue of background tasks does not grow beyond
       * bounds by going through the queue and deleting all std::future
       * objects located at the beginning of the queue that have already
       * completed.
       */
      void trim_background_queue();
  };



  template <typename InputType>
  requires (Concepts::is_valid_sampletype<InputType>)
  Consumer<InputType>::Consumer (const ParallelMode supported_parallel_modes)
    :
    parallel_mode (static_cast<int>(ParallelMode::synchronous)),
    supported_parallel_modes (supported_parallel_modes),
    queue_size (1)
  {}


  template <typename InputType>
  requires (Concepts::is_valid_sampletype<InputType>)
  Consumer<InputType>::Consumer (Consumer &&consumer)
    :
    parallel_mode (consumer.parallel_mode.load()),
    supported_parallel_modes (consumer.supported_parallel_modes)
  {
    // Assert that there are no connections yet, as stated in the documentation.
    // If there are no connections, then there can also be no samples
    // in the queue yet.
    std::lock_guard<std::mutex> parallel_lock (consumer.asynchronous_mode_mutex);

    assert (consumer.connections_to_producers.size() == 0);
    assert (queue_size == 0);
    assert (background_tasks.size()==0);
  }



  template <typename InputType>
  requires (Concepts::is_valid_sampletype<InputType>)
  Consumer<InputType>::~Consumer ()
  {
    // The destructor of derived classes needs to call
    // disconnect_and_flush().
    assert (connections_to_producers.size() == 0);
  }



  template <typename InputType>
  requires (Concepts::is_valid_sampletype<InputType>)
  void
  Consumer<InputType>::
  connect_to_producer (Producer<InputType> &producer)
  {
    // Create a lambda function that receives a new sample and that in turn
    // calls the consume() member function of the current object.
    //
    // How exactly the lambda function that is called for each
    // sample looks like depends on the parallel mode of the current
    // object.
    std::function<void(InputType sample, AuxiliaryData aux_data)> sample_consumer;
    switch (static_cast<ParallelMode>(parallel_mode.load()))
      {
        // If we want to process samples synchronously,
        // then the lambda function simply calls the 'consume()'
        // function that derived classes need to implement. This means that
        // the caller will have to wait until the `consume` function
        // returns.
        //
        // But it isn't so simple. If some *upstream* filter is working
        // with tasks, then we may still end up in a situation where the
        // consume() function is called multiple times and on
        // separate threads. We don't want to synchronize these calls
        // (though implementations of the `consume()` function in
        // derived classes typically want to). So we need to expose to the
        // `disconnect_and_flush()` function that some calls to `consume()` are
        // currently running. We do this by acquiring a shared mutex in shared
        // lock state for each call to `consume()`. The shutdown procedure
        // then needs to use a unique lock to ensure that no other `consume()`
        // calls are happening at the same time as we are shutting things down.
        //
        // Finally, we need to be mindful that we could have received a sample
        // just at the same time as someone called `disconnect_and_flush()`.
        // In this case, we may have ended up in the lambda function below,
        // the OS has interrupted us and while we had to wait, the connection
        // to upstream was severed and `flush()` was called (or not yet, but
        // will soon). In that case, we don't want to process more samples.
        // If that is the case, once we get the asynchronous_mode_mutex,
        // we need to decide that we don't want to process this sample
        // any more -- as if the connection had been severed just *before*,
        // not just *after* the sample had been sent. This ensures that once
        // `disconnect_and_flush()` has finished, we no longer process any
        // samples.
        case ParallelMode::synchronous:
        {
          sample_consumer =
            [&](InputType sample, AuxiliaryData aux_data)
          {
            std::shared_lock<std::shared_mutex> one_of_many_lock(synchronous_mode_mutex);

            // If all connections have been severed since we actually
            // got here (via a connection, of course), we pretend that we
            // never received the sample.
            if (connections_to_producers.size() == 0)
              return;

            // Execute the consumer
            consume (std::move(sample), std::move(aux_data));
          };

          break;
        }


        // On the other hand, if we use asynchronous processing,
        // then the logic is substantially more complicated.
        case ParallelMode::asynchronous:
        {
          sample_consumer =
            [&](InputType sample, AuxiliaryData aux_data)
          {
            // Create a task that calls `consume()`. First, because we're going
            // to run this task at some later time, we need to copy the sample
            // and aux data at this point. (If we were using C++14, we could
            // actually *move* these objects into the lambda function, but that
            // is not possible with C++11.)
            auto worker =
              [this,sample,aux_data]()
            {
              this->consume (std::move(sample), std::move(aux_data));
            };

            // We then also need to put a future object into the queue that we
            // can query for unfinished objects. Since the queue is a shared
            // state, we need to access it under a lock. Once we are under the
            // lock, we can also commit to actually launching the task
            {
              std::lock_guard<std::mutex> parallel_lock (asynchronous_mode_mutex);

              // If all connections have been severed since we actually
              // got here (via a connection, of course), we pretend that we
              // never received the sample. This is the same as what happened
              // in the synchronous case above.
              if (connections_to_producers.size() == 0)
                return;

              // Then start the task in the background and let the OS decide when
              // it wants to execute it. The result is a std::future object that
              // we can query for completion of the task, and we will hold on
              // to this future object because we need to wait for tasks to finish
              // in flush().
              std::future<void> future = std::async(worker);

              // Next emplace the future object into the queue, in order
              // to allow other threads to wait for the termination of
              // the current job
              background_tasks.emplace_back (std::move(future));
            }


            // Finally, ensure that the queue does not grow beyond bound by
            // removing futures at the front that have already been satisfied.
            trim_background_queue();
          };

          break;
        }


        default:
          assert(false);
      }


    // Also build something that we can connect to the `flush_consumers`
    // signal. For this, we call flush(), which in the case of Filters is
    // overloaded to also call the flush_consumers() signal of the Producer
    // side of the Filter
    auto flush_slot = [this]()
    {
      this->flush();
    };

    auto disconnect_from_producer = [this](const Producer<InputType> &p)
    {
      assert (connections_to_producers.contains(&p));

      // Find one of the connections to the producer (there may be multiple,
      // but we don't care about that) and terminate it. We have to be mindful
      // that this function may have been called at the same time as a sample
      // was sent, and because the sample processing machinery queries the
      // state of the connections, we need to do things under the
      // mutices for both synchronous and asynchronous mode:
      {
        std::lock_guard<std::mutex> parallel_lock_1 (asynchronous_mode_mutex);
        std::unique_lock<std::shared_mutex> parallel_lock_2 (synchronous_mode_mutex);

        auto x = connections_to_producers.find(&p);

        std::get<0>(x->second).disconnect ();
        std::get<1>(x->second).disconnect ();
        std::get<2>(x->second).disconnect ();

        // Having terminated these connections, remove the entry from the map too.
        connections_to_producers.erase (x);
      }
    };

    // Finally hook it all up:
    connections_to_producers.insert (producer.connect_to_signals (sample_consumer,
                                                                  flush_slot,
                                                                  disconnect_from_producer));
  }



  template <typename InputType>
  requires (Concepts::is_valid_sampletype<InputType>)
  void
  Consumer<InputType>::
  set_parallel_mode (const ParallelMode parallel_mode,
                     const unsigned int queue_size)
  {
    assert (connections_to_producers.size() == 0);
    assert ((static_cast<int>(parallel_mode)
             & static_cast<int>(supported_parallel_modes))
            != 0);

    this->parallel_mode = static_cast<int>(parallel_mode);
    this->queue_size = queue_size;
  }



  template <typename InputType>
  requires (Concepts::is_valid_sampletype<InputType>)
  void
  Consumer<InputType>::
  disconnect_and_flush()
  {
    // Disconnect from anything that could submit more samples
    // to the current class. We have to be mindful that this function
    // may have been called at the same time as a sample was sent,
    // and because the sample processing machinery queries the
    // state of the connections, we need to do things under a
    // mutex.
    //
    // In fact, we have to do that under *both* of the locks used
    // for synchronous and asynchronous processing. In practice, each
    // consumer only ever uses one or the other approach, so we need
    // not worry about creating deadlocks by trying to acquire two
    // locks at the same time.
    {
      std::lock_guard<std::mutex> parallel_lock_1 (asynchronous_mode_mutex);
      std::unique_lock<std::shared_mutex> parallel_lock_2 (synchronous_mode_mutex);

      for (auto &[producer,connection] : connections_to_producers)
        {
          std::get<0>(connection).disconnect ();
          std::get<1>(connection).disconnect ();
          std::get<2>(connection).disconnect ();
        }
      connections_to_producers.clear();
    }

    // Then flush() the current state.
    flush ();
  }



  template <typename InputType>
  requires (Concepts::is_valid_sampletype<InputType>)
  void
  Consumer<InputType>::
  flush()
  {
    std::lock_guard<std::mutex> parallel_lock (asynchronous_mode_mutex);

    // For each std::future object, first check whether it
    // has already been waited on. If that is the case, then just
    // ignore it. If it hasn't, wait for its completion.
    for (std::future<void> &future : background_tasks)
      if (future.valid())
        future.wait();

    // At this point, we have waited for all futures, and because
    // we are under the lock, no new futures can have been added. So
    // just clear the whole array
    background_tasks.clear();
  }



  template <typename InputType>
  requires (Concepts::is_valid_sampletype<InputType>)
  void
  Consumer<InputType>::
  trim_background_queue()
  {
    std::lock_guard<std::mutex> parallel_lock (asynchronous_mode_mutex);

    // Drop elements on the front of the list that have completed (either because
    // someone has waited for it, or because it has finished since someone last looked).
    // If that is the case, then remove it from the list.
    while ((background_tasks.size() > 0) &&
           (background_tasks.front().wait_for(std::chrono::seconds(0))
            == std::future_status::ready))
      background_tasks.pop_front();
  }




  /**
   * A namespace for the implementation of consumers, i.e., classes
   * derived from the Consumer class.
   *
   * Strictly speaking, this should also include filters (i.e., classes
   * derived from the Filter class), but since these are in addition
   * derived from the Producer class, they are in their own namespace
   * Filters.
   */
  namespace Consumers
  {}
}
