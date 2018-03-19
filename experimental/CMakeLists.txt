#
# CMAKELISTS.TXT
# --------------
#
# Build file for JASS experiments
# Copyright (c) 2016 Andrew Trotman
#
# Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
#

include_directories(../source)

#
# examine_index
#

add_executable(examine_index
	examine_index.cpp
	)

target_link_libraries(examine_index
	JASSlib
	)

#
# prm
#

set(PRN_FILES
	prm.cpp
	compress_integer_bitpack.h
	compress_integer_bitpack.cpp
	compress_integer_nybble_8.h
	compress_integer_nybble_8.cpp
	compress_integer_tayste_16.h
	compress_integer_tayste_16.cpp
	compress_integer_bitpack_32_reduced.h
	compress_integer_bitpack_32_reduced.cpp
	compress_integer_bitpack_64.h
	compress_integer_bitpack_64.cpp
	compress_integer_bitpack_128.h
	compress_integer_bitpack_128.cpp
	compress_integer_bitpack_256.h
	compress_integer_bitpack_256.cpp
	)

add_executable(prm
	${PRN_FILES}
	)

target_link_libraries(prm
	JASSlib
	)

source_group ("Source Files" FILES ${PRN_FILES})
