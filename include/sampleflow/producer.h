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

#ifndef SAMPLEFLOW_PRODUCER_H
#define SAMPLEFLOW_PRODUCER_H

#include <sampleflow/auxiliary_data.h>
#include <boost/signals2.hpp>
#include <functional>


namespace SampleFlow
{
  template <typename OutputType>
  class Producer
  {
    public:
      void connect_to_signal (const std::function<void (OutputType, AuxiliaryData)> &f)
      {
        issue_sample.connect (f);
      }

    protected:
      boost::signals2::signal<void (OutputType, AuxiliaryData)> issue_sample;
  };
}


#endif
