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

cmake_minimum_required (VERSION 3.5.0)

# set(CMAKE_VERBOSE_MAKEFILE on)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CONFIGURATION_TYPES Debug)

#
#	Set the debug flags based on the compiler
#
if(WIN32)
 	add_definitions( "/W3 /wd4005 /wd4996 /wd4514 /wd4996 /wd4820 /nologo" )
else()
	add_definitions("-Wall")
endif()

project(JASS)

set(CMAKE_CXX_STANDARD 11)

#
# Download any external dependencies
#
add_subdirectory(external)

#
# Build the toolset
#
add_subdirectory(tools)

#
# Build the JASS library
#
add_subdirectory(source)

#
# Build the documentation examples
#
add_subdirectory(examples)

#
#  build the unit tests
#
include_directories(source)
add_executable(unittest tools/unittest.cpp)
target_link_libraries(unittest JASSlib)

#
# build the indexer
#
add_executable(index tools/index.cpp)
target_link_libraries(index JASSlib)

#
# Build the documentation with Doxygen (but Apple's HeaderDoc also works)
#
find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/build/Doxyfile.in ${CMAKE_CURRENT_SOURCE_DIR}/build/Doxyfile  @ONLY)
	add_custom_target(
		docs ${DOXYGEN_EXECUTABLE}
		${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dox
		VERBATIM
		)
endif(DOXYGEN_FOUND)
