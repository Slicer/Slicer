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
#  This file was originally developed by Zach Mullen and Jean-Christophe Fillion-Robin, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################

if(NOT DEFINED MIDAS_API_DISPLAY_URL)
  set(MIDAS_API_DISPLAY_URL 0)
endif()

include(CMakeParseArguments)

#
# Uploads a package to the MIDAS server.
#
#   SERVER_URL The url of the MIDAS server
#   SERVER_EMAIL The email to use to authenticate to the server
#   SERVER_APIKEY The default api key to use to authenticate to the server
#   SUBMISSION_TYPE The dashboard mode: experimental | nightly | continuous
#
#   RESULT_VARNAME Will set the value of ${RESULT_VARNAME} to either "ok" or "fail".

function(midas_api_upload_package)
  set(options)
  set(oneValueArgs SERVER_URL SERVER_EMAIL SERVER_APIKEY SUBMISSION_TYPE SOURCE_REVISION SOURCE_CHECKOUTDATE OPERATING_SYSTEM ARCHITECTURE PACKAGE_FILEPATH PACKAGE_TYPE RELEASE RESULT_VARNAME)
  set(multiValueArgs)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Sanity check
  set(expected_nonempty_vars SERVER_URL SERVER_EMAIL SERVER_APIKEY SUBMISSION_TYPE SOURCE_REVISION SOURCE_CHECKOUTDATE OPERATING_SYSTEM ARCHITECTURE PACKAGE_TYPE RESULT_VARNAME)
  foreach(var ${expected_nonempty_vars})
    if("${MY_${var}}" STREQUAL "")
      message(FATAL_ERROR "error: ${var} CMake variable is empty !")
    endif()
  endforeach()

  set(expected_existing_vars PACKAGE_FILEPATH)
  foreach(var ${expected_existing_vars})
    if(NOT EXISTS "${MY_${var}}")
      message(FATAL_ERROR "Variable ${var} is set to an inexistent directory or file ! [${${var}}]")
    endif()
  endforeach()

  include(MIDASAPILogin)
  midas_api_login(
    API_URL ${MY_SERVER_URL}
    API_EMAIL ${MY_SERVER_EMAIL}
    API_KEY ${MY_SERVER_APIKEY}
    RESULT_VARNAME midas_api_token
    )
  if(midas_api_token STREQUAL "")
    set(${MY_RESULT_VARNAME} "fail" PARENT_SCOPE)
    return()
  endif()

  get_filename_component(basename "${MY_PACKAGE_FILEPATH}" NAME)
  midas_api_escape_for_url(basename "${basename}")
  midas_api_escape_for_url(revision "${MY_SOURCE_REVISION}")
  midas_api_escape_for_url(checkoutdate "${MY_SOURCE_CHECKOUTDATE}")
  midas_api_escape_for_url(os "${MY_OPERATING_SYSTEM}")
  midas_api_escape_for_url(arch "${MY_ARCHITECTURE}")
  midas_api_escape_for_url(packagetype "${MY_PACKAGE_TYPE}")
  midas_api_escape_for_url(package "${MY_PACKAGE_FILEPATH}")
  string(TOLOWER ${MY_SUBMISSION_TYPE} MY_SUBMISSION_TYPE)
  midas_api_escape_for_url(submissiontype "${MY_SUBMISSION_TYPE}")
  midas_api_escape_for_url(release "${MY_RELEASE}")

  set(api_method "midas.slicerpackages.package.upload")
  set(params "&token=${midas_api_token}")
  set(params "${params}&revision=${revision}")
  set(params "${params}&os=${os}")
  set(params "${params}&arch=${arch}")
  set(params "${params}&submissiontype=${submissiontype}")
  set(params "${params}&packagetype=${packagetype}")
  set(params "${params}&name=${basename}")
  set(params "${params}&checkoutdate=${checkoutdate}")
  set(params "${params}&productname=Slicer")
  set(params "${params}&codebase=Slicer4")
  set(params "${params}&release=${release}")
  set(url "${MY_SERVER_URL}/api/json?method=${api_method}${params}")

  if("${MIDAS_API_DISPLAY_URL}")
    message(STATUS "URL: ${url}")
  endif()

  file(UPLOAD ${MY_PACKAGE_FILEPATH} ${url} INACTIVITY_TIMEOUT 120 STATUS status LOG log SHOW_PROGRESS)
  string(REGEX REPLACE ".*{\"stat\":[ ]*\"([^\"]*)\".*" "\\1" status ${log})

  set(api_call_log ${CMAKE_CURRENT_BINARY_DIR}/${api_method}_response.txt)
  file(WRITE ${api_call_log} ${log})

  if(status STREQUAL "ok")
    set(${MY_RESULT_VARNAME} "ok" PARENT_SCOPE)
  else()
    message(WARNING "Upload of package to MIDAS failed: ${log}")
    set(${MY_RESULT_VARNAME} "fail" PARENT_SCOPE)
  endif()
