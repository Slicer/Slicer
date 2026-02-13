"""Unit tests for pip-related functions in slicer.util.

Tests for: load_requirements, pip_check, pip_ensure, pip_install (with progress
and non-blocking modes), and _PipProgressDialog.
"""

import importlib.metadata
import os
import tempfile
import threading
import unittest
import unittest.mock

from packaging.requirements import Requirement

import slicer
import slicer.util


class LoadRequirementsTest(unittest.TestCase):
    """Tests for slicer.util.load_requirements."""

    def test_simple_requirements(self):
        """Test loading simple requirements."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False) as f:
            f.write("numpy>=1.20\n")
            f.write("scipy>=1.0\n")
            temp_path = f.name

        try:
            reqs = slicer.util.load_requirements(temp_path)
            self.assertEqual(len(reqs), 2)
            self.assertEqual(reqs[0].name, "numpy")
            self.assertEqual(reqs[1].name, "scipy")
        finally:
            os.unlink(temp_path)

    def test_skip_comments(self):
        """Test that comments are skipped."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False) as f:
            f.write("# This is a comment\n")
            f.write("numpy>=1.0\n")
            f.write("# Another comment\n")
            temp_path = f.name

        try:
            reqs = slicer.util.load_requirements(temp_path)
            self.assertEqual(len(reqs), 1)
            self.assertEqual(reqs[0].name, "numpy")
        finally:
            os.unlink(temp_path)

    def test_skip_empty_lines(self):
        """Test that empty lines are skipped."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False) as f:
            f.write("numpy>=1.0\n")
            f.write("\n")
            f.write("   \n")
            f.write("scipy>=1.0\n")
            temp_path = f.name

        try:
            reqs = slicer.util.load_requirements(temp_path)
            self.assertEqual(len(reqs), 2)
        finally:
            os.unlink(temp_path)

    def test_skip_pip_options(self):
        """Test that pip options (-r, -c, --index-url) are skipped."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False) as f:
            f.write("numpy>=1.0\n")
            f.write("-r other_requirements.txt\n")
            f.write("-c constraints.txt\n")
            f.write("--index-url https://example.com\n")
            f.write("scipy>=1.0\n")
            temp_path = f.name

        try:
            reqs = slicer.util.load_requirements(temp_path)
            self.assertEqual(len(reqs), 2)
            names = [r.name for r in reqs]
            self.assertEqual(names, ["numpy", "scipy"])
        finally:
            os.unlink(temp_path)

    def test_returns_requirement_objects(self):
        """Test that returned objects are Requirement instances."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False) as f:
            f.write("numpy>=1.20,<2.0\n")
            temp_path = f.name

        try:
            reqs = slicer.util.load_requirements(temp_path)
            self.assertEqual(len(reqs), 1)
            self.assertIsInstance(reqs[0], Requirement)
            # Check specifier contains expected constraints (order may vary)
            self.assertTrue(reqs[0].specifier.contains("1.25"))
            self.assertFalse(reqs[0].specifier.contains("1.19"))
            self.assertFalse(reqs[0].specifier.contains("2.1"))
        finally:
            os.unlink(temp_path)


class PipCheckTest(unittest.TestCase):
    """Tests for slicer.util.pip_check."""

    def test_satisfied_requirement(self):
        """Test that installed package is detected as satisfied."""
        # numpy is installed in Slicer
        req = Requirement("numpy>=1.0")
        self.assertTrue(slicer.util.pip_check(req))

    def test_unsatisfied_version(self):
        """Test that version too high is detected as unsatisfied."""
        req = Requirement("numpy>=99999.0")
        self.assertFalse(slicer.util.pip_check(req))

    def test_missing_package(self):
        """Test that missing package is detected."""
        req = Requirement("nonexistent-package-xyz123>=1.0")
        self.assertFalse(slicer.util.pip_check(req))

    def test_list_all_satisfied(self):
        """Test checking a list of requirements - all satisfied."""
        reqs = [
            Requirement("numpy>=1.0"),
            Requirement("scipy>=1.0"),
        ]
        self.assertTrue(slicer.util.pip_check(reqs))

    def test_list_one_missing(self):
        """Test checking a list of requirements - one missing."""
        reqs = [
            Requirement("numpy>=1.0"),
            Requirement("nonexistent-package-xyz123>=1.0"),
        ]
        self.assertFalse(slicer.util.pip_check(reqs))

    def test_marker_not_applicable(self):
        """Test that inapplicable marker is considered satisfied."""
        # This marker will never apply
        req = Requirement('somepackage>=1.0; sys_platform == "nonexistent_platform"')
        self.assertTrue(slicer.util.pip_check(req))

    def test_empty_list(self):
        """Test that empty list returns True."""
        self.assertTrue(slicer.util.pip_check([]))


class PipCheckMockedTest(unittest.TestCase):
    """Tests for slicer.util.pip_check using mocks for isolation."""

    def test_version_specifier_not_equal(self):
        """Test != version specifier."""
        with unittest.mock.patch("importlib.metadata.version", return_value="1.24.0"):
            req = Requirement("numpy>=1.20,!=1.24.0")
            self.assertFalse(slicer.util.pip_check(req))

        with unittest.mock.patch("importlib.metadata.version", return_value="1.23.0"):
            req = Requirement("numpy>=1.20,!=1.24.0")
            self.assertTrue(slicer.util.pip_check(req))

    def test_extras_satisfied(self):
        """Test that extras dependencies are checked."""
        def mock_version(name):
            versions = {"requests": "2.28.0", "pysocks": "1.7.0"}
            if name.lower() in versions:
                return versions[name.lower()]
            raise importlib.metadata.PackageNotFoundError()

        def mock_requires(name):
            if name.lower() == "requests":
                return ['PySocks>=1.5.6; extra == "socks"']
            return []

        with unittest.mock.patch("importlib.metadata.version", side_effect=mock_version):
            with unittest.mock.patch("importlib.metadata.requires", side_effect=mock_requires):
                req = Requirement("requests[socks]>=2.0")
                self.assertTrue(slicer.util.pip_check(req))

    def test_extras_missing(self):
        """Test that missing extras dependency is detected."""
        def mock_version(name):
            if name.lower() == "requests":
                return "2.28.0"
            raise importlib.metadata.PackageNotFoundError()

        def mock_requires(name):
            if name.lower() == "requests":
                return ['PySocks>=1.5.6; extra == "socks"']
            return []

        with unittest.mock.patch("importlib.metadata.version", side_effect=mock_version):
            with unittest.mock.patch("importlib.metadata.requires", side_effect=mock_requires):
                req = Requirement("requests[socks]>=2.0")
                self.assertFalse(slicer.util.pip_check(req))


class PipEnsureTest(unittest.TestCase):
    """Tests for slicer.util.pip_ensure."""

    def test_all_satisfied_no_install(self):
        """Test that no installation happens when all satisfied."""
        reqs = [
            Requirement("numpy>=1.0"),
            Requirement("scipy>=1.0"),
        ]
        # Should return without error or calling pip_install
        with unittest.mock.patch("slicer.util._pip_install_simple") as mock_install:
            slicer.util.pip_ensure(reqs, prompt_install=False)
            mock_install.assert_not_called()

    def test_skip_in_testing_mode(self):
        """Test that installation is skipped in testing mode."""
        reqs = [Requirement("nonexistent-package-xyz123>=1.0")]

        # Note: Can't mock slicer.app.testingEnabled() because it's a Qt slot
        if slicer.app.testingEnabled():
            with unittest.mock.patch("slicer.util._pip_install_simple") as mock_install:
                # Should not raise, should not install
                slicer.util.pip_ensure(reqs, prompt_install=False, skip_in_testing=True)
                mock_install.assert_not_called()
        else:
            self.skipTest("Not in testing mode")

    def test_calls_pip_install_for_missing(self):
        """Test that pip_install is called for missing packages."""
        reqs = [Requirement("nonexistent-package-xyz123>=1.0")]

        # Force skip_in_testing=False to test the install path
        if slicer.app.testingEnabled():
            with unittest.mock.patch("slicer.util._pip_install_simple") as mock_install:
                slicer.util.pip_ensure(reqs, prompt_install=False, skip_in_testing=False)
                mock_install.assert_called_once()
                # Check that the requirement string was passed
                call_args = mock_install.call_args
                self.assertIn("nonexistent-package-xyz123>=1.0", call_args[0][0])
        else:
            self.skipTest("Not in testing mode - would show dialog")


class PipInstallNonBlockingTest(unittest.TestCase):
    """Tests for slicer.util.pip_install non-blocking mode."""

    def test_nonblocking_returns_immediately(self):
        """Test that non-blocking mode returns immediately."""
        import qt

        completed = threading.Event()
        log_lines = []

        def on_log(line):
            log_lines.append(line)

        def on_complete(return_code):
            completed.set()

        # Use --version which is quick
        slicer.util._executePythonModule(
            "pip", ["--version"],
            blocking=False,
            logCallback=on_log,
            completedCallback=on_complete,
        )

        # Wait for completion with processEvents
        timeout_ms = 10000
        start = qt.QTime.currentTime()
        while not completed.is_set():
            qt.QCoreApplication.processEvents()
            if start.msecsTo(qt.QTime.currentTime()) > timeout_ms:
                self.fail("Timeout waiting for non-blocking completion")
            qt.QThread.msleep(50)

        self.assertTrue(len(log_lines) > 0)

    def test_blocking_mode_unchanged(self):
        """Test that blocking mode still works (backward compatibility)."""
        # pip --version should succeed without callbacks
        try:
            slicer.util._executePythonModule("pip", ["--version"])
        except Exception as e:
            self.fail(f"Blocking mode failed: {e}")


class PipUninstallNonBlockingTest(unittest.TestCase):
    """Tests for slicer.util.pip_uninstall non-blocking mode."""

    def test_nonblocking_with_nonexistent_package(self):
        """Test that non-blocking pip_uninstall works with callbacks.

        Uses a nonexistent package to avoid actually uninstalling anything.
        The command will fail (return code != 0) but callbacks should work.
        """
        import qt

        completed = threading.Event()
        log_lines = []
        result = {"return_code": None}

        def on_log(line):
            log_lines.append(line)

        def on_complete(return_code):
            result["return_code"] = return_code
            completed.set()

        # Try to uninstall a nonexistent package
        slicer.util.pip_uninstall(
            "nonexistent-package-xyz123",
            blocking=False,
            logCallback=on_log,
            completedCallback=on_complete,
        )

        # Wait for completion with processEvents
        timeout_ms = 10000
        start = qt.QTime.currentTime()
        while not completed.is_set():
            qt.QCoreApplication.processEvents()
            if start.msecsTo(qt.QTime.currentTime()) > timeout_ms:
                self.fail("Timeout waiting for non-blocking completion")
            qt.QThread.msleep(50)

        # Verify callbacks were called
        self.assertIsNotNone(result["return_code"])
        # Nonexistent package should produce some output
        self.assertTrue(len(log_lines) > 0)


class PipProgressDialogTest(unittest.TestCase):
    """Tests for slicer.util._PipProgressDialog."""

    def test_append_log(self):
        """Test appendLog adds lines correctly."""
        dialog = slicer.util._PipProgressDialog(requester="Test")
        dialog.appendLog("Line 1")
        dialog.appendLog("Line 2")

        log = dialog.getFullLog()
        self.assertIn("Line 1", log)
        self.assertIn("Line 2", log)

    def test_get_full_log(self):
        """Test getFullLog returns all lines."""
        dialog = slicer.util._PipProgressDialog(requester="Test")
        dialog.appendLog("First")
        dialog.appendLog("Second")
        dialog.appendLog("Third")

        log = dialog.getFullLog()
        self.assertEqual(log, "First\nSecond\nThird")


class IntegrationTest(unittest.TestCase):
    """Integration tests combining multiple functions."""

    def test_load_check_ensure_workflow(self):
        """Test the complete workflow: load -> check -> ensure."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False) as f:
            f.write("numpy>=1.0\n")
            f.write("scipy>=1.0\n")
            temp_path = f.name

        try:
            # Load
            reqs = slicer.util.load_requirements(temp_path)
            self.assertEqual(len(reqs), 2)

            # Check
            satisfied = slicer.util.pip_check(reqs)
            self.assertTrue(satisfied)

            # Ensure (should be no-op since all satisfied)
            with unittest.mock.patch("slicer.util._pip_install_simple") as mock_install:
                slicer.util.pip_ensure(reqs, prompt_install=False)
                mock_install.assert_not_called()
        finally:
            os.unlink(temp_path)


