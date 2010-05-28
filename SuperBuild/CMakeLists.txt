
#-----------------------------------------------------------------------------
project(Slicer3)
#-----------------------------------------------------------------------------

enable_language(C)
enable_language(CXX)

#-----------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.8)
#-----------------------------------------------------------------------------

#-----------------------------------------------------------------------------
# See http://cmake.org/cmake/help/cmake-2-8-docs.html#section_Policies for details
#-----------------------------------------------------------------------------

SET(project_policies
  #CMP0001 # NEW: CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.
  #CMP0002 # NEW: Logical target names must be globally unique.
  CMP0003 # NEW: Libraries linked via full path no longer produce linker search paths.
  #CMP0004 # NEW: Libraries linked may NOT have leading or trailing whitespace.
  #CMP0005 # NEW: Preprocessor definition values are now escaped automatically.
  #CMP0006 # NEW: Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.
  #CMP0007 # NEW: List command no longer ignores empty elements.
  #CMP0008 # NEW: Libraries linked by full-path must have a valid library file name.
  #CMP0009 # NEW: FILE GLOB_RECURSE calls should not follow symlinks by default.
  #CMP0010 # NEW: Bad variable reference syntax is an error.
  #CMP0011 # NEW: Included scripts do automatic cmake_policy PUSH and POP.
  #CMP0012 # NEW: if() recognizes numbers and boolean constants.
  #CMP0013 # NEW: Duplicate binary directories are not allowed.
  #CMP0014 # NEW: Input directories must have CMakeLists.txt
  )
FOREACH(policy ${project_policies})
  IF(POLICY ${policy})
    CMAKE_POLICY(SET ${policy} NEW)
  ENDIF()
ENDFOREACH()


#-----------------------------------------------------------------------------
# Update CMake module path
#------------------------------------------------------------------------------

set(CMAKE_MODULE_PATH
  ${CMAKE_SOURCE_DIR}/CMake
  ${CMAKE_BINARY_DIR}/CMake
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../CMake # Slicer3 CMake directory
  ${CMAKE_MODULE_PATH}
  )

#-----------------------------------------------------------------------------
# Convenient macro allowing to download a file
#------------------------------------------------------------------------------

MACRO(downloadFile url dest)
  FILE(DOWNLOAD ${url} ${dest} STATUS status)
  LIST(GET status 0 error_code)
  LIST(GET status 1 error_msg)
  IF(error_code)
    MESSAGE(FATAL_ERROR "error: Failed to download ${url} - ${error_msg}")
  ENDIF()
ENDMACRO()

#-----------------------------------------------------------------------------
# Prerequisites
#------------------------------------------------------------------------------

set(git_repo http://github.com/pieper/CTK/raw/master/)

set(file CMake/ExternalProject.cmake)
downloadFile(${git_repo}/${file} ${CMAKE_BINARY_DIR}/${file})

#-----------------------------------------------------------------------------
# Include External project module
#------------------------------------------------------------------------------

set(base "${CMAKE_BINARY_DIR}/CMakeExternals")
set(prefix "${base}/Install")

include(ExternalProject)

# Compute -G arg for configuring external projects with the same CMake generator:
if(CMAKE_EXTRA_GENERATOR)
  set(gen "${CMAKE_EXTRA_GENERATOR} - ${CMAKE_GENERATOR}")
else()
  set(gen "${CMAKE_GENERATOR}")
endif()

#-----------------------------------------------------------------------------
# Disable the warnings that DevStudio 2005 emits wrt to sprintf, strcpu, etc.
#------------------------------------------------------------------------------

if(CMAKE_COMPILER_2005)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_CRT_NONSTDC_NO_DEPRECATE)
endif(CMAKE_COMPILER_2005)

#-----------------------------------------------------------------------------
# Always build Shared library
#------------------------------------------------------------------------------

set(BUILD_SHARED_LIBS ON)

#-----------------------------------------------------------------------------
# Setup build type
#------------------------------------------------------------------------------

