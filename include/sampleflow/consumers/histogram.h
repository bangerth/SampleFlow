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

#ifndef SAMPLEFLOW_CONSUMERS_HISTOGRAM_H
#define SAMPLEFLOW_CONSUMERS_HISTOGRAM_H

#include <sampleflow/consumer.h>
#include <mutex>
#include <type_traits>
#include <vector>
#include <tuple>
#include <cmath>
#include <ostream>


namespace SampleFlow
{
  namespace Consumers
  {
    template <typename InputType>
    class Histogram : public Consumer<InputType>
    {
      public:
        static_assert (std::is_arithmetic<InputType>::value == true,
                       "This class can only be used for scalar input types.");

        using value_type = std::vector<std::tuple<double,double,std::size_t>>;

        enum class SubdivisionScheme
        {
          linear, logarithmic
        };

        Histogram (const double min_value,
                   const double max_value,
                   const unsigned int n_subdivisions,
                   const SubdivisionScheme subdivision_scheme = SubdivisionScheme::linear);

        virtual
        void
        consume (InputType sample, AuxiliaryData /*aux_data*/) override;

        value_type
        get () const;

        void
        write_gnuplot (std::ostream &&output_stream) const;

      private:
        mutable std::mutex mutex;
        const double min_value;
        const double max_value;
        const unsigned int n_subdivisions;
        const SubdivisionScheme subdivision_scheme;

        std::vector<std::size_t> bins;

        unsigned int bin_number (const double value) const;
    };


    template <typename InputType>
    Histogram<InputType>::
    Histogram (const double min_value,
               const double max_value,
               const unsigned int n_subdivisions,
               const SubdivisionScheme subdivision_scheme)
      :
      min_value (min_value),
      max_value (max_value),
      n_subdivisions (n_subdivisions),
      subdivision_scheme (subdivision_scheme),
      bins (n_subdivisions)
    {}



    template <typename InputType>
    void
    Histogram<InputType>::
    consume (InputType sample, AuxiliaryData /*aux_data*/)
    {
      // If a sample lies outside the bounds, just discard it:
      if (sample<min_value || sample>max_value)
        return;

      // Otherwise we need to update the histogram bins
      const unsigned int bin = bin_number(sample);

      std::lock_guard<std::mutex> lock(mutex);
      ++bins[bin];
    }



    template <typename InputType>
    typename Histogram<InputType>::value_type
    Histogram<InputType>::
    get () const
    {
      // First create the output table and breakpoints
      value_type return_value (n_subdivisions);
      for (unsigned int bin=0; bin<n_subdivisions; ++bin)
        {
          double bin_min, bin_max;

          switch (subdivision_scheme)
            {
              case SubdivisionScheme::linear:
              {
                bin_min = min_value + bin*(max_value-min_value)/n_subdivisions;
                bin_max = min_value + (bin+1)*(max_value-min_value)/n_subdivisions;

                break;
              }

              case SubdivisionScheme::logarithmic:
              {
                bin_min = std::exp(std::log(min_value) + bin*(std::log(max_value)-std::log(min_value))/n_subdivisions);
                bin_max = std::exp(std::log(min_value) + (bin+1)*(std::log(max_value)-std::log(min_value))/n_subdivisions);

                break;
              }

              default:
                bin_min = bin_max = 0;
            }

          std::get<0>(return_value[bin]) = bin_min;
          std::get<1>(return_value[bin]) = bin_max;
        }

      // Now fill the bin sizes under a lock
      std::lock_guard<std::mutex> lock(mutex);
      for (unsigned int bin=0; bin<n_subdivisions; ++bin)
        {
          std::get<2>(return_value[bin]) = bins[bin];
        }

      return return_value;
    }



    template <typename InputType>
    void
    Histogram<InputType>::
    write_gnuplot(std::ostream &&output_stream) const
    {
      const auto histogram = get();

      // For each bin, draw three sides of a rectangle over the x-axis
      for (const auto &bin : histogram)
        {
          output_stream << std::get<0>(bin) << ' ' << 0 << '\n';
          output_stream << std::get<0>(bin) << ' ' << std::get<2>(bin) << '\n';
          output_stream << std::get<1>(bin) << ' ' << std::get<2>(bin) << '\n';
          output_stream << std::get<1>(bin) << ' ' << 0 << '\n';
          output_stream << '\n';
        }

      output_stream << std::flush;
    }


    template <typename InputType>
    unsigned int
    Histogram<InputType>::
    bin_number (const double value) const
    {
      assert (value>=min_value);
      assert (value<=max_value);
      switch (subdivision_scheme)
        {
          case SubdivisionScheme::linear:
            return std::max(0,
                            std::min(static_cast<int>(n_subdivisions)-1,
                                     static_cast<int>((value-min_value)/
                                                      ((max_value-min_value)/n_subdivisions))));

          case SubdivisionScheme::logarithmic:
            return std::max(0,
                            std::min(static_cast<int>(n_subdivisions)-1,
                                     static_cast<int>((std::log(value)-std::log(min_value))/
                                                      ((std::log(max_value)-std::log(min_value))/n_subdivisions))));

          default:
            return 0;
        }
    }

  }
}

#endif
