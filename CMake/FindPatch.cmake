# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindPatch
# ---------
#
# The module defines the following variables:
#
# ``PATCH_EXECUTABLE``
#   Path to patch command-line executable.
# ````Patch_FOUND``, ``PATCH_FOUND``
#   True if the patch command-line executable was found.
#
# Example usage:
#
# .. code-block:: cmake
#
#    find_package(Patch)
#    if(Patch_FOUND)
#      message("Patch found: ${Patch_EXECUTABLE}")
#    endif()

set(_x86 "(x86)")  # Indirection required to avoid error related to CMP0053
find_program(PATCH_EXECUTABLE
  NAME patch
  PATHS "$ENV{ProgramFiles}/Git/usr/bin"
        "$ENV{ProgramFiles${_x86}}/Git/usr/bin"
        "$ENV{ProgramFiles}/GnuWin32/bin"
        "$ENV{ProgramFiles${_x86}}/GnuWin32/bin"
        "$ENV{ProgramFiles}/Git/bin"
        "$ENV{ProgramFiles${_x86}}/Git/bin"
        "$ENV{LOCALAPPDATA}/Programs/Git/bin"
        "$ENV{LOCALAPPDATA}/Programs/Git/usr/bin"
        "$ENV{APPDATA}/Programs/Git/bin"
        "$ENV{APPDATA}/Programs/Git/usr/bin"
  DOC "Patch command line executable"
  )

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Patch
                                  REQUIRED_VARS PATCH_EXECUTABLE)
