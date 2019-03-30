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

namespace SampleFlow
{
  template <typename InputType, typename OutputType>
  class Filter : public Consumer<InputType>, public Producer<OutputType>
  {
    public:
      virtual
      void
      process_sample (InputType sample,
                      AuxiliaryData aux_data) override
      {
        boost::optional<std::pair<OutputType, AuxiliaryData>> maybe_sample =
                                                             filter (std::move (sampe), std::move (aux_data));

        if (maybe_sample)
          issue_sample (std::move (maybe_sample->first),
                        std::move (maybe_sample->second));
      }

      virtual boost::optional<std::pair<OutputType, AuxiliaryData>>
                                                                 filter (InputType sample,
                                                                         AuxiliaryData aux_data) = 0;
  };

  namespace Filters
  {
    // InputType==OutputType
    template <typename InputType>
    class TakeEveryNth : public Filter<InputType, InputType>
    {
      public:
        TakeEveryNth (const unsigned int every_nth)
          : counter (0),
            every_nth (every_nth)
        {
        }

        virtual boost::optional<std::pair<OutputType, AuxiliaryData>>
                                                                   filter (InputType sample,
                                                                           AuxiliaryData aux_data) override
        {
          std::lock<std::mutex> lock(mutex);

          ++counter;
          if (counter % every_nth == 0)
            {
              counter = 0;
              return
              { std::move(sample), std::move(aux_data)};
            }
          else
            return
              {};
        }

      private:
        std::mutex mutex;

        unsigned int counter;
        const unsigned int every_nth;
    };
  }
}

#endif
