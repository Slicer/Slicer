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

#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
# Settings specific to the build tree.
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# LIBRARY_PATHS
#-----------------------------------------------------------------------------
set(SLICER_LIBRARY_PATHS_BUILD
  <APPLAUNCHER_DIR>/bin/<CMAKE_CFG_INTDIR>
  )

if(NOT Slicer_USE_SYSTEM_QT)
  list(APPEND SLICER_LIBRARY_PATHS_BUILD ${QT_LIBRARY_DIR})
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

# External projects
foreach(varname IN LISTS Slicer_EP_LABEL_LIBRARY_PATHS_LAUNCHER_BUILD)
  list(APPEND SLICER_LIBRARY_PATHS_BUILD ${${varname}})
endforeach()

#-----------------------------------------------------------------------------
# PATHS
#-----------------------------------------------------------------------------
set(SLICER_PATHS_BUILD
  <APPLAUNCHER_DIR>/bin/<CMAKE_CFG_INTDIR>
  ${QT_BINARY_DIR}
  )

if(Slicer_BUILD_CLI_SUPPORT AND Slicer_BUILD_CLI)
  list(APPEND SLICER_PATHS_BUILD
    <APPLAUNCHER_DIR>/${Slicer_CLIMODULES_BIN_DIR}/<CMAKE_CFG_INTDIR>
    )
endif()

# External projects
foreach(varname IN LISTS Slicer_EP_LABEL_PATHS_LAUNCHER_BUILD)
  list(APPEND SLICER_PATHS_BUILD ${${varname}})
endforeach()

#-----------------------------------------------------------------------------
# ENVVARS
#-----------------------------------------------------------------------------
set(SLICER_ENVVARS_BUILD
  "SLICER_HOME=${Slicer_BINARY_DIR}" # See note below
  )
set(SLICER_QT_PLUGIN_PATH_BUILD
  "<APPLAUNCHER_DIR>/bin"
  "${CTK_DIR}/CTK-build/bin"
  "${QT_PLUGINS_DIR}"
  )
set(SLICER_ADDITIONAL_PATH_ENVVARS_BUILD
  "QT_PLUGIN_PATH"
  )

# Note: The addition of SLICER_HOME to the build environment has been motivated by
#       EMSegmentCommandLine executable. Indeed, this CLI is instantiating
#       qSlicerApplication which requires SLICER_HOME to be set so that it could
#       properly initialize the python environment and execute both python and Tcl scripts.
#       Please note that the environment variable is SLICER_HOME,
#       CMake refers to the variable as Slicer_HOME, and the tcl variable is SlicerHome.

if(Slicer_USE_PYTHONQT)

  list(APPEND SLICER_ADDITIONAL_PATH_ENVVARS_BUILD
    "PYTHONPATH"
    )

  set(SLICER_PYTHONPATH_BUILD
    "<APPLAUNCHER_DIR>/bin/<CMAKE_CFG_INTDIR>"
    "<APPLAUNCHER_DIR>/bin/Python"
    )
  if(Slicer_BUILD_QTLOADABLEMODULES)
    list(APPEND SLICER_PYTHONPATH_BUILD
      "<APPLAUNCHER_DIR>/${Slicer_QTLOADABLEMODULES_LIB_DIR}/<CMAKE_CFG_INTDIR>"
      "<APPLAUNCHER_DIR>/${Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR}"
      )
  endif()

  # External projects - pythonpath
  foreach(varname IN LISTS Slicer_EP_LABEL_PYTHONPATH_LAUNCHER_BUILD)
    list(APPEND SLICER_PYTHONPATH_BUILD ${${varname}})
  endforeach()

  # External projects - environment variables
  foreach(varname IN LISTS Slicer_EP_LABEL_ENVVARS_LAUNCHER_BUILD)
    list(APPEND SLICER_ENVVARS_BUILD ${${varname}})
  endforeach()
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
  )

# The following lines allow Slicer to load a CLI module extension that depends
# on libraries (i.e a logic class) provided by a loadable module extension.
list(APPEND SLICER_LIBRARY_PATHS_INSTALLED ../${Slicer_QTLOADABLEMODULES_LIB_DIR})
if(CMAKE_CONFIGURATION_TYPES)
  list(APPEND SLICER_LIBRARY_PATHS_INSTALLED ../${Slicer_QTLOADABLEMODULES_LIB_DIR}/<CMAKE_CFG_INTDIR>)
endif()

# External projects
foreach(varname IN LISTS Slicer_EP_LABEL_LIBRARY_PATHS_LAUNCHER_INSTALLED)
  list(APPEND SLICER_LIBRARY_PATHS_INSTALLED ${${varname}})
endforeach()

#-----------------------------------------------------------------------------
# PATHS
#-----------------------------------------------------------------------------
set(SLICER_PATHS_INSTALLED
  <APPLAUNCHER_DIR>/${Slicer_INSTALL_BIN_DIR}
  <APPLAUNCHER_DIR>/${Slicer_INSTALL_CLIMODULES_BIN_DIR}
  <APPLAUNCHER_DIR>/${Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR}
  )

# External projects
foreach(varname IN LISTS Slicer_EP_LABEL_PATHS_LAUNCHER_INSTALLED)
  list(APPEND SLICER_PATHS_INSTALLED ${${varname}})
endforeach()

#-----------------------------------------------------------------------------
# ENVVARS
#-----------------------------------------------------------------------------
set(SLICER_ENVVARS_INSTALLED
  # SLICER_HOME might already be set on the machine, overwrite it because it
  # could have unwanted side effects
  "SLICER_HOME=<APPLAUNCHER_DIR>"
  )
set(SLICER_QT_PLUGIN_PATH_INSTALLED
  "<APPLAUNCHER_DIR>/${Slicer_INSTALL_QtPlugins_DIR}"
  )
set(SLICER_ADDITIONAL_PATH_ENVVARS_INSTALLED
  "QT_PLUGIN_PATH"
  )

if(Slicer_USE_PYTHONQT)
  set(SLICER_PYTHONPATH_INSTALLED
    "<APPLAUNCHER_DIR>/${Slicer_INSTALL_LIB_DIR}"
    "<APPLAUNCHER_DIR>/${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}"
    "<APPLAUNCHER_DIR>/${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR}"
    "<APPLAUNCHER_DIR>/lib/vtkTeem"
    "<APPLAUNCHER_DIR>/bin/Python"
    "<APPLAUNCHER_DIR>/${Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR}"
    )

  # External projects - pythonpath
  foreach(varname IN LISTS Slicer_EP_LABEL_PYTHONPATH_LAUNCHER_INSTALLED)
    list(APPEND SLICER_PYTHONPATH_INSTALLED ${${varname}})
  endforeach()

  list(APPEND SLICER_ADDITIONAL_PATH_ENVVARS_INSTALLED
    "PYTHONPATH"
    )

  # External projects - environment variables
  foreach(varname IN LISTS Slicer_EP_LABEL_ENVVARS_LAUNCHER_INSTALLED)
    list(APPEND SLICER_ENVVARS_INSTALLED ${${varname}})
  endforeach()

endif()

