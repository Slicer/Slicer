#!/bin/sh
#
# Wrapper around hdiutil used by the CPack DragNDrop generator.
#
# CPack runs "hdiutil detach" only once after attaching the temporary disk image.
# If another process is still accessing the newly attached volume, for example
# Finder in a logged-in GUI session, the detach fails with "Resource busy" and
# packaging fails, leaving the volume mounted.
#
# This wrapper retries "detach" a few times before forcing it. All other hdiutil
# commands are run unchanged.

hdiutil=/usr/bin/hdiutil

if [ "$1" != "detach" ]; then
  exec "$hdiutil" "$@"
fi

attempts=10
attempt=1
while [ "$attempt" -le "$attempts" ]; do
  if "$hdiutil" "$@"; then
    exit 0
  fi
  echo "SlicerCPackHdiutil: 'hdiutil detach' failed (attempt $attempt of $attempts), retrying in $attempt second(s)" >&2
  sleep "$attempt"
  attempt=$((attempt + 1))
done

echo "SlicerCPackHdiutil: 'hdiutil detach' failed after $attempts attempts, forcing detach" >&2
shift
exec "$hdiutil" detach -force "$@"
