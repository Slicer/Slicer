"""Validate Slicer's pinned wheel environment without executing package code.

License results are aggregated once per package version, outside platform tables.
Identical license declarations are checked once; distinct declarations from
selected wheels must all pass. Packages without a usable wheel are not assessed.
Verified legacy aliases in the policy bind an SPDX expression and source evidence
to an exact package version and license-metadata fingerprint, not a global label.
Reports link release metadata, declared project/license URLs, and selected wheels
containing license files; repository license-file URLs are never guessed.
Policy exceptions record explicit maintainer approvals, not a permissive-license
classification. Approval does not remove notice or source-distribution duties.
"""

import argparse
from dataclasses import dataclass
from email.parser import BytesParser
from email.policy import default as email_policy
from functools import cache
import hashlib
import io
import json
from pathlib import Path
import re
import subprocess
import sys
from tempfile import TemporaryDirectory
from urllib.error import URLError
from urllib.parse import quote, urlparse
from urllib.request import Request, urlopen
from zipfile import ZipFile

from license_expression import ExpressionError, get_spdx_licensing
from packaging.licenses import InvalidLicenseExpression, canonicalize_license_expression
from packaging.requirements import Requirement
from packaging.specifiers import SpecifierSet
from packaging.tags import compatible_tags, cpython_tags, mac_platforms
from packaging.utils import canonicalize_name, parse_wheel_filename
from packaging.version import Version


class ValidationError(Exception):
    pass


@dataclass(frozen=True)
class Target:
    name: str
    system: str
    machine: str
    platforms: tuple[str, ...]
    required: bool = True

    def environment(self, python: str) -> dict[str, str]:
        return {
            "implementation_name": "cpython",
            "implementation_version": python,
            "os_name": "nt" if self.system == "Windows" else "posix",
            "platform_machine": self.machine,
            "platform_release": "",
            "platform_system": self.system,
            "platform_version": "",
            "python_full_version": python,
            "platform_python_implementation": "CPython",
            "python_version": ".".join(python.split(".")[:2]),
            "sys_platform": {"Windows": "win32", "Darwin": "darwin", "Linux": "linux"}[self.system],
            "extra": "",
        }


def linux_platforms(arch: str) -> tuple[str, ...]:
    platforms = [f"manylinux_2_{minor}_{arch}" for minor in range(28, 16, -1)]
    platforms.append(f"manylinux2014_{arch}")
    if arch == "x86_64":
        platforms.extend(["manylinux_2_12_x86_64", "manylinux2010_x86_64",
                          "manylinux_2_5_x86_64", "manylinux1_x86_64"])
    return tuple(platforms)


TARGETS = (
    Target("windows-x64", "Windows", "AMD64", ("win_amd64",)),
    Target("macos-x64", "Darwin", "x86_64", tuple(mac_platforms((14, 0), "x86_64"))),
    Target("macos-arm64", "Darwin", "arm64", tuple(mac_platforms((14, 0), "arm64"))),
    Target("linux-x64", "Linux", "x86_64", linux_platforms("x86_64")),
    Target("linux-arm64", "Linux", "aarch64", linux_platforms("aarch64"), False),
    Target("windows-arm64", "Windows", "ARM64", ("win_arm64",), False),
)


@cache
def target_tags(target: Target, python: str) -> dict:
    version = tuple(int(part) for part in python.split(".")[:2])
    interpreter = f"cp{version[0]}{version[1]}"
    tags = list(cpython_tags(version, abis=[interpreter], platforms=target.platforms))
    tags.extend(compatible_tags(version, interpreter=interpreter, platforms=target.platforms))
    return {tag: rank for rank, tag in reversed(list(enumerate(tags)))}


@dataclass
class Pin:
    requirement: Requirement
    hashes: set[str]
    path: Path
    start: int
    end: int

    @property
    def name(self) -> str:
        return canonicalize_name(self.requirement.name)

    @property
    def version(self) -> str:
        return next(iter(self.requirement.specifier)).version


