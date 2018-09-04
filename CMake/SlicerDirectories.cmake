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
#  W A R N I N G
#  -------------
#
# This module was designed to be included in (1) the Slicer top-level CMakeLists.txt
# and (2) a SlicerCustomApp CMakeLists.txt to conveniently set the value of the common
# Slicer relative directories for build and install tree.
#
# Using this module outside of these contexts may break from version to version without notice.
#
# We mean it.
#


#-----------------------------------------------------------------------------
# Sanity checks
#-----------------------------------------------------------------------------
set(expected_defined_vars
  Slicer_MAIN_PROJECT
  ${Slicer_MAIN_PROJECT}_APPLICATION_NAME
  Slicer_VERSION_MAJOR
  Slicer_VERSION_MINOR
  )
foreach(var IN LISTS expected_defined_vars)
  if(NOT DEFINED ${var})
    message(FATAL_ERROR "Variable ${var} is expected to be defined !")
  endif()
endforeach()

if(NOT DEFINED Slicer_MAIN_PROJECT_APPLICATION_NAME)
  set(Slicer_MAIN_PROJECT_APPLICATION_NAME ${${Slicer_MAIN_PROJECT}_APPLICATION_NAME})
endif()

#-----------------------------------------------------------------------------
# Slicer relative directories
#-----------------------------------------------------------------------------
# for build tree
set(Slicer_BIN_DIR "bin")
set(Slicer_LIB_DIR "lib/${Slicer_MAIN_PROJECT_APPLICATION_NAME}-${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}")
set(Slicer_INCLUDE_DIR "include/${Slicer_MAIN_PROJECT_APPLICATION_NAME}-${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}")
set(Slicer_SHARE_DIR "share/${Slicer_MAIN_PROJECT_APPLICATION_NAME}-${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}")
set(Slicer_LIBEXEC_DIR "libexec/${Slicer_MAIN_PROJECT_APPLICATION_NAME}-${Slicer_VERSION_MAJOR}.${Slicer_VERSION_MINOR}")
set(Slicer_ITKFACTORIES_DIR "${Slicer_LIB_DIR}/ITKFactories")
set(Slicer_QM_DIR "${Slicer_BIN_DIR}/Translations")

# for install tree
set(Slicer_INSTALL_ROOT "./")
set(Slicer_BUNDLE_LOCATION "${Slicer_MAIN_PROJECT_APPLICATION_NAME}.app/Contents")
if(APPLE)
  set(Slicer_INSTALL_ROOT "${Slicer_BUNDLE_LOCATION}/") # Set to create Bundle
endif()
set(Slicer_INSTALL_BIN_DIR "${Slicer_INSTALL_ROOT}${Slicer_BIN_DIR}")
set(Slicer_INSTALL_LIB_DIR "${Slicer_INSTALL_ROOT}${Slicer_LIB_DIR}")
set(Slicer_INSTALL_INCLUDE_DIR "${Slicer_INSTALL_ROOT}${Slicer_INCLUDE_DIR}")
set(Slicer_INSTALL_SHARE_DIR "${Slicer_INSTALL_ROOT}${Slicer_SHARE_DIR}")
set(Slicer_INSTALL_LIBEXEC_DIR "${Slicer_INSTALL_ROOT}${Slicer_LIBEXEC_DIR}")
set(Slicer_INSTALL_ITKFACTORIES_DIR "${Slicer_INSTALL_LIB_DIR}/ITKFactories")
set(Slicer_INSTALL_QM_DIR "${Slicer_INSTALL_ROOT}${Slicer_QM_DIR}")


#-----------------------------------------------------------------------------
# Slicer CLI relative directories
#-----------------------------------------------------------------------------

# NOTE: Make sure to update vtkSlicerApplicationLogic::GetModuleShareDirectory()
#       if the following variables are changed.

set(Slicer_CLIMODULES_SUBDIR "cli-modules")

# for build tree
set(Slicer_CLIMODULES_BIN_DIR "${Slicer_LIB_DIR}/${Slicer_CLIMODULES_SUBDIR}")
set(Slicer_CLIMODULES_LIB_DIR "${Slicer_LIB_DIR}/${Slicer_CLIMODULES_SUBDIR}")
set(Slicer_CLIMODULES_SHARE_DIR "${Slicer_SHARE_DIR}/${Slicer_CLIMODULES_SUBDIR}")

# for install tree
set(Slicer_INSTALL_CLIMODULES_BIN_DIR "${Slicer_INSTALL_ROOT}${Slicer_CLIMODULES_BIN_DIR}")
set(Slicer_INSTALL_CLIMODULES_LIB_DIR "${Slicer_INSTALL_ROOT}${Slicer_CLIMODULES_LIB_DIR}")
set(Slicer_INSTALL_CLIMODULES_SHARE_DIR "${Slicer_INSTALL_ROOT}${Slicer_CLIMODULES_SHARE_DIR}")


