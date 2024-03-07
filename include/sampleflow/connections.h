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

#ifndef SAMPLEFLOW_CONNECTIONS_H
#define SAMPLEFLOW_CONNECTIONS_H

#include <sampleflow/concepts.h>
#include <sampleflow/consumer.h>
#include <sampleflow/producer.h>


namespace SampleFlow
{
  /**
   * Connect a consumer to a producer of samples, by writing
   * @code
   *   producer >> consumer;
   * @endcode
   * Both consumer and producer may themselves be "filters", i.e.,
   * derived from the Filter class. Filters are both consumers and
   * producers, and so qualify for both the left and right hand side
   * of `operator<<`.
   */
  template <typename LeftType, typename RightType>
  requires (Concepts::is_producer<LeftType> && Concepts::is_consumer<RightType>)
  void operator>> (LeftType &producer, RightType &consumer)
  {
    consumer.connect_to_producer (producer);
  }
}


#endif /* SAMPLEFLOW_CONNECTIONS_H */
