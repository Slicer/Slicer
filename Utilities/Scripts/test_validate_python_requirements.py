from email import message_from_string
import hashlib
import json
from pathlib import Path
from tempfile import TemporaryDirectory
import unittest
from unittest.mock import Mock, patch

import validate_python_requirements as validator


HASH = "a" * 64


class FakeIndex:
    def __init__(self):
        self.requirements = {}
        self.licenses = {}

    def files(self, name, version):
        return [{
            "filename": f"{name}-{version}-py3-none-any.whl", "url": f"https://example.org/{name}",
            "digests": {"sha256": HASH},
        }]

    def metadata(self, name, filename, url, digest):
        version = filename.split("-")[1]
        requirements = "".join(f"Requires-Dist: {text}\n" for text in self.requirements.get(name, []))
        return message_from_string(
            f"Name: {name}\nVersion: {version}\n"
            f"License-Expression: {self.licenses.get(name, 'MIT')}\n{requirements}")


class RequirementsTest(unittest.TestCase):
    def setUp(self):
        self.tmp = TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.manifests = self.root / "SuperBuild" / "Python"
        self.manifests.mkdir(parents=True)
        (self.root / "CMakeLists.txt").write_text('set(Slicer_REQUIRED_PYTHON_VERSION "3.12.10")\n')
        self.write_group("first", ["foo==1.0", "bar==2.0"])
        self.index = FakeIndex()
        self.policy = {"allowed_licenses": ["MIT", "Apache-2.0"], "license_exceptions": {}}

    def write_group(self, name, requirements, dependencies=""):
        (self.manifests / f"python-{name}-requirements.txt").write_text(
            "".join(f"{requirement} --hash=sha256:{HASH}\n" for requirement in requirements))
        (self.root / "SuperBuild" / f"External_python-{name}.cmake").write_text(
            f"set(proj python-{name})\n"
            "set(${proj}_DEPENDENCIES " + dependencies + ")\n"
            f'"${{CMAKE_CURRENT_LIST_DIR}}/Python/python-{name}-requirements.txt"\n')

    def validate(self):
        return validator.validate(self.root, self.index, self.policy)

    def test_exact_compatible_environment(self):
        self.index.requirements["foo"] = ["bar>=2,<3"]
        report = self.validate()
        self.assertEqual(report["errors"], [])
        self.assertEqual(report["targets"]["windows-x64"]["edges"], [["foo", "bar<3,>=2"]])

    def test_conflicting_duplicate_pins(self):
        self.write_group("second", ["bar==3.0"])
        self.assertTrue(any("conflicting pins" in error for error in self.validate()["errors"]))

    def test_incompatible_dependency(self):
        self.index.requirements["foo"] = ["bar<2"]
        self.assertTrue(any("pinned 2.0" in error for error in self.validate()["errors"]))

    def test_new_unpinned_dependency(self):
        self.index.requirements["foo"] = ["missing>=1"]
        self.assertTrue(any("no exact PyPI pin" in error for error in self.validate()["errors"]))

    def test_cmake_installation_order(self):
        self.write_group("first", ["foo==1.0"])
        self.write_group("second", ["bar==2.0"])
        self.index.requirements["foo"] = ["bar>=2"]
        self.assertTrue(any("CMake dependency closure" in error for error in self.validate()["errors"]))
        self.write_group("first", ["foo==1.0"], "python-second")
        self.assertEqual(self.validate()["errors"], [])

    def test_target_markers_do_not_use_host(self):
        self.index.requirements["foo"] = ["missing; sys_platform == 'win32'"]
        report = self.validate()
        self.assertTrue(report["targets"]["windows-x64"]["issues"])
        self.assertEqual(report["targets"]["linux-x64"]["issues"], [])

    def test_transitive_extra_activation(self):
        self.index.requirements["foo"] = ["bar[feature]"]
        self.index.requirements["bar"] = ["missing; extra == 'feature'"]
        self.assertTrue(any("missing" in error for error in self.validate()["errors"]))

    def test_hash_required_for_every_occurrence(self):
        self.write_group("second", ["foo==1.0"])
        path = self.manifests / "python-second-requirements.txt"
        path.write_text(path.read_text().replace(HASH, "b" * 64))
        self.assertTrue(any("hash-approved" in error for error in self.validate()["errors"]))

    def test_license_copyleft_and_or(self):
        self.assertTrue(validator.permissive("MIT OR GPL-3.0-only", ["MIT"]))
        self.assertFalse(validator.permissive("MIT AND GPL-3.0-only", ["MIT"]))
        self.assertFalse(validator.permissive("GPL-3.0-only WITH GCC-exception-3.1", ["MIT"]))
        self.index.licenses["foo"] = "MPL-2.0"
        self.assertTrue(any("license rejected" in error for error in self.validate()["errors"]))

    def test_unknown_license_fails(self):
        licensed, detail = validator.check_license("foo", "1.0", message_from_string("License: Mystery\n"), self.policy)
        self.assertFalse(licensed)
        self.assertIn("not a precise SPDX identifier or expression", detail)
        self.assertIn("metadata_sha256", detail)

    def test_license_evidence_links_and_raw_declarations(self):
        self.index.licenses["foo"] = "MPL-2.0"
        report = self.validate()
        license_data = report["licenses"]["foo==1.0"]
        self.assertEqual(license_data["declarations"][0]["License-Expression"], ["MPL-2.0"])
        self.assertEqual(license_data["links"]["SPDX MPL-2.0"], "https://spdx.org/licenses/MPL-2.0.html")
        self.assertEqual(license_data["links"]["PyPI release / license metadata"],
                         "https://pypi.org/project/foo/1.0/#license")
        self.assertEqual(license_data["wheels"], {"foo-1.0-py3-none-any.whl": "https://example.org/foo"})
        text = validator.markdown(report)
        self.assertIn("[SPDX MPL-2.0](<https://spdx.org/licenses/MPL-2.0.html>)", text)
        self.assertIn("[Wheel 1](<https://example.org/foo>)", text)

    def test_license_links_use_declared_urls_not_guessed_license_paths(self):
        metadata = message_from_string(
            "License: Custom terms\n"
            "Project-URL: License, https://example.org/license\n"
            "Project-URL: Source, https://github.com/example/project\n"
            "Project-URL: Homepage, javascript:alert(1)\n"
            "Home-page: https://example.org/home\n")
        links = validator.license_links("foo", "1.0", metadata, self.policy)
        self.assertEqual(links["Declared License"], "https://example.org/license")
        self.assertEqual(links["Declared Source"], "https://github.com/example/project")
        self.assertNotIn("Declared Homepage", links)
        self.assertFalse(any("blob/" in url for url in links.values()))
        self.assertFalse(any(label.startswith("SPDX") for label in links))

    def test_license_checked_and_reported_once_per_package(self):
        self.index.licenses["foo"] = "MPL-2.0"
        with patch.object(validator, "check_license", wraps=validator.check_license) as check:
            report = self.validate()
        self.assertEqual(check.call_count, 2)
        self.assertEqual(len([error for error in report["errors"] if "license rejected" in error]), 1)
        self.assertFalse(report["licenses"]["foo==1.0"]["approved"])
        self.assertTrue(all(not data["issues"] for data in report["targets"].values()))
        text = validator.markdown(report)
        self.assertEqual(text.count("MPL-2.0; metadata_sha256="), 1)
        self.assertEqual(text.count("| foo==1.0 |"), 1)
        self.assertNotIn("| Wheel | License |", text)
        for data in report["targets"].values():
            self.assertNotIn("license", data["packages"]["foo"])

    def test_different_wheel_license_declarations_are_not_ignored(self):
        original_metadata = self.index.metadata
        count = 0

        def metadata(name, filename, url, digest):
            nonlocal count
            result = original_metadata(name, filename, url, digest)
            if name == "foo":
                count += 1
                if count == 2:
                    result.replace_header("License-Expression", "MPL-2.0")
            return result

        with patch.object(self.index, "metadata", side_effect=metadata):
            report = self.validate()
        self.assertFalse(report["licenses"]["foo==1.0"]["approved"])
        self.assertEqual(len(report["licenses"]["foo==1.0"]["details"]), 2)
        self.assertEqual(len([error for error in report["errors"] if "license rejected" in error]), 1)

    def test_ambiguous_legacy_text_does_not_get_overridden_by_classifier(self):
        metadata = message_from_string("License: Custom terms apply\nClassifier: License :: OSI Approved :: MIT License\n")
        self.assertFalse(validator.check_license("foo", "1.0", metadata, self.policy)[0])

    def test_reviewed_license_exception_is_version_and_metadata_bound(self):
        metadata = message_from_string("License: Reviewed custom license\n")
        declared = {key: metadata.get_all(key, [])
                    for key in ("License-Expression", "License", "Classifier", "License-File")}
        fingerprint = hashlib.sha256(json.dumps(declared, sort_keys=True).encode()).hexdigest()
        self.policy["license_exceptions"]["foo==1.0"] = {
            "metadata_sha256": fingerprint, "reason": "Test fixture approval",
        }
        self.assertTrue(validator.check_license("foo", "1.0", metadata, self.policy)[0])
        self.assertFalse(validator.check_license("foo", "1.1", metadata, self.policy)[0])
        self.assertFalse(validator.check_license("foo", "1.0", message_from_string("License: Changed\n"), self.policy)[0])

    def test_exception_fingerprint_list_and_review_source(self):
        metadata = message_from_string("License: MPL-2.0\n")
        declared = {key: metadata.get_all(key, [])
                    for key in ("License-Expression", "License", "Classifier", "License-File")}
        fingerprint = hashlib.sha256(json.dumps(declared, sort_keys=True).encode()).hexdigest()
        exception = {
            "metadata_sha256": ["b" * 64, fingerprint],
            "reason": "Explicit maintainer approval; license obligations remain applicable",
            "source": "https://spdx.org/licenses/MPL-2.0.html",
        }
        self.policy["license_exceptions"]["foo==1.0"] = exception
        approved, detail = validator.check_license("foo", "1.0", metadata, self.policy)
        self.assertTrue(approved)
        self.assertIn(exception["reason"], detail)
        self.assertEqual(validator.license_links("foo", "1.0", metadata, self.policy)["Exception review source"],
                         exception["source"])
        self.assertFalse(validator.check_license("bar", "1.0", metadata, self.policy)[0])
        self.assertFalse(validator.check_license("foo", "1.1", metadata, self.policy)[0])
        exception["metadata_sha256"] = ["b" * 64]
        self.assertFalse(validator.check_license("foo", "1.0", metadata, self.policy)[0])

    def test_verified_legacy_alias_is_version_metadata_and_policy_bound(self):
        metadata = message_from_string(
            "License: Apache\nClassifier: License :: OSI Approved :: Apache Software License\n")
        declared = {key: metadata.get_all(key, [])
                    for key in ("License-Expression", "License", "Classifier", "License-File")}
        alias = {
            "expression": "Apache-2.0",
            "metadata_sha256": hashlib.sha256(json.dumps(declared, sort_keys=True).encode()).hexdigest(),
            "source": "https://github.com/SimpleITK/SimpleITK/blob/v2.5.5/LICENSE",
        }
        self.policy["legacy_license_aliases"] = {"simpleitk==2.5.5": alias}
        approved, detail = validator.check_license("simpleitk", "2.5.5", metadata, self.policy)
        self.assertTrue(approved)
        self.assertIn("Apache-2.0 (verified legacy alias 'Apache'", detail)
        self.assertIn(alias["source"], detail)
        self.assertFalse(validator.check_license("other", "2.5.5", metadata, self.policy)[0])
        self.assertFalse(validator.check_license("simpleitk", "2.5.6", metadata, self.policy)[0])
        metadata["License-Expression"] = "GPL-3.0-only"
        self.assertFalse(validator.check_license("simpleitk", "2.5.5", metadata, self.policy)[0])
        del metadata["License-Expression"]
        self.policy["allowed_licenses"] = ["MIT"]
        self.assertFalse(validator.check_license("simpleitk", "2.5.5", metadata, self.policy)[0])
        self.policy["allowed_licenses"] = ["Apache-2.0"]
        alias["source"] = ""
        self.assertFalse(validator.check_license("simpleitk", "2.5.5", metadata, self.policy)[0])

    def test_dependency_removal_is_reported_not_automatically_deleted(self):
        self.index.requirements["foo"] = ["bar>=2"]
        base = self.validate()
        self.index.requirements["foo"] = []
        current = self.validate()
        changes = validator.compare(base, current)
        self.assertEqual(changes["windows-x64"]["removed"], [("foo", "bar>=2")])
        self.assertIn("bar", current["targets"]["windows-x64"]["packages"])

    def test_exact_pins_and_hashes_mandatory(self):
        path = self.manifests / "bad.txt"
        for text in ["foo>=1", "foo==1.0", f"foo==1.* --hash=sha256:{HASH}"]:
            path.write_text(text)
            with self.assertRaises(validator.ValidationError):
                validator.read_pins(path)

    def test_normalization_preserves_pins_and_removes_source_hashes(self):
        path = self.manifests / "python-first-requirements.txt"
        path.write_text("# [foo]\n# Preserve this explanation\n"
                        "# Hashes correspond to the following packages:\n"
                        "#  - foo-0.9-cp311-cp311-win_amd64.whl\n"
                        f"foo==1.0 --hash=sha256:{HASH} \\\n --hash=sha256:{'b' * 64}\n# [/foo]\n")
        validator.normalize(self.root, self.index)
        self.assertEqual(validator.read_pins(path)[0].hashes, {HASH})
        self.assertEqual(validator.read_pins(path)[0].version, "1.0")
        self.assertIn("# [/foo]", path.read_text())
        self.assertIn("# Preserve this explanation", path.read_text())
        self.assertNotIn("Hashes correspond", path.read_text())
        self.assertNotIn(".whl", path.read_text())
        once = path.read_text()
        validator.normalize(self.root, self.index)
        self.assertEqual(path.read_text(), once)

    def test_advisory_wheel_absence_does_not_block_required_targets(self):
        original_files = self.index.files

        def files(name, version):
            file = original_files(name, version)[0]
            file["filename"] = f"{name}-{version}-py3-none-manylinux_2_28_x86_64.whl"
            return [file]

        self.index.files = files
        with patch.object(validator, "TARGETS", (validator.TARGETS[3], validator.TARGETS[4])):
            report = self.validate()
        self.assertEqual(report["errors"], [])
        self.assertTrue(report["warnings"])

    def test_normalization_does_not_write_when_required_wheel_is_missing(self):
        before = {path: path.read_bytes() for path in self.manifests.glob("*.txt")}
        with patch.object(self.index, "files", return_value=[]):
            with self.assertRaises(validator.ValidationError):
                validator.normalize(self.root, self.index)
        self.assertEqual({path: path.read_bytes() for path in before}, before)

    def test_native_pip_exact_version_and_no_install(self):
        output = self.root / "pip-report.json"

        def resolve(command, **kwargs):
            self.assertIn("--dry-run", command)
            self.assertIn("--ignore-installed", command)
            self.assertIn("--only-binary=:all:", command)
            self.assertIn("--require-hashes", command)
            output.write_text(json.dumps({"install": [
                {"metadata": {"name": "foo", "version": "1.0"}},
                {"metadata": {"name": "bar", "version": "2.0"}},
            ]}))
            return Mock(returncode=0)

        with patch.object(validator.subprocess, "run", side_effect=resolve):
            self.assertEqual(validator.check_pip(self.root, output), [])
        with patch.object(validator.subprocess, "run", return_value=Mock(returncode=0)):
            output.write_text(json.dumps({"install": [{"metadata": {"name": "foo", "version": "9.0"}}]}))
            self.assertTrue(validator.check_pip(self.root, output))

    def test_cached_download_hash_is_verified(self):
        index = validator.Index(self.root / "cache")
        url = "https://files.pythonhosted.org/example.whl"
        (index.cache_dir / hashlib.sha256(url.encode()).hexdigest()).write_bytes(b"tampered")
        with self.assertRaises(validator.ValidationError):
            index.fetch(url, HASH)
        with self.assertRaises(validator.ValidationError):
            index.fetch("https://example.com/untrusted.whl")


