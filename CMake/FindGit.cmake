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
# The module defines the following variables:
#   GIT_EXECUTABLE - path to git command line client
#   GIT_FOUND - true if the command line client was found
#
# If the command line client executable is found the macro
#  GIT_WC_INFO(<dir> <var-prefix>)
# is defined to extract information of a git working copy at
# a given location. 
# 
# The macro defines the following variables:
#  <var-prefix>_WC_REVISION_HASH - Current SHA1 hash
#  <var-prefix>_WC_REVISION - Current SHA1 hash
#  <var-prefix>_WC_REVISION_NAME - Name associated with <var-prefix>_WC_REVISION_HASH
#  <var-prefix>_WC_URL - output of command `git config --get remote.origin.url'
#  <var-prefix>_WC_GITSVN - Set to false
# 
# ... and also the following ones if it's a git-svn repository:
#  <var-prefix>_WC_GITSVN - Set to True if it is a 
#  <var-prefix>_WC_INFO - output of command `git svn info'
#  <var-prefix>_WC_URL - url of the associated SVN repository
#  <var-prefix>_WC_REVISION - current SVN revision number
#  <var-prefix>_WC_LAST_CHANGED_AUTHOR - author of last commit
#  <var-prefix>_WC_LAST_CHANGED_DATE - date of last commit
#  <var-prefix>_WC_LAST_CHANGED_REV - revision of last commit
#  <var-prefix>_WC_LAST_CHANGED_LOG - last log of base revision
#
# Example usage:
#   find_package(Git)
#   if(GIT_FOUND)
#    GIT_WC_INFO(${PROJECT_SOURCE_DIR} Project)
#    message("Current revision is ${Project_WC_REVISION_HASH}")
#    message("git found: ${GIT_EXECUTABLE}")
#   endif()
#

# Look for 'git' or 'eg' (easy git)
#
set(git_names git eg)

# Prefer .cmd variants on Windows unless running in a Makefile
# in the MSYS shell.
#
if(WIN32)
  if(NOT CMAKE_GENERATOR MATCHES "MSYS")
    set(git_names git.cmd git eg.cmd eg)
  endif()
endif()

FIND_PROGRAM(GIT_EXECUTABLE ${git_names}
  PATHS
    "C:/Program Files/Git/bin"
    "C:/Program Files (x86)/Git/bin"
  DOC "git command line client")
MARK_AS_ADVANCED(GIT_EXECUTABLE)
 
IF(GIT_EXECUTABLE)
  MACRO(GIT_WC_INFO dir prefix)
    EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} rev-list -n 1 HEAD
       WORKING_DIRECTORY ${dir}
       ERROR_VARIABLE GIT_error
       OUTPUT_VARIABLE ${prefix}_WC_REVISION_HASH
       OUTPUT_STRIP_TRAILING_WHITESPACE)
    SET(${prefix}_WC_REVISION ${${prefix}_WC_REVISION_HASH})
    if(NOT ${GIT_error} EQUAL 0)
      MESSAGE(SEND_ERROR "Command \"${GIT_EXECUTBALE} rev-list -n 1 HEAD\" in directory ${dir} failed with output:\n${GIT_error}")
    ELSE(NOT ${GIT_error} EQUAL 0)
      EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} name-rev ${${prefix}_WC_REVISION_HASH}
         WORKING_DIRECTORY ${dir}
         OUTPUT_VARIABLE ${prefix}_WC_REVISION_NAME
          OUTPUT_STRIP_TRAILING_WHITESPACE)
    ENDIF(NOT ${GIT_error} EQUAL 0)
    
    EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} config --get remote.origin.url
       WORKING_DIRECTORY ${dir}
       OUTPUT_VARIABLE ${prefix}_WC_URL
       OUTPUT_STRIP_TRAILING_WHITESPACE)
       
    SET(${prefix}_WC_GITSVN False)
    
    # In case git-svn is used, attempt to extract svn info
    EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} svn info
      WORKING_DIRECTORY ${dir}
      ERROR_VARIABLE git_svn_info_error
      OUTPUT_VARIABLE ${prefix}_WC_INFO
      RESULT_VARIABLE git_svn_info_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)
      
    IF(${git_svn_info_result} EQUAL 0)
      SET(${prefix}_WC_GITSVN True)
      STRING(REGEX REPLACE "^(.*\n)?URL: ([^\n]+).*"
        "\\2" ${prefix}_WC_URL "${${prefix}_WC_INFO}")
      STRING(REGEX REPLACE "^(.*\n)?Revision: ([^\n]+).*"
        "\\2" ${prefix}_WC_REVISION "${${prefix}_WC_INFO}")
      STRING(REGEX REPLACE "^(.*\n)?Last Changed Author: ([^\n]+).*"
        "\\2" ${prefix}_WC_LAST_CHANGED_AUTHOR "${${prefix}_WC_INFO}")
      STRING(REGEX REPLACE "^(.*\n)?Last Changed Rev: ([^\n]+).*"
        "\\2" ${prefix}_WC_LAST_CHANGED_REV "${${prefix}_WC_INFO}")
      STRING(REGEX REPLACE "^(.*\n)?Last Changed Date: ([^\n]+).*"
        "\\2" ${prefix}_WC_LAST_CHANGED_DATE "${${prefix}_WC_INFO}")
    ENDIF(${git_svn_info_result} EQUAL 0)
    
  ENDMACRO(GIT_WC_INFO)
ENDIF(GIT_EXECUTABLE)

# Handle the QUIETLY and REQUIRED arguments and set GIT_FOUND to TRUE if
# all listed variables are TRUE

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Git DEFAULT_MSG GIT_EXECUTABLE)