class ConstraintsTest(unittest.TestCase):
    """Tests for constraints file support in pip functions."""

    def test_pip_install_with_constraints_builds_correct_args(self):
        """Test that pip_install passes -c flag when constraints provided."""
        with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False) as f:
            f.write("numpy<2.0\n")
            constraints_path = f.name

        try:
            with unittest.mock.patch("slicer.util._executePythonModule") as mock_exec:
                slicer.util.pip_install("scipy", constraints=constraints_path)

                mock_exec.assert_called_once()
                call_args = mock_exec.call_args
                args = call_args[0][1]  # Second positional arg is the args list

                self.assertIn("-c", args)
                self.assertIn(constraints_path, args)
                # -c should come after install and the package
                c_index = args.index("-c")
                self.assertGreater(c_index, 0)
                self.assertEqual(args[c_index + 1], constraints_path)
        finally:
            os.unlink(constraints_path)

    def test_pip_install_without_constraints_no_c_flag(self):
        """Test that pip_install does not pass -c flag when constraints is None."""
        with unittest.mock.patch("slicer.util._executePythonModule") as mock_exec:
            slicer.util.pip_install("scipy")

            mock_exec.assert_called_once()
            call_args = mock_exec.call_args
            args = call_args[0][1]

            self.assertNotIn("-c", args)

    def test_pip_ensure_passes_constraints(self):
        """Test that pip_ensure passes constraints to pip_install."""
        reqs = [Requirement("nonexistent-package-xyz123>=1.0")]

        with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False) as f:
            f.write("numpy<2.0\n")
            constraints_path = f.name

        try:
            if slicer.app.testingEnabled():
                with unittest.mock.patch("slicer.util._pip_install_simple") as mock_install:
                    slicer.util.pip_ensure(
                        reqs,
                        constraints=constraints_path,
                        prompt_install=False,
                        skip_in_testing=False,
                    )

                    mock_install.assert_called_once()
                    # Check that constraints was passed as second positional arg
                    call_args = mock_install.call_args
                    self.assertEqual(call_args[0][1], constraints_path)
            else:
                self.skipTest("Not in testing mode - would show dialog")
        finally:
            os.unlink(constraints_path)

    def test_load_requirements_can_load_constraints_file(self):
        """Test that load_requirements works with constraints file format."""
        # Constraints files have the same format as requirements files
        with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False) as f:
            f.write("numpy<2.0\n")
            f.write("scipy>=1.0,<2.0\n")
            f.write("# This is a constraint comment\n")
            temp_path = f.name

        try:
            reqs = slicer.util.load_requirements(temp_path)
            self.assertEqual(len(reqs), 2)
            self.assertEqual(reqs[0].name, "numpy")
            self.assertEqual(reqs[1].name, "scipy")
        finally:
            os.unlink(temp_path)

    def test_pip_install_constraints_with_pathlib_path(self):
        """Test that pip_install accepts pathlib.Path for constraints."""
        from pathlib import Path

        with tempfile.NamedTemporaryFile(mode="w", suffix=".txt", delete=False) as f:
            f.write("numpy<2.0\n")
            constraints_path = Path(f.name)

        try:
            with unittest.mock.patch("slicer.util._executePythonModule") as mock_exec:
                slicer.util.pip_install("scipy", constraints=constraints_path)

                mock_exec.assert_called_once()
                call_args = mock_exec.call_args
                args = call_args[0][1]

                self.assertIn("-c", args)
                # Path should be converted to string
                self.assertIn(str(constraints_path), args)
        finally:
            constraints_path.unlink()


