# - Extract information from a subversion working copy
# The module defines the following variables:
#  Subversion_SVN_EXECUTABLE - path to svn command line client
#  Subversion_VERSION_SVN - version of svn command line client
#  Subversion_FOUND - true if the command line client was found
# If the command line client executable is found the macro
#  Subversion_WC_INFO(<dir> <var-prefix>)
# is defined to extract information of a subversion working copy at
# a given location. The macro defines the following variables:
#  <var-prefix>_WC_URL - url of the repository (at <dir>)
#  <var-prefix>_WC_ROOT - root url of the repository
#  <var-prefix>_WC_REVISION - current revision
#  <var-prefix>_WC_LAST_CHANGED_AUTHOR - author of last commit
#  <var-prefix>_WC_LAST_CHANGED_DATE - date of last commit
#  <var-prefix>_WC_LAST_CHANGED_REV - revision of last commit
#  <var-prefix>_WC_LAST_CHANGED_LOG - last log of base revision
#  <var-prefix>_WC_INFO - output of command `svn info <dir>'
# Example usage:
#  find_package(Subversion)
#  if(Subversion_FOUND)
#    Subversion_WC_INFO(${PROJECT_SOURCE_DIR} Project)
#    message("Current revision is ${Project_WC_REVISION}")
#    Subversion_WC_LOG(${PROJECT_SOURCE_DIR} Project)
#    message("Last changed log is ${Project_LAST_CHANGED_LOG}")
#  endif(Subversion_FOUND)

#=============================================================================
# Copyright 2006-2009 Kitware, Inc.
# Copyright 2006 Tristan Carel
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distributed this file outside of CMake, substitute the full
#  License text for the above reference.)

set(Subversion_FOUND FALSE)
set(Subversion_SVN_FOUND FALSE)

find_program(Subversion_SVN_EXECUTABLE svn
  DOC "subversion command line client")
mark_as_advanced(Subversion_SVN_EXECUTABLE)

if(Subversion_SVN_EXECUTABLE)
  set(Subversion_SVN_FOUND TRUE)
  set(Subversion_FOUND TRUE)

  macro(Subversion_WC_INFO dir prefix)
    # the subversion commands should be executed with the C locale, otherwise
    # the message (which are parsed) may be translated, Alex
    set(_Subversion_SAVED_LC_ALL "$ENV{LC_ALL}")
    set(ENV{LC_ALL} C)

    execute_process(COMMAND ${Subversion_SVN_EXECUTABLE} --version
      WORKING_DIRECTORY ${dir}
      OUTPUT_VARIABLE Subversion_VERSION_SVN
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    execute_process(COMMAND ${Subversion_SVN_EXECUTABLE} info ${dir}
      OUTPUT_VARIABLE ${prefix}_WC_INFO
      ERROR_VARIABLE Subversion_svn_info_error
      RESULT_VARIABLE Subversion_svn_info_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(NOT ${Subversion_svn_info_result} EQUAL 0)
      message(SEND_ERROR "Command \"${Subversion_SVN_EXECUTABLE} info ${dir}\" failed with output:\n${Subversion_svn_info_error}")
    else(NOT ${Subversion_svn_info_result} EQUAL 0)

      string(REGEX REPLACE "^(.*\n)?svn, version ([.0-9]+).*"
        "\\2" Subversion_VERSION_SVN "${Subversion_VERSION_SVN}")
      string(REGEX REPLACE "^(.*\n)?URL: ([^\n]+).*"
        "\\2" ${prefix}_WC_URL "${${prefix}_WC_INFO}")
      string(REGEX REPLACE "^(.*\n)?Repository Root: ([^\n]+).*"
        "\\2" ${prefix}_WC_ROOT "${${prefix}_WC_INFO}")
      string(REGEX REPLACE "^(.*\n)?Revision: ([^\n]+).*"
        "\\2" ${prefix}_WC_REVISION "${${prefix}_WC_INFO}")
      string(REGEX REPLACE "^(.*\n)?Last Changed Author: ([^\n]+).*"
        "\\2" ${prefix}_WC_LAST_CHANGED_AUTHOR "${${prefix}_WC_INFO}")
      string(REGEX REPLACE "^(.*\n)?Last Changed Rev: ([^\n]+).*"
        "\\2" ${prefix}_WC_LAST_CHANGED_REV "${${prefix}_WC_INFO}")
      string(REGEX REPLACE "^(.*\n)?Last Changed Date: ([^\n]+).*"
        "\\2" ${prefix}_WC_LAST_CHANGED_DATE "${${prefix}_WC_INFO}")

    endif(NOT ${Subversion_svn_info_result} EQUAL 0)

    # restore the previous LC_ALL
    set(ENV{LC_ALL} ${_Subversion_SAVED_LC_ALL})

  endmacro(Subversion_WC_INFO)

  macro(Subversion_WC_LOG dir prefix)
    # This macro can block if the certificate is not signed:
    # svn ask you to accept the certificate and wait for your answer
    # This macro requires a svn server network access (Internet most of the time)
    # and can also be slow since it access the svn server
    execute_process(COMMAND
      ${Subversion_SVN_EXECUTABLE} log -r BASE ${dir}
      OUTPUT_VARIABLE ${prefix}_LAST_CHANGED_LOG
      ERROR_VARIABLE Subversion_svn_log_error
      RESULT_VARIABLE Subversion_svn_log_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    if(NOT ${Subversion_svn_log_result} EQUAL 0)
      message(SEND_ERROR "Command \"${Subversion_SVN_EXECUTABLE} log -r BASE ${dir}\" failed with output:\n${Subversion_svn_log_error}")
    endif(NOT ${Subversion_svn_log_result} EQUAL 0)
  endmacro(Subversion_WC_LOG)

endif(Subversion_SVN_EXECUTABLE)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Subversion DEFAULT_MSG Subversion_SVN_EXECUTABLE)
