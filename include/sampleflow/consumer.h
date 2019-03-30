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

#ifndef SAMPLEFLOW_CONSUMER_H
#define SAMPLEFLOW_CONSUMER_H

#include <sampleflow/auxiliary_data.h>
#include <sampleflow/producer.h>
#include <boost/signals2.hpp>
#include <list>

namespace SampleFlow
{
  template <typename InputType>
    class Consumer
    {
      public:
        virtual
        ~Consumer ()
        {
          for (auto &connection : connections_to_producers)
            connection.disconnect ();
        }

        void
        connect_to_producer (Producer<InputType> &producer)
        {
          connections_to_producers.push_back (
              producer.connect_to_signal (
                  [&](InputType sample, AuxiliaryData aux_data)
                    { this->process (std::move(sample), std::move(aux_data));}));
        }

        virtual void
        process_sample (InputType sample,
                        AuxiliaryData aux_data) = 0;

      private:
        std::list<boost::signals2::connection> connections_to_producers;
    };



  namespace Consumers
  {
    template <typename InputType>
      class MeanValue : public Consumer<InputType>
      {
        public:
          using value_type = InputType;

          MeanValue ()
          :
          n_samples (0)
          {}

          virtual void process_sample (InputType sample, AuxiliaryData /*aux_data*/) override
            {
            std::lock<std::mutex> lock(mutex);

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

          value_type
          get () const
          {
            value_type mean = sum;
            mean /= n_samples;
            return std::move (mean);
          }

        private:
          std::mutex mutex;
          InputType sum;
          std::size_t n_samples;
      };

  }
}

#endif
