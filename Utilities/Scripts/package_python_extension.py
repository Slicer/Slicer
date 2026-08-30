#!/usr/bin/env python3
"""
package_extension.py - Package a 3D Slicer extension without a full Slicer build.

Creates a minimal dummy Slicer build tree containing only the CMake infrastructure
needed to configure and package pure Python extensions (scripted modules).

Usage:
  python package_extension.py --ext-src <path> [options]

Examples:
  python package_extension.py --ext-src d:/D/EI-build2/MONAIAuto3DSeg
  python package_extension.py --ext-src d:/D/EI-build2/MONAIAuto3DSeg \\
      --slicer-src c:/D/S4 --output-dir d:/D/packages
"""

import argparse
import os
import platform
import re
import shutil
import subprocess
import sys
import textwrap
from pathlib import Path


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def run(cmd, cwd=None, check=True, capture=False):
    """Run a command, print it, and return the CompletedProcess."""
    print(f"  $ {' '.join(str(c) for c in cmd)}")
    kwargs = dict(cwd=cwd)
    if capture:
        kwargs["capture_output"] = True
        kwargs["text"] = True
    result = subprocess.run(cmd, **kwargs)
    if check and result.returncode != 0:
        sys.exit(f"Command failed with exit code {result.returncode}")
    return result


def cmake_path(p):
    """Convert a path to forward-slash form for use in CMake files."""
    return str(p).replace("\\", "/")


def detect_slicer_version(slicer_src):
    """Parse Slicer version from CMakeLists.txt."""
    cml = Path(slicer_src) / "CMakeLists.txt"
    if not cml.exists():
        return "5", "11", "0"
    text = cml.read_text(encoding="utf-8", errors="ignore")
    major = re.search(r'set\s*\(\s*Slicer_VERSION_MAJOR\s+"?(\d+)"?\s*\)', text)
    minor = re.search(r'set\s*\(\s*Slicer_VERSION_MINOR\s+"?(\d+)"?\s*\)', text)
    patch = re.search(r'set\s*\(\s*Slicer_VERSION_PATCH\s+"?(\d+)"?\s*\)', text)
    return (
        major.group(1) if major else "5",
        minor.group(1) if minor else "11",
        patch.group(1) if patch else "0",
    )


def _read_commit_count_offset(slicer_src):
    """Parse Slicer_WC_COMMIT_COUNT_OFFSET from CMakeLists.txt."""
    cmake_lists = pathlib.Path(slicer_src) / "CMakeLists.txt"
    try:
        text = cmake_lists.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return 0
    # Extract all quoted integer values inside the _commit_count_offsets list
    m = re.search(
        r'set\(_commit_count_offsets\s*(.*?)\s*\)',
        text, re.DOTALL
    )
    if not m:
        return 0
    return sum(int(n) for n in re.findall(r'"(\d+)"', m.group(1)))


def detect_slicer_revision(slicer_src):
    """Return Slicer revision = git commit count + Slicer_WC_COMMIT_COUNT_OFFSET."""
    try:
        result = subprocess.run(
            ["git", "-c", "safe.directory=*", "rev-list", "--count", "HEAD"],
            cwd=slicer_src, capture_output=True, text=True, timeout=10
        )
        if result.returncode == 0:
            commit_count = int(result.stdout.strip())
            offset = _read_commit_count_offset(slicer_src)
            return str(commit_count + offset)
        print(f"  [warning] git rev-list failed (exit {result.returncode}): {result.stderr.strip()}")
    except Exception as e:
        print(f"  [warning] Could not detect Slicer revision: {e}")
    return "0"


def detect_platform():
    """Return (os_name, arch) matching Slicer's naming convention."""
    system = platform.system().lower()
    machine = platform.machine().lower()
    if system == "windows":
        os_name = "win"
    elif system == "darwin":
        os_name = "macosx"
    else:
        os_name = "linux"
    arch = "amd64" if machine in ("amd64", "x86_64") else machine
    return os_name, arch


def find_cmake(cmake_hint=None):
    """Locate the cmake executable."""
    candidates = []
    if cmake_hint:
        candidates.append(Path(cmake_hint) / "cmake.exe")
        candidates.append(Path(cmake_hint) / "bin" / "cmake.exe")
        candidates.append(Path(cmake_hint))
    candidates += [
        Path(r"C:/Program Files/CMake/bin/cmake.exe"),
        Path(r"C:/Program Files (x86)/CMake/bin/cmake.exe"),
    ]
    for p in candidates:
        if p.exists():
            return str(p)
    # Fallback: search PATH
    found = shutil.which("cmake")
    if found:
        return found
    sys.exit("cmake not found. Use --cmake to specify its location.")