# By default, let's build as Debug
if(NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

# let a dashboard override the default.
if(CTEST_BUILD_CONFIGURATION)
  set(CMAKE_BUILD_TYPE "${CTEST_BUILD_CONFIGURATION}")
endif()


#-----------------------------------------------------------------------------
# Slicer Options
#-----------------------------------------------------------------------------

#
#                         ----------------------------
#                         | Case 1 | Case 2 | Case 3 |
# ----------------------------------------------------
# | MAC{32,64}{Tiger,Leo} |  ON    |   -    |  OK    |
# ----------------------------------------------------
# | WIN{32,64} VS2008     |  OK    |  OK    |  OK    |
# ----------------------------------------------------
# | Linux{32,64}          |  OK    |  OK    |  OK    |
# ----------------------------------------------------
# ----------------------------------------------------
# | Slicer3_USE_KWWIDGETS |  ON    |  ON    |  OFF   |
# |---------------------------------------------------
# | Slicer3_USE_QT        |  OFF   |  ON    |  ON    |
# |--------------------------------------------------|
# | Slicer3_USE_PYTHON    | ON|OFF | ON|OFF |  OFF   |
# |--------------------------------------------------|
# | Slicer3_USE_PYTHONQT  |   OFF  |  OFF   | ON|OFF |
# |---------------------------------------------------

#
# Case 1: [Slicer3_USE_KWWIDGETS: ON, Slicer3_USE_QT: OFF, VTK_WRAP_TCL:ON]
#   a) KwwOnly-NoPython  
#   b) KwwOnly-Python    
# 
# Case 2: [Slicer3_USE_KWWIDGETS: ON, Slicer3_USE_QT: ON, VTK_WRAP_TCL:ON]
#   a) KwwAndQt-NoPython-NoPythonQt
#   b) KwwAndQt-Python
#   c) KwwAndQt-PythonQt          [VTK_WRAP_PYTHON: ON]  -> NOT SUPPORTED
#   d) KwwAndQt-Python-PythonQt                          -> NOT SUPPORTED
#
# Case 3: [Slicer3_USE_KWWIDGETS: OFF, Slicer3_USE_QT: ON,  VTK_WRAP_TCL:OFF]
#   a) QtOnly-NoPythonQt
#   b) QtOnly-PythonQt            [VTK_WRAP_PYTHON: ON]
#
#
# Note:
#   Tcl, Tk, blt, incrTcl and iwidgets are build only if Slicer3_USE_KWWIDGETS is ON
#   Python is build if either Slicer3_USE_PYTHON or Slicer3_USE_PYTHONQT are ON
# 

#------------------------------------------------------------------------------
# KWWidgets option
#------------------------------------------------------------------------------  

# KWWidgets on by default.
option(Slicer3_USE_KWWIDGETS "Build Slicer3 KWWidgets GUI" ON)


#------------------------------------------------------------------------------
# Qt option
#------------------------------------------------------------------------------  

option(Slicer3_USE_QT  "Build Slicer3 Qt GUI" OFF)

if(Slicer3_USE_QT)
  SET(minimum_required_qt_version "4.6.2")
  
  find_package(Qt4)
  
  if(QT4_FOUND)
    
    set(current_qt_version "${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH}")
  
    if("${current_qt_version}" VERSION_LESS "${minimum_required_qt_version}")
      message(FATAL_ERROR "error: Slicer requires Qt >= ${minimum_required_qt_version} -- you cannot use Qt ${current_qt_version}.")
    endif()
  
    # Enable modules
    SET(QT_USE_QTNETWORK ON)
    
    # Includes Qt headers
    INCLUDE(${QT_USE_FILE})

    option(Slicer3_USE_PYTHONQT "Integrate a python-QT interpreter into Slicer." OFF)
  
  else()
    MESSAGE(FATAL_ERROR "error: Qt >= ${minimum_required_qt_version} was not found on your system. You probably need to set the QT_QMAKE_EXECUTABLE variable")
  endif()
  
endif()
  
#------------------------------------------------------------------------------
# Python option
#------------------------------------------------------------------------------  

option(Slicer3_USE_PYTHON "Build Slicer with Python support" ON)

if(Slicer3_USE_PYTHON)

  # Attempt to detect if we have a Fortran Compiler available.
  include(CheckFortran)

  if(CMAKE_Fortran_COMPILER)
    option(Slicer3_BUILD_NUMPY "Build Slicer with NumPy (builds Blas and Lapack)." ON)

    if(Slicer3_BUILD_NUMPY)
      option(Slicer3_BUILD_SCIPY "Build Slicer with SciPy (requires NumPy)" ON)
    endif()

  endif(CMAKE_Fortran_COMPILER)
endif() # Slicer3_USE_PYTHON

#------------------------------------------------------------------------------
# Additional options
#------------------------------------------------------------------------------  

# Enable VTK Debug Leaks
option(Slicer3_USE_VTK_DEBUG_LEAKS 
  "Turn on VTKs Debug Leaks functionality in both VTK and Slicer." ON)
  
#------------------------------------------------------------------------------
# Validate options
#------------------------------------------------------------------------------  

if (NOT Slicer3_USE_KWWIDGETS AND NOT Slicer3_USE_QT)
  set(err "error: Slicer SuperBuild requires at least Slicer3_USE_KWWIDGETS or Slicer3_USE_QT to be ON.

Slicer3_USE_KWWIDGETS = ON
 -> Means Slicer will be built only with KWWidgets support.
 
Slicer3_USE_KWWIDGETS = ON and Slicer3_USE_QT = ON
 -> Means Slicer will be built with both KWWidgets and QT support. Note: NOT supported on Mac
 
Slicer3_USE_QT = ON
 -> Means Slicer will be build only with QT support.
")

  message(FATAL_ERROR "${err}")
endif()

if (Slicer3_USE_PYTHON AND Slicer3_USE_PYTHONQT)
  set(err "error: Slicer SuperBuild doesn't support Slicer3_USE_PYTHON and Slicer3_USE_PYTHONQT ON.")
  message(FATAL_ERROR "${err}")
endif()

if (Slicer3_USE_KWWIDGETS AND Slicer3_USE_PYTHONQT)
  set(err "error: Slicer SuperBuild doesn't support Slicer3_USE_KWWIDGETS and Slicer3_USE_PYTHONQT ON.")
  message(FATAL_ERROR "${err}")
endif()

if (Slicer3_USE_KWWIDGETS AND Slicer3_USE_QT AND APPLE)
  set(err "error: Slicer SuperBuild on Mac doesn't support Slicer3_USE_KWWIDGETS and Slicer3_USE_QT ON")
  message(FATAL_ERROR "${err}")
endif()
  

#------------------------------------------------------------------------------
# List of external projects
#------------------------------------------------------------------------------  

set(external_project_list tk incrTcl iwidgets blt python blas lapack numpy scipy vtk CTK KWWidgets Insight BatchMake OpenIGTLink teem cmcurl slicer)

#------------------------------------------------------------------------------
# Configure the CTest custom file - Used to suppress external project configure/build warnings
#------------------------------------------------------------------------------  

# Project that should report warning
set(report_warning_projects slicer CTK)

# Generate custom warning exception
set(external_project_to_exclude ${external_project_list})
list(REMOVE_ITEM external_project_to_exclude ${report_warning_projects})
set(AUTOGENERATED_CUSTOM_WARNING_EXCEPTION "\n  \# The following exception have been auto-generated")
foreach(ep ${external_project_to_exclude})
  set(AUTOGENERATED_CUSTOM_WARNING_EXCEPTION 
    "${AUTOGENERATED_CUSTOM_WARNING_EXCEPTION}\n  \"${ep}.\"")
endforeach()

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/../SuperBuild/CTestCustom.cmake.in
  ${Slicer3_BINARY_DIR}/CTestCustom.cmake
  @ONLY IMMEDIATE)

#------------------------------------------------------------------------------
# Establish Target Dependencies based on Selected Options
#------------------------------------------------------------------------------

set(tk_DEPENDENCIES tcl)
set(incrTcl_DEPENDENCIES tcl tk)
set(iwidgets_DEPENDENCIES tcl itcl)
set(blt_DEPENDENCIES tcl tk)
set(python_DEPENDENCIES)
if(Slicer3_USE_KWWIDGETS)
  if(WIN32)
    set(python_DEPENDENCIES tcl)
  else()
    set(python_DEPENDENCIES tk)
  endif()
endif()
set(blas_DEPENDENCIES)
set(lapack_DEPENDENCIES)
set(numpy_DEPENDENCIES blas lapack python)
set(scipy_DEPENDENCIES numpy)
set(vtk_DEPENDENCIES)
set(CTK_DEPENDENCIES vtk)
if(Slicer3_USE_PYTHONQT)
  list(APPEND CTK_DEPENDENCIES python)
endif()
set(KWWidgets_DEPENDENCIES vtk)
set(Insight_DEPENDENCIES)
set(BatchMake_DEPENDENCIES Insight)
set(OpenIGTLink_DEPENDENCIES)
set(teem_DEPENDENCIES vtk)
set(cmcurl_DEPENDENCIES)
set(slicer_DEPENDENCIES vtk Insight BatchMake OpenIGTLink teem cmcurl)

#------------------------------------------------------------------------------
# Conditionnaly include ExternalProject Target
#------------------------------------------------------------------------------

if(Slicer3_USE_KWWIDGETS)
  include(External_Tcl)
  include(External_Tk)
  include(External_incrTcl)
  include(External_iwidgets)
  include(External_blt)
endif(Slicer3_USE_KWWIDGETS)

if(Slicer3_USE_PYTHON OR Slicer3_USE_PYTHONQT)
  include(External_Python26)
endif(Slicer3_USE_PYTHON OR Slicer3_USE_PYTHONQT)

if(CMAKE_Fortran_COMPILER AND Slicer3_BUILD_NUMPY)
  include(External_BLAS)
  include(External_LAPACK)
  include(External_NumPy)

  if(Slicer3_BUILD_SCIPY)
    include(External_SciPy)
  endif(Slicer3_BUILD_SCIPY)
endif(CMAKE_Fortran_COMPILER AND Slicer3_BUILD_NUMPY)

include(External_VTK)

if(Slicer3_USE_QT)
  include(External_CTK)
endif(Slicer3_USE_QT)

if(Slicer3_USE_KWWIDGETS)
  include(External_KWWidgets)
endif(Slicer3_USE_KWWIDGETS)

include(External_Insight)
include(External_teem)
include(External_OpenIGTLink)
include(External_BatchMake)
include(External_cmcurl)

#-----------------------------------------------------------------------------
# Update external project dependencies
#------------------------------------------------------------------------------

# For now, tk and itcl are used only when Slicer3_USE_KWWIDGETS is ON
if(Slicer3_USE_KWWIDGETS)
  if(WIN32)
    list(APPEND slicer_DEPENDENCIES tcl KWWidgets)
  else()
    list(APPEND slicer_DEPENDENCIES tcl tk itcl KWWidgets)
  endif()
endif(Slicer3_USE_KWWIDGETS)

if(CMAKE_Fortran_COMPILER)
  list(APPEND slicer_DEPENDENCIES numpy blas lapack)
endif(CMAKE_Fortran_COMPILER)

if(Slicer3_USE_QT)
  list(APPEND slicer_DEPENDENCIES CTK)
endif(Slicer3_USE_QT)

if(Slicer3_USE_PYTHON)
  list(APPEND slicer_DEPENDENCIES python)
endif(Slicer3_USE_PYTHON)

#-----------------------------------------------------------------------------
# Dump external project dependencies
#------------------------------------------------------------------------------

set(ep_dependency_graph "# External project dependencies")
foreach(ep ${external_project_list})
  set(ep_dependency_graph "${ep_dependency_graph}\n${ep}:${${ep}_DEPENDENCIES}")
endforeach()
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/ExternalProjectDependencies.txt "${ep_dependency_graph}\n")

