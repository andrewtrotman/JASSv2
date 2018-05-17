FROM        gcc:latest
MAINTAINER  Andrew

RUN         apt-get update  -y
RUN         apt-get upgrade -y
RUN         apt-get install -y cmake valgrind vim

COPY	    build.sh /

ENTRYPOINT  /build.sh
