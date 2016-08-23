#
# CMAKELISTS.TXT
# --------------
#
# Build file for JASS
# Copyright (c) 2016 Andrew Trotman
#
# Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
#
# Current supported build targets inclide:
#    make unittests
#       This will build the JASS library alongside the unittest executable designed to test each class
#

add_library(JASSlib
	bitstring.h
	bitstring.cpp
	file.h
	file.cpp
	maths.h
	unicode_ctype.h
	)


