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
#        Tool to convert UnicodeData.txt into a set of ctype like methods that work on Unicode
#    ascii_database_to_c
#        Tool to generate a set of ctype like methods that work on ASCII and are locale free
#

include_directories(../source)

#
#	Explain how to make unicode.cpp
#
add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../source/unicode.cpp
	COMMAND unicode_database_to_c  ${CMAKE_CURRENT_SOURCE_DIR}/../external/Unicode/UnicodeData.txt ${CMAKE_CURRENT_SOURCE_DIR}/../external/Unicode/PropList.txt ${CMAKE_CURRENT_SOURCE_DIR}/../external/Unicode/CaseFolding.txt > ${CMAKE_CURRENT_BINARY_DIR}/../source/unicode.cpp
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
	../source/asserts.cpp
	../source/file.cpp
	../source/bitstring.cpp)


#
# Explain how to make ascii.cpp
#

add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../source/ascii.cpp
	COMMAND ascii_database_to_c > ${CMAKE_CURRENT_BINARY_DIR}/../source/ascii.cpp
	DEPENDS ascii_database_to_c
	)

add_custom_target(generate_ascii_cpp DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../source/ascii.cpp)

add_executable(ascii_database_to_c
	ascii_database_to_c.cpp
	)

#
# JASSv1_to_human
#

add_executable(JASSv1_to_human
	JASSv1_to_human.cpp
	../source/asserts.cpp
	../source/file.cpp)

#
# bin_to_human
#

add_executable(bin_to_human
	bin_to_human.cpp
	../source/asserts.cpp
	../source/file.cpp)

#
# test_integer_compress
#

add_executable(test_integer_compress
	test_integer_compress.cpp
	)

target_link_libraries(test_integer_compress
	JASSlib
	)

