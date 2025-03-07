Companion Git Repository for "Lecture Slides for the Clang Libraries"
=====================================================================

This repository contains all of the code examples that are associated
with the following slide deck:

  - Michael D. Adams.
    Lecture Slides for the Clang Libraries [LLVM/Clang 20].
    Edition 0.3.0,
    Mar. 2025.

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

This repository employs a CI workflow based on GitHub Actions.  Each time
a new commit is pushed, the code examples in the repository are built
and run as a basic sanity test.  The workflow employs Podman/Docker
container images based on Ubuntu and Fedora Linux.  The Ubuntu container
image was generated using the Ubuntu APT packages that are provided by
the LLVM Project (at <https://apt.llvm.org/>).  The code examples are
built using both Clang and GCC.

Prerequisites to Building the Software
--------------------------------------

The code examples have the following software dependencies:

  - CMake
  - Make
  - version 20 of LLVM/Clang
  - GCC (if application programs are to be built with GCC)
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
std::format.

For convenience, a Podman/Docker containerized environment is
provided that includes all of the necessary software dependencies.
More information on this containerized environment is provided in a
later section.

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

Podman/Docker Containerized Demonstration Environment
-----------------------------------------------------

A Dockerfile is provided that can be used to create a Podman/Docker
container image with all of the necessary software dependencies for
building and running the code examples in this repository.  Building
this image is quite time consuming since it requires building LLVM.
For this reason, a prebuilt version of the Podman/Docker container
image has been made available via the following repository in the GitHub
Container Registry:

  - `ghcr.io/mdadams/clang_libs-fedora_41-llvm_20`

Instructions are given below on how to use this containerized environment.
Although these instructions use (rootless) Podman, the `podman` and `docker`
programs have almost identical command-line interfaces.  So, it should
be possible to substitute `docker` for `podman` in the instructions.

Let $TOP_DIR denote the top-level directory of the working tree of the
cloned Git repository (i.e., the directory that contains the file named
README.md which you are currently reading).  Note that $TOP_DIR should
be an absolute path.

1. Set the working directory to the top-level directory of the working
   tree using the command:

       cd $TOP_DIR

2. Obtain the Podman/Docker container image.  Two options are available
   in this step.  You can either use the author's prebuilt image available
   from the GitHub Container Registry or build the image yourself.
   In the interest of simplicity, the use of the prebuilt image is
   recommended.

   - Option 1 (Use the prebuilt image).
     To retrieve the prebuilt image, use the following command:

         podman pull ghcr.io/mdadams/clang_libs-fedora_41-llvm_20

   - Option 2 (Build the image from scratch).
     To build the image from scratch, use the following command:

         podman build --tag clang_libs-fedora_41-llvm_20 \
           -f $TOP_DIR/podman/Dockerfile-fedora_41-llvm_20 $TOP_DIR

     Note that building the container image from scratch involves
     building LLVM, which takes a considerable amount of time.

3. Create a temporary instance of the container and run a Bash shell in the
   container using the command:

       podman run -i -t --rm -v $TOP_DIR:$TOP_DIR:rw -w $TOP_DIR \
         --cap-add=SYS_PTRACE --security-opt label=disable \
         clang_libs-fedora_41-llvm_20 /bin/bash

   Note that the "--cap-add" and "--security-opt" options may not be
   needed.

   If you do not want the container to be deleted after the Bash shell
   is exited, omit the "--rm" option.

4. Proceed to build and run the demo scripts as described in detail in
   an earlier section.  For example, one might invoke the following
   command from the Bash shell running in the container:

       ./build --defaults

Remarks on the Use of Address Sanitizer (ASan)
----------------------------------------------

Sometimes the use of Address Sanitizer (ASan) can be problematic, due,
for example, to quirks in the platform on which the code is being run.
The runtime behavior of ASan can be controlled via the environment
variable ASAN_OPTIONS, whose value is a colon-separated list of
key-value pairs (e.g., "verbosity=1:detect_leaks=0").

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