def find_cpack(cmake_exe):
    """Derive cpack location from cmake location."""
    p = Path(cmake_exe)
    cpack = p.parent / ("cpack.exe" if platform.system() == "Windows" else "cpack")
    if cpack.exists():
        return str(cpack)
    # Try same dir without .exe
    cpack2 = p.parent / "cpack"
    if cpack2.exists():
        return str(cpack2)
    found = shutil.which("cpack")
    if found:
        return found
    sys.exit("cpack not found alongside cmake.")


# ---------------------------------------------------------------------------
# Dummy build tree generation
# ---------------------------------------------------------------------------

SLICER_CONFIG_TEMPLATE = """\
#
# Minimal dummy SlicerConfig.cmake for packaging pure Python Slicer extensions
# without a full Slicer build.
#
# Generated by package_extension.py
# Slicer source: {slicer_src}
#

if(Slicer_SOURCE_DIR)
  return()
endif()

# --------------------------------------------------------------------------
# Source and cmake dirs
# --------------------------------------------------------------------------
set(Slicer_CMAKE_DIR "{slicer_src}/CMake")
set(Slicer_EXTENSIONS_CMAKE_DIR "{slicer_src}/Extensions/CMake")

# Prepend our stub CMake dir then the Slicer source dirs
list(PREPEND CMAKE_MODULE_PATH
  "${{CMAKE_CURRENT_LIST_DIR}}/CMake"
  "${{Slicer_EXTENSIONS_CMAKE_DIR}}"
  "${{Slicer_CMAKE_DIR}}"
)

# --------------------------------------------------------------------------
# Version
# --------------------------------------------------------------------------
set(Slicer_VERSION_MAJOR "{major}")
set(Slicer_VERSION_MINOR "{minor}")
set(Slicer_VERSION_PATCH "{patch}")
set(Slicer_REVISION "{revision}")
set(Slicer_WC_REVISION "{revision}")
set(Slicer_WC_REVISION_HASH "{revision}")
set(Slicer_WC_URL "https://github.com/Slicer/Slicer")
set(Slicer_WC_ROOT "https://github.com/Slicer/Slicer")

if(NOT "$ENV{{Slicer_REVISION}}" STREQUAL "")
  set(Slicer_REVISION "$ENV{{Slicer_REVISION}}")
endif()

# --------------------------------------------------------------------------
# Platform
# --------------------------------------------------------------------------
set(Slicer_OS "{os_name}")
set(Slicer_ARCHITECTURE "{arch}")

# --------------------------------------------------------------------------
# Main application
# --------------------------------------------------------------------------
set(Slicer_MAIN_PROJECT "SlicerApp")
set(Slicer_MAIN_PROJECT_APPLICATION_NAME "Slicer")
set(Slicer_MAIN_PROJECT_APPLICATION_DISPLAY_NAME "3D Slicer")
set(Slicer_ORGANIZATION_NAME "NA-MIC")
set(Slicer_ORGANIZATION_DOMAIN "slicer.org")

# --------------------------------------------------------------------------
# License and readme (from Slicer source)
# --------------------------------------------------------------------------
set(Slicer_LICENSE_FILE "{slicer_src}/License.txt")
set(Slicer_README_FILE "{slicer_src}/README.md")

# --------------------------------------------------------------------------
# Use file and extension cmake scripts
# --------------------------------------------------------------------------
set(Slicer_USE_FILE "${{CMAKE_CURRENT_LIST_DIR}}/UseSlicer.cmake")
set(Slicer_EXTENSION_GENERATE_CONFIG "{slicer_src}/CMake/SlicerExtensionGenerateConfig.cmake")
set(Slicer_EXTENSION_CPACK "{slicer_src}/CMake/SlicerExtensionCPack.cmake")
set(Slicer_EXTENSION_CPACK_BUNDLE_FIXUP "{slicer_src}/CMake/SlicerExtensionCPackBundleFixup.cmake.in")
set(Slicer_EXTENSIONS_DIRBASENAME "Extensions")
set(Slicer_EXTENSIONS_DIRNAME "${{Slicer_EXTENSIONS_DIRBASENAME}}-${{Slicer_REVISION}}")

# --------------------------------------------------------------------------
# Module support flags
# --------------------------------------------------------------------------
set(Slicer_BUILD_CLI ON)
set(Slicer_BUILD_CLI_SUPPORT ON)
set(Slicer_BUILD_QTLOADABLEMODULES ON)
set(Slicer_BUILD_QTSCRIPTEDMODULES ON)
set(Slicer_BUILD_SHARED ON)
set(Slicer_USE_PYTHONQT ON)
set(Slicer_USE_CTKAPPLAUNCHER ON)
set(Slicer_DEFAULT_BUILD_TYPE "Release")

# --------------------------------------------------------------------------
# Build/home dirs
# --------------------------------------------------------------------------
set(Slicer_HOME "${{CMAKE_CURRENT_LIST_DIR}}")
set(Slicer_BINARY_DIR "${{CMAKE_CURRENT_LIST_DIR}}")

# --------------------------------------------------------------------------
# Module output directories (relative)
# --------------------------------------------------------------------------
set(_slicer_ver "{major}.{minor}")
set(Slicer_CLIMODULES_BIN_DIR   "lib/Slicer-${{_slicer_ver}}/cli-modules")
set(Slicer_CLIMODULES_LIB_DIR   "lib/Slicer-${{_slicer_ver}}/cli-modules")
set(Slicer_CLIMODULES_SHARE_DIR "share/Slicer-${{_slicer_ver}}/cli-modules")

set(Slicer_QTLOADABLEMODULES_BIN_DIR       "lib/Slicer-${{_slicer_ver}}/qt-loadable-modules")
set(Slicer_QTLOADABLEMODULES_LIB_DIR       "lib/Slicer-${{_slicer_ver}}/qt-loadable-modules")
set(Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR "lib/Slicer-${{_slicer_ver}}/qt-loadable-modules/Python")
set(Slicer_QTLOADABLEMODULES_INCLUDE_DIR   "include/Slicer-${{_slicer_ver}}/qt-loadable-modules")
set(Slicer_QTLOADABLEMODULES_SHARE_DIR     "share/Slicer-${{_slicer_ver}}/qt-loadable-modules")

set(Slicer_QTSCRIPTEDMODULES_SUBDIR  "qt-scripted-modules")
set(Slicer_QTSCRIPTEDMODULES_BIN_DIR "lib/Slicer-${{_slicer_ver}}/qt-scripted-modules")
set(Slicer_QTSCRIPTEDMODULES_LIB_DIR "lib/Slicer-${{_slicer_ver}}/qt-scripted-modules")
set(Slicer_QTSCRIPTEDMODULES_INCLUDE_DIR "include/Slicer-${{_slicer_ver}}/qt-scripted-modules")
set(Slicer_QTSCRIPTEDMODULES_SHARE_DIR   "share/Slicer-${{_slicer_ver}}/qt-scripted-modules")

set(Slicer_THIRDPARTY_BIN_DIR   "bin")
set(Slicer_THIRDPARTY_LIB_DIR   "lib/Slicer-${{_slicer_ver}}")
set(Slicer_THIRDPARTY_SHARE_DIR "share/Slicer-${{_slicer_ver}}")

# --------------------------------------------------------------------------
# Install dirs
# --------------------------------------------------------------------------
set(Slicer_INSTALL_ROOT "./")
set(Slicer_SHARE_DIR    "share/Slicer-${{_slicer_ver}}")
set(Slicer_INSTALL_BIN_DIR     "./bin")
set(Slicer_INSTALL_LIB_DIR     "./lib/Slicer-${{_slicer_ver}}")
set(Slicer_INSTALL_INCLUDE_DIR "./include/Slicer-${{_slicer_ver}}")
set(Slicer_INSTALL_SHARE_DIR   "./share/Slicer-${{_slicer_ver}}")

# --------------------------------------------------------------------------
# Python version
# --------------------------------------------------------------------------
set(Slicer_REQUIRED_PYTHON_VERSION "3.9.0")
set(Slicer_REQUIRED_PYTHON_VERSION_MAJOR "3")
set(Slicer_REQUIRED_PYTHON_VERSION_MINOR "9")
set(PYTHON_STDLIB_SUBDIR "Lib")
set(PYTHON_SITE_PACKAGES_SUBDIR "Lib/site-packages")

# --------------------------------------------------------------------------
# VTK version (used by some macros)
# --------------------------------------------------------------------------
set(Slicer_VTK_VERSION_MAJOR "9")
set(Slicer_VTK_VERSION_MINOR "2")

# --------------------------------------------------------------------------
# Export header template
# --------------------------------------------------------------------------
set(Slicer_EXPORT_HEADER_TEMPLATE "{slicer_src}/CMake/qSlicerExport.h.in")
set(Slicer_LOGOS_RESOURCE         "{slicer_src}/Resources/qSlicer.qrc")

# --------------------------------------------------------------------------
# Test template dirs
# --------------------------------------------------------------------------
set(Slicer_CXX_MODULE_TEST_TEMPLATES_DIR    "{slicer_src}/Base/QTGUI/Testing/Cxx")
set(Slicer_PYTHON_MODULE_TEST_TEMPLATES_DIR "{slicer_src}/Base/QTCore/Testing/Python")

# --------------------------------------------------------------------------
# Extension upload prerequisites
# (normally set by SlicerBlockUploadExtensionPrerequisites)
# --------------------------------------------------------------------------
set(EXTENSION_OPERATING_SYSTEM "${{Slicer_OS}}")
set(EXTENSION_ARCHITECTURE     "${{Slicer_ARCHITECTURE}}")
set(Slicer_EXTENSION_CMAKE_GENERATOR "${{CMAKE_GENERATOR}}")

# --------------------------------------------------------------------------
# Skip external project find_package calls - not needed for Python modules
# --------------------------------------------------------------------------
set(Slicer_EXTERNAL_PROJECTS "")
set(Slicer_EXTERNAL_PROJECTS_NO_USEFILE "")

# CTKAppLauncher: point to our stub
set(CTKAppLauncher_DIR "${{CMAKE_CURRENT_LIST_DIR}}/CTKAppLauncher")

# Git executable
find_program(GIT_EXECUTABLE git)
"""

