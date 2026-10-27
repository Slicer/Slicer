# Installer environment variables

The one-line installers are run by piping them straight into a shell:

```sh
curl -fsSL https://download.slicer.org/install.sh | sh                 # Linux, macOS
```

```powershell
powershell -c "irm https://download.slicer.org/install.ps1 | iex"      # Windows
```

A script piped into a shell gets no command-line arguments, so every option is
an environment variable. [`install.sh`](install.sh) and
[`install.ps1`](install.ps1) read the same variables; where one behaves
differently on some platform, this page says so.

## Summary

| Variable | Values | Default | What it does |
| -------- | ------ | ------- | ------------ |
| [`SLICER_RELEASE_TYPE`](#slicer_release_type) | `stable`, `preview`, `any` | `stable` | The release channel to install from |
| [`SLICER_VERSION`](#slicer_version) | a version or a prefix of one: `5.12.0`, `5.12` | the latest | Install a particular version |
| [`SLICER_REVISION`](#slicer_revision) | a revision number: `34627` | — | Install one exact build |
| [`SLICER_INSTALL_DIR`](#slicer_install_dir) | a path | depends on the platform | Where to install |
| [`SLICER_IF_EXISTING`](#slicer_if_existing) | `prompt`, `abort`, `reinstall`, `uninstall` | `prompt` | What to do when it is already installed |
| [`SLICER_NONINTERACTIVE`](#slicer_noninteractive) | any non-empty value | unset | Never ask anything |
| [`SLICER_QUIET`](#slicer_quiet) | any non-empty value | unset | Print only warnings, errors and questions |
| [`NO_COLOR`](#no_color) | any non-empty value | unset | No colors and no logo |
| [`XDG_CACHE_HOME`](#xdg_cache_home) | a path | `~/.cache` | Where downloads are kept (Linux, macOS) |

## Setting a variable

### Linux and macOS

Put the variables right before `sh`, after the pipe:

```sh
curl -fsSL https://download.slicer.org/install.sh | SLICER_VERSION=5.12.0 sh
```

Written before `curl` instead — `SLICER_VERSION=5.12.0 curl ... | sh` — the
variable reaches `curl`, not the installer, and is silently ignored. Exporting
it first works too:

```sh
export SLICER_VERSION=5.12.0
curl -fsSL https://download.slicer.org/install.sh | sh
```

### Windows

In PowerShell, set the variables before the pipe, separated by `;`:

```powershell
$env:SLICER_VERSION="5.12.0"; irm https://download.slicer.org/install.ps1 | iex
```

A variable set this way stays set for the rest of that PowerShell window, so it
applies to every later run there too. Clear it with
`Remove-Item Env:\SLICER_VERSION`, or open a new window.

From cmd, set them inside the PowerShell command, where they end with it:

```bat
powershell -c "$env:SLICER_VERSION='5.12.0'; irm https://download.slicer.org/install.ps1 | iex"
```

### Values

- `SLICER_NONINTERACTIVE`, `SLICER_QUIET` and `NO_COLOR` are switches: any
  non-empty value turns them on, `0`, `false` and `no` included. To turn one
  off, unset it (`unset SLICER_QUIET`, `Remove-Item Env:\SLICER_QUIET`). An
  empty value counts as unset.
- On Linux and macOS the values of `SLICER_RELEASE_TYPE` and
  `SLICER_IF_EXISTING` must be lowercase; Windows accepts any case.
- An invalid `SLICER_RELEASE_TYPE` or `SLICER_REVISION` stops the installer
  before anything is downloaded. So does an invalid `SLICER_IF_EXISTING` on
  Windows; on Linux and macOS it is only noticed once an installation is
  actually found.

## Choosing what to install

### `SLICER_RELEASE_TYPE`

The release channel:

- `stable` (the default) — the latest Slicer Stable Release.
- `preview` — the latest Slicer Preview Release, built every night from the
  development branch.
- `any` — whichever of the two is newer.

It is ignored when `SLICER_REVISION` is set.

### `SLICER_VERSION`

Install a particular version instead of the latest, for example `5.12.0`. The
download server compares versions part by part, so a prefix picks the newest
version that starts with it: `5.12` installs the newest 5.12.x, and `5` the
newest 5.x. It works within the channel `SLICER_RELEASE_TYPE` selects, so by
default it finds a Stable Release.

Preview Releases keep the same version number for weeks at a time, so a version
cannot single one out; use `SLICER_REVISION` for that.

`SLICER_VERSION` and `SLICER_REVISION` cannot both be set.

### `SLICER_REVISION`

Install one exact build, named by its revision number — the number shown next
to the version in Slicer's *Help → About* dialog and on
<https://download.slicer.org>, for example `34627`. Use it when every machine
must get the very same package: a version can be rebuilt, and a Preview
Release's version does not identify it.

- It must be a number; anything else is an error.
- A revision names one build on one channel, so `SLICER_RELEASE_TYPE` is
  ignored, with a warning if it is set to anything other than `stable`.
- It cannot be combined with `SLICER_VERSION`; setting both is an error.
- If the server resolves a different revision than the one asked for, the
  installer stops rather than install it.

## Where to install

### `SLICER_INSTALL_DIR`

Where Slicer goes. The value means something different on Windows:

|                         | Linux                                 | macOS                       | Windows                                  |
| ----------------------- | ------------------------------------- | --------------------------- | ---------------------------------------- |
| The value names         | the folder that holds the versions    | the folder for `Slicer.app` | the installation folder itself           |
| Default                 | `~/.local/opt`                        | `~/Applications`            | `%LOCALAPPDATA%\NA-MIC\Slicer <version>` |
| Slicer is installed to  | `<dir>/Slicer-<version>-linux-<arch>` | `<dir>/Slicer.app`          | `<dir>`                                  |

- The folder must be writable without root or Administrator rights: the
  installers never ask for either. macOS checks this before downloading; Linux
  only finds out once the download is done.
- **Linux** also links `<dir>/Slicer` to the version installed last. The
  launcher link `~/.local/bin/Slicer` and the menu entry
  `~/.local/share/applications/slicer.desktop` always go in those places, and
  point at the version installed last, whatever `SLICER_INSTALL_DIR` says.
- **Windows** hands the path to Slicer's own installer, which installs straight
  into it, so give every version a folder of its own (`C:\Slicer\5.12.3`). A
  *different* version already in the folder is not noticed, and gets installed
  over. The path must contain only printable ASCII characters, because some of
  Slicer's Python packages fail to load from a path with any other character;
  the installer stops if it does not.

On Linux and macOS, pointing `SLICER_INSTALL_DIR` at another folder installs a
second copy beside the first without asking anything. On Windows it does not
avoid the question; see [Installing a second copy](#installing-a-second-copy).

## When it is already installed

### `SLICER_IF_EXISTING`

Before downloading, the installer looks for an installation that it would
overwrite:

- **Linux** — the same version, at `<dir>/Slicer-<version>-linux-<arch>` in the
  target folder.
- **macOS** — any `Slicer.app` in the target folder, whatever its version,
  because every version installs under that one name.
- **Windows** — the same version anywhere, found through Windows' list of
  installed programs and Slicer's default folders, whatever `SLICER_INSTALL_DIR`
  says.

What happens next depends on `SLICER_IF_EXISTING`, and on whether there is
anyone to ask:

| `SLICER_IF_EXISTING` | The same version is there | A different version is there (macOS only) |
| -------------------- | ------------------------- | ----------------------------------------- |
| `prompt` (default), in a terminal | asks: abort, reinstall, install elsewhere, or uninstall | asks: abort, replace, install elsewhere, or uninstall |
| `prompt`, unattended (see [`SLICER_NONINTERACTIVE`](#slicer_noninteractive)) | exits successfully, downloading nothing | replaces it: that is an upgrade |
| `abort` | exits successfully, downloading nothing | exits successfully, downloading nothing |
| `reinstall` | replaces it with a fresh download | replaces it |
| `uninstall` | removes it and exits, installing nothing | removes it and exits, installing nothing |

When no installation is found, the installer installs as usual, whatever the
value. So `SLICER_IF_EXISTING=uninstall` means "remove that version if it is
there, otherwise install it".

On macOS, keep in mind that any `Slicer.app` counts: `abort` also refuses to
upgrade an older version, and `uninstall` removes whatever version is there,
even when `SLICER_VERSION` names another one.

Uninstalling removes:

- **Linux** — the version's folder, and the launcher link, the `<dir>/Slicer`
  link, the menu entry, the desktop icon and the dock pin, each one only if it
  still points at that version.
- **macOS** — `Slicer.app`, after quitting Slicer if it is running, with its
  desktop link and Dock tile.
- **Windows** — Slicer's own uninstaller runs, silently, after the desktop
  shortcut is removed.

The check needs the version the server is about to send. When the server's
metadata cannot be read, Linux checks after downloading instead, once the
archive has named its folder; Windows cannot check at all and simply installs.

#### Installing a second copy

"Install elsewhere" is only offered at the question. Unattended:

- **Linux, macOS** — set `SLICER_INSTALL_DIR` to another folder. The existing
  copy is not in the way there, so nothing is asked.
- **Windows** — the existing copy is found wherever it is, so set both
  `SLICER_INSTALL_DIR` (a new folder) and `SLICER_IF_EXISTING=reinstall`. With
  `SLICER_INSTALL_DIR` set, "reinstall" installs into that folder and leaves the
  other copy alone. *Add or remove programs* then lists only the new one.

### `SLICER_NONINTERACTIVE`

The "already installed" question is the only one either installer ever asks, so
with `SLICER_NONINTERACTIVE` set they never stop for input. When the version
being installed is already there, they exit successfully without downloading
anything, so a repeated run — a configuration-management converge, a re-run CI
job — costs next to nothing. Set `SLICER_IF_EXISTING=reinstall` to replace it
anyway. On macOS, a different version in the way is still replaced, since that
is an upgrade.

Often it is not even needed: the installers notice when there is no one to ask,
and behave just the same.

- **Linux, macOS** — when `/dev/tty` cannot be opened: CI jobs, containers,
  `ssh host 'command'` without a terminal. A plain `curl ... | sh` in a terminal
  window *does* ask, because the question goes to the terminal, not to the
  pipe.
- **Windows** — when the session is not interactive, or its input is redirected.

So set it when a terminal is attached but nothing should wait on it, for example
a provisioning script that someone starts by hand.

Every other variable still applies. To install a known version, unattended and
quietly:

```sh
curl -fsSL https://download.slicer.org/install.sh | \
  SLICER_NONINTERACTIVE=1 SLICER_VERSION=5.12.0 SLICER_QUIET=1 sh
```

```powershell
$env:SLICER_NONINTERACTIVE="1"; $env:SLICER_VERSION="5.12.0"; $env:SLICER_QUIET="1"
irm https://download.slicer.org/install.ps1 | iex
```

## Output

### `SLICER_QUIET`

Hides the progress messages (the `==>` lines, including the list of helper
scripts at the end), the logo and the download progress bar. Warnings, errors
and the "already installed" question still show.

### `NO_COLOR`

Turns off colored output and the logo, following <https://no-color.org>.

- **Linux, macOS** — colors and the logo are left out anyway whenever the
  output is not a terminal, for example when it goes to a log file.
- **Windows** — the logo is also left out whenever the output is redirected.
  Warnings and errors are colored by PowerShell itself, which `NO_COLOR` does not
  control.

## Download cache

### `XDG_CACHE_HOME`

The installers keep the package they downloaded, so a re-run does not download
it again:

- **Linux, macOS** — in `${XDG_CACHE_HOME:-$HOME/.cache}/slicer-installer`, as
  `Slicer-<version>-linux.tar.gz` or `Slicer-<version>-macosx.dmg`.
- **Windows** — in `%LOCALAPPDATA%\slicer-installer`, as `Slicer-<version>.exe`.
  This place cannot be changed.

A cached package is used only when it still matches the publisher's SHA-512
checksum; otherwise it is downloaded again and replaced. When the server's
metadata cannot be read, nothing is cached. Cached packages are never deleted:
each version installed adds one, from a few hundred megabytes up to about a
gigabyte, until you delete the folder.

In an image build (Docker, a VM image, any snapshot of `$HOME`) the cached
package would stay in the image for good. Point `XDG_CACHE_HOME` at a throwaway
folder and delete it **in the same step**. Deleting it in a later `RUN` does not
help, because the earlier layer still holds the file.

```dockerfile
RUN curl -fsSL https://download.slicer.org/install.sh \
    | SLICER_NONINTERACTIVE=1 XDG_CACHE_HOME=/tmp/slicer-cache sh \
    && rm -rf /tmp/slicer-cache
```

## Helper scripts

The scripts the installer leaves in Slicer's `bin` folder read some of the same
variables:

| Script | `SLICER_QUIET` | `NO_COLOR` | `SLICER_NONINTERACTIVE` |
| ------ | -------------- | ---------- | ----------------------- |
| `slicer-deps` (Linux) | yes; the command it runs is still printed | yes | same as `--yes`: fail at once instead of waiting for a `sudo` password; does not imply `--install` |
| `slicer-language`, `slicer-language.ps1` | yes | yes | — |
| `slicer-shortcuts`, `slicer-shortcuts.ps1` | yes | yes | — |

The other variables concern installing only.
