SampleFlow -- a library for composing complex statistical sampling algorithms
=============================================================================

SampleFlow is a library that allows composing complex statistical sampling
algorithms from building blocks without writing a lot of spaghetti
code. It is written in C++11.


## The idea

Sampling algorithms often consist of a number of different components:

* "Producers": These are classes that actually generate
  samples. A typical example might be a [Metropolis-Hastings
  algorithm](https://en.wikipedia.org/wiki/Metropolis%E2%80%93Hastings_algorithm),
  but samples could also simply be lines from a file or be picked out
  of a spreadsheet of measurements.
* "Filters": These are classes that are connected to one or more
  streams of samples from producers and that either select a subset of
  samples (e.g., ever N-th sample) or that somehow transform them
  (e.g., out of a vector, pick out the first component).
* "Consumers": These are classes that are connected to one or more
  streams of samples from either producers or filters and that do
  something with them -- say, compute a mean value, a covariance
  matrix, a histogram.

Most codes that deal with mathematical sampling algorithms have
components that fall in these three categories, but they are often not
very well separated: The main loop that generates samples also
computes the mean value, or maybe just directly calls a function that
computes a mean value. Such code is difficult to maintain and extend:
Adding another statistical evaluation -- say, computing an
autocorrelation length -- requires understanding what the sample
generation code does and where one would call the piece of code that
computes the new evaluation.

SampleFlow is based on the idea that all of these kinds of codes can
be written in the form of a [directed acyclic graph
(DAG)](https://en.wikipedia.org/wiki/Directed_acyclic_graph): Samples
flow from Producers through Filters to Consumers, with Producer nodes
the sources of information and Consumers the sinks. In this view,
Filters can be seen as being both Producer and Consumer. Each Producer
may be connected to one or more downstream Consumers (including
Filters), and each Consumer (including Filters) may be connected to
one ore more Producers (including other Filters).

SampleFlow formalizes this scheme by providing a wide variety of
Producer, Filter, and Consumer classes and a framework in which they
can be connected. Moreover, in SampleFlow, samples are strongly typed:
A Producer generates samples of a very specific type, and a Consumer
can only be connected if it accepts this very type as input. Filters
can, of course, have different input and output types. The approach
encoded by this scheme is related to [dataflow
programming](https://en.wikipedia.org/wiki/Dataflow). The well-known
"[MapReduce](https://en.wikipedia.org/wiki/MapReduce)" is a particularly
simple example of an algorithm that can be represented in SampleFlow:
The stage that reads the database records is the Producer; the mapping
phase a Filter; and the reduction phase is a Consumer.

The principal goal of SampleFlow is to allow for the construction of
complex sampling and sample evaluation schemes while avoiding the
temptation of writing hard-to-maintain code that intermixes sample
generation, processing, and evaluation. In particular, it makes it
possible to feed back information from Consumers to Producers; for
example, it makes it simple to write sampling algorithms in which new
samples are generated based on the covariance matrix of previously
generated samples, without having to intermingle the code for these
two aspects.

SampleFlow is written in standards conforming
[C++11](https://en.wikipedia.org/wiki/C%2B%2B11).


## Installation

SampleFlow is a header-only library, so configuration and installation
is relatively easy and quick. Here are the commands necessary, once
you are in the SampleFlow directory

```
  cmake .
  make
```

For this to work, you need to have `cmake` installed on your machine,
along with a C++11-capable compiler that cmake can find.

If you work in an integrated development environment (IDE) such as
[Eclipse](https://www.eclipse.org/), you may want to use a command
such as

```
  cmake . -G"Eclipse CDT4 - Unix Makefiles"
```
instead, as that generates an Eclipse project that you can then import
into the IDE. This way, Eclipse will know about all relevant files,
and know how to compile and execute things. If Eclipse is not your
thing, you can let cmake generate projects for other IDEs as well; to
see a list of possibilities, say

```
  cmake --help
```

The "generators" are listed at the bottom.



## Documentation

Most classes and functions are extensively documented, using the
[doxygen](http://www.doxygen.nl/) documentation generation
program. You will have to have the `doxygen` program installed for
SampleFlow. You can then simply do

```
  cmake .
  make doc
```

and start browsing through the documentation in `doc/doxygen/index.html`.



## Testing

There are numerous tests in the `tests/` directory. To execute them,
say

```
  make test
```

A description of how testing works can be found in
[tests/README.md](tests/README.md).
