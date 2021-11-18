#
# CMAKELISTS.TXT
# --------------
#
# Build file for JASS Compiled Indexes experiment
# Copyright (c) 2016 Andrew Trotman
#
# Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
#

include_directories(../source)

#
# This builds the compiled indexes executable for the 10 document unit test document collection,
# but in the process it also compiles and tests the generic components of compiled indexes.
#

set(JASS_ANYTIME_FILES
	JASS_anytime.cpp
	JASS_anytime_api.h
	JASS_anytime_api.cpp
	JASS_anytime_query.h
	JASS_anytime_result.h
	JASS_anytime_stats.h
	JASS_anytime_thread_result.h
	)

add_executable(JASS_anytime ${JASS_ANYTIME_FILES})
target_link_libraries(JASS_anytime JASSlib ${CMAKE_THREAD_LIBS_INIT})

source_group("Source Files" FILES ${JASS_ANYTIME_FILES})


#
#	Python interface to JASS_anytime
#
find_package(SWIG 4.0 COMPONENTS python)
if (SWIG_FOUND)
	#
	# Remove some cmake warnings
	#
	cmake_policy(SET CMP0078 NEW)
	cmake_policy(SET CMP0086 NEW)

	#
	# Set up SWIG with Python
	#
	message("SWIG found: ${SWIG_EXECUTABLE}")
	include(${SWIG_USE_FILE})
	set(SWIG_SOURCE_FILE_EXTENSIONS ".i" ".swg")

	#
	#	Work out which version of Python we're on and set up CMAKE_SWIG appropriately
	#
	find_package(Python REQUIRED COMPONENTS Interpreter Development)
	if(Python_VERSION VERSION_GREATER_EQUAL 3)
		message("Building for Python 3")
		set(CMAKE_SWIG_FLAGS -py3 -doxygen)
	else()
		message("Building for Python 2")
		set(CMAKE_SWIG_FLAGS -classic -doxygen)
	endif()

	#
	#	Finally, we're on to our program, which is written in C++
	#
	set_property(SOURCE PyJASS.swg PROPERTY CPLUSPLUS ON)

	#
	#	The Python library is called example and made up of example.i and example.cpp
	#
	swig_add_library(pyjass TYPE MODULE LANGUAGE python SOURCES PyJASS.swg
	JASS_anytime_api.h
	JASS_anytime_api.cpp
	JASS_anytime_query.h
	JASS_anytime_result.h
	JASS_anytime_stats.h
	JASS_anytime_thread_result.h
	)

	#
	#	Tell CMAKE_SWIG that we have our own include directories
	#
	set_property(GLOBAL PROPERTY SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON)
	include_directories(${Python_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

	#
	#	This Python modile is called 'PyJASS'
	#
	set_property(SOURCE PyJASS.swk PROPERTY SWIG_MODULE_NAME pyjass)

	#
	#	Finally tell cmake to use the Python library when linking
	#
	swig_link_libraries(pyjass ${Python_LIBRARIES} JASSlib ${CMAKE_THREAD_LIBS_INIT})
endif()
