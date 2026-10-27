# Slicer's runtime system libraries

Slicer bundles Qt, Python, VTK and the rest of its own dependencies, but it
still links against a handful of libraries that come from the distribution. The
files in this directory are the canonical list of those packages — one file per
package manager, one package per line.

| File | Package manager | Distributions |
| ---- | --------------- | ------------- |
| [`apt.txt`](apt.txt) | `apt-get` | Debian, Ubuntu and derivatives |
| [`dnf.txt`](dnf.txt) | `dnf` | Fedora, RHEL, CentOS Stream |
| [`pacman.txt`](pacman.txt) | `pacman` | Arch Linux and derivatives |

## Format

- One package per line; leading and trailing whitespace is ignored.
- Blank lines and `#` comments are ignored.
- `a | b` marks two names for the same library: the installer picks the first
  one the package manager actually knows about. Only `apt.txt` uses this, for
  the `libasound2` → `libasound2t64` rename.

## How these reach the user

The script that installs these packages is `slicer-deps` (in `inner/` in the
slicer-installer repository, beside `install.sh` in Slicer's source tree), which
lives inside the Slicer installation and is run by the user afterwards —
installing distribution packages needs root, so it is never part of installing
Slicer itself.

Neither that script nor `install.sh` can read this directory at run time: one
ships inside a Slicer package, the other is fetched and piped to `sh` in one
line. So the lists are embedded, in two hops:

```
deps/*.txt  ->  slicer-deps  ->  install.sh
```

The first hop fills the `BEGIN/END generated dependency lists` markers in
`slicer-deps`; the second carries that whole script into the
`BEGIN/END generated inner scripts` markers of `install.sh`, which writes it out
at install time for packages that do not ship it themselves. Both hops are done
by:

```sh
tools/sync-embedded.sh
```

Run that after editing any file here and commit all the changes together.
`tools/sync-embedded.sh --check` re-renders every block and fails if any differs
from what is committed; CI runs it on every change to these scripts, so the
embedded copies cannot drift away from these files.
