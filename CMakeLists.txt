#
# CMAKELISTS.TXT
# --------------
#
# Build file for JASS, including all the tools
# Copyright (c) 2016 Andrew Trotman
#
# Current supported build targets inclide:
#    make refresh_externals
#       This will re-download any external tools that are used as part of JASS.  This is usefule when,
#       for example, some external tool has been updated.  The initial use case was UnicodeData.txt the
#       data file that describes all the Unicode characters - which is used for the isalpha() and similar
#       methods.

cmake_minimum_required (VERSION 3.6.1)
project (JASS)

# Recurse into the "Hello" and "Demo" subdirectories. This does not actually
# cause another cmake executable to run. The same process will walk through
# the project's entire directory structure.

add_subdirectory (external)