USE_SLICER_TEMPLATE = """\
#
# Minimal UseSlicer.cmake for packaging pure Python Slicer extensions.
# Generated by package_extension.py
#

if(DEFINED Slicer_USE_FILE_INCLUDED)
  return()
endif()
set(Slicer_USE_FILE_INCLUDED TRUE)

if(Slicer_SOURCE_DIR)
  return()
endif()

# --------------------------------------------------------------------------
# Set EXTENSION_NAME
# --------------------------------------------------------------------------
if(NOT Slicer_DONT_USE_EXTENSION)
  if(NOT DEFINED EXTENSION_NAME)
    set(EXTENSION_NAME ${{PROJECT_NAME}})
  endif()
endif()

# --------------------------------------------------------------------------
# Stubs for superbuild macros not needed here
# --------------------------------------------------------------------------
macro(mark_as_superbuild)
endmacro()

macro(slicer_add_python_qt_resources)
endmacro()

# --------------------------------------------------------------------------
# CTK cmake dir (our stubs)
# --------------------------------------------------------------------------
set(CTK_CMAKE_DIR "${{CMAKE_CURRENT_LIST_DIR}}/CMake")

# --------------------------------------------------------------------------
# Include utilities
# --------------------------------------------------------------------------
include(SlicerInitializeBuildType)
include(UseSlicerMacros)
include(ctkMacroCompilePythonScript)
include(ctkFunctionAddExecutableUtf8)
include(ctkMacroWrapPythonQt)
include(SlicerMacroBuildScriptedModule)
include(SlicerMacroExtractRepositoryInfo)

# --------------------------------------------------------------------------
# Compute install dirs
# --------------------------------------------------------------------------
set(Slicer_BUNDLE_EXTENSIONS_LOCATION "")

foreach(_type CLIMODULES QTLOADABLEMODULES QTSCRIPTEDMODULES THIRDPARTY)
  foreach(_sub BIN LIB SHARE INCLUDE PYTHON_LIB)
    set(_var "Slicer_${{_type}}_${{_sub}}_DIR")
    set(_base "Slicer_${{_type}}_${{_sub}}_DIR")
    if(DEFINED ${{_base}})
      set("Slicer_INSTALL_${{_type}}_${{_sub}}_DIR"
        "${{Slicer_INSTALL_ROOT}}${{Slicer_BUNDLE_EXTENSIONS_LOCATION}}${{${{_base}}}}")
    endif()
  endforeach()
endforeach()

# Explicit assignments for the most important ones
set(Slicer_INSTALL_CLIMODULES_BIN_DIR
    "${{Slicer_INSTALL_ROOT}}${{Slicer_CLIMODULES_BIN_DIR}}")
set(Slicer_INSTALL_CLIMODULES_LIB_DIR
    "${{Slicer_INSTALL_ROOT}}${{Slicer_CLIMODULES_LIB_DIR}}")
set(Slicer_INSTALL_CLIMODULES_SHARE_DIR
    "${{Slicer_INSTALL_ROOT}}${{Slicer_CLIMODULES_SHARE_DIR}}")
set(Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR
    "${{Slicer_INSTALL_ROOT}}${{Slicer_QTLOADABLEMODULES_BIN_DIR}}")
set(Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR
    "${{Slicer_INSTALL_ROOT}}${{Slicer_QTLOADABLEMODULES_LIB_DIR}}")
set(Slicer_INSTALL_QTLOADABLEMODULES_PYTHON_LIB_DIR
    "${{Slicer_INSTALL_ROOT}}${{Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR}}")
set(Slicer_INSTALL_QTLOADABLEMODULES_INCLUDE_DIR
    "${{Slicer_INSTALL_ROOT}}${{Slicer_QTLOADABLEMODULES_INCLUDE_DIR}}")
set(Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR
    "${{Slicer_INSTALL_ROOT}}${{Slicer_QTLOADABLEMODULES_SHARE_DIR}}")
set(Slicer_INSTALL_QTSCRIPTEDMODULES_BIN_DIR
    "${{Slicer_INSTALL_ROOT}}${{Slicer_QTSCRIPTEDMODULES_BIN_DIR}}")
set(Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR
    "${{Slicer_INSTALL_ROOT}}${{Slicer_QTSCRIPTEDMODULES_LIB_DIR}}")
set(Slicer_INSTALL_QTSCRIPTEDMODULES_INCLUDE_DIR
    "${{Slicer_INSTALL_ROOT}}${{Slicer_QTSCRIPTEDMODULES_INCLUDE_DIR}}")
set(Slicer_INSTALL_QTSCRIPTEDMODULES_SHARE_DIR
    "${{Slicer_INSTALL_ROOT}}${{Slicer_QTSCRIPTEDMODULES_SHARE_DIR}}")
set(Slicer_INSTALL_THIRDPARTY_BIN_DIR
    "${{Slicer_INSTALL_ROOT}}${{Slicer_THIRDPARTY_BIN_DIR}}")
set(Slicer_INSTALL_THIRDPARTY_LIB_DIR
    "${{Slicer_INSTALL_ROOT}}${{Slicer_THIRDPARTY_LIB_DIR}}")
set(Slicer_INSTALL_THIRDPARTY_SHARE_DIR
    "${{Slicer_INSTALL_ROOT}}${{Slicer_THIRDPARTY_SHARE_DIR}}")

# --------------------------------------------------------------------------
# Extension defaults
# --------------------------------------------------------------------------
if(NOT Slicer_DONT_USE_EXTENSION)
  if(NOT DEFINED EXTENSION_DEPENDS)
    set(EXTENSION_DEPENDS NA)
  endif()
  if(NOT DEFINED EXTENSION_LICENSE_FILE)
    set(EXTENSION_LICENSE_FILE "${{Slicer_LICENSE_FILE}}")
  endif()
  if(NOT DEFINED EXTENSION_README_FILE)
    set(EXTENSION_README_FILE "${{Slicer_README_FILE}}")
  endif()
  if(NOT DEFINED EXTENSION_SOURCE_DIR)
    set(EXTENSION_SOURCE_DIR "${{CMAKE_SOURCE_DIR}}")
  endif()
  if(NOT DEFINED EXTENSION_SUPERBUILD_BINARY_DIR)
    set(EXTENSION_SUPERBUILD_BINARY_DIR "${{CMAKE_BINARY_DIR}}")
  endif()
  if(NOT DEFINED EXTENSION_BUILD_SUBDIRECTORY)
    set(EXTENSION_BUILD_SUBDIRECTORY ".")
  endif()
  if(NOT DEFINED EXTENSION_ENABLED)
    set(EXTENSION_ENABLED 1)
  endif()

  include(SlicerExtensionDescriptionSpec)
  foreach(var IN LISTS Slicer_EXT_OPTIONAL_METADATA_NAMES)
    if(NOT DEFINED EXTENSION_${{var}})
      set(EXTENSION_${{var}} "${{Slicer_EXT_METADATA_${{var}}_DEFAULT}}")
    endif()
  endforeach()

  if(NOT DEFINED Slicer_SKIP_SlicerEnableExtensionTesting)
    set(Slicer_SKIP_SlicerEnableExtensionTesting FALSE)
  endif()
  if(NOT ${{Slicer_SKIP_SlicerEnableExtensionTesting}})
    include(CTest)
    include(ExternalData)
    find_package(Git QUIET)
  endif()

  include(SlicerMacroSimpleTest)
  include(SlicerMacroPythonTesting)
  include(SlicerMacroConfigureGenericPythonModuleTests)
endif()
"""

