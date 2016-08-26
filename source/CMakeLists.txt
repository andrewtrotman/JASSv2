#
# CMAKELISTS.TXT
# --------------
#
# Build file for JASS
# Copyright (c) 2016 Andrew Trotman
#
# Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
#

add_library(JASSlib
	bitstring.h
	bitstring.cpp
	file.h
	file.cpp
	maths.h
	unicode_utype.h
	)

set_source_files_properties(unicode_utype.cpp PROPERTIES GENERATED 1)
add_dependencies(JASSlib generate_unicode_utype_cpp)