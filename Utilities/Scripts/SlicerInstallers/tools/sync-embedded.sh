#!/bin/sh
# Embed the canonical sources into the scripts that are fetched and piped to a
# shell.
#
# install.sh and install.ps1 are downloaded and executed in one line, so at run
# time they have no repo to read from: whatever they need has to live inside the
# script itself. The canonical copies are still ordinary files — the package
# lists under deps/, the inner scripts under inner/ — and this script renders
# them into the generated blocks of their consumers. `--check` re-renders and
# fails when a committed block no longer matches, so the copies cannot drift.
#
# Two stages, in order (stage 1 feeds stage 2):
#   1. deps/*.txt                 -> inner/slicer-deps
#   2. inner/slicer-language      -> install.sh
#      inner/slicer-deps          -> install.sh
#      inner/slicer-shortcuts     -> install.sh
#      inner/slicer-language.ps1  -> install.ps1
#      inner/slicer-shortcuts.ps1 -> install.ps1
#
# The same file also runs inside Slicer's source tree, where these scripts are
# vendored into Utilities/Scripts/SlicerInstallers: there the inner scripts sit
# beside install.sh instead of in inner/, and everything else is laid out alike.
#
# Usage:
#   tools/sync-embedded.sh            rewrite every generated block
#   tools/sync-embedded.sh --check    exit 1 (with a diff) if any is out of date
set -eu

repo_root="$(CDPATH='' cd -- "$(dirname -- "$0")/.." && pwd)"
deps_dir="$repo_root/deps"
inner_dir="$repo_root/inner"
[ -d "$inner_dir" ] || inner_dir="$repo_root"

deps_begin='# --- BEGIN generated dependency lists (tools/sync-embedded.sh) --- #'
deps_end='# --- END generated dependency lists --- #'
inner_begin='# --- BEGIN generated inner scripts (tools/sync-embedded.sh) --- #'
inner_end='# --- END generated inner scripts --- #'

# The delimiter of the heredocs that carry the inner scripts inside install.sh.
# Deliberately unlikely to occur in a shell script; asserted below regardless,
# because a collision would silently truncate the embedded copy.
heredoc_delim='SLICER_INNER_SCRIPT_EOF'

check=''
case "${1:-}" in
  --check) check=1 ;;
  '') ;;
  *) printf 'usage: %s [--check]\n' "$0" >&2; exit 2 ;;
esac

die() { printf 'sync-embedded: %s\n' "$1" >&2; exit 1; }

work="$(mktemp -d)"
trap 'rm -rf "$work"' EXIT INT HUP TERM

status=0