CTK_APP_LAUNCHER_CONFIG_TEMPLATE = """\
# Stub CTKAppLauncherConfig.cmake - provides no-op launcher macro
set(CTKAppLauncher_CMAKE_DIR "${CMAKE_CURRENT_LIST_DIR}/CMake")
include("${CTKAppLauncher_CMAKE_DIR}/ctkAppLauncher.cmake")
"""

CTK_APP_LAUNCHER_CMAKE_TEMPLATE = """\
# Stub ctkAppLauncher.cmake - no-op launcher macros
macro(ctkAppLauncherConfigureForExecutable)
endmacro()
macro(ctkAppLauncherConfigure)
endmacro()
"""

CTK_COMPILE_PYTHON_TEMPLATE = """\
# Minimal ctkMacroCompilePythonScript - copies files and sets up install rules.
# Does not perform .pyc byte compilation.
cmake_policy(SET CMP0177 NEW)

macro(ctkMacroCompilePythonScript)
  cmake_parse_arguments(_ctk_py
    "NO_INSTALL_SUBDIR;GLOBAL_TARGET"
    "TARGET_NAME;SOURCE_DIR;DESTINATION_DIR;INSTALL_DIR"
    "SCRIPTS;RESOURCES"
    ${ARGN}
  )

  if(NOT DEFINED _ctk_py_SOURCE_DIR)
    set(_ctk_py_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  endif()

  set(_ctk_py_outputs)

  foreach(_file ${_ctk_py_SCRIPTS} ${_ctk_py_RESOURCES})
    get_filename_component(_file_ext "${_file}" EXT)
    if("${_file_ext}" STREQUAL "")
      set(_file "${_file}.py")
    endif()

    set(_src "${_ctk_py_SOURCE_DIR}/${_file}")
    set(_dst "${_ctk_py_DESTINATION_DIR}/${_file}")
    get_filename_component(_dst_dir "${_dst}" DIRECTORY)

    add_custom_command(
      OUTPUT "${_dst}"
      COMMAND "${CMAKE_COMMAND}" -E make_directory "${_dst_dir}"
      COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${_src}" "${_dst}"
      DEPENDS "${_src}"
      COMMENT "Copying ${_file}"
    )
    list(APPEND _ctk_py_outputs "${_dst}")
  endforeach()

  if(_ctk_py_outputs)
    add_custom_target(${_ctk_py_TARGET_NAME} ALL DEPENDS ${_ctk_py_outputs})
  else()
    add_custom_target(${_ctk_py_TARGET_NAME} ALL)
  endif()

  if(_ctk_py_NO_INSTALL_SUBDIR)
    install(DIRECTORY "${_ctk_py_DESTINATION_DIR}/"
      DESTINATION "${_ctk_py_INSTALL_DIR}"
      COMPONENT RuntimeLibraries
      USE_SOURCE_PERMISSIONS
    )
  else()
    install(DIRECTORY "${_ctk_py_DESTINATION_DIR}"
      DESTINATION "${_ctk_py_INSTALL_DIR}"
      COMPONENT RuntimeLibraries
      USE_SOURCE_PERMISSIONS
    )
  endif()
endmacro()
"""

