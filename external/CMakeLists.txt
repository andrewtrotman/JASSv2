#
# CMAKELISTS.TXT
# --------------
#
# Build file for JASS external tools.
# Copyright (c) 2016 Andrew Trotman
#
# Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
#
# Current tools include:
#    UnicodeData.txt from the Unicode Consortium (http://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt)
#

add_custom_target(refresh_externals)

#
# get the latest version of the Unicode data file "UnicodeData.txt" from the Univode web site
#

add_custom_command(
	TARGET refresh_externals PRE_BUILD
	COMMAND curl http://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt > UnicodeData.txt
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/Unicode
	)

add_custom_command(
	TARGET refresh_externals PRE_BUILD
	COMMAND curl http://www.unicode.org/Public/UCD/latest/ucd/PropList.txt > PropList.txt
	WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/Unicode
	)

