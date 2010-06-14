
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
# Platform check
#-----------------------------------------------------------------------------

set(PLATFORM_CHECK true)

if(PLATFORM_CHECK)
  # See CMake/Modules/Platform/Darwin.cmake)
  #   6.x == Mac OSX 10.2 (Jaguar)
  #   7.x == Mac OSX 10.3 (Panther)
  #   8.x == Mac OSX 10.4 (Tiger)
  #   9.x == Mac OSX 10.5 (Leopard)
  #  10.x == Mac OSX 10.6 (Snow Leopard)
  if (DARWIN_MAJOR_VERSION LESS "8")
    message(FATAL_ERROR "Only Mac OSX >= 10.4 are supported !")
  endif()
endif()

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

FUNCTION(downloadFile url dest)
  FILE(DOWNLOAD ${url} ${dest} STATUS status)
  LIST(GET status 0 error_code)
  LIST(GET status 1 error_msg)
  IF(error_code OR NOT EXISTS ${dest})
    MESSAGE(FATAL_ERROR "error: Failed to download ${url}")
  ENDIF()
ENDFUNCTION()

#-----------------------------------------------------------------------------
# Prerequisites
#------------------------------------------------------------------------------

set(git_repo http://github.com/commontk/CTK/raw/master/)

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
endif()

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
  set(minimum_required_qt_version "4.6.2")
  # For Mac OS X 10.6, Apple has started recommending developers to build their applications 64-bit.
  # The main reason is that there is a small speed increase due to the extra registers on Intel 
  #CPU's, all their machine offerings have been 64-bit since 2007, and there is a cost for reading 
  # all the 32-bit libraries into memory if everything else is 64-bit. If you want to follow this
  # advice, there is only one choice, 64-bit Cocoa.
  #
  # See CMake/Modules/Platform/Darwin.cmake)
  #   6.x == Mac OSX 10.2 (Jaguar)
  #   7.x == Mac OSX 10.3 (Panther)
  #   8.x == Mac OSX 10.4 (Tiger)
  #   9.x == Mac OSX 10.5 (Leopard)
  #  10.x == Mac OSX 10.6 (Snow Leopard)

  if(DARWIN_MAJOR_VERSION GREATER "10")
    set(minimum_required_qt_version "4.7.0")
  endif()
  
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

if(Slicer3_USE_PYTHON OR Slicer3_USE_PYTHONQT)

  option(Slicer3_BUILD_NUMPY "Build Slicer with NumPy (builds CLapack)." ON)

  # Work in progress ...
  #if(Slicer3_BUILD_NUMPY)
  #  option(Slicer3_BUILD_SCIPY "Build Slicer with SciPy (requires NumPy)" ON)
  #endif()
  
endif()

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
# Configure the CTest custom file - Used to suppress external project configure/build warnings
#------------------------------------------------------------------------------  

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
set(CLAPACK_DEPENDENCIES)
set(NUMPY_DEPENDENCIES CLAPACK python)
set(scipy_DEPENDENCIES NUMPY)

set(VTK_DEPENDENCIES)
set(CTK_DEPENDENCIES VTK)
if(Slicer3_USE_PYTHONQT)
  list(APPEND CTK_DEPENDENCIES python)
endif()
set(KWWidgets_DEPENDENCIES VTK)
set(Insight_DEPENDENCIES)
set(BatchMake_DEPENDENCIES Insight)
set(OpenIGTLink_DEPENDENCIES)
set(teem_DEPENDENCIES VTK)
set(cmcurl_DEPENDENCIES)
set(slicer_DEPENDENCIES VTK Insight BatchMake OpenIGTLink teem cmcurl)

#------------------------------------------------------------------------------
# Conditionnaly include ExternalProject Target
#------------------------------------------------------------------------------

if(Slicer3_USE_KWWIDGETS)
  include(External_Tcl)
  include(External_Tk)
  include(External_incrTcl)
  include(External_iwidgets)
  include(External_blt)
endif()

if(Slicer3_USE_PYTHON OR Slicer3_USE_PYTHONQT)
  include(External_Python26)
  if(Slicer3_BUILD_NUMPY)
    include(External_CLAPACK)
    include(External_NUMPY)
    if(Slicer3_BUILD_SCIPY)
      include(External_SciPy)
    endif(Slicer3_BUILD_SCIPY)
  endif()
endif()

include(External_VTK)

if(Slicer3_USE_QT)
  include(External_CTK)
endif()

if(Slicer3_USE_KWWIDGETS)
  include(External_KWWidgets)
endif()

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
    list(APPEND slicer_DEPENDENCIES tcl tk blt iwidgets itcl KWWidgets)
  endif()
endif()

if(Slicer3_USE_QT)
  list(APPEND slicer_DEPENDENCIES CTK)
endif()

if(Slicer3_USE_PYTHON OR Slicer3_USE_PYTHONQT)
  list(APPEND slicer_DEPENDENCIES python)
  if(Slicer3_BUILD_NUMPY)
    list(APPEND slicer_DEPENDENCIES NUMPY)
    if(Slicer3_BUILD_SCIPY)
      list(APPEND slicer_DEPENDENCIES scipy)
    endif()
  endif()
endif()

#------------------------------------------------------------------------------
# List of external projects
#------------------------------------------------------------------------------  

set(external_project_list tk tcl incrTcl iwidgets blt python CLAPACK NUMPY scipy VTK CTK KWWidgets Insight BatchMake OpenIGTLink teem cmcurl slicer)

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
    -DITK_DIR:PATH=${ITK_DIR}
    # Teem
    -DTeem_DIR:PATH=${CMAKE_BINARY_DIR}/teem-build
    # OpenIGTLink
    -DSlicer3_USE_OPENIGTLINK:BOOL=ON
    -DOpenIGTLink_DIR:PATH=${CMAKE_BINARY_DIR}/OpenIGTLink-build
    # BatchMake
    -DSlicer3_USE_BatchMake=ON
    -DBatchMake_DIR:PATH=${CMAKE_BINARY_DIR}/BatchMake-build
    # VTK
    -DVTK_DIR:PATH=${VTK_DIR}
    -DVTK_DEBUG_LEAKS:BOOL=${Slicer3_USE_VTK_DEBUG_LEAKS}
    # TCL/Tk
    -DINCR_TCL_LIBRARY:FILEPATH=${slicer_TCL_LIB}
    -DINCR_TK_LIBRARY:FILEPATH=${slicer_TK_LIB}
    -DSlicer3_TCL_DIR:PATH=${tcl_build}
    # cmcurl
    -DSLICERLIBCURL_DIR:PATH=${CMAKE_BINARY_DIR}/cmcurl-build
    # Python
    -DSlicer3_USE_PYTHON:BOOL=${Slicer3_USE_PYTHON}
    -DSlicer3_USE_PYTHONQT:BOOL=${Slicer3_USE_PYTHONQT}
    -DSlicer3_USE_SYSTEM_PYTHON:BOOL=OFF
    -DSlicer3_USE_NUMPY:BOOL=${Slicer3_BUILD_NUMPY}
    -DPYTHON_EXECUTABLE:FILEPATH=${slicer_PYTHON_EXECUTABLE}
    -DPYTHON_INCLUDE_DIR:PATH=${slicer_PYTHON_INCLUDE}
    -DPYTHON_LIBRARY:FILEPATH=${slicer_PYTHON_LIBRARY}
    # Qt
    -DSlicer3_USE_QT:BOOL=${Slicer3_USE_QT}
    -DQT_QMAKE_EXECUTABLE:PATH=${QT_QMAKE_EXECUTABLE}
    -DCTK_DIR:PATH=${CTK_DIR}
    # KWWidgets
    -DSlicer3_USE_KWWIDGETS:BOOL=${Slicer3_USE_KWWIDGETS}
    -DKWWidgets_DIR:PATH=${CMAKE_BINARY_DIR}/KWWidgets-build
  INSTALL_COMMAND ""
)
