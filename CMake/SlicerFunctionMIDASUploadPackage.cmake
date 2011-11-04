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
#  This file was originally developed by Zach Mullen, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################

# Uploads a package to the MIDAS server.
# The following variables should be set in your dashboard script before calling this:
#   MIDAS_PACKAGE_URL The url of the MIDAS server
#   MIDAS_PACKAGE_EMAIL The email to use to authenticate to the server
#   MIDAS_PACKAGE_API_KEY The default api key to use to authenticate to the server
#   SCRIPT_MODE The dashboard mode: experimental | nightly | continuous
#
# Will set the value of the variable in resultvar to either "ok" or "fail".
# Will output warning messages in the fail condition
function(SlicerFunctionMIDASUploadPackage package packagetype resultvar)
  include("${CTEST_SOURCE_DIRECTORY}/CMake/SlicerMacroExtractRepositoryInfo.cmake")
  include("${CTEST_SOURCE_DIRECTORY}/CMake/SlicerMacroGetPlatformArchitectureBitness.cmake")
  SlicerMacroExtractRepositoryInfo(VAR_PREFIX Slicer)
  SlicerMacroGetPlatformArchitectureBitness(VAR_PREFIX Slicer)

  if(NOT DEFINED MIDAS_PACKAGE_EMAIL)
    message(WARNING "Skipped uploading package to MIDAS: MIDAS_PACKAGE_EMAIL is not set")
    set(${resultvar} "fail" PARENT_SCOPE)
    return()
  endif()
  if(NOT DEFINED MIDAS_PACKAGE_API_KEY)
    message(WARNING "Skipped uploading package to MIDAS: MIDAS_PACKAGE_API_KEY is not set")
    set(${resultvar} "fail" PARENT_SCOPE)
    return()
  endif()

  _SlicerEscapeForUrl(email ${MIDAS_PACKAGE_EMAIL})
  _SlicerEscapeForUrl(apikey ${MIDAS_PACKAGE_API_KEY})

  file(DOWNLOAD "${MIDAS_PACKAGE_URL}/api/json?method=midas.login&appname=Default&email=${email}&apikey=${apikey}"
       "${CTEST_BINARY_DIRECTORY}/MIDAStoken.txt"
       INACTIVITY_TIMEOUT 120)
  file(READ "${CTEST_BINARY_DIRECTORY}/MIDAStoken.txt" resp)
  file(REMOVE "${CTEST_BINARY_DIRECTORY}/MIDAStoken.txt")
  string(REGEX REPLACE ".*token\":\"(.*)\".*" "\\1" token ${resp})
  string(LENGTH ${token} tokenlength)

  if(NOT tokenlength EQUAL 40)
    message(WARNING "Skipped uploading package to MIDAS: login failed: ${resp}")
    set(${resultvar} "fail" PARENT_SCOPE)
    return()
  endif()

  get_filename_component(basename "${package}" NAME)
  _SlicerEscapeForUrl(basename ${basename})
  _SlicerEscapeForUrl(rev ${Slicer_WC_VERSION})
  _SlicerEscapeForUrl(os ${Slicer_PLATFORM})
  _SlicerEscapeForUrl(arch ${Slicer_ARCHITECTURE})
  _SlicerEscapeForUrl(packagetype ${packagetype})
  _SlicerEscapeForUrl(package ${package})
  _SlicerEscapeForUrl(dashboardmode ${SCRIPT_MODE})

  set(params "&revision=${rev}&os=${os}&arch=${arch}&submissiontype=${dashboardmode}&packagetype=${packagetype}&name=${basename}&token=${token}")
  set(url "${MIDAS_PACKAGE_URL}/api/json?method=midas.slicerpackages.uploadpackage${params}")
  file(UPLOAD ${package} ${url} INACTIVITY_TIMEOUT 120 STATUS status LOG log SHOW_PROGRESS)
  string(REGEX REPLACE ".*{\"stat\":\"([^\"]*)\".*" "\\1" status ${log})

  if(status STREQUAL "ok")
    set(${resultvar} "ok" PARENT_SCOPE)
  else()
    message(WARNING "Upload of package to MIDAS failed: ${log}")
    set(${resultvar} "fail" PARENT_SCOPE)
  endif()
endfunction()

macro(_SlicerEscapeForUrl var str)
  # Escape spaces in the url
  string(REPLACE " " "%20" ${var} ${str})
endmacro()
