###########################################################################
#
#  Library:   CTK
#
#  Copyright (c) Kitware Inc.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.commontk.org/LICENSE
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
###########################################################################

# - Find a CTK installation or build tree.
# The following variables are set if CTK is found.  If CTK is not
# found, CTK_FOUND is set to false.
#  CTK_FOUND         - Set to true when CTK is found.
#  CTK_USE_FILE      - CMake file to use CTK.
#  CTK_MAJOR_VERSION - The CTK major version number.
#  CTK_MINOR_VERSION - The CTK minor version number
#                       (odd non-release).
#  CTK_BUILD_VERSION - The CTK patch level
#                       (meaningless for odd minor).
#  CTK_INCLUDE_DIRS  - Include directories for CTK
#  CTK_LIBRARY_DIRS  - Link directories for CTK libraries
#  CTK_LIBRARIES     - List of libraries
#
# The following cache entries must be set by the user to locate VTK:
#  CTK_DIR  - The directory containing CTKConfig.cmake.
#             This is either the root of the build tree,
#             or the lib/ctk directory.  This is the
#             only cache entry.
#

# Assume not found.
SET(CTK_FOUND 0)

# Construct consitent error messages for use below.
SET(CTK_DIR_MESSAGE "CTK not found.  Set the CTK_DIR cmake cache entry to the directory containing CTKConfig.cmake.  This is either the root of the build tree, or PREFIX/lib/ctk for an installation.")

# Use the Config mode of the find_package() command to find CTKConfig.
# If this succeeds (possibly because CTK_DIR is already set), the
# command will have already loaded CTKConfig.cmake and set CTK_FOUND.
IF(NOT CTK_FOUND)
  FIND_PACKAGE(CTK QUIET NO_MODULE)
ENDIF()

#-----------------------------------------------------------------------------
IF(NOT CTK_FOUND)
  # CTK not found, explain to the user how to specify its location.
  IF(CTK_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR ${CTK_DIR_MESSAGE})
  ELSE(CTK_FIND_REQUIRED)
    IF(NOT CTK_FIND_QUIETLY)
      MESSAGE(STATUS ${CTK_DIR_MESSAGE})
    ENDIF(NOT CTK_FIND_QUIETLY)
  ENDIF(CTK_FIND_REQUIRED)
ENDIF()

