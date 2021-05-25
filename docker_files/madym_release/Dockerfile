#Pull the base image as Ubuntu
FROM madym_depends:latest

RUN groupadd -g 1000 madymuser \
    && useradd -d /home/madymuser -s /bin/bash -m madymuser -u 1000 -g 1000
    
RUN git clone https://gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx.git \
    && mkdir madym_build \
    && cd madym_build \
    && cmake ../madym_cxx/ -DCMAKE_BUILD_TYPE=Release \
	-D BUILD_DOCUMENTATION=OFF \
	-D BUILD_INSTALL=OFF \
	-D BUILD_QT_GUI=ON \
	-D BUILD_SHARED_LIBS=ON \
	-D BUILD_TESTING=ON \
	-D BUILD_TOOLS=ON \
	-D BUILD_WITH_DCMTK=ON \
	-D BUILD_WITH_ZLIB=ON \
    && make all \
    && ctest

#Run Container as nonroot
USER madymuser

ENV HOME /home/madymuser
ENV MADYM_ROOT /madym_build/bin

#Start command in bash terminal
CMD ["/bin/bash"]
