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

# See http://github.com/commontk/CTK/blob/master/CMake/ctkFunctionCheckCompilerFlags.cmake

#
# Helper macro allowing to check if the given flags are supported 
# by the underlying build tool
#
# If the flag(s) is/are supported, they will be appended to the string identified by RESULT_VAR
#
# Usage:
#   ctkFunctionCheckCompilerFlags(FLAGS_TO_CHECK VALID_FLAGS_VAR)
#
# Example:
#
#   set(myflags)
#   ctkFunctionCheckCompilerFlags("-fprofile-arcs" myflags)
#   message(1-myflags:${myflags})
#   ctkFunctionCheckCompilerFlags("-fauto-bugfix" myflags)
#   message(2-myflags:${myflags})
#   ctkFunctionCheckCompilerFlags("-Wall" myflags)
#   message(1-myflags:${myflags})
#
#   The output will be:
#    1-myflags: -fprofile-arcs
#    2-myflags: -fprofile-arcs
#    3-myflags: -fprofile-arcs -Wall

INCLUDE(TestCXXAcceptsFlag)

FUNCTION(SlicerFunctionCheckCompilerFlags CXX_FLAG_TO_TEST RESULT_VAR)
  
  IF(CXX_FLAG_TO_TEST STREQUAL "")
    MESSAGE(FATAL_ERROR "CXX_FLAG_TO_TEST shouldn't be empty")
  ENDIF()
  
  CHECK_CXX_ACCEPTS_FLAG(${CXX_FLAG_TO_TEST} HAS_FLAG)

  IF(HAS_FLAG)
    SET(${RESULT_VAR} "${${RESULT_VAR}} ${CXX_FLAG_TO_TEST}" PARENT_SCOPE)
  ENDIF()

ENDFUNCTION()

