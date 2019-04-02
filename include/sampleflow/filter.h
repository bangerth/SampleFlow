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

#ifndef SAMPLEFLOW_FILTER_H
#define SAMPLEFLOW_FILTER_H

#include <sampleflow/producer.h>
#include <sampleflow/consumer.h>

#include <boost/optional.hpp>
#include <mutex>

namespace SampleFlow
{
  template <typename InputType, typename OutputType>
  class Filter : public Consumer<InputType>, public Producer<OutputType>
  {
    public:
      virtual
      void
      consume (InputType sample,
               AuxiliaryData aux_data) override;

      virtual
      boost::optional<std::pair<OutputType, AuxiliaryData> >
      filter (InputType sample,
              AuxiliaryData aux_data) = 0;
  };


  template <typename InputType, typename OutputType>
  void
  Filter<InputType,OutputType>::
  consume (InputType sample,
           AuxiliaryData aux_data)
  {
    boost::optional<std::pair<OutputType, AuxiliaryData> >
    maybe_sample =
      filter (std::move (sample), std::move (aux_data));

    if (maybe_sample)
      this->issue_sample (std::move (maybe_sample->first),
                          std::move (maybe_sample->second));
  }
}

#endif
