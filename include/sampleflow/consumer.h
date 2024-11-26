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

#ifndef SAMPLEFLOW_CONSUMER_H
#define SAMPLEFLOW_CONSUMER_H

#include <sampleflow/config.h>

#include <sampleflow/auxiliary_data.h>
#include <sampleflow/concepts.h>
#include <sampleflow/producer.h>
#include <sampleflow/parallel_mode.h>
#include <boost/signals2.hpp>

#include <map>
#include <utility>
#include <future>
#include <atomic>
#include <deque>
#include <mutex>
#include <shared_mutex>

// Import the implementation of the things for this header file:
#include <sampleflow/consumer.impl.h>

#endif
