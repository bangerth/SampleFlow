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


// Start the global module fragment that contains all of the #includes
// of C++ headers as well as of header files from other libraries we use.
module;


#include <any>
#include <atomic>
#include <cassert>
#include <complex>
#include <concepts>
#include <cstddef>
#include <deque>
#include <functional>
#include <future>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <shared_mutex>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include <boost/signals2.hpp>
#include <eigen3/Eigen/Dense>

#include <sampleflow/config.h>

import SampleFlow.core;

// Then start the SampleFlow module for producers:
export module SampleFlow.producers;
export {

#include <sampleflow/producers/delayed_rejection_mh.impl.h>
#include <sampleflow/producers/differential_evaluation_mh.impl.h>
#include <sampleflow/producers/metropolis_hastings.impl.h>
#include <sampleflow/producers/range.impl.h>

}
