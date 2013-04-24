#!/usr/bin/env bash

# Run this script to set up the git hooks for committing changes

printErrorAndExit() {
  echo 'Failure during hook setup' 1>&2
  echo '-------------------------' 1>&2
  echo '' 1>&2
  echo "$@" 1>&2
  exit 1
}

u=$(cd "$(echo "$0"|sed 's/[^/]*$//')"; pwd)
cd "$u/../../.git/hooks"

# We need to have a git repository to do a pull.
if ! test -d ./.git; then
  git init || printErrorAndExit "Could not run git init."
fi &&

# Grab the hooks.
# Use the local hooks if possible.
echo "Pulling the hooks..." &&
if GIT_DIR=.. git for-each-ref refs/remotes/origin/hooks 2>/dev/null | \
  grep -q '\<refs/remotes/origin/hooks$'; then
  # Use hooks cloned from origin
  url=.. && branch=remotes/origin/hooks
else
  url=https://github.com/Slicer/Slicer.git
  branch=hooks
fi &&

git fetch -q "$url" "$branch" &&
git reset -q --hard FETCH_HEAD ||
    printErrorAndExit "Downloading the hooks failed."

cd ../..

echo "Done."
