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
	unicode_ctype.h
	unicode_ctype.cpp
	)

set_source_files_properties(unicode_ctype.cpp PROPERTIES GENERATED 1)

