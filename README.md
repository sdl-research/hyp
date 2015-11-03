# `hyp` -- The Hypergraph Toolkit

`hyp` is a general C++11 toolkit for the representation,
manipulation, and optimization of weighted hypergraphs. Finite-state
machines are modeled as a special case.

The `hyp` toolkit provides data structures and algorithms to process
weighted directed hypergraphs.

Such hypergraphs are important in natural language processing and
machine learning. Their use arises naturally in parsing, syntax-based
machine translation and other tree-based models, as well as in logic
and weighted logic programming.

This library and the `hyp` command line tools provide data structures
for hypergraphs as well as many algorithms: `compose`, `project`,
`invert`, `union`, `concatenate`, the shortest path algorithm, the
inside and outside algorithm, and more. In addition, it provides
functionality to optimize hypergraph feature weights from training
data.

## Documentation

For documentation see
[`hyp-tutorial.pdf`](https://github.com/sdl-research/hyp/raw/master/hyp-tutorial.pdf)
provided in this directory for your immediate gratification. (You can
also build the PDF with [TeX](https://www.tug.org/texlive/):
`docs/hyp/hyp-tutorial.tex`)

For documentation of the source code API, consider generating
[Doxygen](http://www.doxygen.org/): `doxygen doxy/doxy.conf`.

## Build

To build `hyp`, follow the following steps:

* In addition to the `hyp` repository, clone the
[sdl-externals](https://github.com/sdl-research/sdl-externals)
repository, which contains pre-built third-party libraries used by
`hyp`.

### Windows

* Set an environment variable `SDL_EXTERNALS_PATH` to point to the
  `Windows` subdirectory of your `sdl-externals` clone. Example: `set
  SDL_EXTERNALS_PATH=c:/src/sdl-externals/Windows`

* Create and change to a directory `build-hyp` inside your `hyp`
  directory.

* Run [cmake](http://www.cmake.org/) like this: `cmake
  -DCMAKE_BUILD_TYPE=Release -G "Visual Studio 10 Win64"
  c:/src/hyp/sdl`

* Then build the project as usual with Visual Studio 2015 or later.

### Linux

* For older versions of gcc or clang, edit SdlHelperFunction.cmake to
 replace `-std=c++14` by `-std=c++11`

* Set an environment variable `SDL_EXTERNALS_PATH` to point to the
  `FC12` subdirectory of your `sdl-externals` clone. This also works
  for CentOS 6 and possibly other Linux distributions. Example:
  `export SDL_EXTERNALS_PATH=$HOME/sdl-externals/FC12`

* Create and change to a directory `build-hyp` inside your `hyp`
  directory.

* Run [cmake](http://www.cmake.org/) like this: `cmake
-DCMAKE_BUILD_TYPE=Release ../sdl`

* Run `make`: make -j4

## Pull Requests

If you're willing to give us permission to use your source code, and
any patents required for it, we welcome pull requests. Keep your diffs
small, or talk to us before getting too crazy so we can save you some
wasted effort.

## Citations

If you use the `hyp` toolkit in your publication, please cite the
tutorial document:

Markus Dreyer and Jonathan Graehl (2014): Tutorial: The hyp hypergraph
toolkit. Url:
https://github.com/sdl-research/hyp/raw/master/hyp-tutorial.pdf
