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
#  This file was originally developed by
#   Dave Partyka and Jean-Christophe Fillion-Robin, Kitware Inc.
#  and was partially funded by NIH grant 3P41RR013218-12S1
#
################################################################################

#-----------------------------------------------------------------------------
# Git protocole option
#-----------------------------------------------------------------------------
option(${CMAKE_PROJECT_NAME}_USE_GIT_PROTOCOL "If behind a firewall turn this off to use http instead." ON)
set(git_protocol "git")
if(NOT ${CMAKE_PROJECT_NAME}_USE_GIT_PROTOCOL)
  set(git_protocol "http")
endif()

#-----------------------------------------------------------------------------
# Qt - Let's check if a valid version of Qt is available
#-----------------------------------------------------------------------------
include(SlicerBlockFindQtAndCheckVersion)

#-----------------------------------------------------------------------------
# Enable and setup External project global properties
#-----------------------------------------------------------------------------
include(ExternalProject)
include(SlicerMacroCheckExternalProjectDependency)

set(ep_base        "${CMAKE_BINARY_DIR}")
#set(ep_install_dir "${ep_base}/Install")

set(ep_common_c_flags "${CMAKE_C_FLAGS_INIT} ${ADDITIONAL_C_FLAGS}")
set(ep_common_cxx_flags "${CMAKE_CXX_FLAGS_INIT} ${ADDITIONAL_CXX_FLAGS}")

## On linux SimpleITK requires that ITK be built with -fPIC to allow static libraries to be used.
if(Slicer_USE_SimpleITK)
  if ( UNIX AND NOT APPLE )
    if ( NOT ${ep_common_cxx_flags} MATCHES "-fPIC")
      set(ep_common_cxx_flags "${CMAKE_CXX_FLAGS_INIT} ${ADDITIONAL_CXX_FLAGS} -fPIC")
      message ( WARNING "SimpleITK wrapping requires CMAKE_CXX_FLAGS (or equivalent) to include -fPIC and ITK built with this flag" )
      message ( WARNING "USING CXX_FLAGS = ${ep_common_cxx_flags}")
    endif()
  endif()
endif()

set(ep_common_compiler_args
  -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
  -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
  -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
  -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags}
  )

# Compute -G arg for configuring external projects with the same CMake generator:
if(CMAKE_EXTRA_GENERATOR)
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()

#------------------------------------------------------------------------------
# Slicer dependency list
#------------------------------------------------------------------------------

set(ITK_EXTERNAL_NAME ITKv${ITK_VERSION_MAJOR})

set(Slicer_DEPENDENCIES cmcurl teem VTK ${ITK_EXTERNAL_NAME} CTK jqPlot)

if(Slicer_USE_OpenIGTLink)
  list(APPEND Slicer_DEPENDENCIES OpenIGTLink)
endif()

if(Slicer_BUILD_OpenIGTLinkIF)
  list(APPEND Slicer_DEPENDENCIES OpenIGTLinkIF)
endif()

if(Slicer_USE_SimpleITK)
  list(APPEND Slicer_DEPENDENCIES SimpleITK)
endif()

if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND Slicer_DEPENDENCIES SlicerExecutionModel)
endif()

if(Slicer_BUILD_BRAINSTOOLS)
    list(APPEND Slicer_DEPENDENCIES BRAINSTools)
endif()

if(Slicer_BUILD_ChangeTrackerPy)
  list(APPEND Slicer_DEPENDENCIES ChangeTrackerPy)
endif()

if(Slicer_BUILD_EMSegment)
  list(APPEND Slicer_DEPENDENCIES EMSegment)
endif()

if(Slicer_BUILD_SkullStripper)
  list(APPEND Slicer_DEPENDENCIES SkullStripper)
endif()

if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND Slicer_DEPENDENCIES LibArchive qMidasAPI)
endif()

if(Slicer_USE_BatchMake)
  list(APPEND Slicer_DEPENDENCIES BatchMake)
endif()

if(Slicer_USE_CTKAPPLAUNCHER)
  list(APPEND Slicer_DEPENDENCIES CTKAPPLAUNCHER)
