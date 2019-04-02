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

#ifndef SAMPLEFLOW_FILTERS_COMPONENT_SPLITTER_H
#define SAMPLEFLOW_FILTERS_COMPONENT_SPLITTER_H

#include <sampleflow/filter.h>

namespace SampleFlow
{
  namespace Filters
  {
    template <typename InputType>
    class ComponentSplitter : public Filter<InputType, typename InputType::value_type>
    {
      public:
        ComponentSplitter (const unsigned int selected_component);

        virtual
        boost::optional<std::pair<InputType, AuxiliaryData> >
        filter (InputType sample,
                AuxiliaryData aux_data) override;

      private:
        const unsigned int selected_component;
    };



    template <typename InputType>
    ComponentSplitter<InputType>::
    ComponentSplitter (const unsigned int selected_component)
      : selected_component(selected_component)
    {}



    template <typename InputType>
    boost::optional<std::pair<InputType, AuxiliaryData> >
    ComponentSplitter<InputType>::
    filter (InputType sample,
            AuxiliaryData aux_data)
    {
      return
      { std::move(sample[selected_component]), std::move(aux_data)};
    }

  }
}

#endif