CTK_STUB_TEMPLATE = """\
# Stub: {name} - not needed for Python-only extensions
{stub_type}({name})
{end_type}()
"""


def generate_dummy_build_tree(dummy_dir, slicer_src, slicer_revision=None):
    """Create or update the minimal dummy Slicer build tree."""
    dummy_dir = Path(dummy_dir)
    slicer_src = Path(slicer_src)

    print(f"\n[dummy build] Generating tree at: {dummy_dir}")

    major, minor, patch = detect_slicer_version(slicer_src)
    if slicer_revision is None:
        slicer_revision = detect_slicer_revision(slicer_src)
    os_name, arch = detect_platform()
    src = cmake_path(slicer_src)

    # Create directory structure
    cmake_stubs_dir = dummy_dir / "CMake"
    ctk_launcher_dir = dummy_dir / "CTKAppLauncher"
    ctk_launcher_cmake_dir = ctk_launcher_dir / "CMake"
    for d in [cmake_stubs_dir, ctk_launcher_cmake_dir]:
        d.mkdir(parents=True, exist_ok=True)

    # SlicerConfig.cmake
    (dummy_dir / "SlicerConfig.cmake").write_text(
        SLICER_CONFIG_TEMPLATE.format(
            slicer_src=src,
            major=major, minor=minor, patch=patch,
            revision=slicer_revision,
            os_name=os_name, arch=arch,
        ),
        encoding="utf-8",
    )

    # UseSlicer.cmake
    (dummy_dir / "UseSlicer.cmake").write_text(
        USE_SLICER_TEMPLATE.format(),
        encoding="utf-8",
    )

    # CTKAppLauncher stubs
    (ctk_launcher_dir / "CTKAppLauncherConfig.cmake").write_text(
        CTK_APP_LAUNCHER_CONFIG_TEMPLATE, encoding="utf-8"
    )
    (ctk_launcher_cmake_dir / "ctkAppLauncher.cmake").write_text(
        CTK_APP_LAUNCHER_CMAKE_TEMPLATE, encoding="utf-8"
    )

    # CTK cmake stubs
    (cmake_stubs_dir / "ctkMacroCompilePythonScript.cmake").write_text(
        CTK_COMPILE_PYTHON_TEMPLATE, encoding="utf-8"
    )
    for name, stub_type, end_type in [
        ("ctkFunctionAddExecutableUtf8", "function", "endfunction"),
        ("ctkMacroWrapPythonQt", "macro", "endmacro"),
    ]:
        (cmake_stubs_dir / f"{name}.cmake").write_text(
            f"# Stub: {name} - not needed for Python-only extensions\n"
            f"{stub_type}({name})\n{end_type}()\n",
            encoding="utf-8",
        )

    print(f"  Slicer version : {major}.{minor}.{patch} (revision {slicer_revision})")
    print(f"  Platform       : {os_name}-{arch}")
    return dummy_dir


