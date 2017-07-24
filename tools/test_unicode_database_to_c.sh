#!/bin/bash
#
# TEST_UNICODE_DATABASE_TO_C.SH
# -----------------------------
# Copyright (c) 2016 Andrew Trotman
# Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
#
# Build a version of the unicode C methods from version 6.3 of the Unicode database and compare to what the D
# Language standard libraries (Phobos) claim is the correct answer (which themselves are wrong!)
#
# It is assumes that:
#	This program is running in a Unix-like shell (written under Mac OS X)
#	g++ (the C++ compiler) is in the path
#	dmd (the D compiler) is in the path
#
../build/tools/Debug/unicode_database_to_c ../external/Unicode/UnicodeData_v6_3.txt  ../external/Unicode/PropList_v6_3.txt  ../external/Unicode/CaseFolding_v6_3.txt> unicode_v6_3.cpp
g++ -c unicode_v6_3.cpp -o unicode_v6_3.o
dmd ../tools/cmp_d_to_utype.d unicode_v6_3.o
./cmp_d_to_utype > test_unicode_database_to_c_with_d.txt
rm unicode_v6_3.cpp
rm unicode_v6_3.o
rm cmp_d_to_utype.o
rm cmp_d_to_utype

