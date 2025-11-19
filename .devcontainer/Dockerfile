# syntax=docker/dockerfile:1.20.0
ARG DEV_FROM_STAGE=builder
ARG EXPORT_FROM_STAGE=compiler
ARG OVERLAY_WS=/opt/slicer_ws
ARG SHIP_FROM_STAGE=runner
ARG WS_CACHE_ID=slicer

ARG FROM_IMAGE=base
# Stage from full image tag name for dependabot detection
FROM ubuntu:noble-20251013 AS base

################################################################################
# MARK: baser - setup base image using snapshots
################################################################################
FROM $FROM_IMAGE AS baser

# install setup packages
RUN apt-get update && \
  apt-get install -y --no-install-recommends \
    ca-certificates \
    curl \
    gnupg2 \
    locales \
    tzdata \
  && rm -rf /var/lib/apt/lists/*

# configure ubuntu snapshot
RUN UBUNTU_DEB_SNAPSHOT=$(date -r /var/lib/dpkg/info +%Y%m%dT%H%M%SZ) && \
  echo "APT::Snapshot \"${UBUNTU_DEB_SNAPSHOT}\";" \
  | tee /etc/apt/apt.conf.d/50snapshot

# update package cache
RUN cat <<EOF > /etc/apt/apt.conf.d/docker-clean && apt-get update && echo v1
Binary::apt::APT::Keep-Downloaded-Packages "true";
APT::Install-Recommends "false";
APT::Install-Suggests "false";
EOF

# setup environment
ENV LANG=C.UTF-8
ENV LC_ALL=C.UTF-8

# configure workspace
ARG OVERLAY_WS
ENV OVERLAY_WS=$OVERLAY_WS
WORKDIR $OVERLAY_WS

# install bootstrap tools
RUN --mount=type=cache,sharing=locked,target=/var/cache/apt \
  apt-get install -y \
    gettext-base \
    sudo \
    unzip \
    wget \
    zstd

# add default user for devcontainer
ENV DEV_USER=ubuntu
RUN echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

################################################################################
# MARK: cacher - cache source and dependency instructions
################################################################################
FROM baser AS cacher

# copy overlay source
COPY ./ ./src/Slicer

################################################################################
# MARK: runner - setup runtime dependencies for deployment
################################################################################
FROM baser AS runner

# install runtime dependencies
RUN --mount=type=cache,sharing=locked,target=/var/cache/apt \
  apt-get install -y \
    libasound2t64 \
    libecore-x1 \
    libglu1-mesa \
    libnss3 \
    libpulse-mainloop-glib0 \
    qt5dxcb-plugin

################################################################################
# MARK: tester - setup test dependencies for validation
################################################################################
FROM runner AS tester

# RUN --mount=type=cache,sharing=locked,target=/var/cache/apt \
#   apt-get install -y \
#     TODO

################################################################################
# MARK: builder - setup build dependencies for compilation
################################################################################
FROM tester AS builder

# install build tools
RUN --mount=type=cache,sharing=locked,target=/var/cache/apt \
  apt-get install -y \
    build-essential \
    ccache \
    cmake \
    git \
    git-lfs

# install build dependencies
RUN --mount=type=cache,sharing=locked,target=/var/cache/apt \
  apt-get install -y \
    libqt5opengl5-dev \
    libqt5svg5-dev \
    libqt5x11extras5-dev \
    libqt5xmlpatterns5-dev \
    libxt-dev \
    qtbase5-private-dev \
    qtmultimedia5-dev \
    qttools5-dev \
    qtwebengine5-dev

# setup default ccache configuration
ENV CCACHE_DIR="$OVERLAY_WS/.ccache"
ENV CCACHE_TEMPDIR="$CCACHE_DIR/tmp"
ENV CCACHE_CONFIGPATH="$OVERLAY_WS/src/Slicer/.devcontainer/config/.ccache/ccache.conf"

# capture environment to modify layer digest
RUN env > /tmp/env.txt

################################################################################
# MARK: dever - setup user account for development
################################################################################
FROM $DEV_FROM_STAGE AS dever

# install development tools
RUN --mount=type=cache,sharing=locked,target=/var/cache/apt \
  apt-get install -y \
    # Debug utils
    gdb \
    hotspot \
    python3-ipdb \
    valgrind \
    # Editors utils
    micro \
    nano \
    # Graphic utils
    mesa-utils \
    # Profiling utils
    btop \
    htop \
    iftop \
    lsof \
    ncdu \
    nethogs \
    usbtop \
    # Shell utils
    bash-completion \
    byobu \
    fish \
    ssh \
    tmux \
    # Source utils
    gh \
    pre-commit

# Symlink for docker outside of docker
RUN ln -s /var/run/docker-host.sock /var/run/docker.sock

################################################################################
# MARK: seeder - seed workspace artifacts for caching
################################################################################
FROM alpine AS seeder
ARG OVERLAY_WS
ARG WS_CACHE_ID

ARG CLEAR_WS_CACHE
RUN --mount=type=cache,id=$WS_CACHE_ID,sharing=private,target=$OVERLAY_WS \
  if [ -n "$CLEAR_WS_CACHE" ]; then \
    echo "Clearing cache!" && \
    rm -rf $OVERLAY_WS/* && \
    echo "Cache cleared!"; \
  fi

ARG SEED_WS_CACHE
RUN --mount=type=bind,source=./,target=/seeder/\
    --mount=type=cache,id=$WS_CACHE_ID,sharing=private,target=$OVERLAY_WS \
  if [ -n "$SEED_WS_CACHE" ]; then \
    echo "Seeding cache!" && \
    cp -rT /seeder/cache/$OVERLAY_WS $OVERLAY_WS && \
    echo "Cache seeded!"; \
  fi

RUN echo $(date) > /tmp/seeder_stamp.txt

################################################################################
# MARK: compiler - compile workspace artifacts for deployment
################################################################################
FROM builder AS compiler
ARG WS_CACHE_ID

# build overlay source
ARG BUST_BUILD_CACHE
ARG CMAKE_BUILD_TYPE=Release
RUN --mount=type=cache,from=cacher,target=/cacher \
    --mount=type=cache,from=seeder,target=/seeder \
    --mount=type=cache,id=$WS_CACHE_ID,sharing=private,target=$OVERLAY_WS \
  ln -s /cacher/$OVERLAY_WS/src src && \
  mkdir build/Slicer-SuperBuild && \
  cd build/Slicer-SuperBuild && \
  cmake \
    -DCMAKE_BUILD_TYPE:STRING=$CMAKE_BUILD_TYPE \
    -DCMAKE_CXX_COMPILER:STRING=/usr/bin/ccache \
    -DCMAKE_C_COMPILER:STRING=/usr/bin/ccache \
    ../src/Slicer && \
  make -j$(nproc) \
    || true && \
  echo $(date) > /tmp/compiler_stamp.txt

ARG FAIL_ON_BUILD_FAILURE=1
RUN --mount=type=cache,from=cacher,target=/cacher \
    --mount=type=cache,id=$WS_CACHE_ID,sharing=private,target=$OVERLAY_WS \
  echo $(date) > /tmp/compiler_stamp.txt && \
  make -j$(nproc) \
    || ([ -z "$FAIL_ON_BUILD_FAILURE" ] || exit 1)

################################################################################
# MARK: validator - validate workspace artifacts for development
################################################################################
FROM tester AS validator
ARG WS_CACHE_ID

# test overlay build
ARG BUST_TEST_CACHE
ARG FAIL_ON_TEST_FAILURE=1
RUN --mount=type=cache,from=cacher,target=/cacher \
    --mount=type=cache,from=compiler,target=/compiler \
    --mount=type=cache,id=$WS_CACHE_ID,sharing=private,target=$OVERLAY_WS \
  echo $(date) > /tmp/validator_stamp.txt && \
  make test \
  || ([ -z "$FAIL_ON_TEST_FAILURE" ] || exit 1)

################################################################################
# MARK: dancer - multi-stage for caches dancing
################################################################################
FROM $EXPORT_FROM_STAGE AS dancer
ARG WS_CACHE_ID

RUN --mount=type=cache,id=$WS_CACHE_ID,sharing=private,target=$OVERLAY_WS,readonly \
    echo "Exporting cache!" && \
    mkdir -p /dancer/$OVERLAY_WS && \
    cp -rT $OVERLAY_WS /dancer/$OVERLAY_WS && \
    echo "Cache exported!"

################################################################################
# MARK: exporter - multi-stage for exporting caches
################################################################################
FROM $EXPORT_FROM_STAGE AS exporter

COPY --link --from=dancer /dancer/$OVERLAY_WS $OVERLAY_WS

################################################################################
# MARK: shipper - setup production images using shared instructions
################################################################################
FROM $SHIP_FROM_STAGE AS shipper

# TODO

################################################################################
# MARK: debugger - stage target for debuggin in production
################################################################################
FROM shipper AS debugger

COPY --link --from=dancer /dancer/$OVERLAY_WS/build $OVERLAY_WS/build

################################################################################
# MARK: releaser - stage target for releasing in production
################################################################################
FROM shipper AS releaser

COPY --link --from=dancer /dancer/$OVERLAY_WS/build $OVERLAY_WS/build
