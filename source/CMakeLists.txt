#
# CMAKELISTS.TXT
# --------------
#
# Build file for JASS
# Copyright (c) 2016 Andrew Trotman
#
# Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
#

set(JASSlib_FILES
	bitstring.h
	bitstring.cpp
	file.h
	file.cpp
	maths.h
	unicode.h
	unicode.cpp
	)
add_library(JASSlib ${JASSlib_FILES})

set_source_files_properties(unicode.cpp PROPERTIES GENERATED 1)
add_dependencies(JASSlib generate_unicode_cpp)
source_group ("Source Files" FILES ${JASSlib_FILES})
