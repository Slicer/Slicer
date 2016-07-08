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
#  and was partially funded by NIH grant 1U24CA194354-01
#
################################################################################

#
# Once included, this module will define
#
# (1) the following variables:
#
#  Slicer_EXT_METADATA_NAMES: List of metadata names (e.g SCM, SCMURL, ...)
#
#  Slicer_EXT_REQUIRED_METADATA_NAMES: List of required metadata names
#
#  Slicer_EXT_OPTIONAL_METADATA_NAMES: List of optional metadata names
#
#  And for each metadata:
#
#    Slicer_EXT_METADATA_<name>_REQUIRED: Set to 1 if mandatory.
#
#    Slicer_EXT_<name>_DEFAULT          : Default value if any.
#
#    Slicer_EXT_<name>_DESCRIPTION      : Describe the metadata.
#
# where <name> is the metadata name (e.g SCM, SCMURL, ...)
#
#
# (2) the following macro(s):
#
#  slicer_extension_metadata_set_defaults(<prefix>):
#
#  For each optional metadata, set variables "<prefix>_<name>"
#  initialized to "Slicer_EXT_<name>_DEFAULT" if "<prefix>_<name>"
#  was not already defined.
#

set(Slicer_EXT_METADATA_NAMES )

macro(_define_metadata name description required default)
  set(Slicer_EXT_METADATA_${name}_REQUIRED    "${required}")
  set(Slicer_EXT_METADATA_${name}_DEFAULT     "${default}")
  set(Slicer_EXT_METADATA_${name}_DESCRIPTION "${description}")
  list(APPEND Slicer_EXT_METADATA_NAMES ${name})
endmacro()

_define_metadata(
  "SCM"
  "Source code management system: git or svn"
  1
  "")

_define_metadata("SCMURL"
  "Read-only url used to checkout the extension source code"
  1
  "")

_define_metadata("SCMREVISION"
  "Revision allowing to checkout the expected source code"
  1
  "")

_define_metadata(
  "SVNUSERNAME"
  "Allow to specify username associated with svn checkout"
  0
  "")

_define_metadata("SVNPASSWORD"
  "Allow to specify password associated with svn checkout"
  0
  "")

_define_metadata("DEPENDS"
  "Space separated list of extensions required to build this extension"
  0
  "NA")

_define_metadata("BUILD_SUBDIRECTORY"
  "Name of the inner build directory in case of superbuild based extension"
  0
  ".")

_define_metadata("HOMEPAGE"
  "Url of the web page describing the extension."
  0
  "")

_define_metadata("CONTRIBUTORS"
  "Extension contributor specified as Firstname1 Lastname1 ([SubOrg1, ]Org1), Firstname2 Lastname2 ([SubOrg2, ]Org2)"
  0
  "")

_define_metadata("CATEGORY"
  "Extension category"
  0
  "")

_define_metadata("ICONURL"
  "Url to an icon (png, size 128x128 pixels)"
  0
  "")

_define_metadata("DESCRIPTION"
  "One line describing what is the purpose of the extension"
  0
  "")

_define_metadata("SCREENSHOTURLS"
  "Space separated list of urls to images"
  0
  "")

_define_metadata("ENABLED"
  "1 or 0 - Specify if the extension should be enabled after its installation"
  0
  "1")

_define_metadata("STATUS"
  "Give people an idea what to expect from this code"
  0
  "")

set(Slicer_EXT_REQUIRED_METADATA_NAMES )
set(Slicer_EXT_OPTIONAL_METADATA_NAMES )
foreach(name IN LISTS Slicer_EXT_METADATA_NAMES)
  # Sanity check
  foreach(var IN ITEMS "REQUIRED" "DEFAULT" "DESCRIPTION")
    set(varname "Slicer_EXT_METADATA_${name}_${var}")
    if(NOT DEFINED "${varname}")
      message(FATAL_ERROR "Variable ${varname} is not defined")
    endif()
  endforeach()
  if(Slicer_EXT_METADATA_${name}_REQUIRED)
    list(APPEND Slicer_EXT_REQUIRED_METADATA_NAMES ${name})
  else()
    list(APPEND Slicer_EXT_OPTIONAL_METADATA_NAMES ${name})
  endif()
endforeach()

