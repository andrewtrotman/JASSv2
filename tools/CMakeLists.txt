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
#        Tool to convert UnicodeData.txt into a set of ctype.h like methods that work on unicode
#

include_directories(../source)
add_executable(unicodedata_to_utype 
	unicodedata_to_utype.cpp)

target_link_libraries (unicodedata_to_utype LINK_PUBLIC JASSlib)

add_custom_command(
	OUTPUT $(CMAKE_CURRENT_SOURCE_DIR)/../source/unicode_ctype.cpp
	COMMAND unicodedata_to_utype  ${CMAKE_CURRENT_SOURCE_DIR}/../external/Unicode/UnicodeData.txt ${CMAKE_CURRENT_SOURCE_DIR}/../external/Unicode/PropList.txt > ${CMAKE_CURRENT_SOURCE_DIR}/../source/unicode_ctype.cpp
	DEPENDS unicode_to_utype
	)
