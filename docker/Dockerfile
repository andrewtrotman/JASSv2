FROM        gcc:latest
MAINTAINER  andrew@cs.otago.ac.nz

RUN         apt-get update  -y & apt-get clean & rm -rf /var/lib/apt/lists/*
RUN         apt-get install -y cmake valgrind vim --no-install-recommends

COPY	    build.sh /

ENTRYPOINT  /build.sh
