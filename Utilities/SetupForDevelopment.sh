#!/usr/bin/env bash

# Run this script to set up development with git.
# For more information, see:
# http://www.slicer.org/slicerWiki/index.php/Documentation/Labs/DevelopmentWithGit
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

# Make 'git pull' on master always use rebase.
git config branch.master.rebase true

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

# Record the version of this setup so the developer can be notified that
# this script and/or hooks have been modified.
SetupForDevelopment_VERSION=8
git config hooks.SetupForDevelopment ${SetupForDevelopment_VERSION}
