################################################################################
#
#  Program: 3D Slicer
#
#  Copyright (c) 2010 Kitware Inc.
#
#  See Doc/copyright/copyright.txt
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

# Note also that script is executed each time the target 'SlicerConfigureLauncher' is built.
#
# It means you could manually trigger the reconfiguration of the launcher settings file 
# by building that target.

# Convenient variable set to the subdirectory containing the libraries.
# Set to 'lib' on Linux and Darwin, set to 'bin' on Windows
SET(LIB_SUBDIR lib)
IF(WIN32)
  SET(LIB_SUBDIR bin)
ENDIF()

#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
# Settings specific to the build tree.
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# LIBRARY_PATHS
#-----------------------------------------------------------------------------
SET(SLICER_LIBRARY_PATHS_BUILD
  ${VTK_DIR}/bin/<CMAKE_CFG_INTDIR>
  ${CTK_DIR}/CTK-build/bin/<CMAKE_CFG_INTDIR>
  ${QT_LIBRARY_DIR}
  ${ITK_DIR}/bin/<CMAKE_CFG_INTDIR>
  ${Teem_DIR}/bin/<CMAKE_CFG_INTDIR>
  ${LibArchive_DIR}/${LIB_SUBDIR}
  ./bin/<CMAKE_CFG_INTDIR>
  ./${Slicer_INSTALL_PLUGINS_LIB_DIR}/<CMAKE_CFG_INTDIR>
  ./${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR}/<CMAKE_CFG_INTDIR>
  )


IF(Slicer_USE_PYTHONQT_WITH_TCL)
  LIST(APPEND SLICER_LIBRARY_PATHS_BUILD
    ${Slicer_TCL_DIR}/lib
    )
  IF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    LIST(APPEND SLICER_LIBRARY_PATHS_BUILD
      ${Slicer_TCL_DIR}/lib/itcl${INCR_TCL_VERSION_DOT}
      ${Slicer_TCL_DIR}/lib/itk${INCR_TCL_VERSION_DOT}
      )
  ENDIF()
ENDIF()

IF(Slicer_USE_BatchMake)
  LIST(APPEND SLICER_LIBRARY_PATHS_BUILD
    ${BatchMake_DIR}/bin/<CMAKE_CFG_INTDIR>
    )
ENDIF()

IF(Slicer_USE_OPENIGTLINK)
  LIST(APPEND SLICER_LIBRARY_PATHS_BUILD
    ${OpenIGTLink_DIR}
    ${OpenIGTLink_DIR}/bin/<CMAKE_CFG_INTDIR>
    )
ENDIF()

IF(Slicer_USE_PYTHONQT)
  get_filename_component(SLICER_PYTHON_LIB_DIR ${PYTHON_LIBRARY} PATH)
  LIST(APPEND SLICER_LIBRARY_PATHS_BUILD 
    ${CTK_DIR}/PythonQt-build/<CMAKE_CFG_INTDIR>
    ${SLICER_PYTHON_LIB_DIR}
    )
ENDIF()

#-----------------------------------------------------------------------------
# PATHS
#-----------------------------------------------------------------------------
SET(SLICER_PATHS_BUILD
  ./bin/<CMAKE_CFG_INTDIR>
  ./${Slicer_INSTALL_PLUGINS_LIB_DIR}/<CMAKE_CFG_INTDIR>
  ./${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR}/<CMAKE_CFG_INTDIR>
  ${Teem_DIR}/bin/<CMAKE_CFG_INTDIR>
  ${QT_BINARY_DIR}
  )
  
IF(Slicer_USE_PYTHONQT_WITH_TCL)
  LIST(APPEND SLICER_PATHS_BUILD
    ${Slicer_TCL_DIR}/bin
    )
ENDIF()

#-----------------------------------------------------------------------------
# ENVVARS
#-----------------------------------------------------------------------------
SET(SLICER_ENVVARS_BUILD
  "QT_PLUGIN_PATH=./bin<PATHSEP>${CTK_DIR}/CTK-build/bin"
  )
  