#-----------------------------------------------------------------------------
# Configure and build Slicer
# Note that the Slicer3Version.txt file will be created at configuration time
#------------------------------------------------------------------------------

set(proj Slicer3)
ExternalProject_Add(${proj}
  DEPENDS ${slicer_DEPENDENCIES}
  DOWNLOAD_COMMAND ""
  SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..
  BINARY_DIR Slicer3-build
  CMAKE_GENERATOR ${gen}
  CMAKE_ARGS
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS}
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
    -DBUILD_EXAMPLES:BOOL=OFF
    -DBUILD_TESTING:BOOL=ON
    # ITK
    -DITK_DIR:PATH=${CMAKE_BINARY_DIR}/Insight-build
    # Teem
    -DTeem_DIR:PATH=${CMAKE_BINARY_DIR}/teem-build
    # OpenIGTLink
    -DSlicer3_USE_OPENIGTLINK:BOOL=ON
    -DOpenIGTLink_DIR:PATH=${CMAKE_BINARY_DIR}/OpenIGTLink-build
    # BatchMake
    -DSlicer3_USE_BatchMake=ON
    -DBatchMake_DIR:PATH=${CMAKE_BINARY_DIR}/BatchMake-build
    # VTK
    -DVTK_DIR:PATH=${CMAKE_BINARY_DIR}/VTK-build
    -DVTK_DEBUG_LEAKS:BOOL=${Slicer3_USE_VTK_DEBUG_LEAKS}
    # TCL/Tk
    -DINCR_TCL_LIBRARY:FILEPATH=${slicer_TCL_LIB}
    -DINCR_TK_LIBRARY:FILEPATH=${slicer_TK_LIB}
    -DSlicer3_TCL_DIR:PATH=${tcl_build}
    # cmcurl
    -DSLICERLIBCURL_DIR:PATH=${CMAKE_BINARY_DIR}/cmcurl-build
    # Python
    -DSlicer3_USE_PYTHON:BOOL=${Slicer3_USE_PYTHON}
    -DSlicer3_USE_SYSTEM_PYTHON:BOOL=OFF
    -DSlicer3_USE_NUMPY:BOOL=${Slicer3_BUILD_NUMPY}
    -DPYTHON_EXECUTABLE:FILEPATH=${slicer_PYTHON_EXECUTABLE}
    -DPYTHON_INCLUDE_DIR:PATH=${slicer_PYTHON_INCLUDE}
    -DPYTHON_LIBRARY:FILEPATH=${slicer_PYTHON_LIBRARY}
    # Qt
    -DSlicer3_USE_QT:BOOL=${Slicer3_USE_QT}
    -DQT_QMAKE_EXECUTABLE:PATH=${QT_QMAKE_EXECUTABLE}
    -DCTK_DIR:PATH=${CMAKE_BINARY_DIR}/CTK-build
    # KWWidgets
    -DSlicer3_USE_KWWIDGETS:BOOL=${Slicer3_USE_KWWIDGETS}
    -DKWWidgets_DIR:PATH=${CMAKE_BINARY_DIR}/KWWidgets-build
  INSTALL_COMMAND ""
)
