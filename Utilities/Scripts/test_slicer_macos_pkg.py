import os
from pathlib import Path
import plistlib
import shlex
import shutil
import subprocess
import sys
import tempfile
import unittest
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parents[2]
PREINSTALL_SCRIPT = ROOT / "CMake" / "SlicerCPackMacOSPkg" / "Preinstall.sh"


def create_staged_app(root, name="Custom.app"):
    app = root / "stage" / "pkg" / "Applications" / name
    contents = app / "Contents"
    contents.mkdir(parents=True)
    with (contents / "Info.plist").open("wb") as stream:
        plistlib.dump({"CFBundleIdentifier": "org.example.custom", "CFBundleVersion": "1.0",
                       "CFBundlePackageType": "APPL"}, stream)
    return app


def prepare_package(root, license_file, pkgbuild=None):
    work = root / "work"
    args = [
        "cmake",
        f"-DCPACK_TEMPORARY_DIRECTORY={root / 'stage'}",
        f"-DCPACK_TOPLEVEL_DIRECTORY={root / 'output'}",
        f"-DCPACK_COMPONENT_PKG_PLIST={work / 'components.plist'}",
        f"-DCPACK_PREFLIGHT_PKG_SCRIPT={PREINSTALL_SCRIPT}",
        f"-DCPACK_POSTFLIGHT_PKG_SCRIPT={work / 'scripts' / 'postinstall'}",
        f"-DCPACK_RESOURCE_FILE_LICENSE={license_file}",
        "-DCPACK_COMPONENT_PKG_DISPLAY_NAME=Custom",
    ]
    if pkgbuild is not None:
        args.append(f"-DCPACK_COMMAND_PKGBUILD={pkgbuild}")
    args.extend(["-P", str(ROOT / "CMake" / "SlicerCPackMacOSPkg.cmake")])
    return subprocess.run(args, check=False, capture_output=True, text=True)