# ---------------------------------------------------------------------------
# Main workflow
# ---------------------------------------------------------------------------

def configure_extension(cmake_exe, ext_src, build_dir, slicer_dir,
                        install_prefix, extra_cmake_args):
    """Run cmake configure for the extension."""
    build_dir = Path(build_dir)
    build_dir.mkdir(parents=True, exist_ok=True)

    cmd = [
        cmake_exe,
        f"-DSlicer_DIR={cmake_path(slicer_dir)}",
        f"-DCMAKE_INSTALL_PREFIX={cmake_path(install_prefix)}",
        *extra_cmake_args,
        str(ext_src),
    ]
    print("\n[configure]")
    run(cmd, cwd=build_dir)


def build_extension(cmake_exe, build_dir, build_config):
    """Run cmake --build."""
    print("\n[build]")
    run([cmake_exe, "--build", str(build_dir), "--config", build_config])


def package_extension(cpack_exe, build_dir, build_config):
    """Run cpack and return the generated package path."""
    print("\n[package]")
    result = run(
        [cpack_exe, "-C", build_config],
        cwd=build_dir,
        capture=True,
    )
    print(result.stdout)
    if result.stderr:
        print(result.stderr, file=sys.stderr)

    # Parse package path from cpack output
    for line in result.stdout.splitlines():
        m = re.search(r"package:\s+(.+?)\s+generated", line)
        if m:
            return m.group(1).strip()
    return None


