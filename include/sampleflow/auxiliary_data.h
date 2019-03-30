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

#ifndef SAMPLEFLOW_AUXILIARY_DATA_H
#define SAMPLEFLOW_AUXILIARY_DATA_H

#include <map>
#include <boost/any.hpp>

namespace SampleFlow
{
  using AuxiliaryData = std::map<std::string, boost::any>;
}


#endif