def read_pins(path: Path) -> list[Pin]:
    lines = path.read_text(encoding="utf-8").splitlines(keepends=True)
    pins = []
    index = 0
    while index < len(lines):
        start = index
        line = lines[index].strip()
        index += 1
        if not line or line.startswith("#"):
            continue
        while line.endswith("\\"):
            if index == len(lines):
                raise ValidationError(f"{path}:{start + 1}: unterminated requirement")
            line = line[:-1] + " " + lines[index].strip()
            index += 1
        line = re.split(r"\s+#", line, maxsplit=1)[0]
        hashes = set(re.findall(r"--hash=sha256:([0-9a-f]{64})\b", line))
        requirement_text = re.sub(r"\s+--hash=sha256:[0-9a-f]{64}\b", "", line)
        requirement = Requirement(requirement_text)
        specs = list(requirement.specifier)
        if (requirement.url or len(specs) != 1 or specs[0].operator != "=="
                or "*" in specs[0].version or not hashes):
            raise ValidationError(f"{path}:{start + 1}: require one exact == pin and SHA-256 hashes")
        pins.append(Pin(requirement, hashes, path, start, index))
    if not pins:
        raise ValidationError(f"{path}: no pinned requirements")
    return pins


def python_version(root: Path) -> str:
    match = re.search(r'set\(Slicer_REQUIRED_PYTHON_VERSION "(\d+\.\d+\.\d+)"\)',
                      (root / "CMakeLists.txt").read_text(encoding="utf-8"))
    if not match:
        raise ValidationError("Cannot determine Slicer_REQUIRED_PYTHON_VERSION")
    return match[1]


def active(requirement: Requirement, target: Target, python: str, extras: set[str]) -> bool:
    return requirement.marker is None or any(
        requirement.marker.evaluate({**target.environment(python), "extra": extra})
        for extra in extras | {""}
    )


def wheel_rank(file: dict, target: Target, python: str) -> int | None:
    if not file["filename"].endswith(".whl") or file.get("yanked"):
        return None
    if file.get("requires_python") and not SpecifierSet(file["requires_python"]).contains(python):
        return None
    tags = parse_wheel_filename(file["filename"])[3]
    ranks = target_tags(target, python)
    compatible = [ranks[tag] for tag in tags if tag in ranks]
    return min(compatible) if compatible else None


class Index:
    def __init__(self, cache_dir: Path):
        self.cache_dir = cache_dir
        cache_dir.mkdir(parents=True, exist_ok=True)
        self._files = {}
        self._metadata = {}

    def fetch(self, url: str, digest: str | None = None) -> bytes:
        if urlparse(url).scheme != "https" or urlparse(url).hostname not in {
            "pypi.org", "files.pythonhosted.org",
        }:
            raise ValidationError(f"Untrusted package metadata URL: {url}")
        path = self.cache_dir / hashlib.sha256(url.encode()).hexdigest()
        if path.exists():
            data = path.read_bytes()
        else:
            request = Request(url, headers={"Accept": "application/vnd.pypi.simple.v1+json",
                                            "User-Agent": "Slicer-requirements-validation"})
            with urlopen(request, timeout=90) as response:
                data = response.read()
            path.write_bytes(data)
        if digest and hashlib.sha256(data).hexdigest() != digest:
            raise ValidationError(f"SHA-256 mismatch: {url}")
        return data

    def files(self, name: str, version: str) -> list[dict]:
        key = (name, version)
        if key not in self._files:
            release = json.loads(self.fetch(f"https://pypi.org/pypi/{quote(name)}/{quote(version)}/json"))
            self._files[key] = release["urls"]
        return self._files[key]

    def metadata(self, name: str, filename: str, url: str, digest: str):
        key = (name, filename, url, digest)
        if key in self._metadata:
            return self._metadata[key]
        simple = json.loads(self.fetch(f"https://pypi.org/simple/{quote(name)}/"))
        file = next(item for item in simple["files"] if item["filename"] == filename)
        metadata_hashes = file.get("core-metadata") or file.get("dist-info-metadata")
        if isinstance(metadata_hashes, dict) and metadata_hashes.get("sha256"):
            data = self.fetch(url + ".metadata", metadata_hashes["sha256"])
        else:
            with ZipFile(io.BytesIO(self.fetch(url, digest))) as wheel:
                entries = [entry for entry in wheel.namelist()
                           if entry.endswith(".dist-info/METADATA") and entry.count("/") == 1]
                if len(entries) != 1:
                    raise ValidationError(f"{filename}: expected exactly one wheel METADATA")
                data = wheel.read(entries[0])
        self._metadata[key] = BytesParser(policy=email_policy).parsebytes(data)
        return self._metadata[key]


