# build the docker image:

docker build . -t andrewtrotman/osirrc2019

# make sure the docker image build by checking for an image called andrewtrotman/osirrc2019 when you go:

docker images


# run an interactive command in the image

docker run -ti andrewtrotman/osirrc2019 bash


python run.py prepare --repo rclancy/anserini-test --tag latest --collection_name robust04 --collection_path /path/to/disk45