endif()

if(Slicer_USE_PYTHONQT)
  list(APPEND Slicer_DEPENDENCIES python)
endif()

if(Slicer_USE_NUMPY)
  list(APPEND Slicer_DEPENDENCIES NUMPY)
endif()

if(Slicer_USE_PYTHONQT_WITH_TCL AND UNIX)
  list(APPEND Slicer_DEPENDENCIES incrTcl)
endif()

if(Slicer_BUILD_MultiVolumeExplorer)
  list(APPEND Slicer_DEPENDENCIES MultiVolumeExplorer)
endif()

if(Slicer_BUILD_MultiVolumeImporter)
  list(APPEND Slicer_DEPENDENCIES MultiVolumeImporter)
endif()

if(Slicer_BUILD_SlicerWebGLExport)
  list(APPEND Slicer_DEPENDENCIES SlicerWebGLExport)
endif()

SlicerMacroCheckExternalProjectDependency(Slicer)

#-----------------------------------------------------------------------------
# Dump Slicer external project dependencies
#------------------------------------------------------------------------------
#set(ep_dependency_graph "# External project dependencies")
#foreach(ep ${Slicer_DEPENDENCIES})
#  set(ep_dependency_graph "${ep_dependency_graph}\n${ep}:${${ep}_DEPENDENCIES}")
#endforeach()
#file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/SlicerDependencies.txt "${ep_dependency_graph}\n")
#message(STATUS "Generated ${CMAKE_CURRENT_BINARY_DIR}/SlicerDependencies.txt")

#-----------------------------------------------------------------------------
# Set superbuild boolean args
#-----------------------------------------------------------------------------
set(ep_cmake_boolean_args
  DOCUMENTATION_TARGET_IN_ALL
  BUILD_TESTING
  BUILD_SHARED_LIBS
  WITH_COVERAGE
  WITH_MEMCHECK
  Slicer_BUILD_CLI
  Slicer_BUILD_CLI_SUPPORT
  Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  Slicer_BUILD_QTLOADABLEMODULES
  Slicer_BUILD_QTSCRIPTEDMODULES
  Slicer_BUILD_OpenIGTLinkIF
  Slicer_USE_PYTHONQT
  Slicer_USE_PYTHONQT_WITH_TCL
  Slicer_USE_CTKAPPLAUNCHER
  Slicer_USE_BatchMake
  Slicer_USE_MIDAS
  Slicer_USE_OpenIGTLink
  Slicer_WITH_LIBRARY_VERSION
  Slicer_USE_NUMPY
  #Slicer_USE_WEAVE
  Slicer_USE_SimpleITK
  Slicer_BUILD_BRAINSTOOLS
  Slicer_BUILD_ChangeTrackerPy
  Slicer_BUILD_EMSegment
  Slicer_BUILD_SkullStripper
  Slicer_BUILD_MultiVolumeExplorer
  Slicer_BUILD_MultiVolumeImporter
  Slicer_BUILD_SlicerWebGLExport
  )

# Add CTEST_USE_LAUNCHER only if already defined and enabled.
# It avoids extra overhead for manual builds and still allow the option
# to be propagated to the external project.
# See http://www.kitware.com/blog/home/post/11
if(DEFINED CTEST_USE_LAUNCHER AND "${CTEST_USE_LAUNCHER}")
  list(APPEND ep_cmake_boolean_args CTEST_USE_LAUNCHER)
endif()

# Add the option to disable Java even when found on the machine.
list(APPEND ep_cmake_boolean_args CMAKE_DISABLE_FIND_PACKAGE_Java)

set(ep_superbuild_boolean_args)
foreach(ep_cmake_arg ${ep_cmake_boolean_args})
  list(APPEND ep_superbuild_boolean_args -D${ep_cmake_arg}:BOOL=${${ep_cmake_arg}})
endforeach()

# message("CMake args:")
# foreach(arg ${ep_superbuild_boolean_args})
#   message("  ${arg}")
# endforeach()

#-----------------------------------------------------------------------------
# Define list of additional options used to configure Slicer
#------------------------------------------------------------------------------
set(ep_superbuild_extra_args)

