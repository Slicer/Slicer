"""Embed a macOS bundle's external dependency closure and rewrite it to @rpath.

This replaces CMake's BundleUtilities ``fixup_bundle()`` for the macOS package.
``fixup_bundle()`` walks the dependency graph one binary at a time, shelling out
to ``otool`` repeatedly and copying/rewriting serially; on a bundle the size of
Slicer that is the dominant cost of ``make package`` (many minutes). The same
result is produced here far faster: the dependency graph is walked once with a
single ``otool -l`` per binary, and the (independent) install-name rewrites are
applied in parallel at the end.

Starting from every Mach-O already installed in the bundle (the application
binary, every module, plugin, CLI executable and Python extension), each
dependency that resolves to a file *outside* the bundle -- excluding the system
locations under ``/usr/lib`` and ``/System`` -- is either:

  - redirected to a library of the same name already embedded in the bundle
    (avoiding a duplicate copy), or
  - copied into the bundle (a plain dylib into ``Contents/lib``; a framework,
    whole, into ``Contents/Frameworks``) and the reference rewritten to
    ``@rpath/...``.

The walk follows copied libraries' own dependencies, and every Mach-O *inside* a
copied framework (for example ``QtWebEngineCore.framework``'s bundled
``Helpers/QtWebEngineProcess``), so the closure ends fully internal. ``@rpath``
resolves to ``Contents`` through the application executable's ``@loader_path/..``
run-path, so embedded libraries are referenced relative to ``Contents``.

External references may be absolute (Homebrew and superbuild install names are
absolute paths) or ``@rpath``/``@loader_path``/``@executable_path`` relative (for
example a binary Qt distribution's bare ``@rpath/libbrotlicommon.dylib``). The
relative forms are resolved against the referring binary's own ``LC_RPATH``
entries and an optional ``--search-path`` list (the library search directories
the build already knows about), mirroring how BundleUtilities resolved them.

Finally every standalone Mach-O executable (the PythonSlicer launcher, the CLI
module executables, ``QtWebEngineProcess``) is given its own ``@loader_path``
relative run-path to ``Contents`` so ``@rpath`` resolves when it is launched on
its own rather than loaded by the application, and every remaining run-path that
points outside the bundle (a build-tree or Homebrew directory) is removed so the
bundle carries no reference to the machine it was built on.

Run before code-signing: rewriting install names invalidates signatures.
"""

import argparse
import os
import re
import shutil
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

_MACHO_MAGIC = {
    b"\xcf\xfa\xed\xfe", b"\xce\xfa\xed\xfe",
    b"\xfe\xed\xfa\xcf", b"\xfe\xed\xfa\xce",
    b"\xca\xfe\xba\xbe", b"\xbe\xba\xfe\xca",
}

_DYLIB_LOAD_COMMANDS = {
    "LC_LOAD_DYLIB", "LC_LOAD_WEAK_DYLIB",
    "LC_REEXPORT_DYLIB", "LC_LOAD_UPWARD_DYLIB",
}

_NAME_RE = re.compile(r"^\s*name (.+) \(offset")
_PATH_RE = re.compile(r"^\s*path (.+) \(offset")


def is_macho(path):
    try:
        if os.path.islink(path) or not os.path.isfile(path):
            return False
        with open(path, "rb") as handle:
            return handle.read(4) in _MACHO_MAGIC
    except OSError:
        return False


def scan(path):
    """Parse a single ``otool -l`` into (loads, rpaths, install_id).

    One subprocess yields every load command, so the whole graph is walked with
    one ``otool`` invocation per binary instead of the three (``-L``/``-D``/``-l``)
    the equivalent separate queries would need.
    """
    out = subprocess.run(["otool", "-l", path],
                         capture_output=True, text=True, check=False).stdout
    loads, rpaths, install_id = [], [], None
    command = None
    for line in out.splitlines():
        stripped = line.strip()
        if stripped.startswith("cmd "):
            command = stripped[4:]
            continue
        if command in _DYLIB_LOAD_COMMANDS:
            match = _NAME_RE.match(line)
            if match:
                loads.append(match.group(1))
        elif command == "LC_ID_DYLIB":
            match = _NAME_RE.match(line)
            if match:
                install_id = match.group(1)
        elif command == "LC_RPATH":
            match = _PATH_RE.match(line)
            if match:
                rpaths.append(match.group(1))
    return loads, rpaths, install_id


def is_system(ref):
    return ref.startswith("/usr/lib") or ref.startswith("/System/")


def framework_relative(path):
    """For a path inside a ``*.framework``, return (framework_dir, inner) where
    inner is the path from the framework directory's parent (i.e. begins with
    ``Name.framework/``); otherwise None.
    """
    parts = path.split(os.sep)
    for index, part in enumerate(parts):
        if part.endswith(".framework"):
            return os.sep.join(parts[: index + 1]), os.sep.join(parts[index:])
    return None


