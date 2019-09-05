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


// A simple test for the Metropolis-Hasting producer using the simple example builded in the beginning of
// code. After that, it uses MaximumProbabilitySample consumer and prints out sample with the biggest likelihood.


#include <iostream>
#include <sampleflow/producers/metropolis_hastings.h>
#include <sampleflow/consumers/maximum_probability_sample.h>
#include <valarray>
#include <random>
#include <cmath>

using SampleType = double;

//Simplified version of functions log_likelihood and perturb. These two doesn't have any random parts to
//initialize testing.

double log_likelihood (const SampleType &x)
{
	return x+1;
}

SampleType perturb (const SampleType &x)
{
	return x+1;
}

int main ()
{

	SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;

	SampleFlow::Consumers::MaximumProbabilitySample<SampleType> MAP_point;
	MAP_point.connect_to_producer (mh_sampler);

	/*Sampler should sample numbers from 1 to 10000*/

	mh_sampler.sample ({0},
			&log_likelihood,
			&perturb,
			10000);

	/*Consumer should print out number 10000*/

	std::cout << MAP_point.get().first << std::endl;

}