CLASSIFIER_LICENSES = {
    "Apache Software License": "Apache-2.0",
    "MIT License": "MIT",
    "ISC License (ISCL)": "ISC",
    "Python Software Foundation License": "PSF-2.0",
    "The Unlicense (Unlicense)": "Unlicense",
    "zlib/libpng License": "Zlib",
}


def license_expression(metadata) -> str:
    if metadata.get("License-Expression"):
        return canonicalize_license_expression(metadata["License-Expression"])
    legacy = metadata.get("License", "").strip()
    if legacy and "\n" not in legacy:
        try:
            return canonicalize_license_expression(legacy)
        except InvalidLicenseExpression:
            pass  # Legacy License is free text, not necessarily an SPDX expression.
    if legacy and legacy != "UNKNOWN":
        raise ValidationError("Legacy License field is not a precise SPDX identifier or expression; requires review")
    classifiers = [value.split(" :: ")[-1] for value in metadata.get_all("Classifier", [])
                   if value.startswith("License :: OSI Approved :: ")]
    if len(classifiers) == 1 and classifiers[0] in CLASSIFIER_LICENSES:
        return CLASSIFIER_LICENSES[classifiers[0]]
    raise ValidationError("No unambiguous SPDX license; requires a reviewed version-specific exception")


def permissive(expression: str, allowed: list[str]) -> bool:
    licensing = get_spdx_licensing()
    parsed = licensing.parse(expression, validate=True, strict=True)
    return parsed.subs({
        symbol: licensing.TRUE if str(symbol) in allowed else licensing.FALSE
        for symbol in parsed.symbols
    }).simplify() == licensing.TRUE


def check_license(name: str, version: str, metadata, policy: dict) -> tuple[bool, str]:
    # An exception binds approval to this exact version and declared metadata.
    declared = {
        key: metadata.get_all(key, [])
        for key in ("License-Expression", "License", "Classifier", "License-File")
    }
    fingerprint = hashlib.sha256(json.dumps(declared, sort_keys=True).encode()).hexdigest()
    exception = policy["license_exceptions"].get(f"{name}=={version}")
    if exception:
        approved = exception.get("metadata_sha256", [])
        if isinstance(approved, str):
            approved = [approved]
        if fingerprint in approved and exception.get("reason"):
            return True, f"Reviewed exception: {exception['reason']}"
    try:
        alias = policy.get("legacy_license_aliases", {}).get(f"{name}=={version}", {})
        verified_alias = fingerprint == alias.get("metadata_sha256") and bool(alias.get("source"))
        expression = (canonicalize_license_expression(alias["expression"]) if verified_alias
                      else license_expression(metadata))
        allowed = permissive(expression, policy["allowed_licenses"])
        detail = (f"{expression} (verified legacy alias {metadata.get('License', '')!r}; "
                  f"source: {alias['source']})") if verified_alias else expression
        return allowed, detail if allowed else f"{detail}; metadata_sha256={fingerprint}"
    except (ValidationError, InvalidLicenseExpression, ExpressionError) as error:
        return False, f"{error}; metadata_sha256={fingerprint}"


