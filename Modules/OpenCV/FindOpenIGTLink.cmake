message(STATUS "Looking for OpenIGTLink")

# - Find an OpenIGTLink installation or build tree.

# When OpenIGTLink is found, the OpenIGTLinkConfig.cmake file is sourced to setup the
# location and configuration of OpenIGTLink.  Please read this file, or
# OpenIGTLinkConfig.cmake.in from the OpenIGTLink source tree for the full list of
# definitions.  Of particular interest is OpenIGTLink_USE_FILE, a CMake source file
# that can be included to set the include directories, library directories,
# and preprocessor macros.  In addition to the variables read from
# OpenIGTLinkConfig.cmake, this find module also defines
#  OpenIGTLink_DIR  - The directory containing OpenIGTLinkConfig.cmake.
#             This is either the root of the build tree,
#             or the lib/InsightToolkit directory.
#             This is the only cache entry.
#
#  OpenIGTLink_FOUND - Whether OpenIGTLink was found.  If this is true,
#              OpenIGTLink_DIR is okay.
#
#  USE_OpenIGTLink_FILE - The full path to the UseOpenIGTLink.cmake file.
#                 This is provided for backward
#                 compatability.  Use OpenIGTLink_USE_FILE
#                 instead.


set(OpenIGTLink_DIR_STRING "directory containing OpenIGTLinkConfig.cmake.  This is either the root of the build tree, or PREFIX/lib/igtl for an installation.")

# Search only if the location is not already known.
if(NOT OpenIGTLink_DIR)
  # Get the system search path as a list.
  if(UNIX)
    string(REGEX MATCHALL "[^:]+" OpenIGTLink_DIR_SEARCH1 "$ENV{PATH}")
  else(UNIX)
    string(REGEX REPLACE "\\\\" "/" OpenIGTLink_DIR_SEARCH1 "$ENV{PATH}")
  endif(UNIX)
  string(REGEX REPLACE "/;" ";" OpenIGTLink_DIR_SEARCH2 ${OpenIGTLink_DIR_SEARCH1})

  # Construct a set of paths relative to the system search path.
  set(OpenIGTLink_DIR_SEARCH "")
  foreach(dir ${OpenIGTLink_DIR_SEARCH2})
    set(OpenIGTLink_DIR_SEARCH ${OpenIGTLink_DIR_SEARCH} "${dir}/../lib/igtl")
  endforeach(dir)

  #
  # Look for an installation or build tree.
  #
  find_path(OpenIGTLink_DIR OpenIGTLinkConfig.cmake
    # Look for an environment variable OpenIGTLink_DIR.
    $ENV{OpenIGTLink_DIR}

    # Look in places relative to the system executable search path.
    ${OpenIGTLink_DIR_SEARCH}

    # Look in standard UNIX install locations.
    /usr/local/lib/igtl
    /usr/lib/igtl

    # Read from the CMakeSetup registry entries.  It is likely that
    # OpenIGTLink will have been recently built.
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild1]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild2]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild3]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild4]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild5]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild6]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild7]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild8]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild9]
    [HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\Settings\\StartPath;WhereBuild10]

    # Help the user find it if we cannot.
    DOC "The ${OpenIGTLink_DIR_STRING}"
  )
endif(NOT OpenIGTLink_DIR)

# If OpenIGTLink was found, load the configuration file to get the rest of the
# settings.
if(OpenIGTLink_DIR)
  set(OpenIGTLink_FOUND 1)
  include(${OpenIGTLink_DIR}/OpenIGTLinkConfig.cmake)

  # Set USE_OpenIGTLink_FILE for backward-compatability.
  set(USE_OpenIGTLink_FILE ${OpenIGTLink_USE_FILE})
else(OpenIGTLink_DIR)
  set(OpenIGTLink_FOUND 0)
  if(OpenIGTLink_FIND_REQUIRED)
    message(FATAL_ERROR "Please set OpenIGTLink_DIR to the ${OpenIGTLink_DIR_STRING}")
  endif(OpenIGTLink_FIND_REQUIRED)
endif(OpenIGTLink_DIR)

if(OpenIGTLink_FOUND)
    message(STATUS "Found OpenIGTLink")
endif(OpenIGTLink_FOUND)
