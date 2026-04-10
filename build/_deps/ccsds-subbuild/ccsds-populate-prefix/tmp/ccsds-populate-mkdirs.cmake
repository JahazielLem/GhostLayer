# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION ${CMAKE_VERSION}) # this file comes with cmake

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/Users/astrobyte/Repos/GhostLayer/build/_deps/ccsds-src")
  file(MAKE_DIRECTORY "/Users/astrobyte/Repos/GhostLayer/build/_deps/ccsds-src")
endif()
file(MAKE_DIRECTORY
  "/Users/astrobyte/Repos/GhostLayer/build/_deps/ccsds-build"
  "/Users/astrobyte/Repos/GhostLayer/build/_deps/ccsds-subbuild/ccsds-populate-prefix"
  "/Users/astrobyte/Repos/GhostLayer/build/_deps/ccsds-subbuild/ccsds-populate-prefix/tmp"
  "/Users/astrobyte/Repos/GhostLayer/build/_deps/ccsds-subbuild/ccsds-populate-prefix/src/ccsds-populate-stamp"
  "/Users/astrobyte/Repos/GhostLayer/build/_deps/ccsds-subbuild/ccsds-populate-prefix/src"
  "/Users/astrobyte/Repos/GhostLayer/build/_deps/ccsds-subbuild/ccsds-populate-prefix/src/ccsds-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/Users/astrobyte/Repos/GhostLayer/build/_deps/ccsds-subbuild/ccsds-populate-prefix/src/ccsds-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/Users/astrobyte/Repos/GhostLayer/build/_deps/ccsds-subbuild/ccsds-populate-prefix/src/ccsds-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
