###########################################################################
#
#  Library:   CTK
#
#  Copyright (c) Kitware Inc.
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.commontk.org/LICENSE
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
###########################################################################

function(slicerFunctionGetGccVersion path_to_gcc output_var)
  if(CMAKE_COMPILER_IS_GNUCXX)
    execute_process(
      COMMAND ${path_to_gcc} -dumpversion
      RESULT_VARIABLE result
      OUTPUT_VARIABLE output
      ERROR_VARIABLE error
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_STRIP_TRAILING_WHITESPACE
      )
    if(result)
      message(FATAL_ERROR "Failed to obtain compiler version running [${path_to_gcc} -dumpversion]: ${error}")
    endif()
    set(${output_var} ${output} PARENT_SCOPE)
  endif()
endfunction()
