################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) Kitware Inc.
#
#  See COPYRIGHT.txt
#  or http://www.slicer.org/copyright/copyright.txt for details.
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################

#
# slicerInstallLibrary(
#   FILE <file>
#   DESTINATION <destination>
#   COMPONENT <component>
#   [PERMISSIONS permissions...]
#   [STRIP [STRIP_CONFIGURATIONS Release|RelWithDebInfo|...]]
#   )
#
#
# When installing system libraries, on non-windows machines, the CMake variable
# pointing to the library may be a sym-link, in which case we don't simply want
# to install the symlink, but the actual library. This macro takes care of that.
#
#
#  FILE  ........: Path to the library
#
#  DESTINATION ..: Subdirectory relative to the install dir
#
#  COMPONENT ....: Usually Runtime or RuntimeLibraries
#
#  PERMISSIONS ..: Specify permissions for installed files. Valid permissions are OWNER_READ,
#                  OWNER_WRITE, OWNER_EXECUTE, GROUP_READ, GROUP_WRITE, GROUP_EXECUTE, WORLD_READ,
#                  WORLD_WRITE, WORLD_EXECUTE, SETUID, and SETGID.
#                  Permissions that do not make sense on certain platforms are ignored
#                  on those platforms.
#
#  STRIP ........: Strip regular symbols from the ELF library.
#
#  STRIP_CONFIGURATIONS: Specify a list of build configurations for which the install
#                        rule applies (Release, etc.). Default is "Release".
#

function(slicerInstallLibrary)
  set(options
    STRIP
    )
  set(oneValueArgs
    FILE
    DESTINATION
    COMPONENT
    )
  set(multiValueArgs
    PERMISSIONS
    STRIP_CONFIGURATIONS
    )
  cmake_parse_arguments(_slicerInstallLibrary
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )
  if(_slicerInstallLibrary_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to slicerInstallLibrary(): \"${_slicerInstallLibrary_UNPARSED_ARGUMENTS}\"")
  endif()

  if(NOT WIN32)

    set(install_permissions)
    if(DEFINED _slicerInstallLibrary_PERMISSIONS)
      set(install_permissions PERMISSIONS ${_slicerInstallLibrary_PERMISSIONS})
    endif()

    set(strip_configs "Release")
    if(DEFINED _slicerInstallLibrary_STRIP_CONFIGURATIONS)
      set(strip_configs ${_slicerInstallLibrary_STRIP_CONFIGURATIONS})
    endif()

    get_filename_component(dir_tmp ${_slicerInstallLibrary_FILE} PATH)
    # Note: Library symlinks are always named "lib.*.dylib" on mac or "lib.so.*" on linux
    get_filename_component(lib_dir ${_slicerInstallLibrary_FILE} PATH)
    get_filename_component(lib_name ${_slicerInstallLibrary_FILE} NAME_WE)
    install(DIRECTORY ${lib_dir}/
      DESTINATION ${_slicerInstallLibrary_DESTINATION} COMPONENT ${_slicerInstallLibrary_COMPONENT}
      FILES_MATCHING PATTERN "${lib_name}*" ${install_permissions}
      PATTERN "${lib_name}*.a" EXCLUDE
      PATTERN "${lib_name}*.debug" EXCLUDE)

    if(_slicerInstallLibrary_STRIP)
      slicerStripInstalledLibrary(
        FILES "${_slicerInstallLibrary_DESTINATION}/${lib_name}.so"
        COMPONENT ${_slicerInstallLibrary_COMPONENT}
        CONFIGURATIONS ${strip_configs}
        )
    endif()
  endif()
endfunction()