IF(Slicer_USE_PYTHONQT)
  SET(pythonpath_subdir python${Slicer_PYTHON_VERSION_DOT})
  IF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    SET(pythonpath_subdir "../Lib")
  ENDIF()
  SET(PYTHONPATH "${SLICER_PYTHON_LIB_DIR}/${pythonpath_subdir}")
  SET(PYTHONPATH "${PYTHONPATH}<PATHSEP>${SLICER_PYTHON_LIB_DIR}/${pythonpath_subdir}/lib-tk")
  SET(PYTHONPATH "${PYTHONPATH}<PATHSEP>${SLICER_PYTHON_LIB_DIR}/${pythonpath_subdir}/site-packages")
  SET(PYTHONPATH "${PYTHONPATH}<PATHSEP>${SLICER_PYTHON_LIB_DIR}/${pythonpath_subdir}/site-packages/numpy")
  SET(PYTHONPATH "${PYTHONPATH}<PATHSEP>${SLICER_PYTHON_LIB_DIR}/${pythonpath_subdir}/site-packages/numpy/lib")
  SET(PYTHONPATH "${PYTHONPATH}<PATHSEP>${SLICER_PYTHON_LIB_DIR}/${pythonpath_subdir}/site-packages/numpy/core")
  SET(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/bin")
  SET(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/bin/python")
  
  LIST(APPEND SLICER_ENVVARS_BUILD "PYTHONPATH=${PYTHONPATH}")
ENDIF()
  
IF(Slicer_USE_PYTHONQT_WITH_TCL)
  # Search locations for TCL packages - space separated list
  SET(TCLLIBPATH "${Slicer_TCL_DIR}/lib/itcl${INCR_TCL_VERSION_DOT}")
  SET(TCLLIBPATH "${TCLLIBPATH} ${Slicer_TCL_DIR}/lib/itk${INCR_TCL_VERSION_DOT}")
  
  LIST(APPEND SLICER_ENVVARS_BUILD
    "TCL_LIBRARY=${Slicer_TCL_DIR}/lib/tcl${TCL_TK_VERSION_DOT}"
    "TK_LIBRARY=${Slicer_TCL_DIR}/lib/tk${TCL_TK_VERSION_DOT}"
    "TCLLIBPATH=${TCLLIBPATH}"
    )
ENDIF()

#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------
# Settings specific to the install tree.
#-----------------------------------------------------------------------------
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# LIBRARY_PATHS
#-----------------------------------------------------------------------------
SET(SLICER_LIBRARY_PATHS_INSTALLED
  ./${Slicer_INSTALL_BIN_DIR}
  ./${Slicer_INSTALL_LIB_DIR}
  ./${Slicer_INSTALL_PLUGINS_LIB_DIR}
  ./${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR}
  # External projects
  ./lib/Teem-${Teem_VERSION_MAJOR}.${Teem_VERSION_MINOR}.${Teem_VERSION_PATCH}
  )

IF(Slicer_USE_OPENIGTLINK)
  LIST(APPEND SLICER_LIBRARY_PATHS_INSTALLED
    # External projects 
    ./lib/igtl
    )
ENDIF()

IF(Slicer_USE_BatchMake)
  LIST(APPEND SLICER_LIBRARY_PATHS_INSTALLED
    ./lib/BatchMake
    ./lib/bmModuleDescriptionParser
    )
ENDIF()

IF(UNIX AND Slicer_USE_PYTHONQT)
  # On windows, both pythonQt and python libraries are installed allong with the executable
  LIST(APPEND SLICER_LIBRARY_PATHS_INSTALLED
    ./lib/Python/lib
    ./lib/PythonQt
    )
ENDIF()

IF(Slicer_USE_PYTHONQT_WITH_TCL)
  SET(tcllib_subdir lib)
  IF(WIN32)
    SET(tcllib_subdir bin)
  ENDIF()
  LIST(APPEND SLICER_LIBRARY_PATHS_INSTALLED
    ./lib/TclTk/${tcllib_subdir}
    ./lib/TclTk/lib/itcl${INCR_TCL_VERSION_DOT}
    ./lib/TclTk/lib/itk${INCR_TCL_VERSION_DOT}
    )
ENDIF()

#-----------------------------------------------------------------------------
# PATHS
#-----------------------------------------------------------------------------
SET(SLICER_PATHS_INSTALLED
  ./${Slicer_INSTALL_BIN_DIR}
  ./${Slicer_INSTALL_PLUGINS_BIN_DIR}
  ./${Slicer_INSTALL_QTLOADABLEMODULES_BIN_DIR}
  )


#-----------------------------------------------------------------------------
# ENVVARS
#-----------------------------------------------------------------------------
SET(SLICER_ENVVARS_INSTALLED
  "QT_PLUGIN_PATH=<APPLAUNCHER_DIR>/${Slicer_INSTALL_LIB_DIR}"
  )
  
IF(Slicer_USE_PYTHONQT)
  SET(pythonpath_subdir lib/python${Slicer_PYTHON_VERSION_DOT})
  IF(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    SET(pythonpath_subdir "Lib")
  ENDIF()
  SET(PYTHONPATH "<APPLAUNCHER_DIR>/${Slicer_INSTALL_LIB_DIR}")
  SET(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/lib/Python/${pythonpath_subdir}")
  SET(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/lib/Python/${pythonpath_subdir}/lib-tk")
  SET(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/lib/Python/${pythonpath_subdir}/site-packages")
  SET(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/lib/Python/${pythonpath_subdir}/site-packages/numpy")
  SET(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/lib/Python/${pythonpath_subdir}/site-packages/numpy/lib")
  SET(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/lib/Python/${pythonpath_subdir}/site-packages/numpy/core")
  SET(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}")
  SET(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/${Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR}")
  SET(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/lib/vtkTeem")
  SET(PYTHONPATH "${PYTHONPATH}<PATHSEP><APPLAUNCHER_DIR>/bin/Python")
  
  LIST(APPEND SLICER_ENVVARS_INSTALLED "PYTHONPATH=${PYTHONPATH}")
ENDIF()
  
IF(Slicer_USE_PYTHONQT_WITH_TCL)
  # Search locations for TCL packages - space separated list
  SET(TCLLIBPATH "<APPLAUNCHER_DIR>/lib/TclTk/lib/itcl${INCR_TCL_VERSION_DOT}")
  SET(TCLLIBPATH "${TCLLIBPATH} <APPLAUNCHER_DIR>/lib/TclTk/lib/itk${INCR_TCL_VERSION_DOT}")
  
  LIST(APPEND SLICER_ENVVARS_INSTALLED
    "TCL_LIBRARY=<APPLAUNCHER_DIR>/lib/TclTk/lib/tcl${TCL_TK_VERSION_DOT}"
    "TK_LIBRARY=<APPLAUNCHER_DIR>/lib/TclTk/lib/tk${TCL_TK_VERSION_DOT}"
    "TCLLIBPATH=${TCLLIBPATH}"
    )
ENDIF()

