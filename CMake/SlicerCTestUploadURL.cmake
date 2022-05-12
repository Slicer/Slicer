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

#
# slicer_ctest_upload_url
#
#   Associate a download link with the current CDash submission.
#
#   The download link is associated by generating a file with the ``.url``
#   extension containing the real URL for downloading the file and uploading
#   it using a command like ``ctest_upload(FILES /path/to/file.url)``.
#
#   The real URL is created by substituting the ``%(algo)`` and ``%(hash)``
#   placeholders in the provided ``DOWNLOAD_URL_TEMPLATE`` with the specified
#   algorithm name for ``%(algo)`` and the cryptographic hash of the specified
#   filename for ``%(hash)``.
#
#   DOWNLOAD_URL_TEMPLATE
#     Download URL template with the placeholders ``%(algo)`` and ``%(hash)``.
#
#   ALGO
#     Hash algorithm for computing the file checksum and substituting ``%(algo)``
#     in the specified URL template.
#     Supported hash algorithms depends on the version of CMake.
#     See https://cmake.org/cmake/help/latest/command/string.html#supported-hash-algorithms
#
#   FILEPATH
#     File for computing the cryptographic hash and substituting ``%(hash)`` in
#     specified URL template.
#
function(slicer_ctest_upload_url)
  set(options)
  set(oneValueArgs ALGO DOWNLOAD_URL_TEMPLATE FILEPATH)
  set(multiValueArgs)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  foreach(var DOWNLOAD_URL_TEMPLATE ALGO)
    if("${MY_${var}}" STREQUAL "")
      message(FATAL_ERROR "error: ${var} CMake variable is empty !")
    endif()
  endforeach()

  foreach(var FILEPATH)
    if(NOT EXISTS "${MY_${var}}")
      message(FATAL_ERROR "Variable ${var} is set to an inexistent directory or file ! [${${var}}]")
    endif()
  endforeach()

  _SlicerCTestUploadURL_compute_hash(hash "${MY_ALGO}" "${MY_FILEPATH}")
  message("hash: ${hash} // ${MY_DOWNLOAD_URL_TEMPLATE}")

  string(REPLACE "%(hash)" "${hash}" url_tmp "${MY_DOWNLOAD_URL_TEMPLATE}")
  string(REPLACE "%(algo)" "${MY_ALGO}" url "${url_tmp}")

  message(STATUS "DOWNLOAD_URL: ${url}")

  set(url_file "${CTEST_BINARY_DIRECTORY}/slicer_ctest_upload_url.url")
  file(WRITE ${url_file} "${url}")

  ctest_upload(FILES ${url_file} CAPTURE_CMAKE_ERROR result_var)
  message(STATUS "Upload status: ${result_var}")
  if(NOT result_var EQUAL 0)
    message(FATAL_ERROR "Failed to upload '${url_file}'")
  endif()
endfunction()

#-----------------------------------------------------------------------------
# Private helper interface

set(_SlicerCTestUploadURL_supported_algo "MD5|SHA1|SHA224|SHA256|SHA384|SHA512|SHA3_224|SHA3_256|SHA3_384|SHA3_512")

function(_SlicerCTestUploadURL_compute_hash var_hash algo file)
  # Adapted from helper function found in ExternalData CMake module.
  if("${algo}" MATCHES "^${_SlicerCTestUploadURL_supported_algo}$")
    file("${algo}" "${file}" hash)
    set("${var_hash}" "${hash}" PARENT_SCOPE)
  else()
    message(FATAL_ERROR "Hash algorithm ${algo} unimplemented.")
  endif()
endfunction()