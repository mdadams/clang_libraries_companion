# This Dockerfile should work for Fedora 37, 38, and 39.

ARG release=39

FROM registry.fedoraproject.org/fedora:$release

RUN \
dnf -y install dnf-plugins-core

RUN \
dnf -y install \
  llvm-devel \
  clang-devel \
  gcc-c++ \
  cmake \
  make \
  ninja-build \
  rpm-build \
  dnf-utils \
  git \
  vim \
  wget \
  hostname \
  meld \
  python3 \
  python3-openstackclient \
  python3-jsonschema \
  python-unversioned-command \
  util-linux-user \
  jq \
  bc \
  which \
  libasan \
  liblsan \
  libtsan \
  libubsan \
  zlib-devel \
  libffi-devel \
  openssl-devel \
  boost-devel \
  boost-static

CMD /bin/bash
