#!/bin/sh
#
# COVERITY_SCAN.SH
# ----------------
#
# Build file for JASS, including all the tools
# Copyright (c) 2016 Andrew Trotman
#
# Released under the 2-clause BSD license (See:https://en.wikipedia.org/wiki/BSD_licenses)
#
# Move the latest version of the source code to the coverity_scan branch so that Coverity Scan is run on github checkin.
#
git checkout coverity_scan
git merge master
git push
git checkout master

