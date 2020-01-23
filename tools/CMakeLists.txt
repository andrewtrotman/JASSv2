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

add_executable(ascii_database_to_c ascii_database_to_c.cpp)

#
# JASSv1_to_human
#

add_executable(JASSv1_to_human JASSv1_to_human.cpp)
target_link_libraries(JASSv1_to_human JASSlib ${CMAKE_THREAD_LIBS_INIT})

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

add_executable(test_integer_compress test_integer_compress.cpp)
target_link_libraries(test_integer_compress JASSlib ${CMAKE_THREAD_LIBS_INIT})

#
# test_integer_compress_average
#

add_executable(test_integer_compress_average test_integer_compress_average.cpp)
target_link_libraries(test_integer_compress_average JASSlib)


#
# ciff_to_JASS: turn Jimmy Lin's common index format protobuf formatted index into a JASSv1 index
#

set(CIFF_TO_JASS_FILES
protobuf.h
ciff_lin.h
ciff_to_JASS.cpp
)

add_executable(ciff_to_JASS ${CIFF_TO_JASS_FILES})

target_link_libraries(ciff_to_JASS JASSlib ${ZLIB_STATIC_LIB} ${ZSTD_STATIC_LIB} ${CMAKE_THREAD_LIBS_INIT})
source_group ("Source Files" FILES ${CIFF_TO_JASS_FILES})

#
# JASS_eval: Just like trec_eval, but the JASS version that includes different metrics
#
add_executable(JASS_eval JASS_eval.cpp) 
target_link_libraries(JASS_eval JASSlib)

#
# ECOM19_to_run: Convert the ECOM19 ebay data challenge run into a trec_eval compatible run
#
add_executable(ECOM19_to_run ECOM19_to_run.cpp)
target_link_libraries(ECOM19_to_run JASSlib)

#
# ECOM19_to_JASS_eval: Convert the ECOM19 ebay data challenge data into JASS_eval format (TREC eval format with prices added)
#
add_executable(ECOM19_to_JASS_eval ECOM19_to_JASS_eval.cpp)
target_link_libraries(ECOM19_to_JASS_eval JASSlib)

#
# trec_to_query: Covert a TREC topic file into a JASS query file
#
add_executable(trec_to_query trec_to_query.cpp)
target_link_libraries(trec_to_query JASSlib ${ZLIB_STATIC_LIB} ${ZSTD_STATIC_LIB} ${CMAKE_THREAD_LIBS_INIT})
