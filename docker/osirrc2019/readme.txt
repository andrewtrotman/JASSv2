# build the docker image:

docker build . -t andrewtrotman/osirrc2019

# make sure the docker image build by checking for an image called andrewtrotman/osirrc2019 when you go:

docker images

# run an interactive command in the image

docker run -ti andrewtrotman/osirrc2019 bash

# The test suite init and index command

python run.py prepare --repo andrewtrotman/osirrc2019 --tag latest --collection_name robust04 --collection_path /Users/andrew/programming/JASSv2/docker/osirrc2019/robust04


# search command

python3 run.py search --repo andrewtrotman/osirrc2019 --topic topics.robust04.301-450.601-700.txt --output /Users/andrew/programming/JASSv2/docker/osirrc2019/output --qrels qrels/qrels.robust2004.txt