def is_executable_macho(path):
    """True for a Mach-O main executable (MH_EXECUTE) -- a binary that can be
    launched on its own and so must carry its own run-path for @rpath to resolve.
    """
    out = subprocess.run(["file", "-b", path],
                         capture_output=True, text=True, check=False).stdout
    return "Mach-O" in out and "executable" in out


def _expand_rpath_entry(entry, binary_dir, exe_dir):
    """An LC_RPATH entry may itself be @loader_path/@executable_path relative."""
    if entry.startswith("@loader_path"):
        return os.path.normpath(os.path.join(binary_dir, entry[len("@loader_path"):].lstrip("/")))
    if entry.startswith("@executable_path"):
        return os.path.normpath(os.path.join(exe_dir, entry[len("@executable_path"):].lstrip("/")))
    return entry


def resolve(ref, binary_dir, rpaths, search_dirs, exe_dir):
    """Resolve an install name to an existing file, or None.

    Absolute references are taken as-is; @rpath is searched against the binary's
    own run-paths first, then the supplied search directories (with a bare
    filename fallback, as BundleUtilities does); @loader_path/@executable_path
    are resolved relative to the binary and the application executable.
    """
    if ref.startswith("@rpath/"):
        suffix = ref[len("@rpath/"):]
        search = [_expand_rpath_entry(entry, binary_dir, exe_dir) for entry in rpaths]
        search += search_dirs
        for directory in search:
            candidate = os.path.join(directory, suffix)
            if os.path.exists(candidate):
                return candidate
        name = os.path.basename(suffix)
        for directory in search:
            candidate = os.path.join(directory, name)
            if os.path.exists(candidate):
                return candidate
        return None
    if ref.startswith("@loader_path/"):
        candidate = os.path.normpath(os.path.join(binary_dir, ref[len("@loader_path/"):]))
        return candidate if os.path.exists(candidate) else None
    if ref.startswith("@executable_path/"):
        candidate = os.path.normpath(os.path.join(exe_dir, ref[len("@executable_path/"):]))
        return candidate if os.path.exists(candidate) else None
    if ref.startswith("/"):
        if os.path.exists(ref):
            return ref
        real = os.path.realpath(ref)
        return real if os.path.exists(real) else None
    # A bare or relative install name (for example DCMTK's "libopenjp2.7.dylib"):
    # dyld would find it via a run-path or a system directory. Search the same
    # places and embed it only if it is found in the build's own library
    # directories; a name not found there is a system library, left untouched.
    search = [_expand_rpath_entry(entry, binary_dir, exe_dir) for entry in rpaths]
    search += search_dirs
    for directory in search:
        candidate = os.path.join(directory, ref)
        if os.path.exists(candidate):
            return candidate
    name = os.path.basename(ref)
    for directory in search:
        candidate = os.path.join(directory, name)
        if os.path.exists(candidate):
            return candidate
    return None


def close_deps(app, search_dirs):
    contents = os.path.join(app, "Contents")
    exe_dir = os.path.join(contents, "MacOS")
    frameworks_dir = os.path.join(contents, "Frameworks")
    lib_dir = os.path.join(contents, "lib")
    contents_real = os.path.realpath(contents)

    def inside_bundle(path):
        return os.path.realpath(path).startswith(contents_real)

    def bundle_machos(root):
        found = []
        for directory, _dirs, files in os.walk(root):
            for name in files:
                path = os.path.join(directory, name)
                if is_macho(path):
                    found.append(path)
        return found

    # Index every Mach-O already in the bundle by basename, so a reference to an
    # external library of that name is redirected to the embedded copy.
    embedded = {}
    worklist = bundle_machos(app)
    for path in worklist:
        embedded.setdefault(os.path.basename(path), path)

    # Deferred install_name_tool work, applied in parallel once the graph is
    # fully walked (each binary's rewrite is independent of the others).
    id_resets = {}          # path -> new LC_ID_DYLIB
    changes = {}            # path -> list of (old_ref, new_ref)
    copied = []
    seen = set()

    def embed(source):
        """Copy an external library (or its whole framework) into the bundle and
        return the embedded path, reindexing any Mach-O it brings along.
        """
        framework = framework_relative(source)
        if framework is not None:
            src_fw, inner = framework
            dest_fw = os.path.join(frameworks_dir, os.path.basename(src_fw))
            if not os.path.exists(dest_fw):
                shutil.copytree(src_fw, dest_fw, symlinks=True)
                # A framework carries its own nested code (its versioned binary,
                # and helpers such as QtWebEngineProcess); register and walk them.
                for macho in bundle_machos(dest_fw):
                    embedded.setdefault(os.path.basename(macho), macho)
                    if macho not in seen:
                        worklist.append(macho)
            target = os.path.join(dest_fw, os.sep.join(inner.split(os.sep)[1:]))
        else:
            os.makedirs(lib_dir, exist_ok=True)
            target = os.path.join(lib_dir, os.path.basename(source))
            if not os.path.exists(target):
                shutil.copy2(source, target)
                os.chmod(target, 0o755)
                worklist.append(target)
        embedded.setdefault(os.path.basename(target), target)
        copied.append(os.path.basename(target))
        return target

    while worklist:
        current = worklist.pop()
        if current in seen:
            continue
        seen.add(current)
        loads, rpaths, install_id = scan(current)
        binary_dir = os.path.dirname(current)

        # An install id that still points outside the bundle (an absolute
        # build/Homebrew path, or a copied library keeping its original id)
        # would leak that path; reset it to @rpath relative to Contents.
        if install_id and install_id.startswith("/") and not is_system(install_id) \
                and not inside_bundle(install_id):
            id_resets[current] = "@rpath/" + os.path.relpath(current, contents)

        for ref in loads:
            if is_system(ref):
                continue
            source = resolve(ref, binary_dir, rpaths, search_dirs, exe_dir)
            if source is None or inside_bundle(source):
                # Unresolvable (leave as-is) or already internal (resolves via
                # the application's @loader_path/.. run-path).
                continue
            name = os.path.basename(source)
            target = embedded[name] if name in embedded else embed(source)
            new_ref = "@rpath/" + os.path.relpath(target, contents)
            if new_ref != ref:
                changes.setdefault(current, []).append((ref, new_ref))

    # Apply every install_name_tool rewrite in parallel.
    def apply(path):
        args = []
        if path in id_resets:
            args += ["-id", id_resets[path]]
        for old_ref, new_ref in changes.get(path, []):
            args += ["-change", old_ref, new_ref]
        if not args:
            return
        subprocess.run(["install_name_tool"] + args + [path],
                       capture_output=True, check=False)

    targets = set(id_resets) | set(changes)
    with ThreadPoolExecutor(max_workers=os.cpu_count()) as executor:
        list(executor.map(apply, targets))

    print("SlicerBundleCloseDeps: embedded %d libraries, rewrote %d files, reset %d ids"
          % (len(set(copied)), len(changes), len(id_resets)))
    for base in sorted(set(copied)):
        print("  embedded:", base)
    return True


