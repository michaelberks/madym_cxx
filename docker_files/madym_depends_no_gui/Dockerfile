FROM ubuntu:22.10

#Install downloaded depends
ENV DEBIAN_FRONTEND noninteractive
RUN apt update \
    && apt install -y \
       git \
       gcc \
       mesa-common-dev \
       wget \
       cmake \
       build-essential \
       doxygen \
       zlib1g \
       libdcmtk-dev \
       libglu1-mesa-dev \
       libgtk-3-dev \
       libdbus-1-dev \
       libxkbcommon-x11-dev \
       libxcb-icccm4-dev \
       libxcb-image0-dev \
       libxcb-keysyms1-dev \
       libxcb-render-util0-dev \
       libxcb-shape0-dev \
       libxcb-util-dev \
       libxcb-xinerama0-dev \
       patchelf \
       libxcb-randr0 \
       file

#Build boost to get recent version
RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.82.0/source/boost_1_82_0.tar.gz \
    && tar -xf boost_1_82_0.tar.gz \
    && cd boost_1_82_0 \
    && ./bootstrap.sh \
    && ./b2 address-model=64 --with-filesystem --with-system --with-program_options --with-test --with-date_time \
    && ./b2 install

#Remove what we don't need       
RUN rm -rf /var/lib/apt/lists/* 

#Add gcc environment variables used in CMake build command
ENV CC /usr/bin/gcc
ENV CXX /usr/bin/g++    

