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

#
# When installing system libraries, on non-windows machines, the CMake variable
# pointing to the library may be a sym-link, in which case we don't simply want
# to install the symlink, but the actual library. This macro takes care of that.
#

FUNCTION(slicerInstallLibrary)
  SLICER_PARSE_ARGUMENTS(_slicerInstallLibrary
    "FILE;DESTINATION;COMPONENT"
    ""
    ${ARGN}
    )

  IF(NOT WIN32)
  
    GET_FILENAME_COMPONENT(dir_tmp ${_slicerInstallLibrary_FILE} PATH)
    SET(name_tmp)
    # libs symlinks are always named lib.*.dylib on mac
    # libs symlinks are always named lib.so.* on linux

    GET_FILENAME_COMPONENT(lib_dir ${_slicerInstallLibrary_FILE} PATH)
    GET_FILENAME_COMPONENT(lib_name ${_slicerInstallLibrary_FILE} NAME_WE)
    INSTALL(DIRECTORY ${lib_dir}/ 
      DESTINATION ${_slicerInstallLibrary_DESTINATION} COMPONENT ${_slicerInstallLibrary_COMPONENT}
      FILES_MATCHING PATTERN "${lib_name}*"
      PATTERN "${lib_name}*.a" EXCLUDE)
  ENDIF()
ENDFUNCTION()