class PackageTests(unittest.TestCase):
    @unittest.skipUnless(sys.platform == "darwin", "requires macOS CMake options")
    def test_package_generator_validation(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "SlicerReadApplicationProperties.cmake").write_text(
                "function(SlicerReadApplicationProperties)\n"
                "  set(application_properties \"\" PARENT_SCOPE)\n"
                "endfunction()\n")
            script = root / "options.cmake"
            script.write_text(f"""
cmake_minimum_required(VERSION 3.28)
set(CMAKE_MODULE_PATH "{root}")
set(Slicer_SOURCE_DIR "{ROOT}")
set(Slicer_MAIN_PROJECT_APPLICATION_NAME Custom)
if(DEFINED TEST_NORMAL_GENERATOR)
  set(CPACK_GENERATOR "${{TEST_NORMAL_GENERATOR}}")
endif()
function(mark_as_superbuild)
  if("${{ARGV0}}" STREQUAL "CPACK_GENERATOR:STRING")
    message(STATUS "PACKAGE_GENERATOR_PROPAGATED")
  endif()
endfunction()
include("{ROOT}/CMake/SlicerApplicationOptions.cmake")
message(STATUS "PACKAGE_GENERATOR=${{CPACK_GENERATOR}}")
""")
            def configure(*args):
                return subprocess.run(["cmake", *args, "-P", str(script)], check=False, text=True, capture_output=True)

            self.assertIn("PACKAGE_GENERATOR=DragNDrop", configure().stdout)
            result = configure("-DCPACK_GENERATOR=productbuild")
            self.assertIn("PACKAGE_GENERATOR=productbuild", result.stdout)
            self.assertIn("PACKAGE_GENERATOR_PROPAGATED", result.stdout)
            self.assertIn("PACKAGE_GENERATOR=productbuild",
                          configure("-DTEST_NORMAL_GENERATOR=productbuild").stdout)
            for value in ("invalid", "DragNDrop;productbuild"):
                with self.subTest(value=value):
                    result = configure(f"-DCPACK_GENERATOR={value}")
                    self.assertNotEqual(result.returncode, 0)
                    self.assertIn("must be DragNDrop or productbuild", result.stderr)

    @unittest.skipUnless(sys.platform == "darwin", "requires macOS packaging tools")
    def test_installer_scripts_assign_only_the_app_to_the_console_user(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            scripts = root / "work" / "scripts"
            app_name = "Custom's $(false); @UNKNOWN@ App.app"
            create_staged_app(root, app_name)
            license_file = root / "license.txt"
            license_file.write_text("license")
            result = prepare_package(root, license_file)
            self.assertEqual(result.returncode, 0, result.stderr)
            shutil.copyfile(PREINSTALL_SCRIPT, scripts / "preinstall")
            tools = {
                "/usr/sbin/scutil": (
                    'if [ "${TEST_SCUTIL_FAILURE:-}" ]; then exit 1; fi\n'
                    'printf "<dictionary> {\\n  SessionInfo : <array> {\\n'
                    '    Name : other\\n  }\\n  Name : %s\\n}\\n" "$TEST_CONSOLE_USER"'),
                "/usr/bin/id": 'printf "%s\\n" "${TEST_USER_ID:-501}"',
                "/usr/sbin/chown": 'printf "%s\\n" "$@" > "$TEST_RECORD"',
            }
            replacements = {}
            for index, (tool, body) in enumerate(tools.items()):
                stub = root / f"tool{index}"
                stub.write_text(f"#!/bin/sh\n{body}\n")
                stub.chmod(0o755)
                replacements[tool] = shlex.quote(str(stub))
            for name in ("preinstall", "postinstall"):
                path = scripts / name
                content = path.read_text()
                for tool, stub in replacements.items():
                    content = content.replace(tool, stub)
                path.write_text(content)
            record = root / "chown-arguments"
            env = dict(os.environ, TEST_CONSOLE_USER="example", TEST_RECORD=str(record))
            app = root / "volume" / "Applications" / app_name
            app.mkdir(parents=True)
            subprocess.run(["/bin/sh", str(scripts / "preinstall")], env=env, check=True)
            subprocess.run(["/bin/sh", str(scripts / "postinstall"), "", "", str(root / "volume")], env=env, check=True)
            self.assertEqual(record.read_text().splitlines(), ["-R", "-P", "-x", "-n", "501", str(app)])
            app.rename(app.with_suffix(".original"))
            app.symlink_to(app.with_suffix(".original"))
            result = subprocess.run(["/bin/sh", str(scripts / "postinstall"), "", "", str(root / "volume")],
                                    check=False, env=env, capture_output=True)
            self.assertNotEqual(result.returncode, 0)
            for user in ("", "root", "loginwindow", "_mbsetupuser"):
                with self.subTest(user=user):
                    env["TEST_CONSOLE_USER"] = user
                    result = subprocess.run(["/bin/sh", str(scripts / "preinstall")], check=False, env=env, capture_output=True, text=True)
                    self.assertNotEqual(result.returncode, 0)
                    self.assertIn("logged-in desktop user", result.stderr)
            env["TEST_CONSOLE_USER"] = "example"
            env["TEST_USER_ID"] = "0"
            result = subprocess.run(["/bin/sh", str(scripts / "preinstall")], check=False, env=env, capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("valid desktop user ID", result.stderr)
            env["TEST_SCUTIL_FAILURE"] = "1"
            result = subprocess.run(["/bin/sh", str(scripts / "preinstall")], check=False, env=env, capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("Cannot determine the logged-in desktop user", result.stderr)

    @unittest.skipUnless(sys.platform == "darwin", "requires macOS console user")
    def test_preinstall_detects_active_desktop_user(self):
        console = subprocess.run(["/usr/sbin/scutil"], input="show State:/Users/ConsoleUser\n",
                                 text=True, capture_output=True, check=True).stdout
        names = [line.split(" : ", 1)[1] for line in console.splitlines() if line.startswith("  Name : ")]
        if not names or names[0] in ("root", "loginwindow", "_mbsetupuser"):
            self.skipTest("no active desktop user")
        subprocess.run(["/bin/sh", str(PREINSTALL_SCRIPT)], check=True, capture_output=True, text=True)

    @unittest.skipUnless(sys.platform == "darwin", "requires macOS packaging tools")
    def test_failed_preparation_preserves_staged_app(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            app = create_staged_app(root, "Custom App.app")
            license_file = root / "license.txt"
            license_file.write_text("license")
            result = prepare_package(root, license_file, "/usr/bin/false")
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("failed", result.stderr)
            self.assertTrue(app.is_dir())

    @unittest.skipUnless(sys.platform == "darwin", "requires macOS packaging tools")
    def test_component_is_nonrelocatable_and_upgrades(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            app = create_staged_app(root)
            child = app / "Contents" / "Helpers" / "Child.app" / "Contents"
            child.mkdir(parents=True)
            with (child / "Info.plist").open("wb") as stream:
                plistlib.dump({"CFBundleIdentifier": "org.example.custom.child", "CFBundleVersion": "1.0",
                               "CFBundlePackageType": "APPL"}, stream)
            reference = root / "analyzed.plist"
            subprocess.run(["pkgbuild", "--analyze", "--root", str(root / "stage" / "pkg"), str(reference)],
                           check=True, capture_output=True)
            with reference.open("rb") as stream:
                expected, = plistlib.load(stream)
            expected["BundleIsRelocatable"] = False
            expected["BundleOverwriteAction"] = "upgrade"
            license_file = root / "license.txt"
            license_file.write_text("license")
            result = prepare_package(root, license_file)
            self.assertEqual(result.returncode, 0, result.stderr)
            with (root / "work" / "components.plist").open("rb") as stream:
                component, = plistlib.load(stream)
            self.assertFalse(component["BundleIsRelocatable"])
            self.assertEqual(component["BundleOverwriteAction"], "upgrade")
            self.assertEqual(component["ChildBundles"][0]["RootRelativeBundlePath"],
                             "Applications/Custom.app/Contents/Helpers/Child.app")
            self.assertEqual(component, expected)
            self.assertTrue(app.is_dir())

    @unittest.skipUnless(sys.platform == "darwin", "requires macOS CMake options")
    def test_unsupported_license_format_is_rejected(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            create_staged_app(root)
            result = prepare_package(root, root / "license.md")
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("must be a .txt, .rtf or .html", result.stderr)

    @unittest.skipUnless(sys.platform == "darwin", "requires macOS CMake options")
    def test_preparation_rejects_files_outside_app(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            app = create_staged_app(root)
            for unexpected in (app.parent.parent / "unexpected", app.parent / "unexpected", app.parent / ".hidden"):
                with self.subTest(path=unexpected):
                    unexpected.write_text("unexpected")
                    result = prepare_package(root, root / "license.txt")
                    self.assertNotEqual(result.returncode, 0)
                    self.assertIn("Unexpected files outside", result.stderr)
                    unexpected.unlink()

    @unittest.skipUnless(sys.platform == "darwin", "requires macOS CMake options")
    def test_preparation_requires_one_application_bundle(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "stage" / "pkg" / "Applications").mkdir(parents=True)
            result = prepare_package(root, root / "license.txt")
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("Expected exactly one application bundle", result.stderr)
            create_staged_app(root)
            create_staged_app(root, "Other.app")
            result = prepare_package(root, root / "license.txt")
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("Expected exactly one application bundle", result.stderr)

    @unittest.skipUnless(sys.platform == "darwin", "requires macOS packaging tools")
    def test_preparation_rejects_unrecognized_application_bundle(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            app = create_staged_app(root)
            (app / "Contents" / "Info.plist").unlink()
            license_file = root / "license.txt"
            license_file.write_text("license")
            result = prepare_package(root, license_file)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("pkgbuild did not recognize the application bundle", result.stderr)

    @unittest.skipUnless(sys.platform == "darwin", "requires macOS packaging tools")
    def test_cpack_package_contains_fixed_up_app_and_license(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "src"
            build = root / "build"
            source.mkdir()
            app = source / "Custom.app" / "Contents"
            app.mkdir(parents=True)
            with (app / "Info.plist").open("wb") as stream:
                plistlib.dump({"CFBundleIdentifier": "org.example.custom",
                               "CFBundleName": "Custom", "CFBundleVersion": "1.2.3",
                               "CFBundlePackageType": "APPL"}, stream)
            child = app / "Helpers" / "Child.app" / "Contents"
            child.mkdir(parents=True)
            with (child / "Info.plist").open("wb") as stream:
                plistlib.dump({"CFBundleIdentifier": "org.example.custom.child", "CFBundleVersion": "1.0",
                               "CFBundlePackageType": "APPL"}, stream)
            (source / "dependency").write_text("dependency")
            license_bytes = "Custom @UNKNOWN@ & license \N{LATIN SMALL LETTER E WITH ACUTE}\r\n".encode()
            (source / "license.TXT").write_bytes(license_bytes)
            (source / "fixup.cmake").write_text(
                'if(NOT EXISTS "${CMAKE_INSTALL_PREFIX}/Custom.app/Contents/dependency")\n'
                '  message(FATAL_ERROR "Missing dependency during fixup")\n'
                'endif()\n'
                'file(RENAME "${CMAKE_INSTALL_PREFIX}/Custom.app" '
                '"${CMAKE_INSTALL_PREFIX}/Custom Display 1.2.3.app")\n')
            (source / "CMakeLists.txt").write_text(f"""
cmake_minimum_required(VERSION 3.28)
project(PkgFixture NONE)
install(DIRECTORY "${{CMAKE_SOURCE_DIR}}/Custom.app/" DESTINATION Custom.app COMPONENT Runtime)
install(FILES "${{CMAKE_SOURCE_DIR}}/dependency" DESTINATION Custom.app/Contents COMPONENT RuntimeLibraries)
install(SCRIPT "${{CMAKE_SOURCE_DIR}}/fixup.cmake" COMPONENT pkg)
set(CPACK_GENERATOR productbuild)
set(CPACK_MONOLITHIC_INSTALL OFF)
set(CPACK_COMPONENTS_ALL pkg)
set(CPACK_PACKAGING_INSTALL_PREFIX "/Applications")
set(CPACK_INSTALL_CMAKE_PROJECTS
  "${{CMAKE_BINARY_DIR}};PkgFixture;RuntimeLibraries;/pkg;${{CMAKE_BINARY_DIR}};PkgFixture;Runtime;/pkg;${{CMAKE_BINARY_DIR}};PkgFixture;ALL;/")
set(CPACK_PACKAGE_NAME Custom)
set(CPACK_PACKAGE_VERSION 1.2.3-2026-11-09)
set(CPACK_PRODUCTBUILD_IDENTIFIER org.example.custom)
set(CPACK_COMPONENT_PKG_DISPLAY_NAME "Custom & <Display>")
set(CPACK_COMPONENT_PKG_REQUIRED ON)
set(CPACK_COMPONENT_PKG_PLIST "${{CMAKE_BINARY_DIR}}/work/components.plist")
set(CPACK_PREFLIGHT_PKG_SCRIPT "{PREINSTALL_SCRIPT}")
set(CPACK_POSTFLIGHT_PKG_SCRIPT "${{CMAKE_BINARY_DIR}}/work/scripts/postinstall")
set(CPACK_PRE_BUILD_SCRIPTS "{ROOT}/CMake/SlicerCPackMacOSPkg.cmake")
set(CPACK_MODULE_PATH "{ROOT}/CMake/SlicerCPackMacOSPkg")
set(CPACK_RESOURCE_FILE_LICENSE "${{CMAKE_SOURCE_DIR}}/${{FIXTURE_LICENSE}}")
include(CPack)
""")
            subprocess.run(["cmake", "-S", str(source), "-B", str(build), "-DFIXTURE_LICENSE=license.TXT"],
                           check=True, capture_output=True)
            subprocess.run(["cpack", "--config", str(build / "CPackConfig.cmake"), "-B", str(build)],
                           check=True, capture_output=True)
            subprocess.run(["cpack", "--config", str(build / "CPackConfig.cmake"), "-B", str(build)],
                           check=True, capture_output=True)
            package, = build.glob("*.pkg")
            expanded = root / "expanded"
            subprocess.run(["pkgutil", "--expand-full", str(package), str(expanded)], check=True, capture_output=True)
            distribution = ET.parse(expanded / "Distribution").getroot()
            self.assertEqual(distribution.findtext("title"), "Custom & <Display>")
            license_name = distribution.find("license").get("file")
            self.assertEqual((expanded / "Resources" / license_name).read_bytes(), license_bytes)
            choices = distribution.findall("./choices-outline/line")
            self.assertEqual(len(choices), 1)
            choice = distribution.find(f"./choice[@id='{choices[0].get('choice')}']")
            self.assertIsNotNone(choice)
            self.assertEqual(choice.get("start_selected"), "true")
            self.assertEqual(choice.get("start_enabled"), "false")
            self.assertEqual(choice.find("pkg-ref").get("id"), "org.example.custom.pkg")
            component, = expanded.glob("*.pkg")
            payload = component / "Payload"
            self.assertTrue((component / "Scripts" / "preinstall").is_file())
            self.assertTrue((component / "Scripts" / "postinstall").is_file())
            self.assertEqual((component / "Scripts" / "preinstall").read_bytes(), PREINSTALL_SCRIPT.read_bytes())
            self.assertTrue(os.access(component / "Scripts" / "preinstall", os.X_OK))
            self.assertEqual([p.name for p in payload.iterdir()], ["Applications"])
            applications = payload / "Applications"
            self.assertEqual([p.name for p in applications.iterdir()], ["Custom Display 1.2.3.app"])
            self.assertEqual((applications / "Custom Display 1.2.3.app" / "Contents" / "dependency").read_text(),
                             "dependency")
            package_info = ET.parse(component / "PackageInfo").getroot()
            self.assertEqual(package_info.get("install-location"), "/")
            self.assertEqual(package_info.get("identifier"), "org.example.custom.pkg")
            self.assertEqual(package_info.get("relocatable"), "false")
            self.assertEqual(package_info.find("./upgrade-bundle/bundle").get("id"), "org.example.custom")
            self.assertIsNotNone(package_info.find("./bundle[@id='org.example.custom.child']"))
            staged_apps = list((build / "_CPack_Packages").rglob("Custom Display 1.2.3.app"))
            self.assertTrue(staged_apps)
            generator_dir, = (build / "_CPack_Packages").glob("*/productbuild")
            verification = subprocess.run(
                ["cmake", f"-DSlicer_INSTALL_DIR={generator_dir}",
                 "-P", str(ROOT / "CMake" / "SlicerCPackBundleVerify.cmake")],
                check=True, capture_output=True, text=True)
            self.assertIn(str(staged_apps[0]), verification.stdout)
            for extension, contents in (
                ("RTF", b"{\\rtf1\\ansi Custom @UNKNOWN@ & license\\par}\r\n"),
                ("HTML", b"<html><body>Custom @UNKNOWN@ &amp; license</body></html>\r\n"),
            ):
                with self.subTest(extension=extension):
                    filename = "license." + extension
                    (source / filename).write_bytes(contents)
                    subprocess.run(["cmake", "-S", str(source), "-B", str(build), f"-DFIXTURE_LICENSE={filename}"],
                                   check=True, capture_output=True)
                    subprocess.run(["cpack", "--config", str(build / "CPackConfig.cmake"), "-B", str(build)],
                                   check=True, capture_output=True)
                    expanded_format = root / ("expanded-" + extension)
                    subprocess.run(["pkgutil", "--expand-full", str(package), str(expanded_format)],
                                   check=True, capture_output=True)
                    distribution = ET.parse(expanded_format / "Distribution").getroot()
                    license_name = distribution.find("license").get("file")
                    self.assertEqual((expanded_format / "Resources" / license_name).read_bytes(), contents)


if __name__ == "__main__":
    unittest.main()
