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
# ------------------[ Slicer Launcher settings ] ------------------
#
# This file contains variables that can be grouped into two broad categories.
#
# There are the variables that are used to configure the launcher for a BUILD tree and also
# the ones for an INSTALLED tree.
#
# Each one of these categories contain: the PATHS, LIBRARY_PATHS and ENVVARS variables
#
# To summarize, you will find the following variables:
#
#   SLICER_LIBRARY_PATHS_BUILD
#   SLICER_PATHS_BUILD
#   SLICER_ENVVARS_BUILD
#
#   SLICER_LIBRARY_PATHS_INSTALLED
#   SLICER_PATHS_INSTALLED
#   SLICER_ENVVARS_INSTALLED
#

#
# Usually, if you are building on a system handling multiple build configrations
# (i.e Visual studio with Debug, Release, ...), the libraries and executables could be built in a
# subdirectory matching the active configuration.
#
# In case of a BUILD tree, each time a path containing such a sub directory should be considered,
# it's possible to rely on a special string: <CMAKE_CFG_INTDIR>
#
# At build time, <CMAKE_CFG_INTDIR> will be replaced by the active configuration name.
# This happens within the script "ctkAppLauncher-configure.cmake".

# Note also that script is executed each time the target '{Application_Name}ConfigureLauncher' is built.
#
# It means you could manually trigger the reconfiguration of the launcher settings file
# by building that target.

# Convenient variable set to the subdirectory containing the libraries.
# Set to 'lib' on Linux and Darwin, set to 'bin' on Windows
set(LIB_SUBDIR lib)
if(WIN32)
  set(LIB_SUBDIR bin)
endif()

#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
# Settings specific to the build tree.
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# LIBRARY_PATHS
#-----------------------------------------------------------------------------
set(SLICER_LIBRARY_PATHS_BUILD
  ${DCMTK_DIR}/bin/<CMAKE_CFG_INTDIR>
  ${VTK_DIR}/bin/<CMAKE_CFG_INTDIR>
  ${CTK_DIR}/CTK-build/bin/<CMAKE_CFG_INTDIR>
  ${QT_LIBRARY_DIR}
  ${ITK_DIR}/bin/<CMAKE_CFG_INTDIR>
  ${Teem_DIR}/bin/<CMAKE_CFG_INTDIR>
  ${LibArchive_DIR}/${LIB_SUBDIR}
  <APPLAUNCHER_DIR>/bin/<CMAKE_CFG_INTDIR>
  )

if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND SLICER_LIBRARY_PATHS_BUILD
    ${SlicerExecutionModel_DIR}/ModuleDescriptionParser/bin/<CMAKE_CFG_INTDIR>
    )
endif()

# The following lines allow Slicer to load a CLI module extension that depends
# on libraries (i.e a logic class) provided by a loadable module extension.
list(APPEND SLICER_LIBRARY_PATHS_BUILD ../${Slicer_QTLOADABLEMODULES_LIB_DIR})
if(CMAKE_CONFIGURATION_TYPES)
  list(APPEND SLICER_LIBRARY_PATHS_BUILD ../${Slicer_QTLOADABLEMODULES_LIB_DIR}/<CMAKE_CFG_INTDIR>)
endif()

if(Slicer_BUILD_CLI_SUPPORT AND Slicer_BUILD_CLI)
  list(APPEND SLICER_LIBRARY_PATHS_BUILD
    <APPLAUNCHER_DIR>/${Slicer_CLIMODULES_LIB_DIR}/<CMAKE_CFG_INTDIR>
    )
endif()

if(Slicer_BUILD_QTLOADABLEMODULES)
  list(APPEND SLICER_LIBRARY_PATHS_BUILD
    <APPLAUNCHER_DIR>/${Slicer_QTLOADABLEMODULES_LIB_DIR}/<CMAKE_CFG_INTDIR>
    )
endif()

