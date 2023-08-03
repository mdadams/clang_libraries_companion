Companion Git Repository for "Lecture Slides for the Clang Libraries"
=====================================================================

This repository contains all of the code examples that are associated
with the following slide deck:

  - Michael D. Adams.
    Lecture Slides for the Clang Libraries [LLVM/Clang 15].
    Edition 0.1.0,
    Aug. 2023.

Obtaining the Slide Deck
------------------------

The slide deck is available for download at:

  - <https://www.ece.uvic.ca/~mdadams/cppbook/#clang_slides>

Organization of Repository
--------------------------

The repository is organized as follows:

- slides/examples
  - This directory hierarchy contains all of the code examples from the
    slide deck.
  - Most of the code examples in this directory hierarchy have demo
    scripts that can be used to run the various example programs after
    they are built.
- miscellany/examples
  - This directory hierarchy contains a few slightly longer code examples
    that are not shown on the slides in the slide deck.

Each code example or group of code examples is structured as a separate
CMake project.
This allows users to experiment with an individual code example without
having to build all of the code examples.
For convenience, two CMakeLists.txt files are provided that build all of
their subordinate projects.  All of the projects can be built using a
provided build script (which invokes these two CMake superbuilds).

GitHub CI Workflow
------------------

This repository employs a CI workflow based on GitHub Actions.
Each time a new commit is pushed, the code examples in the repository
are built and run as a basic sanity test.
This CI workflow serves as an example to show how the LLVM/Clang
Ubuntu APT packages that are provided by the LLVM Project (at
<https://apt.llvm.org/>) can be used in GitHub-hosted Linux runners.
The CI workflow currently builds for a few combinations of the following:

  - operating system:
    - Ubuntu 22.04
    - Ubuntu 20.04
    - macOS 13
    - macOS 12
  - application programs built using:
    - Clang
    - GCC

Prerequisites to Building the Software
--------------------------------------

The code examples have the following software dependencies:

  - CMake
  - Make
  - version 15 of LLVM/Clang
  - GCC (if application programs are to be build with GCC)
  - Boost
  - Python
  - numerous basic Unix utilities such as bash, awk, grep, and so on
    (which should be included in any reasonable Unix-based system)

These dependencies must be installed prior to building the code examples.

If the C++ standard library does not support std::format, a custom version
of the fmt library can be automatically installed (as part of the build
process) to provide this support.  (This custom version of the fmt library
provides a standard library header called "format" and places a few key
declarations in the std namespace in that header.)  The C++ standard
library included with version 13 and above of GCC has support for
std::format.  (As of the time of this writing, however, GCC 13 is still
under development.)

Building the Software
---------------------

The code examples employ a CMake-based build process.  Each code example
or group of related examples is structured as a separate CMake project.
For convenience, a script is provided for building all of the code
examples in one step.

Some of the code examples require std::format (introduced in C++20).
If the C++ standard library implementation being used does not support
std::format, a custom version of the fmt library can be automatically
installed (as part of the build process) to provide this support.

To build all of the code examples (and optionally run all of the associated
demos), do the following:

0. Initialize the environment such that the necessary software dependencies
   (e.g., executables, headers, or libraries) will be found successfully
   at build time.
   **This step is typically only required if some of the software dependencies
   are installed in locations where they would not normally be found by the
   build process.**
   When this step is required, it might look something like the following:

       # Initialize the following variables used to configure the
       # environment:
       #   cmake_dir
       #   - The directory under which CMake has been installed
       #     (e.g., /usr, /usr/local).
       #   clang_dir
       #   - The directory under which LLVM/Clang has been installed
       #     (e.g., /usr, /usr/local).
       #   gcc_dir
       #   - The directory under which GCC has been installed
       #     (e.g., /usr, /usr/local).
       #   boost_dir
       #   - The directory under which Boost has been installed
       #     (e.g., /usr, /usr/local).

       # Use the preceding variables to configure the environment by
       # setting several key environment variables:
       export BOOST_INCLUDEDIR=$boost_dir/include
       export BOOST_LIBRARYDIR=$boost_dir/lib
       export PATH=$cmake_dir/bin:$clang_dir/bin:$gcc_dir/bin:$PATH
       export LD_LIBRARY_PATH=$BOOST_LIBRARYDIR:$LD_LIBRARY_PATH
       export CPATH=$boost_dir/include:$CPATH

1. Set the current working directory to the top-level directory of the
working tree of the cloned Git repository.

2. Invoke the build script with the appropriate options.  Nominally, the
   script is invoked as follows:

       ./build --defaults

   If the C++ standard library being used happens to support std::format, the
   "--no-fmt" option can be added to the invocation of the build script above
   (so that the custom version of the fmt library is not used).  That is,
   the following command can be used:

       ./build --defaults --no-fmt

   The build script supports numerous options.  For detailed usage
   information, invoke the script with the "-h" or "--help" option.
   The command-line arguments are processed in left-to-right order.
   So, in the case where a setting is established by more than one
   command-line option, the setting from the rightmost option
   takes effect.

3. If desired, run the demo scripts (as a basic sanity test) with the
command:

       ./build --demo

The output of the build process is placed in the directories:

  - slides/examples/tmp_build
  - miscellany/examples/tmp_build

The output for each CMake project is placed in a directory having the same
name as that project.  For example, the build output for the project called
cyclomatic_complexity from the slide-deck examples is placed in the directory:

    slides/examples/tmp_build/cyclomatic_complexity

Most projects have a demo script (either called "demo" or with a name
containing "demo").  For example, to run the demo script for the
cyclomatic_complexity project, use the command:

    slides/examples/tmp_build/cyclomatic_complexity/demo

Remarks on the Use of Address Sanitizer (ASan)
----------------------------------------------

Sometimes the use of Address Sanitizer (ASan) can be problematic, due,
for example, to quirks in the platform on which the code is being run.

On some platforms, some of the libraries used by the code examples
have been observed to have memory leaks.  If ASan complains about some
libraries having memory leaks, memory leak detection can be disabled
by adding "detect_leaks=0" to the list of ASan options in the ASAN_OPTIONS
environment variable.  For example, ASAN_OPTIONS can be set as follows:

    ASAN_OPTIONS=detect_leaks=0

It appears that user poisoning of memory can sometimes result in false
positives from ASan (namely, use-after-poison errors), depending on how
LLVM/Clang was built.  This is likely due to inconsistencies in how
user poisoning is handled in the LLVM/Clang libraries and the application
using these libraries.  If this problem is encountered, user poisoning
can be disabled by adding "allow_user_poisoning=0" to the list of ASan
options in the ASAN_OPTIONS environment variable.  For example,
ASAN_OPTIONS can be set as follows:

    ASAN_OPTIONS=allow_user_poisoning=0

Supported Platforms
-------------------

This software should work with most Unix-based systems (provided that
the necessary software dependencies are installed).
The GitHub CI workflow (discussed above) ensures that the software should
build and run reasonably reliably on Ubuntu Linux and macOS.
The author's main development platform is Fedora Linux.
So, the software should also work fairly reliably on this platform as well.
