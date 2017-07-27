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
	JASS_compiled_index.cpp
	JASS_postings.h
	JASS_postings.cpp
	JASS_primary_keys.cpp
	JASS_vocabulary.h
	JASS_vocabulary.cpp
	)

add_executable(JASS_compiled_index ${COMPILED_INDEX_FILES})
target_link_libraries(JASS_compiled_index JASSlib ${CMAKE_THREAD_LIBS_INIT})

source_group("Source Files" FILES ${COMPILED_INDEX_FILES})
