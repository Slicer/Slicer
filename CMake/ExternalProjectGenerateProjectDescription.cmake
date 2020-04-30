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
#!   [LICENSE_FILES <file> [...]]
#!   [SOURCE_DIR <source_dir>]
#!   )
#!
#! Generate the project description files describing its version
#! and the license.
#!
#! The generate files are:
#!
#! * 'version-<projectname>.txt' containing one line of the form '<projectname> <version>'.
#!
#! * 'license-<projectname>.txt' containing the project licenses.
#!
#! VERSION If no <version> is specified, the version is extracted from the
#!         <source_dir> running the command '${GIT_EXECUTABLE} describe --always'.
#!
#! LICENSE_FILES If no license files is specified, the first file found in this list
#!               "NOTICE COPYRIGHT Copyright COPYING LICENSE License" is appended to 'license-<projectname>.txt'.
#!               Files with the following extension are also considered: .md .rst .txt
#!
#!               If one or multiple URLs and/or paths relative to the project source
#!               directory and/or absolute paths are specified, their content is appended
#!               to 'license-<projectname>.txt'.
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
  set(multiValueArgs LICENSE_FILES)
  cmake_parse_arguments(_epgpd "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  set(name ${projectname})
  if(_epgpd_NAME)
    set(name ${_epgpd_NAME})
  endif()

  set(generated_version_file "version-${name}.txt")
  set(generated_license_file "license-${name}.txt")

  set(extract_git_version 1)

  if(_epgpd_VERSION)
    file(WRITE ${CMAKE_BINARY_DIR}/${generated_version_file} "${name} ${_epgpd_VERSION}")
    set(extract_git_version 0)
  endif()

  set(explicit_licenses 1)

  if(NOT _epgpd_LICENSE_FILES)
    foreach(filename IN ITEMS LICENSE License license LICENCE Licence licence NOTICE COPYRIGHT Copyright copyright COPYING)
      foreach(ext IN ITEMS "" .md .rst .txt)
        list(APPEND _epgpd_LICENSE_FILES ${filename}${ext})
      endforeach()
    endforeach()
    set(explicit_licenses 0)
  endif()

  ExternalProject_Get_property(${projectname} SOURCE_DIR)
  if(_epgpd_SOURCE_DIR)
    set(SOURCE_DIR ${_epgpd_SOURCE_DIR})
  endif()

  set(script "${CMAKE_BINARY_DIR}/CMakeFiles/${projectname}-generate-project-description.cmake")

  file(WRITE "${script}"
"# version
set(extract_git_version \"${extract_git_version}\")
set(license_files ${_epgpd_LICENSE_FILES})
set(explicit_licenses ${explicit_licenses})
set(BINARY_DIR \"${CMAKE_BINARY_DIR}\")
set(SOURCE_DIR \"${SOURCE_DIR}\")
if(\${extract_git_version})
  execute_process(
    COMMAND \"${GIT_EXECUTABLE}\" describe --always
    OUTPUT_VARIABLE output
    WORKING_DIRECTORY \${SOURCE_DIR}
    )
  file(WRITE \"\${BINARY_DIR}/${generated_version_file}\" \"${name} \${output}\")
endif()

# license
file(REMOVE \"\${BINARY_DIR}/${generated_license_file}\")

set(license_found 0)
foreach(license_file IN LISTS license_files)
  if(IS_ABSOLUTE \${license_file})
    set(filepath \${license_file})
    get_filename_component(license_file \${license_file} NAME)
  else()
    set(filepath \${SOURCE_DIR}/\${license_file})
  endif()
  if(license_file MATCHES \"^http\")
    set(filepath \${BINARY_DIR}/CMakeFiles/download-license)
    file(DOWNLOAD \${license_file} \${filepath})
  endif()
  if(EXISTS \${filepath})
    file(READ \${filepath} license_content)
    file(APPEND \"\${BINARY_DIR}/${generated_license_file}\"
\"#------------------------------------------------------------------------------
# \${license_file}
#------------------------------------------------------------------------------
\${license_content}

\")
    set(license_found 1)
    if(NOT explicit_licenses AND license_found)
      break()
    endif()
  endif()
endforeach()

if(NOT license_found AND EXISTS \${SOURCE_DIR}/setup.py)
  # Extract string of the form 'License [:: <text> [...]]:: <license_name>'
  set(license_name )
  file(STRINGS \${SOURCE_DIR}/setup.py content REGEX \"License :: (.*)\" LIMIT_COUNT 1)
  if(content)
    string(STRIP \${content} content)
    # Extract <license_name>
    string(REGEX MATCH \".+:: (.+)$\" _match \${content})
    set(license_name \${CMAKE_MATCH_1})
    string(REGEX REPLACE \"[\\\",']\" \"\" license_name \${license_name})
  endif()
  if(NOT license_name)
    message(AUTHOR_WARNING \"${name}: Failed to extract license information from '\${SOURCE_DIR}/setup.py'\")
    return()
  endif()
  set(content \"${projectname} is distributed under the terms of \${license_name}\")
  file(APPEND \"\${BINARY_DIR}/${generated_license_file}\" \"\${content}
\")
  set(license_found 1)
endif()

if(NOT license_found)
  message(AUTHOR_WARNING \"${name}: Could not find a license file\")
endif()
")
  
  ExternalProject_Add_Step(${projectname} generate_project_description
    COMMAND ${CMAKE_COMMAND} -P ${script}
    COMMENT "Generate ${generated_version_file} and ${generated_license_file}"
    DEPENDEES download
    DEPENDS ${script}
    BYPRODUCTS
      ${CMAKE_BINARY_DIR}/${generated_version_file}
      ${CMAKE_BINARY_DIR}/${generated_license_file}
    WORKING_DIRECTORY ${SOURCE_DIR}
    )

endfunction()
