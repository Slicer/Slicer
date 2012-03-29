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
#  This file was originally developed by Jean-Christophe Fillion-Robin and Zach Mullen, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################

# Uploads an extension to the MIDAS server.
# The following variables should be set in your dashboard script before calling this:
#   MIDAS_PACKAGE_URL The url of the MIDAS server
#   MIDAS_PACKAGE_EMAIL The email to use to authenticate to the server
#   MIDAS_PACKAGE_API_KEY The default api key to use to authenticate to the server
#   SCRIPT_MODE The dashboard mode: experimental | nightly | continuous
#
# Will set the value of the variable in resultvar to either "ok" or "fail".
# Will output warning messages in the fail condition
function(midas_api_upload_extension)
  set(expected_nonempty_args SERVER_URL SERVER_EMAIL SERVER_APIKEY SUBMISSION_TYPE SLICER_REVISION EXTENSION_NAME EXTENSION_CATEGORY EXTENSION_ICONURL EXTENSION_DESCRIPTION EXTENSION_CONTRIBUTORS EXTENSION_HOMEPAGE EXTENSION_SCREENSHOTURLS EXTENSION_REPOSITORY_TYPE EXTENSION_REPOSITORY_URL EXTENSION_SOURCE_REVISION EXTENSION_ENABLED OPERATING_SYSTEM ARCHITECTURE PACKAGE_TYPE RESULT_VARNAME)
  set(expected_existing_args TMP_DIR PACKAGE_FILEPATH)
  include(CMakeParseArguments)
  set(options)
  set(oneValueArgs ${expected_nonempty_args} ${expected_existing_args} RELEASE)
  set(multiValueArgs)
  cmake_parse_arguments(MY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Sanity check
  foreach(var ${expected_nonempty_args})
    if("${MY_${var}}" STREQUAL "")
      message(FATAL_ERROR "error: ${var} CMake variable is empty !")
    endif()
  endforeach()

  foreach(var ${expected_existing_args})
    if(NOT EXISTS "${MY_${var}}")
      message(FATAL_ERROR "Variable ${var} is set to an inexistent directory or file ! [${${var}}]")
    endif()
  endforeach()

  include(MIDASAPILogin)
  midas_api_login(
    SERVER_URL ${MY_SERVER_URL}
    SERVER_EMAIL ${MY_SERVER_EMAIL}
    SERVER_APIKEY ${MY_SERVER_APIKEY}
    TMP_DIR "${MY_TMP_DIR}"
    RESULT_VARNAME server_token
    )

  get_filename_component(basename "${MY_PACKAGE_FILEPATH}" NAME)
  _SlicerEscapeForUrl(basename "${basename}")
  _SlicerEscapeForUrl(productname "${MY_EXTENSION_NAME}")
  _SlicerEscapeForUrl(category "${MY_EXTENSION_CATEGORY}")
  _SlicerEscapeForUrl(icon_url "${MY_EXTENSION_ICONURL}")
  _SlicerEscapeForUrl(description "${MY_EXTENSION_DESCRIPTION}")
  _SlicerEscapeForUrl(contributors "${MY_EXTENSION_CONTRIBUTORS}")
  _SlicerEscapeForUrl(homepage "${MY_EXTENSION_HOMEPAGE}")
  _SlicerEscapeForUrl(screenshots "${MY_EXTENSION_SCREENSHOTURLS}")
  _SlicerEscapeForUrl(slicer_revision "${MY_SLICER_REVISION}")
  _SlicerEscapeForUrl(revision "${MY_EXTENSION_SOURCE_REVISION}")
  _SlicerEscapeForUrl(repository_type "${MY_EXTENSION_REPOSITORY_TYPE}")
  _SlicerEscapeForUrl(repository_url "${MY_EXTENSION_REPOSITORY_URL}")
  _SlicerEscapeForUrl(enabled "${MY_EXTENSION_ENABLED}")
  _SlicerEscapeForUrl(os "${MY_OPERATING_SYSTEM}")
  _SlicerEscapeForUrl(arch "${MY_ARCHITECTURE}")
  _SlicerEscapeForUrl(packagetype "${MY_PACKAGE_TYPE}")
  _SlicerEscapeForUrl(package "${MY_PACKAGE_FILEPATH}")
  string(TOLOWER ${MY_SUBMISSION_TYPE} MY_SUBMISSION_TYPE)
  _SlicerEscapeForUrl(submissiontype "${MY_SUBMISSION_TYPE}")

  set(api_method "midas.slicerpackages.extension.upload")
  set(params "&token=${server_token}")
  set(params "${params}&repository_type=${repository_type}")
  set(params "${params}&repository_url=${repository_url}")
  set(params "${params}&slicer_revision=${slicer_revision}")
  set(params "${params}&revision=${revision}")
  set(params "${params}&os=${os}")
  set(params "${params}&arch=${arch}")
  set(params "${params}&submissiontype=${submissiontype}")
  set(params "${params}&packagetype=${packagetype}")
  set(params "${params}&category=${category}")
  set(params "${params}&icon_url=${icon_url}")
  set(params "${params}&description=${description}")
  set(params "${params}&contributors=${contributors}")
  set(params "${params}&homepage=${homepage}")
  set(params "${params}&screenshots=${screenshots}")
  set(params "${params}&enabled=${enabled}")
  set(params "${params}&name=${basename}")
  set(params "${params}&productname=${productname}")
  set(params "${params}&codebase=Slicer4")
  set(params "${params}&release=${release}")
  set(url "${MY_SERVER_URL}/api/json?method=${api_method}${params}")

  file(UPLOAD ${MY_PACKAGE_FILEPATH} ${url} INACTIVITY_TIMEOUT 120 STATUS status LOG log SHOW_PROGRESS)
  string(REGEX REPLACE ".*{\"stat\":\"([^\"]*)\".*" "\\1" status ${log})

  set(api_call_log ${MY_TMP_DIR}/midas_api_upload_extension_log.txt)
  file(WRITE ${api_call_log} ${log})

  if(status STREQUAL "ok")
    set(${MY_RESULT_VARNAME} "ok" PARENT_SCOPE)
  else()
    message(WARNING "Upload of extension to MIDAS failed: ${log}")
    set(${MY_RESULT_VARNAME} "fail" PARENT_SCOPE)
  endif()

endfunction()


#
# Testing - cmake -DTEST_<TESTNAME>:BOOL=ON -P MIDASAPIUploadExtensionTest.cmake
#

#
# TESTNAME: midas_api_upload_extension_test
#
if(TEST_midas_api_upload_extension_test)

  function(midas_api_upload_extension_test)

    set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/../../CMake ${CMAKE_MODULE_PATH})

    include(SlicerFunctionToday)
    TODAY(Test_TESTDATE)

    # Sanity check
    set(expected_nonempty_vars Test_TESTDATE)
    foreach(var ${expected_nonempty_vars})
      if("${var}" STREQUAL "")
        message(FATAL_ERROR "Problem with midas_api_upload_extension_test()\n"
                            "Variable ${var} is an empty string !")
      endif()
    endforeach()

    set(expected_existing_vars TMP_DIR)
    foreach(var ${expected_existing_vars})
      if(NOT EXISTS "${${var}}")
        message(FATAL_ERROR "Variable ${var} is set to an inexistent directory or file ! [${${var}}]")
      endif()
    endforeach()

    set(server_url "http://karakoram/midas")
    set(server_email "jchris.fillionr@kitware.com")
    set(server_apikey "a4d947d1772e227adf75639b449974d3")

    set(source_checkoutdate "2011-12-26 12:21:42 -0500 (Mon, 26 Dec 2011)")
    set(package_type "installer")

    #set(slicer_revisions "100" "101" "102" "103" "104" "105" "106")
    set(slicer_revisions "19291")
    set(slicer_revision_101_nightly_release "4.0.0")
    set(slicer_revision_104_experimental_release "4.0.1")
    set(slicer_revision_105_nightly_release "4.2")

    set(extension_infos
      "ExtensionA^^git^^git://github.com/nowhere/ExtensionA.git^^83352cd1c5^^Foo^^0"
      "ExtensionB^^git^^git://github.com/nowhere/ExtensionB.git^^45689ae3d4^^Bar^^1")

    include(SlicerBlockOperatingSystemNames)

    foreach(submission_type "experimental" "nightly")
      foreach(operating_system "${Slicer_OS_LINUX_NAME}" "${Slicer_OS_MAC_NAME}" "${Slicer_OS_WIN_NAME}")
        foreach(architecture "i386" "amd64")
          foreach(slicer_revision ${slicer_revisions})
            foreach(extension_info ${extension_infos})
              string(REPLACE "^^" ";" extension_info_list ${extension_info})
              list(GET extension_info_list 0 extension_name)
              list(GET extension_info_list 1 extension_repository_type)
              list(GET extension_info_list 2 extension_repository_url)
              list(GET extension_info_list 3 extension_source_revision)
              list(GET extension_info_list 4 extension_category)
              list(GET extension_info_list 5 extension_enabled)

              set(extension_description "This is description of ${extension_name}")
              set(extension_homepage "http://homepage.${extension_name}.org/foo/bar")
              set(extension_iconurl "http://homepage.${extension_name}.org/foo/bar.png")
              set(extension_contributors "Jean-Christophe Fillion-Robin (Kitware)")
              set(extension_screenshoturls "http://homepage.${extension_name}.org/screenshot1.png http://homepage.${extension_name}.org/screenshot2.png")

              #set(release "${slicer_revision_${slicer_revision}_${submission_type}_release}")

              set(package_filepath ${TMP_DIR}/${slicer_revision}-${operating_system}-${architecture}-${extension_name}-${extension_source_revision}-${Test_TESTDATE}-${submission_type}.txt)
              file(WRITE ${package_filepath} "
                extension_name: ${extension_name}
                extension_category: ${extension_category}
                extension_description: ${extension_description}
                extension_repository_type: ${extension_repository_type}
                extension_repository_url: ${extension_repository_url}
                extension_source_revision: ${extension_source_revision}
                extension_enabled: ${extension_enabled}
                Test_TESTDATE: ${Test_TESTDATE}
                submission_type: ${submission_type}
                operating_system: ${operating_system}
                architecture: ${architecture}
                slicer_revision: ${slicer_revision}
                release: ${release}")

              if(NOT EXISTS ${package_filepath})
                message(FATAL_ERROR "Problem with midas_api_upload_extension_test()\n"
                                    "Failed to create [${package_filepath}]")
              endif()

              midas_api_upload_extension(
                SERVER_URL ${server_url}
                SERVER_EMAIL ${server_email}
                SERVER_APIKEY ${server_apikey}
                TMP_DIR ${TMP_DIR}
                SUBMISSION_TYPE ${submission_type}
                SLICER_REVISION ${slicer_revision}
                EXTENSION_NAME ${extension_name}
                EXTENSION_CATEGORY ${extension_category}
                EXTENSION_DESCRIPTION ${extension_description}
                EXTENSION_HOMEPAGE ${extension_homepage}
                EXTENSION_ICONURL ${extension_iconurl}
                EXTENSION_CONTRIBUTORS ${extension_contributors}
                EXTENSION_SCREENSHOTURLS ${extension_screenshoturls}
                EXTENSION_REPOSITORY_TYPE ${extension_repository_type}
                EXTENSION_REPOSITORY_URL ${extension_repository_url}
                EXTENSION_SOURCE_REVISION ${extension_source_revision}
                EXTENSION_ENABLED ${extension_enabled}
                OPERATING_SYSTEM ${operating_system}
                ARCHITECTURE ${architecture}
                PACKAGE_FILEPATH ${package_filepath}
                PACKAGE_TYPE ${package_type}
                RELEASE ${release}
                RESULT_VARNAME output
                )
              set(expected_output "ok")
              if(NOT "${output}" STREQUAL "${expected_output}")
                message(FATAL_ERROR "Problem with midas_api_upload_extension()\n"
                                    "output:${output}\n"
                                    "expected_output:${expected_output}")
              endif()
              file(REMOVE ${package_filepath})

            endforeach()
          endforeach()
        endforeach()
      endforeach()
    endforeach()

    message("SUCCESS")
  endfunction()
  midas_api_upload_extension_test()
endif()
