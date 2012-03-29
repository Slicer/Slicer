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
# MIDASCTestUploadURL
#

#
# filepath - Compute and upload to CTest the URL of the 'filepath' that has been previously
#            uploaded to MIDAS.
#
function(midas_ctest_upload_url filepath)

  if(NOT EXISTS "${filepath}")
    message(FATAL_ERROR "error: filepath doesn't exist ! [${filepath}]")
  endif()

  if("${MIDAS_PACKAGE_URL}" STREQUAL "")
    message(FATAL_ERROR "error: MIDAS_PACKAGE_URL is either not defined or set to an empty string !")
  endif()

  get_filename_component(filename ${filepath} NAME)

  execute_process(COMMAND ${CMAKE_COMMAND} -E md5sum ${filepath} OUTPUT_VARIABLE output)
  string(SUBSTRING "${output}" 0 32 computedChecksum)

  set(url ${MIDAS_PACKAGE_URL}/api/rest?method=midas.bitstream.download&name=${filename}&checksum=${computedChecksum})

  file(WRITE "${CTEST_BINARY_DIRECTORY}/slicerFunctionCTestUploadURL.url" "${url}")
  ctest_upload(FILES "${CTEST_BINARY_DIRECTORY}/slicerFunctionCTestUploadURL.url")
endfunction()

