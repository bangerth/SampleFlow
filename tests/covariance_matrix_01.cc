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


// Check the CovarianceMatrix consumer
// So far, we couldn't use Ranger producer because of insufficiency in Covariance_matrix.h, where code expects
// input as vallaray, vector, array or etc.


#include <iostream>
#include <fstream>
#include <valarray>

#include <sampleflow/producers/metropolis_hastings.h>
#include <boost/numeric/ublas/matrix.hpp>
#include <sampleflow/consumers/covariance_matrix.h>

using SampleType = std::valarray<double>;

double log_likelihood (const SampleType &x)
{
	return 1;
}

SampleType perturb (const SampleType &x)
{
	SampleType y = x;

	for (auto &el : y)
		el += 1;

	return y;
}

int main ()
{

	SampleFlow::Producers::MetropolisHastings<SampleType> mh_sampler;

	SampleFlow::Consumers::CovarianceMatrix<SampleType> covariance_matrix;
	covariance_matrix.connect_to_producer(mh_sampler);

	mh_sampler.sample ({0,1},
			&log_likelihood,
			&perturb,
			8);

	// At this point, we have sampled two sets: first starts from 1 and goes to 8, second - starts from 2 and
	// goes to 9. Sample variance for both vector is 6 as covariance too. Output whatever we got:
	std::cout << covariance_matrix.get()(0,0) << std::endl;
	std::cout << covariance_matrix.get()(0,1) << std::endl;
	std::cout << covariance_matrix.get()(0,1) << std::endl;
	std::cout << covariance_matrix.get()(1,1) << std::endl;
}