class GetInstalledVersionsTest(unittest.TestCase):
    """Tests for slicer.util._get_installed_versions."""

    def test_returns_dict(self):
        """Test that _get_installed_versions returns a dict."""
        versions = slicer.util._get_installed_versions()
        self.assertIsInstance(versions, dict)

    def test_contains_known_packages(self):
        """Test that known installed packages appear in the result."""
        versions = slicer.util._get_installed_versions()
        # numpy and scipy are always installed in Slicer
        self.assertIn("numpy", versions)
        self.assertIn("scipy", versions)

    def test_names_are_canonical(self):
        """Test that package names are canonicalized (lowercase, hyphens)."""
        versions = slicer.util._get_installed_versions()
        for name in versions:
            self.assertEqual(name, name.lower(), f"Name not lowercase: {name}")
            self.assertNotIn("_", name, f"Name contains underscore: {name}")


class FindUpdatedImportedPackagesTest(unittest.TestCase):
    """Tests for slicer.util._find_updated_imported_packages."""

    def test_no_changes(self):
        """Test that identical before/after returns empty list."""
        versions = {"numpy": "1.24.0", "scipy": "1.11.0"}
        result = slicer.util._find_updated_imported_packages(versions, versions.copy())
        self.assertEqual(result, [])

    def test_version_changed_but_not_imported(self):
        """Test that changed but non-imported packages are not flagged."""
        before = {"fakepkg-not-imported": "1.0.0"}
        after = {"fakepkg-not-imported": "2.0.0"}

        mock_pkg_dists = {"fakepkg_not_imported": ["fakepkg-not-imported"]}
        with unittest.mock.patch(
            "importlib.metadata.packages_distributions",
            return_value=mock_pkg_dists,
        ):
            result = slicer.util._find_updated_imported_packages(before, after)

        # fakepkg_not_imported is not in sys.modules
        self.assertEqual(result, [])

    def test_version_changed_and_imported(self):
        """Test that changed AND imported packages are flagged."""
        before = {"numpy": "1.24.0"}
        after = {"numpy": "1.26.0"}

        # numpy is already imported in sys.modules in Slicer
        mock_pkg_dists = {"numpy": ["numpy"]}
        with unittest.mock.patch(
            "importlib.metadata.packages_distributions",
            return_value=mock_pkg_dists,
        ):
            result = slicer.util._find_updated_imported_packages(before, after)

        self.assertEqual(len(result), 1)
        self.assertEqual(result[0], ("numpy", "1.24.0", "1.26.0"))

    def test_newly_installed_not_flagged(self):
        """Test that packages absent before install are not flagged."""
        before = {}  # Package was not installed before
        after = {"newpkg": "1.0.0"}

        result = slicer.util._find_updated_imported_packages(before, after)
        self.assertEqual(result, [])

    def test_reinstalled_package_flagged(self):
        """Test that reinstalled packages with modules in sys.modules are flagged.

        Scenario: package was imported, then uninstalled, then reinstalled.
        It won't be in before_versions (uninstalled) but its modules are still
        in sys.modules from the earlier import.
        """
        before = {}  # Package was uninstalled before pip_ensure ran
        after = {"numpy": "1.26.0"}  # Now reinstalled

        # numpy is already in sys.modules in Slicer
        mock_pkg_dists = {"numpy": ["numpy"]}
        with unittest.mock.patch(
            "importlib.metadata.packages_distributions",
            return_value=mock_pkg_dists,
        ):
            result = slicer.util._find_updated_imported_packages(before, after)

        self.assertEqual(len(result), 1)
        self.assertEqual(result[0], ("numpy", None, "1.26.0"))


