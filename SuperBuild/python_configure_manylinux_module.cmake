cmake_minimum_required(VERSION 3.20.6...3.22.6 FATAL_ERROR)

# --------------------------------------------------------------------------
# Validate required variables
foreach(varname IN ITEMS
    PYTHON_CONFIGURE_MANYLINUX_MODULE
    PYTHON_MANYLINUX_MODULE_FILEPATH
    )
  if("${${varname}}" STREQUAL "")
    message(FATAL_ERROR "${varname} is empty")
  endif()
endforeach()

# --------------------------------------------------------------------------
# Retrieve GLIBC version
find_program(LDD_EXECUTABLE NAMES ldd)
if(NOT LDD_EXECUTABLE)
  message(FATAL_ERROR "The 'ldd' executable was not found on the system.")
endif()

# Execute ldd to retrieve version information
execute_process(
  COMMAND ${LDD_EXECUTABLE} --version
  OUTPUT_VARIABLE ldd_version_output
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_STRIP_TRAILING_WHITESPACE
)

# Match format: "ldd (any string) X.Y"
# Examples:
# - "ldd (Ubuntu GLIBC 2.31-0ubuntu9.16) 2.31"
# - "ldd (GNU libc) 2.17"
if(NOT ldd_version_output MATCHES "ldd \\(.*\\) ([0-9]+)\\.([0-9]+)")
  message(FATAL_ERROR "Failed to parse GLIBC version from 'ldd' output:\n${ldd_version_output}")
endif()

# Extract the major and minor GLIBC version numbers
set(GLIBC_VERSION_MAJOR "${CMAKE_MATCH_1}")
set(GLIBC_VERSION_MINOR "${CMAKE_MATCH_2}")

# Display the detected version
message(STATUS "Detected GLIBC version: ${GLIBC_VERSION_MAJOR}.${GLIBC_VERSION_MINOR}")

# --------------------------------------------------------------------------
# Generate the _manylinux.py module
set(_msg "Generating _manylinux module at '${PYTHON_MANYLINUX_MODULE_FILEPATH}'")
message(STATUS "${_msg}")

set(_docstring [==[
This module defines the `manylinux_compatible` function, which overrides the
default behavior of Python package installers (like `pip`) when determining
compatibility with "manylinux" platform tags, as specified in PEP 600:
https://peps.python.org/pep-0600/

Key Functionality
-----------------

Compatibility Check:

- The `manylinux_compatible` function is called by `pip` during the installation
  of binary Python wheels (e.g., ITK wheels).

- It evaluates whether the GLIBC version required by the wheel's `manylinux`
  tag is compatible with the GLIBC version available on the system used to build Slicer
  and the associated Python interpreter.

Ensuring Stability:

- Recent wheels may target newer `std::string` ABIs or GLIBC versions (e.g.,
  `manylinux_2_28`), while Slicer and its Python binaries rely on an older ABI.

- This module ensures that only wheels with compatible tags (e.g.,
  `manylinux_2_17` or older) are installed, avoiding crashes due to ABI or
  GLIBC version mismatches.

Behavior
--------

- When invoked, `manylinux_compatible` compares the major and minor GLIBC versions
  specified in the wheel's tag with those detected on the system used to build Slicer
  and the associated Python interpreter. If the
  system's GLIBC version meets or exceeds the tag's requirement, the wheel is
  deemed compatible.

- This behavior overrides the default logic described in PEP 600 and ensures
  that Slicer maintains compatibility with its build environment.

For example, on a system with GLIBC 2.17, wheels tagged `manylinux_2_17` or
older will be compatible. Wheels requiring `manylinux_2_28` will be rejected.

References
----------

- PEP 600 (Manylinux Platform Tag):
  https://peps.python.org/pep-0600/

- GCC Dual ABI Documentation:
  https://gcc.gnu.org/onlinedocs/libstdc++/manual/using_dual_abi.html

- Discussion on PEP 600:
  https://discuss.python.org/t/pep-600-text-and-example-code-for-package-installers-section-disagree/55329
]==])

file(WRITE "${PYTHON_MANYLINUX_MODULE_FILEPATH}" "\"\"\"This module defines the `manylinux_compatible` function.

This file was automatically generated. It should NOT be manually edited.
Any changes must be made to the source CMake configuration script `python_configure_manylinux_module.cmake`.

System Details
--------------

The GLIBC version detected on the system where Slicer and associated Python interpreter were compiled is:
 Major: ${GLIBC_VERSION_MAJOR}
 Minor: ${GLIBC_VERSION_MINOR}

${_docstring}
\"\"\"

from typing import NamedTuple


class _GLibCVersion(NamedTuple):
    major: int
    minor: int


def manylinux_compatible(wheel_tag_major: int, wheel_tag_minor: int, wheel_tag_arch: str, **_) -> bool:
    \"\"\"
    Determines whether a given manylinux wheel tag is compatible with this system.

    :param tag_major: Major version of GLIBC specified in the wheel's tag.
    :param tag_minor: Minor version of GLIBC specified in the wheel's tag.
    :param tag_arch: Architecture specified in the wheel's tag.
    :return: True if the wheel's GLIBC version is compatible with this system.
    \"\"\"

    # Represent the GLIBC version required by the wheel
    wheel_glibc_version = _GLibCVersion(wheel_tag_major, wheel_tag_minor)

    # Represent the GLIBC version available in the Slicer build environment
    slicer_python_glibc_version = _GLibCVersion(${GLIBC_VERSION_MAJOR}, ${GLIBC_VERSION_MINOR})

    # Check compatibility: the system must support at least the wheel's GLIBC version
    return wheel_glibc_version <= slicer_python_glibc_version
")

message(STATUS "${_msg} - done")
