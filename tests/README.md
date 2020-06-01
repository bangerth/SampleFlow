About the testsuite
===================

## Running the testsuite

This directory contains the testsuite of the SampleFlow library. To run it,
execute the command
```
make check
```
in the top-level directory after running the `cmake` command that configures
the entire library. 

Internally, `make check` simply calls `ctest`, and so if
you want to have more control, you can also just execute the command
```
  ctest <options>
```
where `<options>` can be any of the command line options understood by `ctest`.
For example,
```
  ctest -j8
```
will execute up to 8 tests at the same time, substantially reducing the time
necessarily to run through the entire test suite assuming you have multiple
cores in your computer. Likewise, you can do
```
  ctest -V
```
to see the commands executed for each test, along with their output. This is
useful if a test fails and you want to understand why.

Finally, if you want to run only a subset of the tests, you can use
```
  ctest -R <regex>
```
This command will then only run those tests that match the regular expression
`<regex>`. For example, if you call `ctest -R 'range.*01'`, then the test
`range_producer_01` will be run because its name matches the regular
expression, but the test `range_producer_02` will not. (There may of course
be other tests that also satisfy the regular expression, and these will then
also be run.) Running only a subset of tests is useful if you are debugging a
problem and are not interested in running *all* tests, but just those you know
are affected by a bug (or the fix you're currently developing).   


## How tests work

Each test in this directory consists of a `.cc` and a `.output` file. The
`.cc` file is a program that builds on the SampleFlow library, and that
produces output on the konsole (i.e., via `std::cout`). The test suite
harness redirects this output to a file with the ending `.result` when
the test is run, and then compares the `.output` and the `.result`
test. If they match, then the test succeeds. If the two files differ,
then the test fails. The idea of this setup is that at the time of writing
a test, the author runs the test in the then-current form, confirms that
the output is correct, and "blesses" it by copying it into the
`.output` file.

All future invokations of the test suite will then ensure that the output
one gets at any given day is equal to the blessed version from when the
test was written. The test suite therefore helps ensure that future
development does not break past functionality.


## Adding tests

Each piece of new functionality in SampleFlow should be accompanied by a new
test to ensure its correctness and continued functionality in view of
future development. To add a test, simply create a `.cc` file in this
directory that uses whatever functionality you have just implemented,
along with a dummy `.output` file that for the moment can be
empty.
In most cases, one doesn't write this `.cc` file from scratch, but instead
"clones" an existing test by copying the old test's `.cc` file to a new
name and then modifying it as appropriate.

Let's say you have so created files `tests/new_test.cc` and
`tests/new_test.output`. Then go through the following steps:

- In the top-level directory, execute `cmake .`. This will re-run the
  configuration step for all of SampleFlow, and pick up the new test.
  (For this step, it is important to have a dummy `.output` file because
  otherwise the testing system does not recognize that the `.cc` file
  by itself constitutes a test.)
- Run `make check` or `ctest` (possibly with one of the command line
  options mentioned above -- a good choice would be 
  `ctest -V -R new_test`). This will execute the newly added test,
  but because there is no correct, "blessed" output yet, the test will
  fail when comparing the results file with the still-empty output file.
- The executed test will have produced a file `tests/new_test.result`.
  Inspect it and convince yourself that the output shown there is indeed
  what you would have expected and is correct.
- Then copy the so-blessed output file to the location where the test suite
  can compare with it, by saying `cp tests/new_test.result tests/new_test.output`.
- You can then again run `ctest` and the new test should now succeed.

If you plan on submitting whatever feature or bug fix you have developed,
add and commit the `tests/new_test.cc` and `tests/new_test.output` files to the
git repository and create a pull request out of them.