def license_links(name: str, version: str, metadata, policy: dict) -> dict[str, str]:
    """Link declared evidence without guessing repository license filenames."""
    links = {"PyPI release / license metadata": f"https://pypi.org/project/{name}/{version}/#license"}
    candidates = [("Project homepage", metadata.get("Home-page", "")),
                  ("Declared license", metadata.get("License", "")),
                  ("Exception review source",
                   policy["license_exceptions"].get(f"{name}=={version}", {}).get("source", "")),
                  ("Alias review source",
                   policy.get("legacy_license_aliases", {}).get(f"{name}=={version}", {}).get("source", ""))]
    for value in metadata.get_all("Project-URL", []):
        label, separator, url = value.partition(",")
        if separator and label.strip().lower() in {
            "license", "licence", "licensing", "source", "repository", "homepage",
        }:
            candidates.append((f"Declared {label.strip()}", url.strip()))
    for label, url in candidates:
        parsed = urlparse(url)
        if parsed.scheme == "https" and parsed.hostname and not parsed.username:
            links[label] = url
    try:
        expression = license_expression(metadata)
        symbols = get_spdx_licensing().parse(expression, validate=True, strict=True).symbols
        for symbol in symbols:
            identifier = str(symbol)
            if re.fullmatch(r"[A-Za-z0-9.+-]+", identifier):
                links[f"SPDX {identifier}"] = f"https://spdx.org/licenses/{identifier}.html"
    except (ValidationError, InvalidLicenseExpression, ExpressionError):
        pass  # Ambiguous declarations have no precise SPDX license-text link.
    return links


def group_packages(root: Path, pins: list[Pin]) -> dict[Path, set[str]]:
    projects = {}
    dependencies = {}
    for path in (root / "SuperBuild").glob("External_python*.cmake"):
        text = re.sub(r"#[^\n]*", "", path.read_text(encoding="utf-8"))
        project = re.search(r"set\(proj ([^)]+)\)", text)
        if not project:
            continue
        dependency = re.search(r"set\(\$\{proj\}_DEPENDENCIES\s+([^)]*)\)", text)
        if not dependency:
            raise ValidationError(f"{path}: cannot determine external-project dependencies")
        dependencies[project[1]] = set(dependency[1].split())
        manifest = re.search(r'Python/([^"]+requirements\.txt)', text)
        if manifest:
            projects[root / "SuperBuild" / "Python" / manifest[1]] = project[1]
    result = {}
    for path in {pin.path for pin in pins}:
        if path not in projects:
            raise ValidationError(f"{path}: no corresponding external project")
        closure = {projects[path]}
        pending = list(closure)
        while pending:
            for dependency in dependencies.get(pending.pop(), set()) - closure:
                closure.add(dependency)
                pending.append(dependency)
        result[path] = {pin.name for pin in pins if projects[pin.path] in closure}
    return result


