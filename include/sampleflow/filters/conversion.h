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

#ifndef SAMPLEFLOW_FILTERS_CONVERSION_H
#define SAMPLEFLOW_FILTERS_CONVERSION_H

#include <sampleflow/filter.h>

namespace SampleFlow
{
  namespace Filters
  {
    /**
     * An implementation of the Filter interface in which a given sample is
     * to be converted from one data type to another. This is sometimes
     * useful if one wants to compute quantities from a stream of samples
     * in arithmetic that is not possible in the data type used for the sample.
     * An example is computing the mean value of the number of dots one gets
     * when throwing a dice (see the Consumers::MeanValue class): The
     * data type used to store the number of dots would generally be an
     * integer, but it is not possible to compute the mean value in this
     * data type because one gets out of the realm of the integers when
     * dividing by the (integer) number of samples. (In addition, the
     * mean value for the number of dots is 3.5, which is also not an integer,
     * but this is not the root cause of the problem in this example.) As
     * a consequence, if one wanted to compute this mean value, one would
     * want to convert the samples from integers to `double` values, for
     * example. (Whether computing the "average number of dots" actually
     * makes sense is a separate question.)
     *
     * The actual conversion of the data types is done using a function object
     * passed to the constructor. This also opens up the possibility of using
     * entirely different operations than just type conversion. For example,
     * one could imagine the `InputType` to be something that represents
     * triangles, and `OutputType` a floating point number that represents
     * the area of a triangle. In that case, the conversion function is not
     * simply a type cast, but a function that given a triangle outputs
     * its area. Such a filter might be useful if one has a sampling algorithm
     * for "random triangles" and wants to know the statistical distribution
     * of the areas of random triangles. A filter of the current type with
     * such a conversion function could then connect the triangle producer
     * to a Consumers::Histogram or Consumers::MeanValue consumer.
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
     * @tparam OutputType The C++ type used to describe the outgoing samples,
     *   i.e., the data type that the input is to be converted to.
     */
    template <typename InputType, typename OutputType>
    class Conversion : public Filter<InputType, OutputType>
    {
      public:
        /**
         * Constructor.
         *
         * @param[in] conversion_function A function object that is used to
         *   do the actual conversion from `InputType` to `OutputType`.
         *   The default for this function object is a lambda function
         *   that simply calls `static_cast`. This is appropriate for
         *   simple conversions such as from `int` to double.
         */
        Conversion (const std::function<OutputType (const InputType &)> &conversion_function
                    = [] (const InputType &in)
        {
          return static_cast<OutputType>(in);
        });

        /**
         * Destructor. This function also makes sure that all samples this
         * object may have received have been fully processed. To this end,
         * it calls the Consumers::disconnect_and_flush() function of the
         * base class.
         */
        virtual ~Conversion ();

        /**
         * Process one sample by converting it to the output type using the function
         * object passed to the constructor.
         *
         * @param[in] sample The sample to process.
         * @param[in] aux_data Auxiliary data about this sample. The current
         *   class does not know what to do with any such data and consequently
         *   simply passes it on.
         *
         * @return The converted sample and the auxiliary data
         *   originally associated with the sample.
         */
        virtual
        boost::optional<std::pair<OutputType, AuxiliaryData> >
        filter (InputType sample,
                AuxiliaryData aux_data) override;

      private:
        /**
         * The conversion function used.
         */
        const std::function<OutputType (const InputType &)> conversion_function;
    };



    template <typename InputType, typename OutputType>
    Conversion<InputType,OutputType>::
    Conversion (const std::function<OutputType (const InputType &)> &conversion_function)
      : conversion_function (conversion_function)
    {}



    template <typename InputType, typename OutputType>
    Conversion<InputType,OutputType>::
    ~Conversion ()
    {
      this->disconnect_and_flush();
    }


    template <typename InputType, typename OutputType>
    boost::optional<std::pair<OutputType, AuxiliaryData> >
    Conversion<InputType, OutputType>::
    filter (InputType sample,
            AuxiliaryData aux_data)
    {
      return std::make_pair(conversion_function(sample), std::move(aux_data));
    }

  }
}

#endif
