SampleFlow -- a library for composing complex statistical sampling algorithms
=============================================================================

SampleFlow is a library that allows composing complex statistical sampling
algorithms from building blocks without writing a lot of spaghetti
code. It is written in C++11.


## The idea

Still to be written



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
  cmake help
```

The "generators" are listed at the bottom.



## Testing

There are numerous tests in the `tests/` directory. To execute them,
say

```
  make test
```

A description of how testing works can be found in
[tests/README.md](tests/README.md).


## Documentation

Most classes and functions are extensively documented, using the
[doxygen](http://www.doxygen.nl/) documentation generation program. If
you have doxygen installed, say

```
  cd doc
  make
```

and start browsing through the documentation in `doc/doxygen/index.html`.
