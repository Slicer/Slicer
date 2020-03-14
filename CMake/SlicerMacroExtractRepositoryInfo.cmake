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
# The following CMake macro will attempt to 'guess' which type of repository is
# associated with the current source directory. Then, base on the type of repository,
# it will extract its associated information.
#

#
# SlicerMacroExtractRepositoryInfo(VAR_PREFIX <var-prefix> [SOURCE_DIR <dir>])
#
# If no SOURCE_DIR is provided, it will default to CMAKE_SOURCE_DIR.
#
# The macro will define the following variables:
#  <var-prefix>_WC_TYPE - Either 'git', 'svn' or 'local' - The type will also be 'svn' if 'git-svn' is used.
#
# If a GIT repository is associated with SOURCE_DIR, the macro
# will define the following variables:
#  <var-prefix>_WC_URL - url of the repository (at SOURCE_DIR)
#  <var-prefix>_WC_ROOT - Same value as working copy URL
#  <var-prefix>_WC_REVISION_NAME - Name associated with <var-prefix>_WC_REVISION_HASH
#  <var-prefix>_WC_REVISION_HASH - current revision
#  <var-prefix>_WC_REVISION - Equal to <var-prefix>_WC_REVISION_HASH if not a git-svn repository
#
# If a SVN or GIT-SVN repository is associated with SOURCE_DIR, the macro
# will define the following variables:
#  <var-prefix>_WC_URL - url of the repository (at SOURCE_DIR)
#  <var-prefix>_WC_ROOT - root url of the associated SVN repository
#  <var-prefix>_WC_REVISION - current revision
#  <var-prefix>_WC_LAST_CHANGED_AUTHOR - author of last commit
#  <var-prefix>_WC_LAST_CHANGED_DATE - date of last commit
#  <var-prefix>_WC_LAST_CHANGED_REV - revision of last commit
#  <var-prefix>_WC_INFO
#
# If source directory is not a SVN or GIT, the macro will return early
# displaying an warning message:
#
#   -- Skipping repository info extraction: directory [/path/to/src] is not a GIT or SVN checkout

macro(SlicerMacroExtractRepositoryInfo)
  include(CMakeParseArguments)
  set(options)
  set(oneValueArgs VAR_PREFIX SOURCE_DIR)
  set(multiValueArgs)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Sanity checks
  if("${MY_VAR_PREFIX}" STREQUAL "")
    message(FATAL_ERROR "error: VAR_PREFIX should be specified !")
  endif()

  set(wc_info_prefix ${MY_VAR_PREFIX})

  if(NOT EXISTS "${MY_SOURCE_DIR}")
    set(MY_SOURCE_DIR ${CMAKE_SOURCE_DIR})
  endif()

  # Clear variables
  set(${wc_info_prefix}_WC_TYPE local)
  set(${wc_info_prefix}_WC_INFO)

  # Clear SVN, GIT_SVN specific variables
  set(${wc_info_prefix}_WC_URL "NA")
  set(${wc_info_prefix}_WC_ROOT "NA")
  set(${wc_info_prefix}_WC_REVISION 0)
  set(${wc_info_prefix}_WC_LAST_CHANGED_AUTHOR)
  set(${wc_info_prefix}_WC_LAST_CHANGED_DATE "0000-00-00")
  set(${wc_info_prefix}_WC_LAST_CHANGED_REV)
  # Clear GIT specific variables
  set(${wc_info_prefix}_WC_REVISION_NAME "NA")
  set(${wc_info_prefix}_WC_REVISION_HASH "NA")

  if(NOT EXISTS ${MY_SOURCE_DIR}/.git
      AND NOT EXISTS ${MY_SOURCE_DIR}/.svn)

    message(AUTHOR_WARNING "Skipping repository info extraction: directory [${MY_SOURCE_DIR}] is not a GIT or SVN checkout")

  else()
  
    find_package(Git REQUIRED)

    # Is <SOURCE_DIR> a git working copy ?
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-list -n 1 HEAD
      WORKING_DIRECTORY ${MY_SOURCE_DIR}
      RESULT_VARIABLE GIT_result
      OUTPUT_QUIET
      ERROR_QUIET)

    if(${GIT_result} EQUAL 0)

      set(${wc_info_prefix}_WC_TYPE git)
      GIT_WC_INFO(${MY_SOURCE_DIR} ${wc_info_prefix})
      if(${wc_info_prefix}_WC_GITSVN)
        set(${wc_info_prefix}_WC_TYPE svn)
      endif()
   
    else()

      find_package(Subversion REQUIRED)

      # Is <SOURCE_DIR> a svn working copy ?
      execute_process(COMMAND ${Subversion_SVN_EXECUTABLE} info
        WORKING_DIRECTORY ${MY_SOURCE_DIR}
        RESULT_VARIABLE Subversion_result
        OUTPUT_QUIET
        ERROR_QUIET)

      if(${Subversion_result} EQUAL 0)

        set(${wc_info_prefix}_WC_TYPE svn)
        Subversion_WC_INFO(${MY_SOURCE_DIR} ${wc_info_prefix})

      endif()
    endif()
  endif()

endmacro()


