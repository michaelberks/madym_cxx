FROM gcc

RUN apt-get update \
    && apt-get install --yes --no-install-recommends \
       wget cmake build-essential \
       libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*