def validate(root: Path, index: Index, policy: dict) -> dict:
    python = python_version(root)
    pins = [pin for path in sorted((root / "SuperBuild" / "Python").glob("*requirements.txt"))
            for pin in read_pins(path)]
    if not pins:
        raise ValidationError("No SuperBuild requirements found")
    available = group_packages(root, pins)
    report = {"python": python, "errors": [], "warnings": [], "targets": {}, "hashes": [], "licenses": {}}
    license_metadata = {}
    license_wheels = {}
    for target in TARGETS:
        errors = []
        packages = {}
        occurrences = {}
        extras = {}
        for pin in pins:
            if active(pin.requirement, target, python, set()):
                occurrences.setdefault(pin.name, []).append(pin)
                extras.setdefault(pin.name, set()).update(pin.requirement.extras)
        for name, copies in sorted(occurrences.items()):
            versions = {Version(pin.version) for pin in copies}
            if len(versions) != 1:
                errors.append(f"{name}: conflicting pins: {sorted(str(v) for v in versions)}")
                continue
            version = copies[0].version
            files = index.files(name, version)
            compatible = [file for file in files if wheel_rank(file, target, python) is not None]
            candidates = [file for file in compatible
                          if all(file["digests"]["sha256"] in pin.hashes for pin in copies)]
            if not candidates:
                reason = ("compatible wheels exist but no wheel is hash-approved in every manifest"
                          if compatible else "no compatible non-yanked wheel is published")
                errors.append(f"{name}=={version}: {reason} for {target.name}")
                continue
            file = min(candidates, key=lambda item: wheel_rank(item, target, python))
            metadata = index.metadata(name, file["filename"], file["url"], file["digests"]["sha256"])
            if (canonicalize_name(metadata["Name"]) != name or Version(metadata["Version"]) != Version(version)
                    or (metadata.get("Requires-Python")
                        and not SpecifierSet(metadata["Requires-Python"]).contains(python))):
                errors.append(f"{file['filename']}: wheel metadata name/version/Python mismatch")
                continue
            declared = json.dumps({
                key: metadata.get_all(key, [])
                for key in ("License-Expression", "License", "Classifier", "License-File")
            }, sort_keys=True)
            license_metadata.setdefault((name, version), {})[declared] = metadata
            license_wheels.setdefault((name, version), {})[file["filename"]] = file["url"]
            packages[name] = {
                "version": version, "wheel": file["filename"], "sha256": file["digests"]["sha256"],
                "wheel_url": file["url"],
                "requires": metadata.get_all("Requires-Dist", []),
                "provides_extras": metadata.get_all("Provides-Extra", []),
            }
        # Dependency-requested extras can activate further transitive edges.
        changed = True
        while changed:
            changed = False
            for name, package in packages.items():
                for text in package["requires"]:
                    requirement = Requirement(text)
                    if active(requirement, target, python, extras.get(name, set())):
                        dep = canonicalize_name(requirement.name)
                        before = extras.setdefault(dep, set()).copy()
                        extras[dep].update(requirement.extras)
                        changed |= before != extras[dep]
        edges = set()
        for name, package in packages.items():
            unknown_extras = extras.get(name, set()) - {
                canonicalize_name(extra) for extra in package["provides_extras"]
            }
            if unknown_extras:
                errors.append(f"{name}: requested extras not provided: {sorted(unknown_extras)}")
            for text in package["requires"]:
                requirement = Requirement(text)
                if not active(requirement, target, python, extras.get(name, set())):
                    continue
                dep = canonicalize_name(requirement.name)
                edges.add((name, str(requirement)))
                if requirement.url or dep not in occurrences:
                    errors.append(f"{name}: dependency {requirement} has no exact PyPI pin")
                elif not requirement.specifier.contains(occurrences[dep][0].version, prereleases=True):
                    errors.append(f"{name}: needs {requirement}, pinned {occurrences[dep][0].version}")
                for pin in occurrences[name]:
                    if dep not in available[pin.path]:
                        errors.append(f"{pin.path.name}: {name} needs {dep} outside its CMake dependency closure")
        report["targets"][target.name] = {
            "required": target.required, "packages": packages,
            "edges": sorted([list(edge) for edge in edges]), "issues": sorted(set(errors)),
            "removal_candidates": sorted(set(packages) - {
                canonicalize_name(Requirement(text).name) for _, text in edges
            }),
        }
        destination = report["errors"] if target.required else report["warnings"]
        destination.extend(f"{target.name}: {error}" for error in sorted(set(errors)))
    for (name, version), declarations in sorted(license_metadata.items()):
        checks = [check_license(name, version, metadata, policy) for metadata in declarations.values()]
        approved = all(licensed for licensed, _ in checks)
        report["licenses"][f"{name}=={version}"] = {
            "approved": approved,
            "details": sorted({detail for _, detail in checks}),
            "license_files": sorted({
                file for metadata in declarations.values() for file in metadata.get_all("License-File", [])
            }),
            "declarations": [json.loads(declared) for declared in sorted(declarations)],
            "links": {
                label: url for metadata in declarations.values()
                for label, url in license_links(name, version, metadata, policy).items()
            },
            "wheels": license_wheels[(name, version)],
        }
        if not approved:
            report["errors"].append(f"{name}=={version}: license rejected; see Package licenses")
    for pin in pins:
        files = index.files(pin.name, pin.version)
        relevant = {file["digests"]["sha256"] for file in files
                    if any(wheel_rank(file, target, python) is not None for target in TARGETS)}
        known = {file["digests"]["sha256"] for file in files}
        if pin.hashes - known:
            report["errors"].append(f"{pin.path.name}: {pin.name}: hashes not found in pinned PyPI release")
        excess = pin.hashes - relevant
        if excess:
            report["hashes"].append(
                f"{pin.path.name}: {pin.name}: {len(excess)} source/incompatible/yanked hashes (not installable)")
    return report