if(DEFINED CTEST_CONFIGURATION_TYPE)
  list(APPEND ep_superbuild_extra_args -DCTEST_CONFIGURATION_TYPE:STRING=${CTEST_CONFIGURATION_TYPE})
endif()

if(WIN32)
  list(APPEND ep_superbuild_extra_args -DSlicer_SKIP_ROOT_DIR_MAX_LENGTH_CHECK:BOOL=ON)
endif()

if(Slicer_USE_PYTHONQT)
  list(APPEND ep_superbuild_extra_args
    -DPYTHON_EXECUTABLE:FILEPATH=${slicer_PYTHON_EXECUTABLE}
    -DPYTHON_INCLUDE_DIR:PATH=${slicer_PYTHON_INCLUDE}
    -DPYTHON_LIBRARY:FILEPATH=${slicer_PYTHON_LIBRARY}
    )
endif()

if(Slicer_USE_PYTHONQT_WITH_TCL)
  list(APPEND ep_superbuild_extra_args
    -DSlicer_TCL_DIR:PATH=${tcl_build}
    -DTCL_TK_VERSION:STRING=${TCL_TK_VERSION}
    -DTCL_TK_VERSION_DOT:STRING=${TCL_TK_VERSION_DOT}
    )
  if(INCR_TCL_VERSION)
    list(APPEND ep_superbuild_extra_args
      -DINCR_TCL_VERSION:STRING=${INCR_TCL_VERSION}
      -DINCR_TCL_VERSION_DOT:STRING=${INCR_TCL_VERSION_DOT}
      )
  endif()
endif()

if(Slicer_BUILD_QTLOADABLEMODULES)
  # Provide a mechanism to disable one or more loadable modules.
  list(APPEND ep_superbuild_extra_args -DSlicer_QTLOADABLEMODULES_DISABLED:STRING=${Slicer_QTLOADABLEMODULES_DISABLED})
endif()

if(Slicer_USE_BatchMake)
  list(APPEND ep_superbuild_extra_args -DBatchMake_DIR:PATH=${BatchMake_DIR})
endif()

if(Slicer_USE_OpenIGTLink)
  list(APPEND ep_superbuild_extra_args -DOpenIGTLink_DIR:PATH=${OpenIGTLink_DIR})
endif()

if(Slicer_BUILD_OpenIGTLinkIF)
  list(APPEND ep_superbuild_extra_args -DOpenIGTLinkIF_SOURCE_DIR:PATH=${OpenIGTLinkIF_SOURCE_DIR})
endif()

if(Slicer_USE_CTKAPPLAUNCHER)
  list(APPEND ep_superbuild_extra_args -DCTKAPPLAUNCHER_DIR:PATH=${CTKAPPLAUNCHER_DIR})
endif()

if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND ep_superbuild_extra_args -DSlicerExecutionModel_DIR:PATH=${SlicerExecutionModel_DIR})
endif()

if(Slicer_BUILD_BRAINSTOOLS)
  list(APPEND ep_superbuild_extra_args -DBRAINSTools_SOURCE_DIR:PATH=${BRAINSTools_SOURCE_DIR})
endif()

if(Slicer_BUILD_ChangeTrackerPy)
  list(APPEND ep_superbuild_extra_args -DChangeTrackerPy_SOURCE_DIR:PATH=${ChangeTrackerPy_SOURCE_DIR})
endif()

if(Slicer_BUILD_MultiVolumeExplorer)
  list(APPEND ep_superbuild_extra_args -DMultiVolumeExplorer_SOURCE_DIR:PATH=${MultiVolumeExplorer_SOURCE_DIR})
endif()

if(Slicer_BUILD_MultiVolumeImporter)
  list(APPEND ep_superbuild_extra_args -DMultiVolumeImporter_SOURCE_DIR:PATH=${MultiVolumeImporter_SOURCE_DIR})
endif()

if(Slicer_BUILD_EMSegment)
  list(APPEND ep_superbuild_extra_args -DEMSegment_SOURCE_DIR:PATH=${EMSegment_SOURCE_DIR})
