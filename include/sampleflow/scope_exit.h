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

#ifndef SAMPLEFLOW_SCOPE_EXIT_H
#define SAMPLEFLOW_SCOPE_EXIT_H


#include <functional>

namespace SampleFlow
{
  namespace Utilities
  {
    /**
     * A class that stores a function object (typically a lambda function)
     * that is executed in the desctructor of the current object. Such
     * a class is useful to execute an action *whenever* the function in
     * which the object is declared exits, whether that is by falling
     * off the end of the function, an explicit `return` statement, or
     * because an exception is thrown. In all of these cases, the
     * destructors of all local objects are run, and the destructor
     * of the ScopeExit object then executes the stored action.
     *
     * The typical use case is because we want to perform a clean-up
     * action. For example, let's say that we want to print to the
     * console that we're leaving a long-running and potentially
     * complicated function. This could be achieved as follows:
     * @code
     * void long_running_and_complex_function ()
     * {
     *   ScopeExit scope_exit_action ([]() { std::cout << "Leaving the long running function!\n"; });
     *
     *   for (...)
     *   {
     *     // some 800 lines of complex code
     *   }
     * @endcode
     * The point here is that in order to have the message printed, we do not
     * need to go through the 800 lines of complex code and find all places
     * where either there is an explicit `return` statement, or where a
     * function that is called may throw an exception that we do not explicitly
     * catch: *Any* way the current function is exited will lead to the
     * message being printed.
     *
     * The function is conceptually very similar to the
     * [std::experimental::scope_exit](https://en.cppreference.com/w/cpp/experimental/scope_exit)
     * class template that may find its way into a future C++ standard.
     */
    class ScopeExit
    {
      public:
        /**
         * Constructor. Takes a function object that is to be executed at the
         * place where the current object goes out of scope as argument.
         */
        ScopeExit (const std::function<void ()> &exit_function);

        /**
         * Destructor. Execute the stored action.
         */
        ~ScopeExit ();

      private:
        /**
         * A copy of the function to be executed.
         */
        const std::function<void ()> exit_function;
    };



    inline
    ScopeExit::ScopeExit(const std::function<void ()> &exit_function)
      :
      exit_function (exit_function)
    {}



    inline
    ScopeExit::~ScopeExit()
    {
      // Actually trigger the stored function
      exit_function();
    }
  }
}

#endif
