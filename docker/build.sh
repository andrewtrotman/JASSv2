#!/bin/bash
set -ev

git clone https://github.com/andrewtrotman/JASSv2.git
cd JASSv2
cd build
export CFLAGS="-g -O0 -fprofile-arcs -ftest-coverage -fno-inline"
export CXXFLAGS="-g -O0 -fprofile-arcs -ftest-coverage -fno-inline -fno-elide-constructors"
cmake -D FORCE_CMAKE_BUILD_TYPE=Debug .. 
make
./unittest
#
# Coverage using codecov.io
#
export CODECOV_TOKEN="531ba3f8-2ebe-40a2-9f40-95ea61dc0186"
export ci_env=$(bash <(curl -s https://codecov.io/env))
/bin/bash <(curl -s https://codecov.io/bash) > ers 2> ers2

#
# Coverage using coveralls.io
#
# pip install --upgrade setuptools
# pip install wheel
# pip install cpp-coveralls

# export COVERALLS_REPO_TOKEN=D5tNCS2tc1XK8guPC5ePloTuxr7ifkzlV
# coveralls ---root . -gcov-options '\-lp' 
# coveralls > ers 2> ers2

#valgrind -q --error-exitcode=42 ./unittest
exit 0
