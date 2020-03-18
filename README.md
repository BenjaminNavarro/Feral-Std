# About

This repository hosts the standard library for the [Feral](https://github.com/Feral-Lang/Feral) programming language.

The standard library contains the following modules:
* `fs` - FileSystem related classes/functions
* `io` - Input/Output related functions
* `lang` - Enum/Struct related functions
* `map` - HashMap related classes/functions
* `os` - OS/environment related functions
* `str` - String manipulation functions
* `sys` - System/Language related variables/functions
* `vec` - Vector related functions

Possible future modules
* `builder` - Helper script to build/install C++ modules

# Installation

Installation steps are same as the language's.

## Prerequisites

To install `Feral-Std`, the following packages are required (same as `Feral`):
* CMake (build system - for compiling the project)
* LibGMP (with C++ support - libgmpxx)
* LibMPFR (floating point numbers)

**Note**: Windows is not yet supported.

## Procedure

Once the prerequisites have been met, clone this repository:
```bash
git clone https://github.com/Feral-Lang/Feral-Std.git
```

Inside the repository, create a directory (say `build`), `cd` in it and run the commands for building and installing Feral:
```bash
cd Feral-Std && mkdir build && cd build
cmake .. # optionally PREFIX_DIR=<dir> can be set before this
sudo make -j<cpu cores on your system> install
```

`sudo` is required to install the program to system directory (`/usr/local` by default). The environment variable `PREFIX_DIR` can be set to overwrite that.

Once installation is done, the standard libraries will be available for use with Feral.