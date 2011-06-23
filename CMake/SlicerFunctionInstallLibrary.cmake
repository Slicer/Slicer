################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) 2010 Kitware Inc.
#
#  See Doc/copyright/copyright.txt
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
# slicerInstallLibrary
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

#
# When installing system libraries, on non-windows machines, the CMake variable
# pointing to the library may be a sym-link, in which case we don't simply want
# to install the symlink, but the actual library. This macro takes care of that.
#

function(slicerInstallLibrary)
  SLICER_PARSE_ARGUMENTS(_slicerInstallLibrary
    "FILE;DESTINATION;COMPONENT;PERMISSIONS"
    ""
    ${ARGN}
    )

  if(NOT WIN32)

    set(install_permissions)
    if(DEFINED _slicerInstallLibrary_PERMISSIONS)
      set(install_permissions PERMISSIONS ${_slicerInstallLibrary_PERMISSIONS})
    endif()

    get_filename_component(dir_tmp ${_slicerInstallLibrary_FILE} PATH)
    set(name_tmp)
    # Note: Library symlinks are always named "lib.*.dylib" on mac or "lib.so.*" on linux
    get_filename_component(lib_dir ${_slicerInstallLibrary_FILE} PATH)
    get_filename_component(lib_name ${_slicerInstallLibrary_FILE} NAME_WE)
    install(DIRECTORY ${lib_dir}/
      DESTINATION ${_slicerInstallLibrary_DESTINATION} COMPONENT ${_slicerInstallLibrary_COMPONENT}
      FILES_MATCHING PATTERN "${lib_name}*" ${install_permissions}
      PATTERN "${lib_name}*.a" EXCLUDE)
  endif()
endfunction()