#
# slicerStripInstalledLibrary(
#   FILES files... | PATTERN <pattern>
#   [COMPONENT <component>]
#   [CONFIGURATIONS Release|RelWithDebInfo|...]
#   )
#
#
# Strip regular symbols from ELF library or exectuable.
#
#
#  FILES ......: Relative paths to the libraries or executables in the install tree.
#
#  PATTERN ....: Specify a globbing pattern to recursively match files encountered in
#                the install tree.
#
#  COMPONENT ..: Usually Runtime or RuntimeLibraries
#
#  CONFIGURATIONS ..: Specify a list of build configurations for which the install
#                     rule applies (Release, etc.). Default is "Release".
#
# Notes:
# * CMAKE_STRIP variable is required to be set to an existing file.
# * Calling this function on Apple or Windows is will not strip symbols.
#
# To learn the differences between dynamic and regular tables in ELF executable.
# See https://stackoverflow.com/questions/9961473/nm-vs-readelf-s/9961534#9961534
# and http://timetobleed.com/dynamic-symbol-table-duel-elf-vs-mach-o-round-2/
#

function(slicerStripInstalledLibrary)
  set(options
    )
  set(oneValueArgs
    COMPONENT
    PATTERN
    )
  set(multiValueArgs
    FILES
    CONFIGURATIONS
    )
  cmake_parse_arguments(_slicerStripInstalledLibrary
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )
  if(_slicerStripInstalledLibrary_UNPARSED_ARGUMENTS)
    message(FATAL_ERROR "Unknown keywords given to slicerStripInstalledLibrary(): \"${_slicerStripInstalledLibrary_UNPARSED_ARGUMENTS}\"")
  endif()

  # Sanity checks
  if(NOT _slicerStripInstalledLibrary_FILES AND NOT _slicerStripInstalledLibrary_PATTERN)
    message(FATAL_ERROR "FILES or PATTERN argument must be specified")
  endif()

  set(expected_defined_vars
    COMPONENT
    )
  foreach(var ${expected_defined_vars})
    if(NOT DEFINED _slicerStripInstalledLibrary_${var})
      message(FATAL_ERROR "${var} is mandatory")
    endif()
  endforeach()

  # Defaults
  set(configs "Release")
  if(DEFINED _slicerStripInstalledLibrary_CONFIGURATIONS)
    set(configs ${_slicerStripInstalledLibrary_CONFIGURATIONS})
  endif()

  if(UNIX AND NOT APPLE)
    if("${CMAKE_BUILD_TYPE}" IN_LIST configs)
      if(NOT EXISTS "${CMAKE_STRIP}")
        message(FATAL_ERROR "failed to add install rule for stripping symbols of '${file}'. CMAKE_STRIP CMake variable is either not set or pointing to an nonexistent file.")
      endif()
      set(file "${_slicerStripInstalledLibrary_FILES}")
      set(dollar "$")
      if(_slicerStripInstalledLibrary_FILES)
        install(
          CODE "message(STATUS \"Stripping: ${dollar}ENV{DESTDIR}${dollar}{CMAKE_INSTALL_PREFIX}/${file}\")
execute_process(COMMAND \"${CMAKE_STRIP}\" \"${dollar}ENV{DESTDIR}${dollar}{CMAKE_INSTALL_PREFIX}/${file}\")"
          COMPONENT ${_slicerStripInstalledLibrary_COMPONENT}
          )
      endif()
      if(_slicerStripInstalledLibrary_PATTERN)
        install(
          CODE "file(
  GLOB_RECURSE libraries FOLLOW_SYMLINKS
  LIST_DIRECTORIES false
  RELATIVE ${dollar}ENV{DESTDIR}${dollar}{CMAKE_INSTALL_PREFIX}
  ${dollar}ENV{DESTDIR}${dollar}{CMAKE_INSTALL_PREFIX}/${_slicerStripInstalledLibrary_PATTERN}
  )
foreach(file IN LISTS libraries)
  message(STATUS \"Stripping: ${dollar}ENV{DESTDIR}${dollar}{CMAKE_INSTALL_PREFIX}/${dollar}{file}\")
  execute_process(COMMAND \"${CMAKE_STRIP}\" \"${dollar}ENV{DESTDIR}${dollar}{CMAKE_INSTALL_PREFIX}/${dollar}{file}\")
endforeach()
"
          COMPONENT ${_slicerStripInstalledLibrary_COMPONENT}
          )
      endif()
    endif()
  endif()
endfunction()
