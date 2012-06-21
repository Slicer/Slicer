cmake_minimum_required(VERSION 2.8.4)

# Given a variable name, this function will display messages indicating what the current
# value of the variable.
# For example:
#
#   slicer_check_variable(bar bar_set)
#   message("bar_set: ${bar_set}")
#
#   set(bar "")
#   slicer_check_variable(bar bar_set)
#   message("bar_set: ${bar_set}")
#
#   set(bar "This is bar")
#   slicer_check_variable(bar bar_set)
#   message("bar_set: ${bar_set}")
#
# will output:
#  -- Checking bar variable
#  -- Checking bar variable - NOTDEFINED
#  bar_set: FALSE
#  -- Checking bar variable
#  -- Checking bar variable - EMPTY
#  bar_set: FALSE
#  -- Checking bar variable
#  -- Checking bar variable - This is bar
#  bar_set: TRUE
#
function(slicer_check_variable name resultvar)
  message(STATUS "Checking ${name} variable")
  set(result FALSE)
  if(NOT DEFINED ${name})
    message(STATUS "Checking ${name} variable - NOTDEFINED")
  elseif("${${name}}" STREQUAL "")
    message(STATUS "Checking ${name} variable - EMPTY")
  else()
    message(STATUS "Checking ${name} variable - ${${name}}")
    set(result TRUE)
  endif()
  set(${resultvar} ${result} PARENT_SCOPE)
endfunction()

# Given a variable name, this function will display the text
#   "-- Setting <varname> ........: ${<varname>}"
# and will ensure that the message is consistenly padded.
#
# If the variable is not defined, it will display:
#   "-- Setting <varname> ........: <NOT DEFINED>"
#
# If the optional argument 'SKIP_TRUNCATE' is provided, the
# text will NOT be truncated it too long.
#
# In the current implementation, the padding is hardcoded to a length of 40
# and the total text will be truncated if longer than 120 characters.
#
# For example:
#   set(SHORTNAME "This is short variable name")
#   slicer_setting_variable_message("SHORTNAME")
#   set(LONGLONGNAME "This is a longer variable name")
#   slicer_setting_variable_message("LONGLONGNAME")
#
# will output:
#   -- Setting SHORTNAME ........: This is short variable name
#   -- Setting LONGLONGNAME .....: This is a longer variable name
#
function(slicer_setting_variable_message varname)
  set(truncate TRUE)
  if("${ARGV1}" STREQUAL "SKIP_TRUNCATE")
    set(truncate FALSE)
  endif()
  set(pretext_right_jusitfy_length 45)
  set(fill_char ".")
  set(truncated_text_length 120)

  set(value ${${varname}})
  if(NOT DEFINED ${varname})
    set(value "NOT DEFINED")
  endif()

  set(pretext "Setting ${varname}")
  string(LENGTH ${pretext} pretext_length)
  math(EXPR pad_length "${pretext_right_jusitfy_length} - ${pretext_length} - 1")
  if(pad_length GREATER 0)
    string(RANDOM LENGTH ${pad_length} ALPHABET ${fill_char} pretext_dots)
    set(text "${pretext} ${pretext_dots}: ${value}")
  elseif(pad_length EQUAL 0)
    set(text "${pretext} : ${value}")
  else()
    set(text "${pretext}: ${value}")
  endif()
  string(LENGTH ${text} text_length)
  if(${truncate} AND ${text_length} GREATER ${truncated_text_length})
    string(SUBSTRING ${text} 0 ${truncated_text_length} text)
    set(text "${text} [...]")
  endif()
  message(STATUS ${text})
endfunction()

################################################################################
# Testing
################################################################################

#
# cmake -DTEST_slicer_setting_variable_message_test:BOOL=ON -P UseSlicerMacros.cmake
#
function(slicer_setting_variable_message_test)

  set(SHORTNAME "This is short variable name")
  slicer_setting_variable_message("SHORTNAME")

  set(LONGLONGNAME "This is a longer variable name")
  slicer_setting_variable_message("LONGLONGNAME")

  set(VARIABLE_NAME_WITH_FORTY_FOUR_CHARS "This is a variable name where len('Setting ') + len(varname) is just 44 characters")
  slicer_setting_variable_message("VARIABLE_NAME_WITH_FORTY_FOUR_CHARS")

  set(A_VARIABLE_NAME_JUST_WITH_FORTY_FIVE "This is a variable name where len('Setting ') + len(varname) is just 45 characters")
  slicer_setting_variable_message("A_VARIABLE_NAME_JUST_WITH_FORTY_FIVE")

  set(VARNAME_WITH_JUST_FORTY_SIX_CHARACTER "This is a variable name where len('Setting ') + len(varname) is just 46 characters")
  slicer_setting_variable_message("VARNAME_WITH_JUST_FORTY_SIX_CHARACTER")

  set(THIS_IS_A_LONG_VARIABLE_NAME_OVER_FORTY_FIVE_CHARS "This is a long variable name over forty five characters")
  slicer_setting_variable_message("THIS_IS_A_LONG_VARIABLE_NAME_OVER_FORTY_FIVE_CHARS")

  message("SUCCESS")
endfunction()
if(TEST_slicer_setting_variable_message_test)
  slicer_setting_variable_message_test()
endif()