def copy_package(package_path, output_dir):
    """Copy the generated package to the output directory."""
    if not package_path or not Path(package_path).exists():
        print("  (could not determine package path)")
        return
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    dest = output_dir / Path(package_path).name
    shutil.copy2(package_path, dest)
    print(f"\n[output] {dest}")
    return dest


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def parse_args():
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--ext-src", required=True, metavar="PATH",
        help="Extension source directory (contains top-level CMakeLists.txt)",
    )
    parser.add_argument(
        "--slicer-src", default=r"C:\D\S4", metavar="PATH",
        help="Slicer source directory (default: C:\\D\\S4)",
    )
    parser.add_argument(
        "--cmake", default=r"C:\Program Files\CMake\bin\cmake.exe", metavar="PATH",
        help="Path to cmake executable or its bin directory",
    )
    parser.add_argument(
        "--slicer-build", default=None, metavar="PATH",
        help="Path to existing Slicer build tree. "
             "If given, used directly instead of the dummy build tree.",
    )
    parser.add_argument(
        "--dummy-build", default=None, metavar="PATH",
        help="Where to create the dummy Slicer build tree "
             "(default: <ext-src>/../SlicerDummyBuild)",
    )
    parser.add_argument(
        "--build-dir", default=None, metavar="PATH",
        help="Extension build directory "
             "(default: <ext-src>/../<ExtName>-build)",
    )
    parser.add_argument(
        "--install-prefix", default=None, metavar="PATH",
        help="CMake install prefix "
             "(default: <ext-src>/../<ExtName>-install)",
    )
    parser.add_argument(
        "--output-dir", default=None, metavar="PATH",
        help="Directory where the final package is copied "
             "(default: same as build-dir)",
    )
    parser.add_argument(
        "--config", default="Release", metavar="CONFIG",
        help="CMake build configuration (default: Release)",
    )
    parser.add_argument(
        "--slicer-revision", default=None, metavar="REV",
        help="Override the Slicer revision embedded in the package filename",
    )
    parser.add_argument(
        "--regen-dummy", action="store_true",
        help="Force regeneration of the dummy Slicer build tree",
    )
    parser.add_argument(
        "--cmake-arg", action="append", default=[], metavar="ARG",
        dest="cmake_args",
        help="Extra argument to pass to cmake configure (can be repeated). "
             "Values starting with '-' (e.g. -DFOO=BAR) are handled correctly.",
    )
    # argparse refuses to consume a value starting with '-' as the argument to
    # an option (it looks like another flag).  Pre-process argv so that
    # '--cmake-arg -DFOO=BAR' becomes '--cmake-arg=-DFOO=BAR' before parsing.
    argv = sys.argv[1:]
    fixed = []
    i = 0
    while i < len(argv):
        if argv[i] == "--cmake-arg" and i + 1 < len(argv) and argv[i + 1].startswith("-"):
            fixed.append(f"--cmake-arg={argv[i + 1]}")
            i += 2
        else:
            fixed.append(argv[i])
            i += 1
    return parser.parse_args(fixed)