def fix_executable_rpaths(app):
    """Give every standalone Mach-O executable in the bundle a run-path pointing
    at Contents, so its @rpath dependencies resolve when it is launched on its
    own rather than loaded by the application (which supplies that run-path).
    """
    contents = os.path.join(app, "Contents")
    fixed = 0
    for directory, _dirs, files in os.walk(app):
        for name in files:
            path = os.path.join(directory, name)
            if not is_macho(path) or not is_executable_macho(path):
                continue
            loads, rpaths, _id = scan(path)
            if not any(ref.startswith("@rpath/") for ref in loads):
                continue
            relative = os.path.relpath(contents, os.path.dirname(path))
            wanted = "@loader_path" if relative == "." else "@loader_path/" + relative
            if wanted.rstrip("/") in [entry.rstrip("/") for entry in rpaths]:
                continue
            subprocess.run(["install_name_tool", "-add_rpath", wanted, path],
                           capture_output=True, check=False)
            fixed += 1
    print("SlicerBundleCloseDeps: added a bundle-relative run-path to %d executables" % fixed)
    return fixed


def strip_external_rpaths(app):
    """Remove run-paths that point outside the bundle (build-tree or Homebrew
    directories left over from linking). After the closure every dependency
    resolves through a @loader_path-relative run-path, so these absolute entries
    are dead weight -- and a lingering Qt directory could load a second copy of a
    framework. Removing them also means the bundle names no build-machine path.
    """
    contents_real = os.path.realpath(os.path.join(app, "Contents"))

    def strip(path):
        _loads, rpaths, _id = scan(path)
        removed = 0
        for entry in rpaths:
            if entry.startswith("@"):
                continue
            if os.path.realpath(entry).startswith(contents_real):
                continue
            subprocess.run(["install_name_tool", "-delete_rpath", entry, path],
                           capture_output=True, check=False)
            removed += 1
        return removed

    machos = []
    for directory, _dirs, files in os.walk(app):
        for name in files:
            path = os.path.join(directory, name)
            if is_macho(path):
                machos.append(path)
    with ThreadPoolExecutor(max_workers=os.cpu_count()) as executor:
        removed = sum(executor.map(strip, machos))
    print("SlicerBundleCloseDeps: removed %d external run-paths" % removed)
    return removed


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--app", required=True, help="Path to the .app bundle")
    parser.add_argument("--search-path", action="append", default=[], metavar="DIR",
                        help="Directory to resolve @rpath dependencies against "
                             "(repeatable). The referring binary's own run-paths "
                             "are always searched first.")
    options = parser.parse_args(argv)
    if not os.path.isdir(options.app):
        print("SlicerBundleCloseDeps: no such bundle: %s" % options.app, file=sys.stderr)
        return 1
    search_dirs = [d for d in options.search_path if d and os.path.isdir(d)]
    if not close_deps(options.app, search_dirs):
        return 1
    fix_executable_rpaths(options.app)
    strip_external_rpaths(options.app)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