endif()

if(Slicer_BUILD_SkullStripper)
  list(APPEND ep_superbuild_extra_args -DSkullStripper_SOURCE_DIR:PATH=${SkullStripper_SOURCE_DIR})
endif()

if(Slicer_BUILD_SlicerWebGLExport)
  list(APPEND ep_superbuild_extra_args -DSlicerWebGLExport_SOURCE_DIR:PATH=${SlicerWebGLExport_SOURCE_DIR})
endif()

if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND ep_superbuild_extra_args
    -DLibArchive_DIR:PATH=${LibArchive_DIR}
    -DqMidasAPI_DIR:PATH=${qMidasAPI_DIR}
    )
endif()

# Set CMake OSX variable to pass down the external project
set(CMAKE_OSX_EXTERNAL_PROJECT_ARGS)
if(APPLE)
  list(APPEND CMAKE_OSX_EXTERNAL_PROJECT_ARGS
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

#------------------------------------------------------------------------------
# Configure and build Slicer
#------------------------------------------------------------------------------
set(proj Slicer)

ExternalProject_Add(${proj}
  DEPENDS ${Slicer_DEPENDENCIES}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  BINARY_DIR Slicer-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    ${ep_superbuild_boolean_args}
    ${ep_superbuild_extra_args}
    ${CMAKE_OSX_EXTERNAL_PROJECT_ARGS}
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DADDITIONAL_C_FLAGS:STRING=${ADDITIONAL_C_FLAGS}
    -DADDITIONAL_CXX_FLAGS:STRING=${ADDITIONAL_CXX_FLAGS}
    -DSlicer_REQUIRED_C_FLAGS:STRING=${Slicer_REQUIRED_C_FLAGS}
    -DSlicer_REQUIRED_CXX_FLAGS:STRING=${Slicer_REQUIRED_CXX_FLAGS}
    -DWITH_COVERAGE:BOOL=${WITH_COVERAGE}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DSubversion_SVN_EXECUTABLE:FILEPATH=${Subversion_SVN_EXECUTABLE}
    -DGIT_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
    -DSlicer_SUPERBUILD:BOOL=OFF
    -DSlicer_SUPERBUILD_DIR:PATH=${Slicer_BINARY_DIR}
    -DSlicer_BUILD_WIN32_CONSOLE:BOOL=${Slicer_BUILD_WIN32_CONSOLE}
    -DDOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY:PATH=${DOCUMENTATION_ARCHIVES_OUTPUT_DIRECTORY}
    -DDOXYGEN_EXECUTABLE:FILEPATH=${DOXYGEN_EXECUTABLE}
    # ITK
    -DITK_VERSION_MAJOR:STRING=${ITK_VERSION_MAJOR}
    -DITK_DIR:PATH=${ITK_DIR}
    # Teem
    -DTeem_DIR:PATH=${Teem_DIR}
    # VTK
    -DVTK_DIR:PATH=${VTK_DIR}
    -DVTK_DEBUG_LEAKS:BOOL=${Slicer_USE_VTK_DEBUG_LEAKS}
    # cmcurl
    -DSLICERLIBCURL_DIR:PATH=${SLICERLIBCURL_DIR}
    # Qt
    -DQT_QMAKE_EXECUTABLE:PATH=${QT_QMAKE_EXECUTABLE}
    # CTK
    -DCTK_DIR:PATH=${CTK_DIR}
    # jqPlot
    -DjqPlot_DIR:PATH=${jqPlot_DIR}
  INSTALL_COMMAND ""
  )

# This custom external project step forces the build and later
# steps to run whenever a top level build is done...
ExternalProject_Add_Step(${proj} forcebuild
  COMMAND ${CMAKE_COMMAND} -E echo_append ""
  COMMENT "Forcing build step for '${proj}'"
  DEPENDEES configure
  DEPENDERS build
  ALWAYS 1
  )

#-----------------------------------------------------------------------------
# Slicer extensions
#-----------------------------------------------------------------------------
add_subdirectory(Extensions/CMake)
