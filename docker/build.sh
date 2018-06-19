#!/bin/bash
git clone https://github.com/andrewtrotman/JASSv2.git
cd JASSv2
cd build
CFLAGS="-g -O0 -fprofile-arcs -ftest-coverage"
CXXFLAGS="-g -O0 -fprofile-arcs -ftest-coverage"
cmake -D FORCE_CMAKE_BUILD_TYPE=Debug .. 
make
valgrind -q --error-exitcode=42 ./unittest
ci_env=`bash <(curl -s https://codecov.io/env)`
/bin/bash <(curl -s https://codecov.io/bash)

