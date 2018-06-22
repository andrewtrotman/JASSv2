#!/bin/bash
git clone https://github.com/andrewtrotman/JASSv2.git
cd JASSv2
cd build
export CFLAGS="-g -O0 -fprofile-arcs -ftest-coverage"
export CXXFLAGS="-g -O0 -fprofile-arcs -ftest-coverage"
cmake -D FORCE_CMAKE_BUILD_TYPE=Debug .. 
make
valgrind -q --error-exitcode=42 ./unittest
export CODECOV_TOKEN="531ba3f8-2ebe-40a2-9f40-95ea61dc0186"
export ci_env=`bash <(curl -s https://codecov.io/env)`
/bin/bash <(curl -s https://codecov.io/bash) > ers

