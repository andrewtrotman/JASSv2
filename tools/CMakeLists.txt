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
#    unicodedata_to_utype
#        Tool to convert UnicodeData.txt into a set of utype.h like methods that work on unicode
#

include_directories(../source)

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/../source/unicode_utype.cpp
	COMMAND unicodedata_to_utype  ${CMAKE_CURRENT_SOURCE_DIR}/../external/Unicode/UnicodeData.txt ${CMAKE_CURRENT_SOURCE_DIR}/../external/Unicode/PropList.txt > ${CMAKE_CURRENT_SOURCE_DIR}/../source/unicode_utype.cpp
	DEPENDS unicodedata_to_utype
	)

add_custom_target(generate_unicode_utype_cpp DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../source/unicode_utype.cpp)

#
# The tool that builds the unicode_utype cannot link against JASSlib because
# JASSlib contains the unicode_utype methods and this is a circular dependency.
# So we have to name the individual files we need rather than the library.
#
add_executable(unicodedata_to_utype
	unicodedata_to_utype.cpp
	../source/file.cpp
	../source/bitstring.cpp)

set_property(TARGET unicodedata_to_utype PROPERTY CXX_STANDARD 11)

