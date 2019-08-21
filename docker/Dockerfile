FROM        gcc:8

RUN         apt-get update -y \
&&          apt-get install -y cmake=3.7.2-1 valgrind=1:3.12.0~svn20160714-1+b1 vim --no-install-recommends \
&&          apt-get clean -y  \
&&          rm -rf /var/lib/apt/lists/*

COPY	    build.sh /

ENTRYPOINT  /build.sh
