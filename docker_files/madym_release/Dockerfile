#Pull the base image as Ubuntu
FROM ubuntu:22.10

ARG MADYM_VERSION

COPY ./madym_v$MADYM_VERSION.deb ./

ENV DEBIAN_FRONTEND noninteractive

RUN ls -full && apt update \
    && apt install -yq \
        git \
        python3-pip \
        ./madym_v$MADYM_VERSION.deb \
    && groupadd -g 1000 madymuser \
    && useradd -d /home/madymuser -s /bin/bash -m madymuser -u 1000 -g 1000 \
    && pip install /usr/madym-v$MADYM_VERSION/python \
    && ln /usr/bin/python3 /usr/bin/python

#Run Container as nonroot
USER madymuser

ENV HOME /home/madymuser
ENV MADYM_ROOT /usr/madym-v$MADYM_VERSION/bin
ENV PATH $PATH:$MADYM_ROOT

#Launch GUI on container start
CMD ["/usr/madym-v$MADYM_VERSION/bin/madym_gui"]
