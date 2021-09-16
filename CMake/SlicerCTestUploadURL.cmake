################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) Kitware Inc.
#
#  See COPYRIGHT.txt
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


if(NOT DEFINED MIDAS_API_DISPLAY_URL)
  set(MIDAS_API_DISPLAY_URL 0)
endif()

include(CMakeParseArguments)

#
# midas_ctest_upload_url
#
#   API_URL    The url of the MIDAS server
#   FILEPATH   Compute and upload to CTest the URL of the 'filepath' that has been previously
#              uploaded to MIDAS.
#
function(midas_ctest_upload_url)
  set(options)
  set(oneValueArgs API_URL FILEPATH)
  set(multiValueArgs)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  foreach(var API_URL)
    if("${MY_${var}}" STREQUAL "")
      message(FATAL_ERROR "error: ${var} CMake variable is empty !")
    endif()
  endforeach()

  foreach(var FILEPATH)
    if(NOT EXISTS "${MY_${var}}")
      message(FATAL_ERROR "Variable ${var} is set to an inexistent directory or file ! [${${var}}]")
    endif()
  endforeach()

  execute_process(COMMAND ${CMAKE_COMMAND} -E md5sum ${MY_FILEPATH} OUTPUT_VARIABLE output)
  string(SUBSTRING "${output}" 0 32 checksum)

  get_filename_component(filename ${MY_FILEPATH} NAME)

  set(api_method "midas.bitstream.download")
  set(params "&name=${filename}")
  set(params "${params}&checksum=${checksum}")
  set(url ${MY_API_URL}/api/rest?method=${api_method}${params})

  if("${MIDAS_API_DISPLAY_URL}")
    message(STATUS "URL: ${url}")
  endif()

  set(url_file "${CTEST_BINARY_DIRECTORY}/midas_ctest_upload_url.url")
  file(WRITE ${url_file} "${url}")
  ctest_upload(FILES ${url_file} CAPTURE_CMAKE_ERROR result_var)
  message(STATUS "Upload status: ${result_var}")
  if(NOT result_var EQUAL 0)
    message(FATAL_ERROR "Failed to upload 'midas_ctest_upload_url.url'")
  endif()
endfunction()