class PipEnsureRestartPromptTest(unittest.TestCase):
    """Tests for restart prompt logic in pip_ensure."""

    def test_restart_check_called_after_install(self):
        """Test that version snapshots are taken before and after install."""
        reqs = [Requirement("nonexistent-package-xyz123>=1.0")]

        if not slicer.app.testingEnabled():
            self.skipTest("Not in testing mode")

        call_order = []

        def mock_get_versions():
            call_order.append("get_versions")
            return {"numpy": "1.24.0"}

        with unittest.mock.patch("slicer.util._pip_install_simple"):
            with unittest.mock.patch(
                "slicer.util._get_installed_versions",
                side_effect=mock_get_versions,
            ):
                with unittest.mock.patch(
                    "slicer.util._find_updated_imported_packages",
                    return_value=[],
                ) as mock_check:
                    slicer.util.pip_ensure(
                        reqs, prompt_install=False, skip_in_testing=False,
                    )

        # _get_installed_versions should be called twice (before and after)
        self.assertEqual(call_order.count("get_versions"), 2)
        mock_check.assert_called_once()

    def test_no_restart_when_no_updates(self):
        """Test that restart() is not called when nothing was updated."""
        reqs = [Requirement("nonexistent-package-xyz123>=1.0")]

        if not slicer.app.testingEnabled():
            self.skipTest("Not in testing mode")

        with unittest.mock.patch("slicer.util._pip_install_simple"):
            with unittest.mock.patch(
                "slicer.util._get_installed_versions",
                return_value={"numpy": "1.24.0"},
            ):
                with unittest.mock.patch(
                    "slicer.util._find_updated_imported_packages",
                    return_value=[],
                ):
                    with unittest.mock.patch("slicer.util.restart") as mock_restart:
                        slicer.util.pip_ensure(
                            reqs, prompt_install=False, skip_in_testing=False,
                        )
                        mock_restart.assert_not_called()

    def test_restart_prompt_skipped_in_testing_mode(self):
        """Test that restart dialog is skipped in testing mode (only logged)."""
        reqs = [Requirement("nonexistent-package-xyz123>=1.0")]

        if not slicer.app.testingEnabled():
            self.skipTest("Not in testing mode")

        updated = [("numpy", "1.24.0", "1.26.0")]

        with unittest.mock.patch("slicer.util._pip_install_simple"):
            with unittest.mock.patch(
                "slicer.util._get_installed_versions",
                return_value={"numpy": "1.24.0"},
            ):
                with unittest.mock.patch(
                    "slicer.util._find_updated_imported_packages",
                    return_value=updated,
                ):
                    with unittest.mock.patch("slicer.util.restart") as mock_restart:
                        slicer.util.pip_ensure(
                            reqs, prompt_install=False, skip_in_testing=False,
                        )
                        # In testing mode, restart should NOT be called
                        # (dialog is skipped, only logged)
                        mock_restart.assert_not_called()

    def test_prompt_restart_false_skips_check(self):
        """Test that prompt_restart=False skips the version check entirely."""
        reqs = [Requirement("nonexistent-package-xyz123>=1.0")]

        if not slicer.app.testingEnabled():
            self.skipTest("Not in testing mode")

        with unittest.mock.patch("slicer.util._pip_install_simple"):
            with unittest.mock.patch(
                "slicer.util._get_installed_versions",
            ) as mock_get_versions:
                slicer.util.pip_ensure(
                    reqs,
                    prompt_install=False,
                    prompt_restart=False,
                    skip_in_testing=False,
                )
                # Should not be called at all when prompt_restart=False
                mock_get_versions.assert_not_called()


class GetInstalledVersionsSubprocessTest(unittest.TestCase):
    """Test that _get_installed_versions sees packages installed by a pip subprocess.

    Uses pip-install-test — a minimal stub package on PyPI specifically
    designed for testing pip installs.
    """

    _TEST_PKG = "pip-install-test"

    def test_get_installed_versions_sees_newly_installed_package(self):
        """Verify _get_installed_versions reflects packages installed by pip subprocess."""
        # Ensure the test package is not installed before we start
        try:
            slicer.util.pip_uninstall(self._TEST_PKG)
        except Exception:
            pass

        versions_before = slicer.util._get_installed_versions()
        self.assertNotIn(self._TEST_PKG, versions_before)

        try:
            # Install the test package via pip subprocess
            slicer.util._pip_install_simple([self._TEST_PKG])

            # _get_installed_versions must see the newly installed package
            versions_after = slicer.util._get_installed_versions()
            self.assertIn(
                self._TEST_PKG, versions_after,
                f"{self._TEST_PKG} not visible to importlib.metadata after pip install",
            )
        finally:
            # Clean up regardless of test outcome
            try:
                slicer.util.pip_uninstall(self._TEST_PKG)
            except Exception:
                pass
