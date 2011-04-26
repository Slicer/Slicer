#
# Today macro - Allows to retrieve current date in a cross-platform fashion (Unix-like and windows)
#
# Adapted from the work of Benoit Rat
# See http://www.cmake.org/pipermail/cmake/2009-February/027014.html
#


FUNCTION (TODAY RESULT_VAR)
  
  set(today_date)
  IF (WIN32)
    EXECUTE_PROCESS(COMMAND cmd /c "date /T"
                    ERROR_VARIABLE getdate_error
                    RESULT_VARIABLE getdate_result
                    OUTPUT_VARIABLE today_date
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    #MESSAGE(STATUS "getdate_error:${getdate_error}")
    #MESSAGE(STATUS "getdate_result:${getdate_result}")
    #MESSAGE(STATUS "today_date:${today_date}")

    string(REGEX REPLACE ".*(..)/(..)/(....)"
                          "\\3-\\1-\\2"
                          today_date
                          ${today_date})
  ELSEIF(UNIX)
    EXECUTE_PROCESS(COMMAND date "+%Y-%m-%d"
                    OUTPUT_VARIABLE today_date
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

  ELSE (WIN32)
    MESSAGE(SEND_ERROR "error: Failed to obtain today date - ${RESULT_VAR} set to 0000-00-00")
    SET(today_date "0000-00-00")
  ENDIF (WIN32)
  SET(${RESULT_VAR} ${today_date} PARENT_SCOPE)
ENDFUNCTION (TODAY)

