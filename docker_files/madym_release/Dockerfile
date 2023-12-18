#Pull the base image as Ubuntu
FROM ubuntu:20.04

ARG MADYM_VERSION

COPY ./madym_v$MADYM_VERSION.deb ./

ENV DEBIAN_FRONTEND noninteractive

#Install system dependencies and Madym
RUN apt update \
    && apt install -yq \
        git \
        python3-pip \
        python3-pyqt5 \
        ./madym_v$MADYM_VERSION.deb \
    && groupadd -g 1000 madymuser \
    && useradd -d /home/madymuser -s /bin/bash -m madymuser -u 1000 -g 1000 \
    && ln /usr/bin/python3 /usr/bin/python 
    
#Install Madym python wrappers    
RUN pip install /usr/madym-v$MADYM_VERSION/python

#Run Container as nonroot
USER madymuser

ENV HOME /home/madymuser
ENV MADYM_ROOT /usr/madym-v$MADYM_VERSION/bin
ENV PATH $PATH:$MADYM_ROOT

#Launch GUI on container start
CMD ["/usr/madym-v$MADYM_VERSION/bin/madym_gui"]