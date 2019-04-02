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
      ~Consumer ();

      void
      connect_to_producer (Producer<InputType> &producer);

      virtual void
      consume (InputType sample,
               AuxiliaryData aux_data) = 0;

    private:
      std::list<boost::signals2::connection> connections_to_producers;
  };


  template <typename InputType>
  Consumer<InputType>::~Consumer ()
  {
    for (auto &connection : connections_to_producers)
      connection.disconnect ();
  }


  template <typename InputType>
  void
  Consumer<InputType>::
  connect_to_producer (Producer<InputType> &producer)
  {
    connections_to_producers.push_back (
      producer.connect_to_signal (
        [&](InputType sample, AuxiliaryData aux_data)
    {
      this->consume (std::move(sample), std::move(aux_data));
    }));
  }
}

#endif