def compare(base: dict, current: dict) -> dict:
    result = {}
    for target, report in current["targets"].items():
        old = base["targets"][target]
        previous = {tuple(edge) for edge in old["edges"]}
        now = {tuple(edge) for edge in report["edges"]}
        result[target] = {
            "added": sorted(now - previous), "removed": sorted(previous - now),
            "packages_added": sorted(set(report["packages"]) - set(old["packages"])),
            "packages_removed": sorted(set(old["packages"]) - set(report["packages"])),
        }
    return result


def check_pip(root: Path, output: Path) -> list[str]:
    """Ask pip itself to resolve the full environment without installing anything."""
    required_python = tuple(int(part) for part in python_version(root).split(".")[:2])
    if sys.version_info[:2] != required_python:
        return [f"Native pip check requires Python {required_python}, running {sys.version_info[:2]}"]
    paths = sorted((root / "SuperBuild" / "Python").glob("*requirements.txt"))
    pins = [pin for path in paths for pin in read_pins(path)]
    with TemporaryDirectory() as tmp:
        requirements = Path(tmp) / "requirements.txt"
        requirements.write_text("".join(path.read_text(encoding="utf-8") + "\n" for path in paths),
                                encoding="utf-8")
        result = subprocess.run(
            [sys.executable, "-m", "pip", "--isolated", "install", "--index-url", "https://pypi.org/simple",
             "--dry-run", "--ignore-installed", "--only-binary=:all:", "--require-hashes",
             "--report", str(output), "-r", str(requirements)],
            capture_output=True, text=True, check=False,
        )
        if result.returncode:
            return [f"Native pip resolution failed:\n{result.stdout}\n{result.stderr}"]
    resolution = json.loads(output.read_text(encoding="utf-8"))
    expected = {}
    for pin in pins:
        if pin.requirement.marker is None or pin.requirement.marker.evaluate():
            version = Version(pin.version)
            if pin.name in expected and expected[pin.name] != version:
                return [f"Native pip: conflicting pins for {pin.name}"]
            expected[pin.name] = version
    actual = {canonicalize_name(item["metadata"]["name"]): Version(item["metadata"]["version"])
              for item in resolution["install"]}
    if actual != expected:
        return [f"Native pip resolved a different environment: expected {expected}, got {actual}"]
    return []


def normalize(root: Path, index: Index) -> None:
    python = python_version(root)
    updates = {}
    for path in sorted((root / "SuperBuild" / "Python").glob("*requirements.txt")):
        lines = path.read_text(encoding="utf-8").splitlines(keepends=True)
        for pin in reversed(read_pins(path)):
            files = [file for file in index.files(pin.name, pin.version)
                     if any(wheel_rank(file, target, python) is not None for target in TARGETS)]
            for target in TARGETS:
                if target.required and not any(wheel_rank(file, target, python) is not None for file in files):
                    raise ValidationError(f"{pin.name}: cannot normalize: missing {target.name} wheel")
            text = f"  {pin.requirement}"
            hashes = sorted({file["digests"]["sha256"] for file in files})
            requirement = text + " " + (" \\\n" + " " * (len(text) + 1)).join(
                f"--hash=sha256:{digest}" for digest in hashes) + "\n"
            start = pin.start
            while start and re.match(r"\s*#\s+(Hashes correspond to the following packages:|- .*"
                                     r"\.(?:whl|tar\.gz|zip).*)$", lines[start - 1].rstrip()):
                start -= 1
            lines[start:pin.end] = [requirement]
        updates[path] = "".join(lines)
    # Validate every release before modifying any manifest.
    for path, contents in updates.items():
        path.write_text(contents, encoding="utf-8")


