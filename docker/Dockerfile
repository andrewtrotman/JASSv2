FROM        gcc:8

RUN         apt-get update -y \
&&          apt-get install -y apt-utils --no-install-recommends \
&&          apt-get install -y cmake valgrind vim python-pip  --no-install-recommends \
&&          apt-get clean -y  \
&&          rm -rf /var/lib/apt/lists/*

COPY	    build.sh /

ENTRYPOINT  /build.sh
