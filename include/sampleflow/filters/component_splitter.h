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

#ifndef SAMPLEFLOW_FILTERS_COMPONENT_SPLITTER_H
#define SAMPLEFLOW_FILTERS_COMPONENT_SPLITTER_H

#include <sampleflow/filter.h>

namespace SampleFlow
{
  namespace Filters
  {
    /**
     * An implementation of the Filter interface in which a given component
     * of a vector-valued sample is passed on. This useful if, for example,
     * one wants to compute the mean value or standard deviation of an
     * individual component of a sample vector is of interest.
     *
     *
     * ### Threading model ###
     *
     * The implementation of this class is thread-safe, i.e., its
     * filter() member function can be called concurrently and from multiple
     * threads.
     *
     *
     * @tparam InputType The C++ type used to describe the incoming samples.
     *   For the current class, the output type of samples is the `value_type`
     *   of the `InputType`, i.e., `typename InputType::value_type`, as this
     *   indicates the type of individual components of the `InputType`.
     */
    template <typename InputType>
    class ComponentSplitter : public Filter<InputType, typename InputType::value_type>
    {
      public:
        /**
         * Constructor.
         *
         * @param[in] selected_component The index of the component that is to
         *   be selected.
         */
        ComponentSplitter (const unsigned int selected_component);

        /**
         * Copy constructor.
         */
        ComponentSplitter (const ComponentSplitter<InputType> &o);

        /**
         * Destructor. This function also makes sure that all samples this
         * object may have received have been fully processed. To this end,
         * it calls the Consumers::disconnect_and_flush() function of the
         * base class.
         */
        virtual ~ComponentSplitter ();

        /**
         * Process one sample by extracting a given component and passing
         * that on as a sample in its own right to downstream consumers.
         *
         * @param[in] sample The sample to process.
         * @param[in] aux_data Auxiliary data about this sample. The current
         *   class does not know what to do with any such data and consequently
         *   simply passes it on.
         *
         * @return The selected component of the sample and the auxiliary data
         *   originally associated with the sample.
         */
        virtual
        boost::optional<std::pair<typename InputType::value_type, AuxiliaryData> >
        filter (InputType sample,
                AuxiliaryData aux_data) override;

      private:
        /**
         * The selected component of samples to be extracted.
         */
        const unsigned int selected_component;
    };



    template <typename InputType>
    ComponentSplitter<InputType>::
    ComponentSplitter (const unsigned int selected_component)
      : selected_component(selected_component)
    {}



    template <typename InputType>
    ComponentSplitter<InputType>::
    ComponentSplitter (const ComponentSplitter<InputType> &o)
      : selected_component(o.selected_component)
    {}



    template <typename InputType>
    ComponentSplitter<InputType>::
    ~ComponentSplitter ()
    {
      this->disconnect_and_flush();
    }


    template <typename InputType>
    boost::optional<std::pair<typename InputType::value_type, AuxiliaryData> >
    ComponentSplitter<InputType>::
    filter (InputType sample,
            AuxiliaryData aux_data)
    {
      assert (selected_component < sample.size());

      return std::make_pair(std::move(sample[selected_component]),
                            std::move(aux_data));
    }

  }
}

#endif