# ---------------------------------------------------------------------------- #
# stage 1: deps/*.txt -> inner/slicer-deps
# ---------------------------------------------------------------------------- #
# Flatten one list file into a single space-separated line: drop comments and
# blank lines, and squeeze the spaces around an "a | b" alternative so it stays
# one shell word for the script to split on.
read_list() {
  [ -f "$1" ] || die "missing list file: $1"
  awk '
    { sub(/#.*/, "") }
    { gsub(/[ \t]*\|[ \t]*/, "|") }
    { for (i = 1; i <= NF; i++) printf "%s%s", (first++ ? " " : ""), $i }
  ' "$1"
}

# Package names never contain a single quote, so quoting them this way is safe.
render_deps() {
  printf '%s\n' '# Generated from deps/*.txt — edit those files, then run tools/sync-embedded.sh.'
  printf "DEPS_APT='%s'\n" "$(read_list "$deps_dir/apt.txt")"
  printf "DEPS_DNF='%s'\n" "$(read_list "$deps_dir/dnf.txt")"
  printf "DEPS_PACMAN='%s'\n" "$(read_list "$deps_dir/pacman.txt")"
}

# ---------------------------------------------------------------------------- #
# stage 2: inner/* -> install.sh, install.ps1
# ---------------------------------------------------------------------------- #
# A line equal to the heredoc delimiter would end the heredoc early and leave
# the rest of the inner script executing as part of install.sh; a line equal to
# one of our markers would confuse the next re-render. Neither can be escaped
# away, so refuse to generate instead of producing a broken installer.
assert_embeddable() {
  src="$1"
  [ -f "$src" ] || die "missing inner script: $src"
  for forbidden in "$heredoc_delim" "$inner_begin" "$inner_end"; do
    if grep -qxF "$forbidden" "$src"; then
      die "${src##*/} contains a line equal to '$forbidden'; it cannot be embedded. Change that line."
    fi
  done
}

# Each inner script becomes a write_inner_<name> function carrying it verbatim
# in a quoted heredoc. The closing delimiter has to sit at column 0, so the
# body is emitted unindented.
render_inner_sh() {
  printf '%s\n' '# Generated from the inner scripts — edit those files, then run tools/sync-embedded.sh.'
  for name in slicer-language slicer-deps slicer-shortcuts; do
    src="$inner_dir/$name"
    assert_embeddable "$src"
    printf 'write_inner_%s() {\n' "$(printf '%s' "$name" | tr - _)"
    printf "  cat > \"\$1\" <<'%s'\n" "$heredoc_delim"
    cat "$src"
    printf '%s\n' "$heredoc_delim"
    printf '}\n'
  done
}

# PowerShell has no heredoc whose delimiter we can choose: a single-quoted
# here-string always ends at a line starting with '@, and the inner script
# contains exactly that (it embeds Python the same way). So the script is
# emitted as one single-quoted string per line instead — the same idiom the
# logo already uses in install.ps1 — which needs only ' doubled and has no
# delimiter to collide with.
render_inner_ps1() {
  printf '%s\n' '# Generated from the inner scripts -- edit those files, then run tools/sync-embedded.sh.'
  for pair in slicer-language.ps1:innerLanguagePs1 slicer-shortcuts.ps1:innerShortcutsPs1; do
    src="$inner_dir/${pair%%:*}"
    [ -f "$src" ] || die "missing inner script: $src"
    # The $ starts a PowerShell variable being written out verbatim, not a
    # shell expansion.
    # shellcheck disable=SC2016
    printf '$%s = @(\n' "${pair#*:}"
    awk -v q="'" '{ gsub(q, q q); printf "    %s%s%s\n", q, $0, q }' "$src"
    printf '%s\n' ') -join "`n"'
  done
}

# ---------------------------------------------------------------------------- #
# splice
# ---------------------------------------------------------------------------- #
# Replace everything between the markers of $1 with the output of renderer $4.
sync_block() {
  target="$1"; begin="$2"; end="$3"; renderer="$4"
  rel="${target#"$repo_root"/}"

  [ -f "$target" ] || die "missing $rel"
  grep -qxF "$begin" "$target" || die "no BEGIN marker in $rel"
  grep -qxF "$end" "$target" || die "no END marker in $rel"

  block="$work/block"
  out="$work/out"
  { printf '%s\n' "$begin"; "$renderer"; printf '%s\n' "$end"; } >"$block"

  # Splice the rendered block in place of everything between the markers.
  awk -v block_file="$block" -v b="$begin" -v e="$end" '
    $0 == b {
      while ((getline line < block_file) > 0) print line
      close(block_file)
      skip = 1
      next
    }
    $0 == e { skip = 0; next }
    !skip
  ' "$target" >"$out"

  if cmp -s "$out" "$target"; then
    return 0
  fi

  if [ -n "$check" ]; then
    printf 'sync-embedded: %s is out of date. Run tools/sync-embedded.sh and commit the result.\n\n' "$rel" >&2
    diff -u "$target" "$out" >&2 || true
    status=1
    return 0
  fi

  cat "$out" >"$target"
  printf 'sync-embedded: updated %s\n' "$rel"
}

# Stage 1 must run before stage 2: install.sh embeds inner/slicer-deps, which
# itself carries the generated package lists.
sync_block "$inner_dir/slicer-deps" "$deps_begin" "$deps_end" render_deps
sync_block "$repo_root/install.sh"  "$inner_begin" "$inner_end" render_inner_sh
sync_block "$repo_root/install.ps1" "$inner_begin" "$inner_end" render_inner_ps1

if [ "$status" -ne 0 ]; then
  exit 1
fi

if [ -n "$check" ]; then
  printf 'sync-embedded: everything is up to date.\n'
fi
