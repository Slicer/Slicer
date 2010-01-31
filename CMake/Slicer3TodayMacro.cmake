#
# Today macro - Allows to retrieve current date in a cross-platform fashion (Unix-like and windows)
#
# Adapted from the work of Benoit Rat
# See http://www.cmake.org/pipermail/cmake/2009-February/027014.html
#


MACRO (TODAY RESULT)
  IF (WIN32)
    EXECUTE_PROCESS(COMMAND cmd /c "date /T"
                    ERROR_VARIABLE getdate_error
                    RESULT_VARIABLE getdate_result
                    OUTPUT_VARIABLE ${RESULT}
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    #MESSAGE(STATUS "getdate_error:${getdate_error}")
    #MESSAGE(STATUS "getdate_result:${getdate_result}")
    #MESSAGE(STATUS "${RESULT}:${${RESULT}}")

    string(REGEX REPLACE ".*(..)/(..)/(....)"
                          "\\3-\\1-\\2"
                          ${RESULT}
                          ${${RESULT}})
  ELSEIF(UNIX)
    EXECUTE_PROCESS(COMMAND date "+%Y-%m-%d"
                    OUTPUT_VARIABLE ${RESULT}
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

  ELSE (WIN32)
    MESSAGE(SEND_ERROR "error: Failed to obtain today date - ${RESULT} set to 0000-00-00")
    SET(${RESULT} 0000-00-00)
  ENDIF (WIN32)
ENDMACRO (TODAY)

