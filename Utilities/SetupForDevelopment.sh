#!/usr/bin/env bash

# Run this script to set up development with git.
# For more information, see:
# https://www.slicer.org/w/index.php/Documentation/Labs/DevelopmentWithGit
#

printErrorAndExit() {
  echo 'Failure during git development setup' 1>&2
  echo '------------------------------------' 1>&2
  echo '' 1>&2
  echo "$@" 1>&2
  exit 1
}

# Make sure we are inside the repository.
cd "$(echo "$0" | sed 's/[^/]*$//')"/..

if test -d .git/.git; then
  printErrorAndExit "The directory '.git/.git' exists, indicating a
  configuration error.

Please 'rm -rf' this directory."
fi

# Make 'git pull' on main always use rebase.
git config branch.main.rebase true

# Make automatic and manual rebase operations to display a summary and stat
# display of changes merged in the fast-forward operation.
git config rebase.stat true

cd Utilities/Scripts

echo "Checking basic user information..."
./SetupUser.sh || exit 1
echo

echo "Setting up git hooks..."
./SetupHooks.sh || exit 1
echo

echo "Suggesting git tips..."
./GitTips.sh || exit 1
echo

# Record the setup version to notify developers of script and hook modifications.
#
# The version is automatically updated by a GitHub Actions workflow using
# the following heuristics:
#
#   - scripts_revision_count: Revisions related to this file and associated scripts.
#   - hooks_rev_count: Revisions associated with the "hooks" branch.
#   - SetupForDevelopment_VERSION: The sum of scripts_revision_count and hooks_rev_count.
#
SetupForDevelopment_VERSION=9
git config hooks.SetupForDevelopment ${SetupForDevelopment_VERSION}
