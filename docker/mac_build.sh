#!/bin/bash
set -ev

#
# We're already done the git clone and changed directories
#
#git clone https://github.com/andrewtrotman/JASSv2.git
#cd JASSv2
#cd build

export CFLAGS="-fprofile-instr-generate -fcoverage-mapping -g -O0"
export CXXFLAGS="-fprofile-instr-generate -fcoverage-mapping -g -O0"
cmake -D FORCE_CMAKE_BUILD_TYPE=Debug .. 
make

LLVM_PROFILE_FILE="coverage/unittest.profraw" ./unittest
xcrun llvm-profdata merge -sparse coverage/unittest.profraw -o coverage/unittest.profdata
xcrun llvm-cov show ./unittest -instr-profile=coverage/unittest.profdata > coverage.txt

export CODECOV_TOKEN="531ba3f8-2ebe-40a2-9f40-95ea61dc0186"
export ci_env=$(bash <(curl -s https://codecov.io/env))
/bin/bash <(curl -s https://codecov.io/bash) 

echo "Done"