macro(slicer_extension_metadata_set_defaults prefix)
  foreach(name IN LISTS Slicer_EXT_OPTIONAL_METADATA_NAMES)
    if("${${prefix}_${name}}" STREQUAL "")
      set(${prefix}_${name} "${Slicer_EXT_METADATA_${name}_DEFAULT}")
    endif()
  endforeach()
endmacro()

################################################################################
# Testing
################################################################################

#
# cmake -DTEST_<testfunction>:BOOL=ON -P <this_script>.cmake
#

function(slicer_extension_description_spec_defaults_test)

  # Check metadata count
  list(LENGTH Slicer_EXT_METADATA_NAMES metadata_count)
  list(LENGTH Slicer_EXT_REQUIRED_METADATA_NAMES required_metadata_count)
  list(LENGTH Slicer_EXT_OPTIONAL_METADATA_NAMES optional_metadata_count)

  set(expected 15)
  set(actual ${metadata_count})
  if(NOT ${actual} EQUAL ${expected})
    message(FATAL_ERROR
      "Problem with metadata_count. Expected: ${expected}, actual: ${actual}")
  endif()

  set(expected 3)
  set(actual ${required_metadata_count})
  if(NOT ${actual} EQUAL ${expected})
    message(FATAL_ERROR
      "Problem with metadata_count. Expected: ${expected}, actual: ${actual}")
  endif()

  set(expected 12)
  set(actual ${optional_metadata_count})
  if(NOT ${actual} EQUAL ${expected})
    message(FATAL_ERROR
      "Problem with metadata_count. Expected: ${expected}, actual: ${actual}")
  endif()

  # Check REQUIRED, DEFAULT and DESCRIPTION are defined for all metadata names
  set(required
    SCM
    SCMURL
    SCMREVISION
    )
  set(optional
    SVNUSERNAME
    SVNPASSWORD
    DEPENDS
    BUILD_SUBDIRECTORY
    HOMEPAGE
    CONTRIBUTORS
    CATEGORY
    ICONURL
    DESCRIPTION
    SCREENSHOTURLS
    ENABLED
    STATUS
    )
  foreach(name IN LISTS required optonal)
    foreach(varsuffix IN ITEMS "REQUIRED" "DEFAULT" "DESCRIPTION")
      set(expected_defined_varname "Slicer_EXT_METADATA_${name}_${varsuffix}")
      if(NOT DEFINED "${expected_defined_varname}")
        message(FATAL_ERROR "Variable ${expected_defined_varname} is NOT defined")
      endif()
    endforeach()
  endforeach()

  message("SUCCESS")
endfunction()
if(TEST_slicer_extension_description_spec_defaults_test)
  slicer_extension_description_spec_defaults_test()
endif()

function(slicer_extension_metadata_set_defaults_test)

  # Check that passing prefix parameter work as expected
  if(DEFINED foo_BUILD_SUBDIRECTORY)
    message(FATAL_ERROR "foo_BUILD_SUBDIRECTORY should not be defined")
  endif()

  set(foo_ENABLED 0)

  slicer_extension_metadata_set_defaults("foo")

  # Check variable already set are not overridden
  if(NOT foo_ENABLED STREQUAL "0")
    message(FATAL_ERROR "Problem with slicer_extension_metadata_set_defaults")
  endif()

  # Check undefined variable are initialized to a default value.
  if(NOT foo_BUILD_SUBDIRECTORY STREQUAL ".")
    message(FATAL_ERROR "Problem with slicer_extension_metadata_set_defaults")
  endif()

  # Check undefined variable or variable set to an empty string are
  # initialized using default value
  set(bar_ENABLED "")
  unset(bar_DEPENDS)

  slicer_extension_metadata_set_defaults("bar")

  if(NOT bar_ENABLED STREQUAL "1")
    message(FATAL_ERROR "Problem with slicer_extension_metadata_set_defaults")
  endif()
  if(NOT bar_DEPENDS STREQUAL "NA")
    message(FATAL_ERROR "Problem with slicer_extension_metadata_set_defaults")
  endif()

  message("SUCCESS")
endfunction()
if(TEST_slicer_extension_metadata_set_defaults_test)
  slicer_extension_metadata_set_defaults_test()
endif()
