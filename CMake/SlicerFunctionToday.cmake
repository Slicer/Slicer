#
# Today macro - Allows to retrieve current date in a cross-platform fashion (Unix-like and windows)
#
# Adapted from the work of Benoit Rat
# See http://www.cmake.org/pipermail/cmake/2009-February/027014.html
#


function(today RESULT_VAR)

  set(today_date)
  if(WIN32)
    execute_process(COMMAND cmd /c "date /T"
                    ERROR_VARIABLE getdate_error
                    RESULT_VARIABLE getdate_result
                    OUTPUT_VARIABLE today_date
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    #message(STATUS "getdate_error:${getdate_error}")
    #message(STATUS "getdate_result:${getdate_result}")
    #message(STATUS "today_date:${today_date}")

    string(REGEX REPLACE ".*(..)/(..)/(....)"
                          "\\3-\\1-\\2"
                          today_date
                          ${today_date})
  elseif(UNIX)
    execute_process(COMMAND date "+%Y-%m-%d"
                    OUTPUT_VARIABLE today_date
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

  else()
    message(SEND_ERROR "error: Failed to obtain today date - ${RESULT_VAR} set to 0000-00-00")
    set(today_date "0000-00-00")
  endif()
  set(${RESULT_VAR} ${today_date} PARENT_SCOPE)
endfunction()

