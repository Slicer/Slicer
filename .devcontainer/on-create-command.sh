#!/usr/bin/env bash

# Immediately catch all errors
set -eo pipefail

# Uncomment for debugging
# set -x
# env

# Generate locales for git shell
sudo locale-gen $LANG

# Set git config for workspace
git config --global --add safe.directory "*"

# NOTE: This is slow if not using a mounted volumes, 
# i.e. using workspace from the docker image directly,
# presumably due to docker overlayfs driver overhead.
# If needing to use workspace pre-baked into base image,
# consider using a new volume to be auto populated with
# the workspace pre-baked in image via devcontainer tools.
# Either by deleting old volume from the docker engine
# Or simpler changing volume name in devcontainer.json
sudo chown -R :ubuntu $OVERLAY_WS
# Recursively update group permissions for workspace
# to allow write access via dev users current group
sudo chmod -R g+rwx $OVERLAY_WS

# Recursively update permissions for user home
# to allow write access such as config dotfiles
sudo chown -R $(id -u):$(id -g) $HOME

# Recursively update group permissions for slicer cache
sudo chown -R :ubuntu /opt/slicer/slicer.org
sudo chmod -R g+rwx /opt/slicer/slicer.org
