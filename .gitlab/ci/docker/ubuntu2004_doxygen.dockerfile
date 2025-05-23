##============================================================================
##  The contents of this file are covered by the Viskores license. See
##  LICENSE.txt for details.
##
##  By contributing to this file, all contributors agree to the Developer
##  Certificate of Origin Version 1.1 (DCO 1.1) as stated in DCO.txt.
##============================================================================

##=============================================================================
##
##  Copyright (c) Kitware, Inc.
##  All rights reserved.
##  See LICENSE.txt for details.
##
##  This software is distributed WITHOUT ANY WARRANTY; without even
##  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
##  PURPOSE.  See the above copyright notice for more information.
##
##=============================================================================

FROM docker.io/ubuntu:20.04
LABEL maintainer "Vicente Adolfo Bolea Sanchez<vicente.bolea@gmail.com>"

ENV TZ=America/New_York

# Base dependencies for building VTK-m projects
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
      cmake \
      curl \
      doxygen \
      g++ \
      git \
      git-lfs \
      libmpich-dev \
      libomp-dev \
      libtbb-dev \
      mpich \
      ninja-build \
      python3-pip \
      rsync \
      software-properties-common \
      ssh

RUN python3 -m pip install \
      breathe \
      sphinx \
      sphinxcontrib-moderncmakedomain \
      sphinxcontrib-packages \
      sphinx-rtd-theme

# Need to run git-lfs install manually on ubuntu based images when using the
# system packaged version
RUN git-lfs install

# Provide CMake 3.17 so we can re-run tests easily
# This will be used when we run just the tests
RUN mkdir /opt/cmake/ && \
    curl -L https://github.com/Kitware/CMake/releases/download/v3.17.3/cmake-3.17.3-Linux-x86_64.sh > cmake-3.17.3-Linux-x86_64.sh && \
    sh cmake-3.17.3-Linux-x86_64.sh --prefix=/opt/cmake/ --exclude-subdir --skip-license && \
    rm cmake-3.17.3-Linux-x86_64.sh && \
    ln -s /opt/cmake/bin/ctest /opt/cmake/bin/ctest-latest

ENV PATH "${PATH}:/opt/cmake/bin"
