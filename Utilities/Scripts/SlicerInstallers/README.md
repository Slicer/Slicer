# Slicer installer scripts

Two kinds of script live here.

## Outer installers — not installed into the package

| File | Used as |
| ---- | ------- |
| `install.sh` | `curl -fsSL <raw-url>/install.sh \| sh` — Linux and macOS |
| `install.ps1` | `powershell -c "irm <raw-url>/install.ps1 \| iex"` — Windows |

They install Slicer onto a machine that does not have it yet, so there is
nothing for them to be installed *into*: they are source only, served over
HTTPS. `CMakeLists.txt` deliberately does not install them.

## Inner scripts — installed into the package

| File | Platform | What it does |
| ---- | -------- | ------------ |
| `slicer-language` | Linux, macOS | `list` prints the available interface languages; a code like `es-419` installs it and switches Slicer to it |
| `slicer-language.ps1` | Windows | The same |
| `slicer-deps` | Linux | Prints the package-manager command for the system libraries Slicer needs; `--install` runs it |

These do the things that are *not* installing Slicer, and are run by the user
afterwards. `CMakeLists.txt` copies them into `bin/` in the build tree and
installs them into `${Slicer_INSTALL_BIN_DIR}`, so each one finds the Slicer
launcher relative to its own path and needs no configuration.

`slicer-deps` is separate precisely because installing distribution packages
needs root: the installer itself never asks for it and only ever writes under
`$HOME`.

## Provenance

These files are developed in <https://github.com/mauigna06/slicer-installer>
and vendored here verbatim — do not edit them in this tree. Make the change
there, run `tools/sync-embedded.sh`, and copy all five files across again.

That repository is also where the two generated blocks come from. The package
lists in its `deps/*.txt` are rendered into `slicer-deps`, and each inner script
is then embedded into the outer installer that carries it:

```
deps/*.txt  ->  slicer-deps          ->  install.sh
                slicer-language      ->  install.sh
                slicer-language.ps1  ->  install.ps1
```

The outer installers need those embedded copies because they run on machines
with no Slicer yet, and must be able to set up a package that predates this
directory. They write them out **only when the installed package does not
already contain them**, so once these scripts ship in the package, the
package's own copies win and the embedded ones are never used.

The comments inside `install.sh` and `install.ps1` refer to `inner/` and
`tools/sync-embedded.sh`; those are paths in the upstream repository, not here.

### Checking the copies agree

An embedded copy that has drifted from its sibling here would ship two different
versions of the same script. To verify:

```sh
# install.sh: run its embedded writers and compare
awk '/^# --- BEGIN generated inner scripts/{f=1;next} /^# --- END generated inner scripts/{f=0} f' \
  install.sh > /tmp/block.sh
( . /tmp/block.sh
  write_inner_slicer_language /tmp/slicer-language
  write_inner_slicer_deps     /tmp/slicer-deps )
cmp slicer-language /tmp/slicer-language && cmp slicer-deps /tmp/slicer-deps
```

The upstream repository enforces this in CI; nothing in this tree does.
