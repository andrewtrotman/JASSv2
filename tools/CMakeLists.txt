#
# CMAKELISTS.TXT
# --------------
#
# Build file for JASS tools.
# Copyright (c) 2016 Andrew Trotman
#
# Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
#
# Current tools include:
#    unicode_database_to_c
#        Tool to convert UnicodeData.txt into a set of ctype.h like methods that work on Unicode
#

include_directories(../source)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../source/unicode.cpp
	COMMAND unicode_database_to_c  ${CMAKE_CURRENT_SOURCE_DIR}/../external/Unicode/UnicodeData.txt ${CMAKE_CURRENT_SOURCE_DIR}/../external/Unicode/PropList.txt > ${CMAKE_CURRENT_BINARY_DIR}/../source/unicode.cpp
	DEPENDS unicode_database_to_c
	)

add_custom_target(generate_unicode_cpp DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../source/unicode.cpp)

#
# The tool that builds the unicode methods from the unicode database cannot link
# against JASSlib because JASSlib contains the unicode methods and this is a
# circular dependency.  So we have to name the individual files we need rather than
# the library.
#
add_executable(unicode_database_to_c
	unicode_database_to_c.cpp
	../source/file.cpp
	../source/bitstring.cpp)

set_property(TARGET unicode_database_to_c PROPERTY CXX_STANDARD 11)

