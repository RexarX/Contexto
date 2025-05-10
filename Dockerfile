FROM ubuntu:22.04

# Avoid interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Set timezone to prevent tzdata interactive prompts
RUN ln -fs /usr/share/zoneinfo/UTC /etc/localtime

# Update and install basic packages
RUN apt-get update && apt-get install -y \
    ca-certificates \
    curl \
    gnupg \
    lsb-release \
    software-properties-common

# Configure repositories
RUN apt-get update && \
    apt-get install -y apt-transport-https && \
    add-apt-repository universe && \
    add-apt-repository multiverse && \
    apt-get update

# Install all userver dependencies as listed in docs for Ubuntu 22.04
RUN apt-get update && apt-get install -y \
    build-essential \
    ccache \
    clang-format \
    cmake \
    git \
    libabsl-dev \
    libbenchmark-dev \
    libboost-context1.74-dev \
    libboost-coroutine1.74-dev \
    libboost-filesystem1.74-dev \
    libboost-iostreams1.74-dev \
    libboost-locale1.74-dev \
    libboost-program-options1.74-dev \
    libboost-stacktrace1.74-dev \
    libboost1.74-dev \
    libbson-dev \
    libbz2-dev \
    libc-ares-dev \
    libcctz-dev \
    libcrypto++-dev \
    libcurl4-openssl-dev \
    libdouble-conversion-dev \
    libev-dev \
    libfmt-dev \
    libgflags-dev \
    libgmock-dev \
    libgrpc++-dev \
    libgrpc++1 \
    libgrpc-dev \
    libgtest-dev \
    libhiredis-dev \
    libicu-dev \
    libidn11-dev \
    libjemalloc-dev \
    libkrb5-dev \
    libldap2-dev \
    librdkafka-dev \
    libre2-dev \
    liblz4-dev \
    liblzma-dev \
    libmariadb-dev \
    libmongoc-dev \
    libnghttp2-dev \
    libpq-dev \
    libprotoc-dev \
    libpugixml-dev \
    libsnappy-dev \
    libsasl2-dev \
    libssl-dev \
    libxxhash-dev \
    libyaml-cpp-dev \
    libzstd-dev \
    netbase \
    ninja-build \
    postgresql-server-dev-14 \
    protobuf-compiler-grpc \
    python3-dev \
    python3-jinja2 \
    python3-protobuf \
    python3-venv \
    python3-voluptuous \
    python3-yaml \
    ragel \
    yasm \
    zlib1g-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install Node.js and npm for frontend development
RUN apt-get update && \
    apt-get install -y nodejs npm && \
    npm install -g n && \
    n stable && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/

# Refresh environment to use the newly installed Node.js version
ENV PATH="/usr/local/bin:${PATH}"
RUN hash -r

# Set working directory
WORKDIR /app

# Copy package.json files first (for better caching)
COPY frontend/package*.json /app/frontend/
RUN cd /app/frontend && npm install --no-progress

# Copy the project source code
COPY . /app/

# Initialize git submodules
RUN git config --global --add safe.directory /app && \
    git submodule update --init --recursive

# Skip the installation script since we've already installed the dependencies
RUN echo "#!/bin/bash\necho 'Dependencies already installed by Dockerfile'\nexit 0" > /app/install-deps.sh && \
    chmod +x /app/install-deps.sh

# Set up volumes for build directories
VOLUME ["/app/backend/build"]
VOLUME ["/app/frontend/node_modules"]

# Default build command
CMD cd /app/backend && ./scripts/build.sh --type Release --compiler gcc --build-system ninja --no-tests
