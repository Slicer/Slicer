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

FUNCTION(slicerFunctionGetGccVersion path_to_gcc output_var)
  IF(CMAKE_COMPILER_IS_GNUCXX)
    EXECUTE_PROCESS(
      COMMAND ${path_to_gcc} -dumpversion
      RESULT_VARIABLE result
      OUTPUT_VARIABLE output
      ERROR_VARIABLE error
      )
    IF(result)
      MESSAGE(FATAL_ERROR "Failed to obtain compiler version running [${path_to_gcc} -dumpversion]: ${error}")
    ENDIF()
    SET(${output_var} ${output} PARENT_SCOPE)
  ENDIF()
ENDFUNCTION()
