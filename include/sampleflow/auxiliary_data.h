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

#ifndef SAMPLEFLOW_AUXILIARY_DATA_H
#define SAMPLEFLOW_AUXILIARY_DATA_H

#include <map>
#include <boost/any.hpp>

namespace SampleFlow
{
  /**
   * A data type used to convey additional information alongside samples that
   * are sent from Producer through Filter to Consumer objects. Oftentimes,
   * consumers may not know what to make of this information, and will then
   * simply ignore it; filters may simply pass it along from input to output.
   * On the other hand, *some* consumers may be written to make use of this
   * information.
   *
   * An example is that some sampling algorithms -- for example the
   * implementation in the Producers::MetropolisHastings class -- generate
   * not just samples from a probability distribution, but also have
   * information about the relative likelihood of each sample. This kind
   * of information can then be passed along, and a consumer can make use
   * of this, for example by looking through the stream of samples to
   * identify what the most likely sample was. The latter function,
   * identifying the MAP point (the "maximum a posterior probability
   * estimator") is implemented in the Consumers::MaximumProbabilitySample
   * class.
   *
   * Since different producer (or filter) classes may want to pass along
   * different kinds of information, the data type used is rather general:
   * It is a map with string keys that identify what the additional
   * information is, and an object of type boost::any that stores the
   * information itself. boost::any is a data type that wraps around an
   * object of any kind, in essence by storing something like a `void`
   * pointer. One must know the type of the object so stored to retrieve
   * it from a boost::any object, but this is not a restriction here
   * because a consumer wishing to process additional data clearly
   * needs to know something about what kind of information a producer
   * may have attached in the first place.
   *
   * Producers passing along such additional data need to document the string
   * under which the data is stored in the map and the type of the data
   * so stored.
   */
  using AuxiliaryData = std::map<std::string, boost::any>;
}


#endif
