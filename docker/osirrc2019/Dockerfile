FROM        gcc:8

RUN         apt-get update -y \
&&          apt-get install -y cmake=3.7.2-1 vim=2:8.0.0197-4+deb9u1 --no-install-recommends \
&&          apt-get clean -y  \
&&          rm -rf /var/lib/apt/lists/*

# Copy scripts into place

COPY init init
COPY index index
COPY search search
COPY index_robust04 index_robust04
COPY search_robust04 search_robust04

# Set working directory

WORKDIR /work

