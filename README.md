Travis (Linux / Mac build): [![Build Status](https://travis-ci.org/andrewtrotman/JASSv2.svg?branch=master)](https://travis-ci.org/andrewtrotman/JASSv2)

Appveyor (Windows build): [![Build status](https://ci.appveyor.com/api/projects/status/sbrjp3l39qf16h2x?svg=true)](https://ci.appveyor.com/project/andrewtrotman/jassv2-rjk8n)

Coverity Scan (static analysis): [![Coverity Scan Build Status](https://scan.coverity.com/projects/11022/badge.svg)](https://scan.coverity.com/projects/jassv2)

CodeCov (code coverage): [![codecov](https://codecov.io/gh/andrewtrotman/JASSv2/branch/master/graph/badge.svg)](https://codecov.io/gh/andrewtrotman/JASSv2)

CodeDocs.xyx (documentation): [![Documentation Status](https://codedocs.xyz/andrewtrotman/JASSv2.svg)](https://codedocs.xyz/andrewtrotman/JASSv2/)

# JASSv2

## Building JASSv2 (Linux/OSX)

The build requires the dependencies: CMake, doxygen

Then perform these steps:

1. cd build
2. cmake ..
3. make
4. ./unittest

## Building JASSv2 (Windows)

The build requires the dependencies: CMake, doxygen, Visual Studio 2015

Open a VS2015 x64 Native Tools CMD, then perform these steps:

1. cd build
2. cmake -G "NMake Makefiles" ..
3. nmake
4. unittest
