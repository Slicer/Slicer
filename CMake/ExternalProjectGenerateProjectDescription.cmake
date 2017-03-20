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
#  and was partially funded by NIH grant 1U24CA194354-01
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
#! ExternalProject_GenerateProjectDescription_Step(<projectname>
#!   [NAME <name>]
#!   [VERSION <version>]
#!   [SOURCE_DIR <source_dir>]
#!   )
#!
#! Generate a project description file named 'version-<projectname>.txt'
#! containing one line of the form '<projectname> <version>'.
#!
#! VERSION If no <version> is specified, the version is extracted from the
#!         <source_dir> running the command '${GIT_EXECUTABLE} describe --always'.
#!
#! NAME Specifying <name> parameter allows to override the name used to generate
#!      the description file.
#!
#! SOURCE_DIR Specifying <source_dir> parameter allows to override the default
#!            source directory set to '${CMAKE_BINARY_DIR}/<projectname>'.
#!
function(ExternalProject_GenerateProjectDescription_Step projectname)
  set(options)
  set(oneValueArgs NAME SOURCE_DIR VERSION)
  set(multiValueArgs)
  cmake_parse_arguments(_epgpd "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(name ${projectname})
  if(_epgpd_NAME)
    set(name ${_epgpd_NAME})
  endif()

  set(description_file "version-${name}.txt")

  if(_epgpd_VERSION)
    file(WRITE ${CMAKE_BINARY_DIR}/${description_file} "${name} ${_epgpd_VERSION}")
    return()
  endif()

  set(script "${CMAKE_BINARY_DIR}/CMakeFiles/${projectname}-generate-project-description.cmake")

  file(WRITE "${script}"
"execute_process(
  COMMAND \"${GIT_EXECUTABLE}\" describe --always
  OUTPUT_VARIABLE output
  )
file(WRITE \"${CMAKE_BINARY_DIR}/${description_file}\" \"${name} \${output}\")
")

  set(source_dir ${CMAKE_BINARY_DIR}/${projectname})
  if(_epgpd_SOURCE_DIR)
    set(source_dir ${_epgpd_SOURCE_DIR})
  endif()
  
  ExternalProject_Add_Step(${projectname} generate_project_description
    COMMAND ${CMAKE_COMMAND} -P ${script}
    COMMENT "Generate ${description_file}"
    DEPENDEES download
    BYPRODUCTS ${CMAKE_BINARY_DIR}/${description_file}
    WORKING_DIRECTORY ${source_dir}
    )

endfunction()
