#!/bin/sh
cd ../../madym_binaries/public/ubuntu
docker build --build-arg MADYM_VERSION=$1 -t registry.gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/madym_release:$1 -t registry.gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/madym_release:latest .
docker push -a registry.gitlab.com/manchester_qbi/manchester_qbi_public/madym_cxx/madym_release