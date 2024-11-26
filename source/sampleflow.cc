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


// Then start the SampleFlow module:
export module SampleFlow;
export {

// First include the basic classes and tools that all producers, filters, and
// consumers rely upon. These need to be in this order:
#include <sampleflow/concepts.h>
#include <sampleflow/auxiliary_data.h>
#include <sampleflow/types.h>
#include <sampleflow/element_access.h>
#include <sampleflow/parallel_mode.h>

#include <sampleflow/producer.h>
#include <sampleflow/filter.h>
#include <sampleflow/consumer.h>

#include <sampleflow/connections.h>
#include <sampleflow/scope_exit.h>

// Then the various producer classes:
#include <sampleflow/producers/delayed_rejection_mh.impl.h>
#include <sampleflow/producers/differential_evaluation_mh.impl.h>
#include <sampleflow/producers/metropolis_hastings.impl.h>
#include <sampleflow/producers/range.impl.h>

// Then the various filter classes:
#include <sampleflow/filters/component_splitter.impl.h>
#include <sampleflow/filters/condition.impl.h>
#include <sampleflow/filters/conversion.impl.h>
#include <sampleflow/filters/discard_first_n.impl.h>
#include <sampleflow/filters/pass_through.impl.h>
#include <sampleflow/filters/take_every_nth.impl.h>

// And finally the various consumer classes:
#include <sampleflow/consumers/acceptance_ratio.impl.h>
#include <sampleflow/consumers/action.impl.h>
#include <sampleflow/consumers/auto_covariance_matrix.impl.h>
#include <sampleflow/consumers/auto_covariance_trace.impl.h>
#include <sampleflow/consumers/average_cosinus.impl.h>
#include <sampleflow/consumers/count_samples.impl.h>
#include <sampleflow/consumers/covariance_matrix.impl.h>
#include <sampleflow/consumers/histogram.impl.h>
#include <sampleflow/consumers/last_sample.impl.h>
#include <sampleflow/consumers/maximum_probability_sample.impl.h>
#include <sampleflow/consumers/mean_value.impl.h>
#include <sampleflow/consumers/pair_histogram.impl.h>
#include <sampleflow/consumers/stream_output.impl.h>

}
