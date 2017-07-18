#
# CMAKELISTS.TXT
# --------------
#
# Build file for the JASS exmaples in the documentation.
# Copyright (c) 2016 Andrew Trotman
#
# Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
#

#
# build the parser example
#
include_directories(../source)
add_executable(parser_use parser_use.h parser_use.cpp)
target_link_libraries(parser_use JASSlib ${CMAKE_THREAD_LIBS_INIT})

