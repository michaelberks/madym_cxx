FROM ubuntu:latest

#Get local depends
COPY Qt Qt

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
       libboost-filesystem-dev \
       libboost-system-dev \
       libboost-program-options-dev \
       libboost-date-time-dev \
       libboost-test-dev \
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
       patchelf

#Remove what we don't need       
RUN rm -rf /var/lib/apt/lists/* 

#Add cqtdeployer
COPY cqtdeployer cqtdeployer
RUN ln -s /cqtdeployer/cqtdeployer.sh /usr/bin/cqtdeployer \
    && patchelf --set-rpath /Qt/5.15.2/gcc_64/lib /Qt/5.15.2/gcc_64/lib/libicuuc.so.56 \
    && patchelf --set-rpath /Qt/5.15.2/gcc_64/lib /Qt/5.15.2/gcc_64/lib/libicudata.so.56 \
    && patchelf --set-rpath /Qt/5.15.2/gcc_64/lib /Qt/5.15.2/gcc_64/lib/libicui18n.so.56 \
    && patchelf --set-rpath /Qt/5.15.2/gcc_64/lib /cqtdeployer/lib/libicuuc.so.56 \
    && patchelf --set-rpath /Qt/5.15.2/gcc_64/lib /cqtdeployer/lib/libicudata.so.56 \
    && patchelf --set-rpath /Qt/5.15.2/gcc_64/lib /cqtdeployer/lib/libicui18n.so.56
    
ENV Qt5Core_DIR /Qt/5.15.2/gcc_64/lib/cmake/Qt5core
ENV Qt5Gui_DIR /Qt/5.15.2/gcc_64/lib/cmake/Qt5Gui
ENV Qt5Widgets_DIR /Qt/5.15.2/gcc_64/lib/cmake/Qt5Widgets
ENV CC /usr/bin/gcc
ENV CXX /usr/bin/g++    

