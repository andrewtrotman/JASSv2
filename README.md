# JASSv2
The JASS experimental search engine is still in development.  Please see the [wiki](../../wiki) for details.

Slack: <a href=https://join.slack.com/t/jassv2/shared_invite/MjE3NzUwNjA2ODM2LTE1MDEwMzQ0NjItMTdiMWFjMjYyNg><img src="https://img.shields.io/badge/slack-talk-brightgreen.svg?style=flatulance" alt="Slack"></a>

Travis (Linux / Mac build): [![Build Status](https://travis-ci.org/andrewtrotman/JASSv2.svg?branch=master)](https://travis-ci.org/andrewtrotman/JASSv2)

Appveyor (Windows build): [![Build status](https://ci.appveyor.com/api/projects/status/sbrjp3l39qf16h2x?svg=true)](https://ci.appveyor.com/project/andrewtrotman/jassv2-rjk8n)

Git Actions (Linux build): [![Build status](https://github.com/andrewtrotman/JASSv2/workflows/Build/badge.svg)](https://github.com/andrewtrotman/JASSv2/actions)

Coverity Scan (static analysis): [![Coverity Scan Build Status](https://scan.coverity.com/projects/11022/badge.svg)](https://scan.coverity.com/projects/jassv2)

Codacy (static analysis): [![Codacy Badge](https://api.codacy.com/project/badge/Grade/01c46a3d10e440f9b54a237889f8326b)](https://www.codacy.com/app/andrewtrotman/JASSv2?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=andrewtrotman/JASSv2&amp;utm_campaign=Badge_Grade)

CodeCov (code coverage): [![codecov](https://codecov.io/gh/andrewtrotman/JASSv2/branch/master/graph/badge.svg)](https://codecov.io/gh/andrewtrotman/JASSv2)

CodeDocs.xyz (documentation): [![Documentation Status](https://codedocs.xyz/andrewtrotman/JASSv2.svg)](https://codedocs.xyz/andrewtrotman/JASSv2/)

OLD:
For XCode use:
cmake -G Xcode -T buildsystem=1 ..

NEW:
For XCode use:
cmake -G Xcode -T buildsystem=1 ..


## Python ##

There are two ways pyjass can be installed

### conda ####

[Anaconda](https://docs.anaconda.com/anaconda/install/index.html) ensures that all JASS dependencies are installed and creates a virtual envirnoment by the name of pyjass

	conda env create -f env.yml
    conda activate pyjass

### pip ####

	pip3 install --user pyjass

## Java ##
The Java version is build in the same way, but run with 

	java JASSjr_index <filename>

and

	java JASSjr_search
