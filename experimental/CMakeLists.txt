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
	)

add_executable(prm
	${PRN_FILES}
	)

target_link_libraries(prm
	JASSlib
	)

source_group ("Source Files" FILES ${PRN_FILES})
