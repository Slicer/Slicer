"""Ad-hoc (or identity) code-sign a macOS application bundle, inside-out.

``install_name_tool`` invalidates the code signature of every Mach-O file it
rewrites during bundle fixup. On Apple Silicon the kernel refuses to map a page
whose signature does not match the binary, so an unsigned or stale-signed file
is killed at load time with EXC_BAD_ACCESS ("Code Signature Invalid").

``codesign --deep`` is unreliable on a bundle this large and deeply nested (the
application's CFBundleExecutable is a small bootstrap, so the real application
binary and every module, plugin and framework are *nested* code): it can leave
individual binaries with an invalid signature, which then fault at load time.

Signing inside-out -- every Mach-O signed on its own, deepest path first, and
the enclosing .app signed last -- avoids that: each file is sealed
independently and the umbrella signature seals the directory tree on top.

Only the ad-hoc identity ("-") is used by default, which is enough for the
binary to load on the machine it was built for and for internal testing (users
still clear the quarantine attribute manually). Pass --identity to sign with a
Developer ID for distribution/notarization.
"""

import argparse
import os
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor

# Mach-O magic numbers (thin and fat, both byte orders).
_MACHO_MAGIC = {
    b"\xcf\xfa\xed\xfe",  # 64-bit LE
    b"\xce\xfa\xed\xfe",  # 32-bit LE
    b"\xfe\xed\xfa\xcf",  # 64-bit BE
    b"\xfe\xed\xfa\xce",  # 32-bit BE
    b"\xca\xfe\xba\xbe",  # fat BE
    b"\xbe\xba\xfe\xca",  # fat LE
}


def is_macho(path):
    try:
        if os.path.islink(path) or not os.path.isfile(path):
            return False
        with open(path, "rb") as handle:
            return handle.read(4) in _MACHO_MAGIC
    except OSError:
        return False


def find_machos(root):
    paths = []
    for directory, _dirs, files in os.walk(root):
        for name in files:
            path = os.path.join(directory, name)
            if is_macho(path):
                paths.append(path)
    return paths


def sign(path, identity):
    result = subprocess.run(
        ["codesign", "--force", "--sign", identity, path],
        capture_output=True,
        text=True,
        check=False,
    )
    return path, result.returncode, result.stderr.strip()


def sign_bundle(app, identity):
    machos = find_machos(app)
    # Deepest paths first so nested code is sealed before its container.
    machos.sort(key=lambda path: path.count(os.sep), reverse=True)

    failures = []
    with ThreadPoolExecutor(max_workers=os.cpu_count()) as executor:
        for path, code, error in executor.map(lambda p: sign(p, identity), machos):
            if code != 0:
                failures.append((path, error))
    print("SlicerSignBundleMacOS: signed %d Mach-O files (%d failures)"
          % (len(machos), len(failures)))
    for path, error in failures[:20]:
        print(f"  failed: {path}: {error}")

    # Seal the application bundle itself last.
    result = subprocess.run(
        ["codesign", "--force", "--sign", identity, app],
        capture_output=True,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        print("SlicerSignBundleMacOS: signing the application bundle failed:\n"
              + result.stderr, file=sys.stderr)
    return len(failures) == 0 and result.returncode == 0


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--app", required=True, help="Path to the .app bundle")
    parser.add_argument("--identity", default="-",
                        help="codesign identity (default: '-', ad-hoc)")
    options = parser.parse_args(argv)
    if not os.path.isdir(options.app):
        print("SlicerSignBundleMacOS: no such bundle: %s" % options.app,
              file=sys.stderr)
        return 1
    return 0 if sign_bundle(options.app, options.identity) else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
