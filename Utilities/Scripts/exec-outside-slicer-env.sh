#!/usr/bin/env bash

# -----------------------------------------------------------------------------
# Executes a system binary (e.g., git) from within a Slicer environment,
# restoring the original environment if the script is run inside a launcher-
# initialized context (detected via the APPLAUNCHER_LEVEL environment variable).
#
# Usage:
#   exec-outside-slicer-env.sh <binary_name> [args...]
#
# This script is intended to be invoked by wrapper scripts (e.g., `git`) that
# forward the name of the system binary.
# -----------------------------------------------------------------------------

set -euo pipefail

err() { echo -e >&2 "ERROR: $*\n"; }
warn() { echo -e >&2 "WARNING: $*\n"; }
die() { err "$@"; exit 1; }

if [[ $# -lt 1 ]]; then
  die "Usage: $0 <binary_name> [args...]"
fi

binary_name="$1"
shift 1

if [[ "$binary_name" == "system-executable-wrapper.sh.in" ]]; then
  die "This wrapper template ('$binary_name') must be installed and renamed to a system binary (e.g., 'git'). It should never be invoked directly."
fi

# Record initial location of the binary before environment restoration
pre_env_executable="$(command -v "$binary_name" || true)"

# Restore original environment only if inside launcher context
if [[ -n "${APPLAUNCHER_LEVEL:-}" ]]; then

  # 1. Unset all current env variables except APPLAUNCHER_*
  declare -a unset_commands=()
  while IFS= read -r line; do
    # Match exported variables in format: declare -x VAR="value" or declare -x "VAR=value"
    [[ "$line" =~ ^declare\ -x\ \"?[A-Za-z_][A-Za-z0-9_]*= ]] || continue
    var_name="${line#declare -x }"
    var_name="${var_name%%=*}"
    var_name="${var_name#\"}"
    var_name="${var_name%\"}"
    unset_commands+=("unset \"$var_name\"")
  done < <(export | grep -v "\"APPLAUNCHER_")

  # 2. Collect `declare -x "VAR=VALUE"` lines from saved environment
  # Transform: declare -x "APPLAUNCHER_0_VAR=val" -> declare -x "VAR=val"
  declare -a restore_commands=()
  while IFS= read -r declare_line; do
    restore_commands+=("$declare_line")
  done < <(export | grep -E '^declare -x \"?APPLAUNCHER_0_' | sed 's/APPLAUNCHER_0_//')

  # Apply unset commands to clean current environment
  for cmd in "${unset_commands[@]}"; do
    eval "$cmd"
  done

  # Apply restore commands from saved environment
  for cmd in "${restore_commands[@]}"; do
    eval "$cmd"
  done
fi

# Recompute system binary location with clean environment
system_executable="$(command -v "$binary_name" || true)"

# Ensure binary was found
if [[ -z "$system_executable" ]]; then
  die "Could not locate system binary: '$binary_name'. Check that it is installed and in PATH."
fi

# Final check (only error if a real recursive install path is detected)
this_wrapper_executable="$(readlink -f "$pre_env_executable" || true)"
resolved_executable="$(readlink -f "$system_executable" || true)"
if [[ -n "$resolved_executable" && "$resolved_executable" == "$this_wrapper_executable" ]]; then
  if [[ "$this_wrapper_executable" == */Slicer-build/bin/* ]]; then
    die "$binary_name wrapper resolved to itself. Ensure system $binary_name is installed and found before the Slicer bin/ directory in PATH."
  fi
fi

# Detect recursion if the wrapper itself is shadowing the real binary
wrapper_exe="$(readlink -f "$pre_env_executable" || true)"
resolved_exe="$(readlink -f "$system_executable" || true)"

if [[ "$resolved_exe" == "$wrapper_exe" && "$wrapper_exe" == */Slicer-build/bin/* ]]; then
  die "'$binary_name' wrapper resolved to itself. Ensure system $binary_name is found before Slicer bin/ in PATH."
fi

# Execute system binary with forwarded arguments
exec "$system_executable" "$@"
