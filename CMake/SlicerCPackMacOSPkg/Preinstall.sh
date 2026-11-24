#!/bin/sh
set -eu
console_info=$(printf 'show State:/Users/ConsoleUser\n' | /usr/sbin/scutil) || {
  echo "Cannot determine the logged-in desktop user." >&2
  exit 1
}
console_user=$(printf '%s\n' "$console_info" | /usr/bin/awk '/^  Name : / { print $3; exit }')
case "$console_user" in
  ""|root|loginwindow|_mbsetupuser)
    echo "Installation requires a logged-in desktop user." >&2
    exit 1
    ;;
esac
user_id=$(/usr/bin/id -u "$console_user") || {
  echo "Cannot determine the desktop user ID." >&2
  exit 1
}
case "$user_id" in
  ""|0|*[!0-9]*)
    echo "Cannot determine a valid desktop user ID." >&2
    exit 1
    ;;
esac
