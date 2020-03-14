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
#
################################################################################

function(SlicerReadApplicationProperties)
  set(options
    )
  set(oneValueArgs
    PROJECT_NAME
    PROPERTIES_VAR
    )
  set(multiValueArgs
    )
  cmake_parse_arguments(MY
    "${options}"
    "${oneValueArgs}"
    "${multiValueArgs}"
    ${ARGN}
    )

  # Check if expected global variables are defined
  set(expected_defined_vars
    Slicer_APPLICATIONS_DIR
    )
  foreach(var ${expected_defined_vars})
    if(NOT DEFINED ${var})
      message(FATAL_ERROR "Variable ${var} is not defined !")
    endif()
  endforeach()

  # Set default
  if(NOT DEFINED MY_PROJECT_NAME)
    set(MY_PROJECT_NAME "${PROJECT_NAME}")
  endif()

  # Sanity checks
  if(NOT EXISTS ${Slicer_APPLICATIONS_DIR})
    message(FATAL_ERROR "Slicer_APPLICATIONS_DIR is defined but corresponds to a nonexistent directory [${Slicer_APPLICATIONS_DIR}]")
  endif()
  if(NOT EXISTS ${Slicer_APPLICATIONS_DIR}/${MY_PROJECT_NAME})
    message(FATAL_ERROR "PROJECT_NAME corresponds to a nonexistent application directory [${Slicer_APPLICATIONS_DIR}/${MY_PROJECT_NAME}]")
  endif()
  set(property_file ${Slicer_APPLICATIONS_DIR}/${MY_PROJECT_NAME}/slicer-application-properties.cmake)
  if(NOT EXISTS ${property_file})
    message(FATAL_ERROR "Couldn't find property file 'slicer-application-properties.cmake' in directory [${Slicer_APPLICATIONS_DIR}/${MY_PROJECT_NAME}]")
  endif()

  include(${property_file})

  set(properties
    APPLICATION_NAME

    VERSION_MAJOR
    VERSION_MINOR
    VERSION_PATCH
    REVISION_TYPE
    WC_COMMIT_COUNT_OFFSET

    DESCRIPTION_SUMMARY
    DESCRIPTION_FILE

    LAUNCHER_SPLASHSCREEN_FILE
    APPLE_ICON_FILE
    WIN_ICON_FILE

    LICENSE_FILE
    )
  foreach(property_name IN LISTS properties)
    # Set property value if not already defined in caller scope
    if(NOT DEFINED ${MY_PROJECT_NAME}_${property_name})
      set(default_value ${${property_name}})
      set(${MY_PROJECT_NAME}_${property_name} "${default_value}" PARENT_SCOPE)
    endif()
    mark_as_superbuild(${MY_PROJECT_NAME}_${property_name})
  endforeach()

  if(DEFINED MY_PROPERTIES_VAR)
    set(${MY_PROPERTIES_VAR} ${properties} PARENT_SCOPE)
  endif()
endfunction()

