import sys

SENTINEL = "/test/slicer-start-preserve-sys-path/directory"

# Insert a sentinel path at the front of sys.path to detect preservation.
# We don't require the path to exist; we only assert presence.
sys.path.insert(0, SENTINEL)
