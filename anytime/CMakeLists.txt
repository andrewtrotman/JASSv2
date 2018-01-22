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

set(COMPILED_INDEX_FILES
	JASS_anytime.cpp
	JASS_anytime_query.h
	JASS_anytime_stats.h
	JASS_anytime_thread_result.h
	)

add_executable(JASS_anytime ${COMPILED_INDEX_FILES})
target_link_libraries(JASS_anytime JASSlib ${CMAKE_THREAD_LIBS_INIT})

source_group("Source Files" FILES ${COMPILED_INDEX_FILES})