if(Slicer_USE_PYTHONQT_WITH_TCL)
  list(APPEND SLICER_LIBRARY_PATHS_BUILD
    ${Slicer_TCL_DIR}/lib
    )
  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    list(APPEND SLICER_LIBRARY_PATHS_BUILD
      ${Slicer_TCL_DIR}/lib/itcl${INCR_TCL_VERSION_DOT}
      ${Slicer_TCL_DIR}/lib/itk${INCR_TCL_VERSION_DOT}
      )
  endif()
endif()

if(Slicer_USE_BatchMake)
  list(APPEND SLICER_LIBRARY_PATHS_BUILD
    ${BatchMake_DIR}/bin/<CMAKE_CFG_INTDIR>
    )
endif()

if(Slicer_USE_OpenIGTLink)
  list(APPEND SLICER_LIBRARY_PATHS_BUILD
    ${OpenIGTLink_DIR}
    ${OpenIGTLink_DIR}/bin/<CMAKE_CFG_INTDIR>
    )
endif()

if(Slicer_USE_PYTHONQT)
  set(SLICER_PYTHONHOME ${Slicer_SUPERBUILD_DIR}/python-install)
  if(UNIX)
    get_filename_component(SLICER_PYTHON_LIB_DIR ${PYTHON_LIBRARY} PATH)
  else()
    get_filename_component(SLICER_PYTHON_LIB_DIR ${PYTHON_EXECUTABLE} PATH)
  endif()
  list(APPEND SLICER_LIBRARY_PATHS_BUILD
    ${CTK_DIR}/PythonQt-build/<CMAKE_CFG_INTDIR>
    ${SLICER_PYTHON_LIB_DIR}
    )

  set(pythonpath_subdir lib/python${Slicer_PYTHON_VERSION_DOT})
  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(pythonpath_subdir Lib)
  endif()
endif()

if(Slicer_USE_QtTesting)
  list(APPEND SLICER_LIBRARY_PATHS_BUILD
    ${CTK_DIR}/QtTesting-build/<CMAKE_CFG_INTDIR>
    )
endif()

if(Slicer_USE_NUMPY)
  list(APPEND SLICER_LIBRARY_PATHS_BUILD
    ${SLICER_PYTHONHOME}/${pythonpath_subdir}/site-packages/numpy/core
    ${SLICER_PYTHONHOME}/${pythonpath_subdir}/site-packages/numpy/lib
    )
endif()

#-----------------------------------------------------------------------------
# PATHS
#-----------------------------------------------------------------------------
set(SLICER_PATHS_BUILD
  <APPLAUNCHER_DIR>/bin/<CMAKE_CFG_INTDIR>
  ${Teem_DIR}/bin/<CMAKE_CFG_INTDIR>
  ${QT_BINARY_DIR}
  )

if(Slicer_BUILD_CLI_SUPPORT AND Slicer_BUILD_CLI)
  list(APPEND SLICER_PATHS_BUILD
    <APPLAUNCHER_DIR>/${Slicer_CLIMODULES_BIN_DIR}/<CMAKE_CFG_INTDIR>
    )
endif()

if(Slicer_USE_PYTHONQT_WITH_TCL)
  list(APPEND SLICER_PATHS_BUILD
    ${Slicer_TCL_DIR}/bin
    )
endif()

#-----------------------------------------------------------------------------
# ENVVARS
#-----------------------------------------------------------------------------
set(SLICER_ENVVARS_BUILD
  "QT_PLUGIN_PATH=<APPLAUNCHER_DIR>/bin<PATHSEP>${CTK_DIR}/CTK-build/bin<PATHSEP>${QT_PLUGINS_DIR}"
  "SLICER_HOME=${Slicer_BINARY_DIR}" # See note below
  )

# Note: The addition of SLICER_HOME to the build environment has been motivated by
#       EMSegmentCommandLine executable. Indeed, this CLI is instantiating
#       qSlicerApplication which requires SLICER_HOME to be set so that it could
#       properly initialize the python environment and execute both python and Tcl scripts.
#       Please note that the environment variable is SLICER_HOME,
#       CMake refers to the variable as Slicer_HOME, and the tcl variable is SlicerHome.

