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


// Say that we are building modules here.
module;

// Declare the principle 'SampleFlow' module that simply re-exports
// what the sub-modules have declared:
export module SampleFlow;

export import SampleFlow.core;
export import SampleFlow.producers;
export import SampleFlow.filters;
export import SampleFlow.consumers;
