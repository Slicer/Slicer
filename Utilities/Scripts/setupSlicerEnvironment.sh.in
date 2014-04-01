#!/usr/bin/env bash

if [ -z "$SLICER_HOME" ]; then
  echo "Environment variable SLICER_HOME is not defined" >&2
  exit 1
fi

cat <<EOF >&2
This script is deprecated. Instead, please run:
  eval \$($SLICER_HOME/Slicer --launcher-show-set-environment-commands)
EOF

eval $($SLICER_HOME/Slicer --launcher-show-set-environment-commands)
