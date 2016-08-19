#
# CMAKELISTS.TXT
# --------------
#
# Build file for JASS, including all the tools
# Copyright (c) 2016 Andrew Trotman
#
# Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
#
# Current supported build targets inclide:
#    make refresh_externals
#       This will re-download any external tools that are used as part of JASS.  This is usefule when,
#       for example, some external tool has been updated.  The initial use case was UnicodeData.txt the
#       data file that describes all the Unicode characters - which is used for the isalpha() and similar
#       methods.

cmake_minimum_required (VERSION 3.0.0)
project(JASS)

#
# Download any external dependencies
#
add_subdirectory(external)

#
# Build the toolset
#
add_subdirectory(tools)

#
# Build the JASS library and unittest
#
add_subdirectory(source)

#
# 
#
include_directories(source)
add_executable(unittest tools/unittest.cpp)
target_link_libraries(unittest JASSlib)

