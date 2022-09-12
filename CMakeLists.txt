#
# CMAKELISTS.TXT
# --------------
#
# Build file for JASS, including all the tools
# Copyright (c) 2016 Andrew Trotman
#
# Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
#
# Current supported build targets include:
#    make refresh_externals
#       This will re-download any external tools that are used as part of JASS.  This is usefule when,
#       for example, some external tool has been updated.  The initial use case was UnicodeData.txt the
#       data file that describes all the Unicode characters - which is used for the isalpha() and similar
#       methods.

project(JASS)

#
#CMake verison 3.2 is required to set CMAKE_CXX_STANDARD to C++14.
#

cmake_minimum_required (VERSION 3.8)

# set(CMAKE_VERBOSE_MAKEFILE on)

#
# default is release build, unless specified on the command line using FORCE_CMAKE_BUILD_TYPE
#

if (NOT FORCE_CMAKE_BUILD_TYPE)
	# set(CMAKE_BUILD_TYPE Debug)
	# set(CMAKE_CONFIGURATION_TYPES Debug)
	set(CMAKE_BUILD_TYPE Release)
	set(CMAKE_CONFIGURATION_TYPES Release)
else()
	set(CMAKE_BUILD_TYPE ${FORCE_CMAKE_BUILD_TYPE})
	set(CMAKE_CONFIGURATION_TYPES ${FORCE_CMAKE_BUILD_TYPE})
endif()

unset(FORCE_CMAKE_BUILD_TYPE CACHE)

message("Build Type:" ${CMAKE_BUILD_TYPE})

# add_definitions(-DUSE_CRT_MALLOC)
# add_definitions("-g -flto")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -g -flto")

#
# Add these lines for clang coverage build
#
# add_definitions("-fprofile-instr-generate -fcoverage-mapping")
# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-instr-generate -fcoverage-mapping")


#
# These lines add an externally configured #define to the compile line
#

if (JASS_EXTERNAL_DEFINE)
	message("Adding compile parameter:" ${JASS_EXTERNAL_DEFINE})
	add_definitions(${JASS_EXTERNAL_DEFINE})
endif()

#
#	Set the debug flags based on the compiler
#	_GLIBCXX_USE_CXX11_ABI=1 is necessary for g++ to allow C++ stateful custom allocators in std::string
#
if(WIN32)
 	add_definitions("/W3 /wd4005 /wd4996 /wd4514 /wd4996 /wd4820 /nologo")
else()
	add_definitions("-Wall -fPIC -D_GLIBCXX_USE_CXX11_ABI=1 -Wno-ignored-attributes -Wno-deprecated-declarations")
endif()

#
#	Add AVX and BMI use.
#
if(WIN32)
	#
	# At present (December 2018) appveyor builds do not support AVX512
	#
	if($ENV{APPVEYOR})
		add_definitions("-D__AVX2__=1")
	else()
		#
		# At present (August 2019) this desktop doesn not support AVX512
		#
		if($ENV{PROCESSOR_IDENTIFIER} STREQUAL "Intel64 Family 6 Model 23 Stepping 6, GenuineIntel")
			add_definitions("-D__AVX2__=1")
		else()
			add_definitions("-D__AVX512F__=1")
		endif()
	endif()
else()
	add_definitions(-march=native -mbmi -mavx2)
endif()

#
#	Compatibility with the FileSystem libraries for g++ prior to verison 9
#
if(UNIX AND NOT APPLE)
	message("Adding stdc++fs library for early g++ compatibility")
	link_libraries(stdc++fs)
endif()

#
# If we're in Debug mode then enble the DEBUG macro'
#
if(CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(-DDEBUG)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads)

#
# Download any external dependencies and get the include paths set up
#
add_subdirectory(external)
include_directories(source ${ZSTD_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR_2})

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
add_executable(unittest tools/unittest.cpp)
target_link_libraries(unittest JASSlib ${ZLIB_STATIC_LIB} ${ZSTD_STATIC_LIB} ${CMAKE_THREAD_LIBS_INIT})

#
# build the indexer
#
add_executable(JASS_index tools/JASS_index.cpp)
target_link_libraries(JASS_index JASSlib ${ZLIB_STATIC_LIB} ${CMAKE_THREAD_LIBS_INIT})

#
# build the compiled_indexes stubs
#
add_subdirectory(compiled_index)

#
# build the anytime search engine (JASS v1 re-implemented)
#
add_subdirectory(anytime)

#
# build the experimental components
#
add_subdirectory(experimental)

#
# build the checksummer
#
add_executable(hash tools/hash.cpp)
target_link_libraries(hash JASSlib ${CMAKE_THREAD_LIBS_INIT})

#
# Build the documentation with Doxygen (but Apple's HeaderDoc also works)
#
find_package(Doxygen)
if(DOXYGEN_FOUND)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_SOURCE_DIR}/build/Doxyfile  @ONLY)
	add_custom_target(
		docs ${DOXYGEN_EXECUTABLE}
		${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
		WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dox
		VERBATIM
		)
endif(DOXYGEN_FOUND)

#
# Do this to build for debugging:
#
#cmake -D FORCE_CMAKE_BUILD_TYPE=Debug ..
#
