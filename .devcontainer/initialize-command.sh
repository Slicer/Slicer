#!/usr/bin/env bash

# Immediately catch all errors
set -eo pipefail

# Uncomment for debugging
# set -x
# env

# Use first argument as target name
target=$1

# Intialize directories for personalized bind mounts
mkdir -p /var/tmp/home/slicer

################################################################################
# MARK: Build image - bake image using local Dockerfile for local dev container
# unset DEV_FROM_STAGE
################################################################################

# Bake the target and export locally to static tag
docker buildx bake --load \
    --set $target.tags=slicer:devcontainer \
    $target