class WheelTest(unittest.TestCase):
    def file(self, filename, **kwargs):
        return {"filename": filename, **kwargs}

    def test_python312_abi3_and_pure_wheels(self):
        target = validator.TARGETS[0]
        for filename in ["foo-1.0-py3-none-any.whl", "foo-1.0-cp39-abi3-win_amd64.whl",
                         "foo-1.0-cp312-cp312-win_amd64.whl"]:
            self.assertIsNotNone(validator.wheel_rank(self.file(filename), target, "3.12.10"))
        for filename in ["foo-1.0-cp313-cp313-win_amd64.whl", "foo-1.0-cp313-abi3-win_amd64.whl",
                         "foo-1.0-cp312-cp312-win_arm64.whl", "foo-1.0.tar.gz"]:
            self.assertIsNone(validator.wheel_rank(self.file(filename), target, "3.12.10"))
        self.assertIsNone(validator.wheel_rank(self.file("foo-1.0-py3-none-any.whl", requires_python=">=3.13"),
                                              target, "3.12.10"))
        self.assertIsNone(validator.wheel_rank(self.file("foo-1.0-py3-none-any.whl", yanked=True),
                                              target, "3.12.10"))

    def test_os_baselines(self):
        linux = next(target for target in validator.TARGETS if target.name == "linux-x64")
        mac = next(target for target in validator.TARGETS if target.name == "macos-arm64")
        self.assertIsNotNone(validator.wheel_rank(self.file("foo-1.0-cp312-cp312-manylinux_2_28_x86_64.whl"),
                                                 linux, "3.12.10"))
        self.assertIsNone(validator.wheel_rank(self.file("foo-1.0-cp312-cp312-manylinux_2_34_x86_64.whl"),
                                              linux, "3.12.10"))
        self.assertIsNotNone(validator.wheel_rank(self.file("foo-1.0-cp312-cp312-macosx_11_0_universal2.whl"),
                                                 mac, "3.12.10"))
        self.assertIsNone(validator.wheel_rank(self.file("foo-1.0-cp312-cp312-macosx_15_0_arm64.whl"),
                                              mac, "3.12.10"))


if __name__ == "__main__":
    unittest.main()