def markdown(report: dict) -> str:
    lines = ["# Python dependency validation", "", f"CPython {report['python']}; macOS 14; Linux glibc 2.28.", ""]
    for section in ("errors", "warnings", "hashes"):
        lines.extend([f"## {section.title()}", ""])
        lines.extend(f"- {message}" for message in report[section])
        if not report[section]:
            lines.append("None.")
        lines.append("")
    lines.extend(["## Package licenses", "",
                  "Review links identify their source; a project homepage is not verified license text.",
                  "Declared license files are inside the linked wheels. Full declarations are in the JSON report.", "",
                  "| Package | Status | Declared license / review | License files | Review links |",
                  "|---|---|---|---|---|"])
    for package, license_data in sorted(report["licenses"].items()):
        detail = "; ".join(license_data["details"]).replace("|", "/").replace("\n", " ")
        files = ", ".join(license_data["license_files"]).replace("|", "/").replace("\n", " ")
        status = "Approved" if license_data["approved"] else "Rejected / review required"
        links = " / ".join(
            f"[{label.replace('|', '/').replace('[', '').replace(']', '')}]"
            f"(<{quote(url, safe=':/?#=&%+@,;~-._')}>)"
            for label, url in sorted(license_data["links"].items())
        )
        wheels = " / ".join(
            f"[Wheel {number}](<{quote(url, safe=':/?#=&%+@,;~-._')}>)"
            for number, url in enumerate(sorted(set(license_data["wheels"].values())), 1)
        )
        lines.append(f"| {package} | {status} | {detail} | {files} | {links} / {wheels} |")
    lines.extend(["", "License results cover selected wheels; packages with no usable wheel cannot be assessed.", ""])
    for target, data in report["targets"].items():
        lines.extend([f"## {target}", "", "| Package | Version | Wheel |",
                      "|---|---|---|"])
        for name, package in sorted(data["packages"].items()):
            lines.append(f"| {name} | {package['version']} | {package['wheel']} |")
        lines.extend(["", "Packages with no incoming dependency edges (NOT automatic removal recommendations):",
                      ", ".join(data["removal_candidates"]), ""])
    if "changes" in report:
        lines.extend(["## Dependency changes against PR base", ""])
        for target, changes in report["changes"].items():
            lines.extend([f"### {target}", ""])
            for category, items in changes.items():
                lines.append(f"- {category}: {json.dumps(items)}")
        if report.get("base_issues"):
            lines.extend(["", ("Base validation also reported issues. Diffs involving missing wheels "
                               "may be incomplete; see base_issues in the JSON report.")])
    if "pip_checked" in report:
        lines.extend(["", f"Native pip exact-environment dry run performed: {report['pip_checked']}."])
    return "\n".join(lines) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--base-root", type=Path)
    parser.add_argument("--policy", type=Path, default=Path(__file__).with_name("python_dependency_policy.json"))
    parser.add_argument("--cache", type=Path, required=True)
    parser.add_argument("--output", type=Path, default=Path("python-dependency-report.json"))
    parser.add_argument("--normalize-hashes", action="store_true")
    parser.add_argument("--check-pip", action="store_true",
                        help="Also dry-run pip on the current host and compare its exact resolved versions")
    args = parser.parse_args()
    index = Index(args.cache)
    try:
        if args.normalize_hashes:
            normalize(args.root.resolve(), index)
            print("Normalized wheel hashes without changing versions. Review the diff before committing.")
            return 0
        policy = json.loads(args.policy.read_text(encoding="utf-8"))
        report = validate(args.root.resolve(), index, policy)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        if args.check_pip:
            report["errors"].extend(check_pip(args.root.resolve(), args.output.with_suffix(".pip.json")))
            report["pip_checked"] = True
        if args.base_root:
            base = validate(args.base_root.resolve(), index, policy)
            report["changes"] = compare(base, report)
            report["base_issues"] = base["errors"] + base["warnings"]
        args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        args.output.with_suffix(".md").write_text(markdown(report), encoding="utf-8")
        print(markdown(report))
        return int(bool(report["errors"]))
    except (ValidationError, ValueError, OSError, URLError, ExpressionError) as error:
        print(f"Dependency validation failed: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
