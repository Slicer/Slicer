#
# FindGit
#

SET(GIT_FOUND FALSE)
 
FIND_PROGRAM(GIT_EXECUTABLE git
  PATHS
    "C:/Program Files/Git/bin"
    "C:/Program Files (x86)/Git/bin"
  DOC "git command line client")
MARK_AS_ADVANCED(GIT_EXECUTABLE)
 
IF(GIT_EXECUTABLE)
  SET(GIT_FOUND TRUE)
  MACRO(GIT_WC_INFO dir prefix)
    EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} rev-list -n 1 HEAD
       WORKING_DIRECTORY ${dir}
       ERROR_VARIABLE GIT_error
       OUTPUT_VARIABLE ${prefix}_WC_REVISION_HASH
       OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT ${GIT_error} EQUAL 0)
      MESSAGE(SEND_ERROR "Command \"${GIT_EXECUTBALE} rev-list -n 1 HEAD\" in directory ${dir} failed with output:\n${GIT_error}")
    ELSE(NOT ${GIT_error} EQUAL 0)
      EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} name-rev ${${prefix}_WC_REVISION_HASH}
         WORKING_DIRECTORY ${dir}
         OUTPUT_VARIABLE ${prefix}_WC_REVISION_NAME
          OUTPUT_STRIP_TRAILING_WHITESPACE)
    ENDIF(NOT ${GIT_error} EQUAL 0)

    # In case, git-svn is used, attempt to extract svn info
    EXECUTE_PROCESS(COMMAND ${GIT_EXECUTABLE} svn info
      WORKING_DIRECTORY ${dir}
      ERROR_VARIABLE git_svn_info_error
      OUTPUT_VARIABLE ${prefix}_WC_INFO
      RESULT_VARIABLE git_svn_info_result
      OUTPUT_STRIP_TRAILING_WHITESPACE)

    IF(NOT ${git_svn_info_result} EQUAL 0)
      #MESSAGE(SEND_ERROR "Command \"${GIT_SVN_EXECUTABLE} info ${dir}\" failed with output:\n${git_svn_info_error}")
    ELSE(NOT ${git_svn_info_result} EQUAL 0)

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

    ENDIF(NOT ${git_svn_info_result} EQUAL 0)
    
  ENDMACRO(GIT_WC_INFO)
ENDIF(GIT_EXECUTABLE)