endfunction()

#
# Testing
#

#
# cmake -DTEST_midas_api_upload_package_test:BOOL=ON -P MIDASAPIUploadPackage.cmake
#
if(TEST_midas_api_upload_package_test)

  function(midas_api_upload_package_test)
    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_MODULE_PATH})

    set(Test_TESTDATE "2011-12-26")

    # Sanity check
    set(expected_nonempty_vars Test_TESTDATE)
    foreach(var ${expected_nonempty_vars})
      if("${var}" STREQUAL "")
        message(FATAL_ERROR "Problem with midas_api_upload_package_test()\n"
                            "Variable ${var} is an empty string !")
      endif()
    endforeach()

    if(NOT DEFINED SERVER_URL)
      set(SERVER_URL "http://karakoram/midas")
    endif()
    if(NOT DEFINED SERVER_EMAIL)
      set(SERVER_EMAIL "jchris.fillionr@kitware.com")
    endif()
    if(NOT DEFINED SERVER_APIKEY)
      set(SERVER_APIKEY "a4d947d1772e227adf75639b449974d3")
    endif()

    set(source_checkoutdate "2011-12-26 12:21:42 -0500 (Mon, 26 Dec 2011)")
    set(package_type "installer")

    set(source_revisions "100" "101" "102" "103" "104" "105" "106")
    set(source_revision_101_nightly_release "4.0.0")
    set(source_revision_104_experimental_release "4.0.1")
    set(source_revision_106_nightly_release "4.2")

    include(SlicerBlockOperatingSystemNames)

    foreach(submission_type "experimental" "nightly")
      foreach(operating_system "${Slicer_OS_LINUX_NAME}" "${Slicer_OS_MAC_NAME}" "${Slicer_OS_WIN_NAME}")
        foreach(architecture "i386" "amd64")
          foreach(source_revision ${source_revisions})

            set(release "${source_revision_${source_revision}_${submission_type}_release}")

            set(package_filepath ${CMAKE_CURRENT_BINARY_DIR}/Test-${Test_TESTDATE}-${submission_type}-${operating_system}-${architecture}-${source_revision}.txt)
            file(WRITE ${package_filepath} "
              Test_TESTDATE: ${Test_TESTDATE}
              source_checkoutdate: ${source_checkoutdate}
              submission_type: ${submission_type}
              operating_system: ${operating_system}
              architecture: ${architecture}
              source_revision: ${source_revision}
              release: ${release}")

            if(NOT EXISTS ${package_filepath})
              message(FATAL_ERROR "Problem with midas_api_upload_package_test()\n"
                                  "Failed to create [${package_filepath}]")
            endif()

            midas_api_upload_package(
              SERVER_URL ${SERVER_URL}
              SERVER_EMAIL ${SERVER_EMAIL}
              SERVER_APIKEY ${SERVER_APIKEY}
              SUBMISSION_TYPE ${submission_type}
              SOURCE_REVISION ${source_revision}
              SOURCE_CHECKOUTDATE ${source_checkoutdate}
              OPERATING_SYSTEM ${operating_system}
              ARCHITECTURE ${architecture}
              PACKAGE_FILEPATH ${package_filepath}
              PACKAGE_TYPE ${package_type}
              RELEASE ${release}
              RESULT_VARNAME output
              )
            set(expected_output "ok")
            if(NOT "${output}" STREQUAL "${expected_output}")
              message(FATAL_ERROR "Problem with midas_api_upload_package()\n"
                                  "output:${output}\n"
                                  "expected_output:${expected_output}")
            endif()
            file(REMOVE ${package_filepath})

          endforeach()
        endforeach()
      endforeach()
    endforeach()

    message("SUCCESS")
  endfunction()
  midas_api_upload_package_test()
endif()