if(Slicer_USE_PYTHONQT)
  set(PYTHONPATH "<APPLAUNCHER_DIR>/bin/<CMAKE_CFG_INTDIR>")
  set(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/bin/Python")
  set(PYTHONPATH "${PYTHONPATH}<PATHSEP>${SLICER_PYTHONHOME}/${pythonpath_subdir}")
  set(PYTHONPATH "${PYTHONPATH}<PATHSEP>${SLICER_PYTHONHOME}/${pythonpath_subdir}/lib-dynload")
  set(PYTHONPATH "${PYTHONPATH}<PATHSEP>${SLICER_PYTHONHOME}/${pythonpath_subdir}/site-packages")

  if(Slicer_BUILD_QTLOADABLEMODULES)
    set(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/${Slicer_QTLOADABLEMODULES_LIB_DIR}/<CMAKE_CFG_INTDIR>")
    set(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/${Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR}")
  endif()

  if(Slicer_USE_PYTHONQT_WITH_TCL)
    set(PYTHONPATH "${PYTHONPATH}<PATHSEP>${SLICER_PYTHONHOME}/${pythonpath_subdir}/lib-tk")
  endif()

  list(APPEND SLICER_ENVVARS_BUILD
    "PYTHONHOME=${SLICER_PYTHONHOME}"
    "PYTHONPATH=${PYTHONPATH}")
endif()

if(Slicer_USE_PYTHONQT_WITH_TCL)
  # Search locations for TCL packages - space separated list
  set(TCLLIBPATH "${Slicer_TCL_DIR}/lib/itcl${INCR_TCL_VERSION_DOT}")
  set(TCLLIBPATH "${TCLLIBPATH} ${Slicer_TCL_DIR}/lib/itk${INCR_TCL_VERSION_DOT}")

  list(APPEND SLICER_ENVVARS_BUILD
    "TCL_LIBRARY=${Slicer_TCL_DIR}/lib/tcl${TCL_TK_VERSION_DOT}"
    "TK_LIBRARY=${Slicer_TCL_DIR}/lib/tk${TCL_TK_VERSION_DOT}"
    "TCLLIBPATH=${TCLLIBPATH}"
    )
endif()

#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
# Settings specific to the install tree.
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# LIBRARY_PATHS
#-----------------------------------------------------------------------------
set(SLICER_LIBRARY_PATHS_INSTALLED
  <APPLAUNCHER_DIR>/${Slicer_INSTALL_BIN_DIR}
  <APPLAUNCHER_DIR>/${Slicer_INSTALL_LIB_DIR}
  <APPLAUNCHER_DIR>/${Slicer_INSTALL_CLIMODULES_LIB_DIR}
  <APPLAUNCHER_DIR>/${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR}
  # External projects
  <APPLAUNCHER_DIR>/lib/Teem-${Teem_VERSION_MAJOR}.${Teem_VERSION_MINOR}.${Teem_VERSION_PATCH}
  )

# The following lines allow Slicer to load a CLI module extension that depends
# on libraries (i.e a logic class) provided by a loadable module extension.
list(APPEND SLICER_LIBRARY_PATHS_INSTALLED ../${Slicer_QTLOADABLEMODULES_LIB_DIR})
if(CMAKE_CONFIGURATION_TYPES)
  list(APPEND SLICER_LIBRARY_PATHS_INSTALLED ../${Slicer_QTLOADABLEMODULES_LIB_DIR}/<CMAKE_CFG_INTDIR>)
endif()

if(Slicer_USE_OpenIGTLink)
  list(APPEND SLICER_LIBRARY_PATHS_INSTALLED
    # External projects
    <APPLAUNCHER_DIR>/lib/igtl
    )
endif()

if(Slicer_USE_BatchMake)
  list(APPEND SLICER_LIBRARY_PATHS_INSTALLED
    <APPLAUNCHER_DIR>/lib/BatchMake
    <APPLAUNCHER_DIR>/lib/bmModuleDescriptionParser
    )
endif()

if(UNIX AND Slicer_USE_PYTHONQT)
  # On windows, both pythonQt and python libraries are installed allong with the executable
  list(APPEND SLICER_LIBRARY_PATHS_INSTALLED
    <APPLAUNCHER_DIR>/lib/Python/lib
    <APPLAUNCHER_DIR>/lib/PythonQt
    )
endif()

if(Slicer_USE_PYTHONQT_WITH_TCL)
  set(tcllib_subdir lib)
  if(WIN32)
    set(tcllib_subdir bin)
  endif()
  list(APPEND SLICER_LIBRARY_PATHS_INSTALLED
    <APPLAUNCHER_DIR>/lib/TclTk/${tcllib_subdir}
    <APPLAUNCHER_DIR>/lib/TclTk/lib/itcl${INCR_TCL_VERSION_DOT}
    <APPLAUNCHER_DIR>/lib/TclTk/lib/itk${INCR_TCL_VERSION_DOT}
    )
endif()

if(Slicer_USE_PYTHONQT)
  set(pythonpath_subdir lib/python${Slicer_PYTHON_VERSION_DOT})
  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(pythonpath_subdir Lib)
  endif()
endif()

if(Slicer_USE_NUMPY)
  list(APPEND SLICER_LIBRARY_PATHS_INSTALLED
    <APPLAUNCHER_DIR>/lib/Python/${pythonpath_subdir}/site-packages/numpy/core
    <APPLAUNCHER_DIR>/lib/Python/${pythonpath_subdir}/site-packages/numpy/lib
    )
endif()

#-----------------------------------------------------------------------------
# PATHS
#-----------------------------------------------------------------------------
set(SLICER_PATHS_INSTALLED
  <APPLAUNCHER_DIR>/${Slicer_INSTALL_BIN_DIR}
  <APPLAUNCHER_DIR>/${Slicer_INSTALL_CLIMODULES_BIN_DIR}
  <APPLAUNCHER_DIR>/${Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR}
  )


#-----------------------------------------------------------------------------
# ENVVARS
#-----------------------------------------------------------------------------
set(SLICER_ENVVARS_INSTALLED
  "QT_PLUGIN_PATH=<APPLAUNCHER_DIR>/${Slicer_INSTALL_QtPlugins_DIR}"
  # SLICER_HOME might already be set on the machine, overwrite it because it
  # could have unwanted side effects
  "SLICER_HOME=<APPLAUNCHER_DIR>"
  )

if(Slicer_USE_PYTHONQT)
  set(PYTHONPATH "<APPLAUNCHER_DIR>/${Slicer_INSTALL_LIB_DIR}")
  set(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/lib/Python/${pythonpath_subdir}")
  set(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/lib/Python/${pythonpath_subdir}/lib-dynload")
  set(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/lib/Python/${pythonpath_subdir}/site-packages")
  set(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}")
  set(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR}")
  set(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/lib/vtkTeem")
  set(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/bin/Python")
  set(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/${Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR}")

  if(Slicer_USE_PYTHONQT_WITH_TCL)
    set(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/lib/Python/${pythonpath_subdir}/lib-tk")
  endif()

  list(APPEND SLICER_ENVVARS_INSTALLED
    "PYTHONHOME=<APPLAUNCHER_DIR>/lib/Python"
    "PYTHONPATH=${PYTHONPATH}")
endif()

if(Slicer_USE_PYTHONQT_WITH_TCL)
  # Search locations for TCL packages - space separated list
  set(TCLLIBPATH "<APPLAUNCHER_DIR>/lib/TclTk/lib/itcl${INCR_TCL_VERSION_DOT}")
  set(TCLLIBPATH "${TCLLIBPATH} <APPLAUNCHER_DIR>/lib/TclTk/lib/itk${INCR_TCL_VERSION_DOT}")

  list(APPEND SLICER_ENVVARS_INSTALLED
    "TCL_LIBRARY=<APPLAUNCHER_DIR>/lib/TclTk/lib/tcl${TCL_TK_VERSION_DOT}"
    "TK_LIBRARY=<APPLAUNCHER_DIR>/lib/TclTk/lib/tk${TCL_TK_VERSION_DOT}"
    "TCLLIBPATH=${TCLLIBPATH}"
    )
endif()