#-----------------------------------------------------------------------------
# Qt Loadable Modules relative directories
#-----------------------------------------------------------------------------

# NOTE: Make sure to update vtkSlicerApplicationLogic::GetModuleShareDirectory()
#       if the following variables are changed.

set(Slicer_QTLOADABLEMODULES_SUBDIR "qt-loadable-modules")

# for build tree
set(Slicer_QTLOADABLEMODULES_BIN_DIR "${Slicer_LIB_DIR}/${Slicer_QTLOADABLEMODULES_SUBDIR}")
set(Slicer_QTLOADABLEMODULES_LIB_DIR "${Slicer_LIB_DIR}/${Slicer_QTLOADABLEMODULES_SUBDIR}")
set(Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR "${Slicer_LIB_DIR}/${Slicer_QTLOADABLEMODULES_SUBDIR}/Python")
set(Slicer_QTLOADABLEMODULES_INCLUDE_DIR "${Slicer_INCLUDE_DIR}/${Slicer_QTLOADABLEMODULES_SUBDIR}")
set(Slicer_QTLOADABLEMODULES_SHARE_DIR "${Slicer_SHARE_DIR}/${Slicer_QTLOADABLEMODULES_SUBDIR}")

# for install tree
set(Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTLOADABLEMODULES_BIN_DIR}")
set(Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTLOADABLEMODULES_LIB_DIR}")
set(Slicer_INSTALL_QTLOADABLEMODULES_PYTHON_LIB_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR}")
set(Slicer_INSTALL_QTLOADABLEMODULES_INCLUDE_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTLOADABLEMODULES_INCLUDE_DIR}")
set(Slicer_INSTALL_QTLOADABLEMODULES_SHARE_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTLOADABLEMODULES_SHARE_DIR}")


#-----------------------------------------------------------------------------
# Scripted Modules relative directories
#-----------------------------------------------------------------------------

# NOTE: Make sure to update vtkSlicerApplicationLogic::GetModuleShareDirectory()
#       if the following variables are changed.

set(Slicer_QTSCRIPTEDMODULES_SUBDIR "qt-scripted-modules")

# for build tree
set(Slicer_QTSCRIPTEDMODULES_BIN_DIR "${Slicer_LIB_DIR}/${Slicer_QTSCRIPTEDMODULES_SUBDIR}")
set(Slicer_QTSCRIPTEDMODULES_LIB_DIR "${Slicer_LIB_DIR}/${Slicer_QTSCRIPTEDMODULES_SUBDIR}")
set(Slicer_QTSCRIPTEDMODULES_INCLUDE_DIR "${Slicer_INCLUDE_DIR}/${Slicer_QTSCRIPTEDMODULES_SUBDIR}")
set(Slicer_QTSCRIPTEDMODULES_SHARE_DIR "${Slicer_SHARE_DIR}/${Slicer_QTSCRIPTEDMODULES_SUBDIR}")

# for install tree
set(Slicer_INSTALL_QTSCRIPTEDMODULES_BIN_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTSCRIPTEDMODULES_BIN_DIR}")
set(Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTSCRIPTEDMODULES_LIB_DIR}")
set(Slicer_INSTALL_QTSCRIPTEDMODULES_INCLUDE_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTSCRIPTEDMODULES_INCLUDE_DIR}")
set(Slicer_INSTALL_QTSCRIPTEDMODULES_SHARE_DIR "${Slicer_INSTALL_ROOT}${Slicer_QTSCRIPTEDMODULES_SHARE_DIR}")


# --------------------------------------------------------------------------
# ThirdParty: Used to superbuild projects built in Slicer extension.
# --------------------------------------------------------------------------

# for build tree
set(Slicer_THIRDPARTY_BIN_DIR ${Slicer_BIN_DIR})
set(Slicer_THIRDPARTY_LIB_DIR ${Slicer_LIB_DIR})
set(Slicer_THIRDPARTY_SHARE_DIR ${Slicer_SHARE_DIR})

# for install tree:
#
# These variables can be used when configuring extension external projects in
# two different scenarios: (1) bundled extensions and (2) regular extensions.
#
# The values set below corresponds to scenario (1). Value for scenario (2) are set
# in UseSlicer.cmake.
set(Slicer_INSTALL_THIRDPARTY_BIN_DIR "${Slicer_INSTALL_ROOT}${Slicer_THIRDPARTY_BIN_DIR}")
set(Slicer_INSTALL_THIRDPARTY_LIB_DIR "${Slicer_INSTALL_ROOT}${Slicer_THIRDPARTY_LIB_DIR}")
set(Slicer_INSTALL_THIRDPARTY_SHARE_DIR "${Slicer_INSTALL_ROOT}${Slicer_THIRDPARTY_SHARE_DIR}")
