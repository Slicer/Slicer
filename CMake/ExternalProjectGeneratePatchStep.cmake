################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) Oslo University Hospital
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
#   This file was originally developed by Rafael Palomar (The Intervention
#   Centre, Oslo University Hospital) and was supported by The Research
#   Council of Norway through the ALive project (grant nr. 311393).
#
################################################################################

#
#  W A R N I N G
#  -------------
#
# This file is not part of the Slicer API.  It exists purely as an
# implementation detail.  This CMake module may change from version to
# version without notice, or even be removed.
#
# We mean it.
#

#!
#! ExternalProject_GeneratePatch_Step(<projectname>)
#!
#! Creates and populates the variable ${projectname}_PATCH_STEP with the
#! commands to apply a set of patch flies, previously defined in ${projectname}_PATCHES.
#!
#! This is useful to provide the patching commands to the PATCH_COMMAND
#! parameter of ExternalProject_Add() cmake function.
#!
#! Example of use:
#!
#!    # Create the list of patches.
#!       list(APPEND ${proj}_PATCHES
#!         ${CMAKE_SOURCE_DIR}/Patches/001.patch
#!         ${CMAKE_SOURCE_DIR}/Patches/001.patch
#!       )
#!
#!    # Call ExternalProject_GeneratePatch_Step function
#!    ExternalProject_GeneratePatch_Step(${proj})
#!
#!    # Add the generated patch steps as parameter of ExternalProject_Add
#!    ExternalProject_Add(${proj}
#!      ...
#!      PATCH_COMMAND
#!        ${${proj}_PATCH_STEP} # Apply patches
#!      CMAKE_CACHE_ARGS
#!      ...
#!    )
#!
#!
function(ExternalProject_GeneratePatch_Step projectname)

  # --------------------------------------------------------------------------
  # Sanity checks
  # --------------------------------------------------------------------------

  # Check expected defined variables
  set(expected_existing_vars ${projectname}_PATCHES GIT_EXECUTABLE)
  foreach(var ${expected_existing_vars})
    if(NOT DEFINED "${var}")
      message(FATAL_ERROR "error: Variable ${var} is expected exist. ")
    endif()
  endforeach()

  # Check patch file exist
  foreach(file ${${projectname}_PATCHES})
    if(NOT EXISTS "${file}")
      message(FATAL_ERROR "error: Patch file ${file} not found. ")
    endif()
  endforeach()

  # --------------------------------------------------------------------------
  # Generate Patching step
  # --------------------------------------------------------------------------
  #
  foreach(PATCH ${${projectname}_PATCHES})
    if(NOT "${${projectname}_PATCH_STEP}" STREQUAL "")
      list(APPEND ${projectname}_PATCH_STEP " && ")
    endif()

    list(APPEND _${projectname}_PATCH_STEP
      echo Applying patch ${PATCH} && ${GIT_EXECUTABLE} apply ${PATCH} &> /dev/null || echo Patch does not apply
      )
  endforeach()

  set(${projectname}_PATCH_STEP ${_${projectname}_PATCH_STEP} PARENT_SCOPE)
endfunction()