def main():
    args = parse_args()

    ext_src = Path(args.ext_src).resolve()
    if not ext_src.exists():
        sys.exit(f"Extension source not found: {ext_src}")

    slicer_src = Path(args.slicer_src).resolve()
    if not slicer_src.exists():
        sys.exit(f"Slicer source not found: {slicer_src}")

    cmake_exe = find_cmake(args.cmake)
    cpack_exe = find_cpack(cmake_exe)
    ext_name = ext_src.name

    # Derive default paths
    base = ext_src.parent
    build_dir = Path(args.build_dir) if args.build_dir else base / f"{ext_name}-build"
    install_prefix = Path(args.install_prefix) if args.install_prefix else base / f"{ext_name}-install"
    output_dir = Path(args.output_dir) if args.output_dir else build_dir

    # Determine the Slicer build tree to use
    if args.slicer_build:
        slicer_dir = Path(args.slicer_build).resolve()
        print(f"Using existing Slicer build tree: {slicer_dir}")
    else:
        dummy_dir = Path(args.dummy_build) if args.dummy_build else base / "SlicerDummyBuild"
        if args.regen_dummy or not (dummy_dir / "SlicerConfig.cmake").exists():
            generate_dummy_build_tree(dummy_dir, slicer_src, args.slicer_revision)
        else:
            print(f"Using existing dummy build tree: {dummy_dir}")
        slicer_dir = dummy_dir

    print(f"\n  Extension      : {ext_name}")
    print(f"  Extension src  : {ext_src}")
    print(f"  Slicer dir     : {slicer_dir}")
    print(f"  Build dir      : {build_dir}")
    print(f"  Install prefix : {install_prefix}")

    configure_extension(
        cmake_exe, ext_src, build_dir, slicer_dir,
        install_prefix, args.cmake_args,
    )
    build_extension(cmake_exe, build_dir, args.config)
    package_path = package_extension(cpack_exe, build_dir, args.config)
    if output_dir != build_dir:
        copy_package(package_path, output_dir)
    else:
        if package_path:
            print(f"\n[package] {package_path}")


if __name__ == "__main__":
    main()
