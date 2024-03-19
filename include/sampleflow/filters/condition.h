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

#ifndef SAMPLEFLOW_FILTERS_CONDITIONAL_H
#define SAMPLEFLOW_FILTERS_CONDITIONAL_H

#include <sampleflow/filter.h>

#include <type_traits>


namespace SampleFlow
{
  namespace Filters
  {
    /**
     * An implementation of the Filter interface in which a given sample is
     * passed through based on whether a user-provided function returns
     * `true` or `false` for the sample and, possibly, its associated
     * AuxiliaryData object.
     *
     * Because the user-provided function returns a boolean value, it
     * is what is called a "predicate" and is referred by that name in the
     * code of this class.
     *
     *
     * ### Threading model ###
     *
     * The implementation of this class is thread-safe, i.e., its
     * filter() member function can be called concurrently and from multiple
     * threads.
     *
     *
     * @tparam SampleType The C++ type used to describe the incoming and outgoing
     *   samples.
     */
    template <typename SampleType>
    class Condition : public Filter<SampleType,SampleType>
    {
      public:
        /**
         * Constructor. This constructor is used when you pass in a predicate
         * that only takes the sample as argument. In other words, the
         * predicate can only use the sample, but not the associated
         * auxiliary data to decide whether a sample shall pass.
         *
         * @param[in] predicate A function object that is used to
         *   select whether a sample should be passed through.
         */
        template <typename PredicateType>
        requires (std::is_invocable_r_v<bool,PredicateType,SampleType>)
        Condition (const PredicateType &predicate);

        /**
         * Constructor. This constructor is used when you pass in a predicate
         * that takes both the sample and the associated auxiliary data as
         * arguments. In other words, the predicate may use both to decide
         * whether a sample shall pass.
         *
         * @param[in] predicate A function object that is used to
         *   select whether a sample should be passed through.
         */
        template <typename PredicateType>
        requires (std::is_invocable_r_v<bool,PredicateType,SampleType,AuxiliaryData>)
        Condition (const PredicateType &predicate);

        /**
         * Destructor. This function also makes sure that all samples this
         * object may have received have been fully processed. To this end,
         * it calls the Consumers::disconnect_and_flush() function of the
         * base class.
         */
        virtual ~Condition ();

        /**
         * Process one sample by converting it to the output type using the function
         * object passed to the constructor.
         *
         * @param[in] sample The sample to process.
         * @param[in] aux_data Auxiliary data about this sample. The current
         *   class does not know what to do with any such data and consequently
         *   simply passes it on.
         *
         * @return The sample and the auxiliary data
         *   originally associated with the sample if the predicate given
         *   to the constructor returned `true` for the current sample.
         */
        virtual
        std::optional<std::pair<SampleType, AuxiliaryData> >
        filter (SampleType sample,
                AuxiliaryData aux_data) override;

      private:
        /**
         * The predicate function used.
         */
        const std::function<bool (const SampleType &, const AuxiliaryData &aux_data)> predicate;
    };



    template <typename SampleType>
    template <typename PredicateType>
    requires (std::is_invocable_r_v<bool,PredicateType,SampleType>)
    Condition<SampleType>::
    Condition (const PredicateType &predicate)
    // Wrap the predicate and pass it on to the other constructor. Capture the
    // predicate by value copy.
      : Condition([p = predicate](const SampleType &sample, const AuxiliaryData &) -> bool
    { return p(sample); })
    {}



    template <typename SampleType>
    template <typename PredicateType>
    requires (std::is_invocable_r_v<bool,PredicateType,SampleType,AuxiliaryData>)
    Condition<SampleType>::
    Condition (const PredicateType &predicate)
      : predicate(predicate)
    {}



    template <typename SampleType>
    Condition<SampleType>::
    ~Condition ()
    {
      this->disconnect_and_flush();
    }


    template <typename SampleType>
    std::optional<std::pair<SampleType, AuxiliaryData> >
    Condition<SampleType>::
    filter (SampleType sample,
            AuxiliaryData aux_data)
    {
      if (predicate (sample, aux_data))
        return std::make_pair(std::move(sample), std::move(aux_data));
      else
        return {};
    }

  }
}

#endif
