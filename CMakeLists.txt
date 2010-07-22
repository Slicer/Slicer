PROJECT(Slicer3)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
  CMAKE_POLICY(SET CMP0012 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)


#-----------------------------------------------------------------------------
#
set(Slicer3_INSTALL_BIN_DIR "bin")
set(Slicer3_INSTALL_LIB_DIR "lib/Slicer3")
set(Slicer3_INSTALL_INCLUDE_DIR "include/Slicer3")
set(Slicer3_INSTALL_SHARE_DIR "share/Slicer3")
set(Slicer3_INSTALL_ITKFACTORIES_DIR "${Slicer3_INSTALL_LIB_DIR}/ITKFactories")

set(Slicer3_INSTALL_PLUGINS_BIN_DIR "${Slicer3_INSTALL_LIB_DIR}/Plugins")
set(Slicer3_INSTALL_PLUGINS_LIB_DIR "${Slicer3_INSTALL_LIB_DIR}/Plugins")
set(Slicer3_INSTALL_PLUGINS_INCLUDE_DIR "${Slicer3_INSTALL_INCLUDE_DIR}/Plugins")
set(Slicer3_INSTALL_PLUGINS_SHARE_DIR "${Slicer3_INSTALL_SHARE_DIR}/Plugins")
set(Slicer3_INSTALL_PLUGINS_CACHE_DIR "${Slicer3_INSTALL_LIB_DIR}/PluginsCache")

set(Slicer3_INSTALL_MODULES_BIN_DIR "${Slicer3_INSTALL_LIB_DIR}/Modules")
set(Slicer3_INSTALL_MODULES_LIB_DIR "${Slicer3_INSTALL_LIB_DIR}/Modules")
set(Slicer3_INSTALL_MODULES_INCLUDE_DIR "${Slicer3_INSTALL_INCLUDE_DIR}/Modules")
set(Slicer3_INSTALL_MODULES_SHARE_DIR "${Slicer3_INSTALL_SHARE_DIR}/Modules")

set(Slicer3_HOME "${Slicer3_BINARY_DIR}")

#-----------------------------------------------------------------------------
# Convenient macro allowing to download a file
#
MACRO(downloadFile url dest)
  IF(NOT EXISTS ${dest})
    FILE(DOWNLOAD ${url} ${dest} STATUS status)
    LIST(GET status 0 error_code)
    LIST(GET status 1 error_msg)
    IF(error_code)
      MESSAGE(FATAL_ERROR "error: Failed to download ${url} - ${error_msg}")
    ENDIF()
  ELSE()
    # Make sure the file is not empty
    FILE(READ ${dest} dest_content LIMIT 4)
    STRING(LENGTH "${dest_content}" dest_length)
    IF(NOT "${dest_length}" EQUAL 4)
      FILE(REMOVE ${dest})
      downloadFile(${url} ${dest})
    ENDIF()
  ENDIF()
ENDMACRO()

#-----------------------------------------------------------------------------
# Disable the warnings that DevStudio 2005 emits wrt to sprintf, strcpu, etc.
#
# On Visual Studio 8 MS deprecated C. This removes all 1.276E1265 security
# warnings (copied from ITK's CMake files so that ITK templated code will compile
# correctly).
IF(WIN32)
  IF(NOT BORLAND)
    IF(NOT CYGWIN)
      IF(NOT MINGW)
        ADD_DEFINITIONS(
          -D_CRT_FAR_MAPPINGS_NO_DEPRECATE
          -D_CRT_IS_WCTYPE_NO_DEPRECATE
          -D_CRT_MANAGED_FP_NO_DEPRECATE
          -D_CRT_NONSTDC_NO_DEPRECATE
          -D_CRT_SECURE_NO_DEPRECATE
          -D_CRT_SECURE_NO_DEPRECATE_GLOBALS
          -D_CRT_SETERRORMODE_BEEP_SLEEP_NO_DEPRECATE
          -D_CRT_TIME_FUNCTIONS_NO_DEPRECATE
          -D_CRT_VCCLRIT_NO_DEPRECATE
          -D_SCL_SECURE_NO_DEPRECATE
          )
      ENDIF(NOT MINGW)
    ENDIF(NOT CYGWIN)
  ENDIF(NOT BORLAND)
ENDIF(WIN32)

# Disable warning C4996: [...] This function or variable may be unsafe. [...]
# To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details
if(MSVC90)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(MSVC90)

# include a test for endianness (used to be done by default)

include(${CMAKE_ROOT}/Modules/TestBigEndian.cmake)
test_big_endian(CMAKE_WORDS_BIGENDIAN)

#-----------------------------------------------------------------------------
# avoid linker bug in Mac OS 10.5
# see:
# http://wiki.finkproject.org/index.php/Fink:Packaging:Preparing_for_10.5#OpenGL_Bug
#
if(APPLE)
  set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
  set(CMAKE_EXE_LINKER_FLAGS "-Wl,-dylib_file,/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib:/System/Library/Frameworks/OpenGL.framework/Versions/A/Libraries/libGL.dylib")
endif(APPLE)

#-----------------------------------------------------------------------------
# Slicer version number.
#
set(Slicer3_VERSION_MAJOR "3")
set(Slicer3_VERSION_MINOR "7")
if(NOT Slicer3_VERSION_PATCH)
  set(Slicer3_VERSION_PATCH "0")
endif(NOT Slicer3_VERSION_PATCH)
set(Slicer3_VERSION "${Slicer3_VERSION_MAJOR}.${Slicer3_VERSION_MINOR}")
set(Slicer3_VERSION_FULL "${Slicer3_VERSION}.${Slicer3_VERSION_PATCH}")

#-----------------------------------------------------------------------------
# Build shared lib by default
#
option(BUILD_SHARED_LIBS "Build Slicer3 with shared libraries." ON)

#-----------------------------------------------------------------------------
# Append the library version information to the library target properties.
#
OPTION(Slicer3_WITH_LIBRARY_VERSION "Build with library version information" OFF)
IF(Slicer3_WITH_LIBRARY_VERSION)
  SET(Slicer3_LIBRARY_PROPERTIES ${Slicer3_LIBRARY_PROPERTIES}
    VERSION ${Slicer3_VERSION_FULL}
    SOVERSION ${Slicer3_VERSION}
  )
ENDIF(Slicer3_WITH_LIBRARY_VERSION)

#-----------------------------------------------------------------------------
# Slicer3 macros
#
set(Slicer3_CMAKE_DIR "${Slicer3_SOURCE_DIR}/CMake" CACHE INTERNAL "")
add_subdirectory(CMake)
# Make sure VTK_CMAKE_DIR is at the beginning so that VTK's FindTCL is
# used before CMake's FindTCL.
set(CMAKE_MODULE_PATH
  "${VTK_CMAKE_DIR}"
  "${Slicer3_CMAKE_DIR}"
  ${Slicer3_BINARY_DIR}/CMake
  ${CMAKE_MODULE_PATH}
  )

include("${Slicer3_CMAKE_DIR}/Slicer3PersistenceMacros.cmake")

# See http://www.cmake.org/Wiki/CMakeMacroParseArguments
include("${Slicer3_CMAKE_DIR}/Slicer3ParseArgumentsMacro.cmake")

## QT
INCLUDE(${Slicer3_CMAKE_DIR}/Slicer3FindQT.cmake)

#-----------------------------------------------------------------------------
# Top level slicer3 configurations
#
# If CMake >= 2.6.0
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")


# Do NOT force into the cache, as plugins need to override locally and scoping
# becomes an issue.
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/ 
#  CACHE PATH "Single output directory for building all libraries." FORCE)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/ 
#  CACHE PATH "Single output directory for building all executables." FORCE)

#-----------------------------------------------------------------------------
# Slicer include and libraries subdirectory
#
slicer3_set_persistent_property(Slicer3_Libs_LIBRARIES "")
slicer3_set_persistent_property(Slicer3_Base_LIBRARIES "")
slicer3_set_persistent_property(Slicer3_Modules_LIBRARIES "")
slicer3_set_persistent_property(Slicer3_QTModules_LIBRARIES "")

slicer3_set_persistent_property(Slicer3_Libs_INCLUDE_DIRS "")
slicer3_set_persistent_property(Slicer3_Base_INCLUDE_DIRS "")
slicer3_set_persistent_property(Slicer3_Modules_INCLUDE_DIRS "")
slicer3_set_persistent_property(Slicer3_QTModules_INCLUDE_DIRS "")

#-----------------------------------------------------------------------------
# Get sytem name and architecture
#
SET(Slicer3_BUILD NA)
INCLUDE(${Slicer3_CMAKE_DIR}/DiscoverSystemNameAndBits.cmake)
MESSAGE(STATUS "Slicer3_BUILD:${Slicer3_BUILD}")

#-----------------------------------------------------------------------------
# Add the testing directories
#
OPTION(BUILD_TESTING "Test the project" ON)
SET(Slicer3_BUILD_TESTING ${BUILD_TESTING})
SET(LAUNCH_EXE ${Slicer3_BINARY_DIR}/Slicer3 --launch)
IF(Slicer3_BUILD_TESTING)
  #IF (NOT BUILDNAME)
  #  SET(BUILDNAME "${Slicer3_BUILD}" CACHE STRING "Name of build on the dashboard")
  #ELSE()
  #  SET(BUILDNAME "${BUILDNAME}" CACHE STRING "Name of build on the dashboard")
  #ENDIF()
  ENABLE_TESTING()
  INCLUDE(CTest)
  SET(CXX_TEST_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  MARK_AS_ADVANCED(TCL_TCLSH DART_ROOT)

#-----------------------------------------------------------------------------
# Run test through valgrind: particularly useful for testing modules.
#
option(USE_VALGRIND "Run tests through valgrind." OFF)

ENDIF(Slicer3_BUILD_TESTING)

#-----------------------------------------------------------------------------
# By default, build with KWWidget
#
option(Slicer3_USE_KWWIDGETS "Build Slicer3 with KWWidgets." ON)

#-----------------------------------------------------------------------------
# Since modules are KWWidgets based, unset there variables
IF (NOT Slicer3_USE_KWWIDGETS)
  # Since the module Logic is shared between QTModules and regular Modules,
  # the associated BIN and LIB directory shouldn't be unset
  #UNSET(Slicer3_INSTALL_MODULES_BIN_DIR)
  #UNSET(Slicer3_INSTALL_MODULES_LIB_DIR)
  UNSET(Slicer3_INSTALL_MODULES_INCLUDE_DIR)
  UNSET(Slicer3_INSTALL_MODULES_SHARE_DIR)
ENDIF (NOT Slicer3_USE_KWWIDGETS)

#-----------------------------------------------------------------------------
# Libs
# - ITK
# - KWWidgets (pulls in VTK)
#

# Relax some constraints related to the build settings file. This was
# preventing people from using the same compiler but with a different generator,
# or just the same compiler installed in a non-standard location.

IF (Slicer3_USE_KWWIDGETS)
  set(SKIP_KWWidgets_BUILD_SETTINGS_FILE 1)
ENDIF(Slicer3_USE_KWWIDGETS)
set(SKIP_VTK_BUILD_SETTINGS_FILE 1)
set(SKIP_ITK_BUILD_SETTINGS_FILE 1)
set(SKIP_Teem_BUILD_SETTINGS_FILE 1)

#-----------------------------------------------------------------------------
## Python
option(Slicer3_USE_PYTHON "Integrate a python interpreter into Slicer." OFF)
if(Slicer3_USE_PYTHON)
  find_package(PythonLibs)
  find_package(PythonInterp)
endif(Slicer3_USE_PYTHON)

#-----------------------------------------------------------------------------
## ITK
find_package(ITK REQUIRED)
if(NOT ITK_FOUND)
  message(FATAL_ERROR "Need ITK to build")
endif(NOT ITK_FOUND)
include(${ITK_USE_FILE})

#-----------------------------------------------------------------------------
# FindCTK
#
SET(git_repo http://github.com/commontk/CTK/raw/master/)

if (Slicer3_USE_QT)
  SET(file Utilities/CMake/FindCTK.cmake)
  downloadFile(${git_repo}/${file} ${Slicer3_BINARY_DIR}/CMake/FindCTK.cmake)

  FIND_PACKAGE(CTK REQUIRED)
  IF(CTK_FOUND)
    INCLUDE(${CTK_USE_FILE})
  ENDIF()
ENDIF()

#-----------------------------------------------------------------------------
IF (Slicer3_USE_KWWIDGETS)
  ## KWWidgets
  FIND_PACKAGE(KWWidgets REQUIRED)
  IF(NOT KWWidgets_FOUND)
    MESSAGE(FATAL_ERROR "Need KWWidgets to build")
  ENDIF(NOT KWWidgets_FOUND)
  INCLUDE(${KWWidgets_USE_FILE})
  
ELSE(Slicer3_USE_KWWIDGETS)

  # VTK
  FIND_PACKAGE(VTK REQUIRED)
  IF(NOT VTK_FOUND)
    MESSAGE(FATAL_ERROR "Need VTK to build")
  ELSE(NOT VTK_FOUND)
    IF(NOT VTK_USE_QVTK)
      MESSAGE("Please make sure VTK is build with VTK_USE_GUISUPPORT and VTK_USE_QVTK enabled")
    ENDIF(NOT VTK_USE_QVTK)
  ENDIF(NOT VTK_FOUND)
  INCLUDE(${VTK_USE_FILE})
  
ENDIF (Slicer3_USE_KWWIDGETS)

#-----------------------------------------------------------------------------
# Slicer3 is built with cvs head
#
option(Slicer3_USE_VTK_CVSHEAD "Slicer3 is configurated with cvs head." OFF)
mark_as_advanced(Slicer3_USE_VTK_CVSHEAD)

#-----------------------------------------------------------------------------
#
# Call macro defined in ${PROJECT_SOURCE_DIR}/CMake/Slicer3FindQT.cmake
#
SLICER3_SETUP_QT()

#-----------------------------------------------------------------------------
# Alert the user if both Slicer3_USE_QT and Slicer3_USE_KWWIDGETS are OFF
IF(NOT Slicer3_USE_QT AND NOT Slicer3_USE_KWWIDGETS)
  MESSAGE(FATAL_ERROR "Both Slicer3_USE_QT and Slicer3_USE_KWWIDGETS are OFF. Please, choose one")
ENDIF(NOT Slicer3_USE_QT AND NOT Slicer3_USE_KWWIDGETS)

#-----------------------------------------------------------------------------
# If Slicer3_USE_QT is ON and Slicer3_USE_KWWIDGETS is OFF, complain if VTK_WRAP_TCL is ON
# Note: Later on we will expect VTK_WRAP_PYTHON to be ON
IF(Slicer3_USE_QT AND NOT Slicer3_USE_KWWIDGETS)
  IF(VTK_WRAP_TCL)
    MESSAGE(FATAL_ERROR "For now, if Slicer3_USE_QT is ON and Slicer3_USE_KWWIDGETS is OFF, we expect VTK_WRAP_TCL to be OFF")
  ENDIF(VTK_WRAP_TCL)
ENDIF(Slicer3_USE_QT AND NOT Slicer3_USE_KWWIDGETS)

#-----------------------------------------------------------------------------
# Add Slicer3_USE_PYTHONQT option if Slicer3_USE_QT is ON
IF(Slicer3_USE_QT)
  option(Slicer3_USE_PYTHONQT "Integrate a python-QT interpreter into Slicer." OFF)
  IF(Slicer3_USE_PYTHONQT)
    IF(NOT VTK_WRAP_PYTHON)
      MESSAGE(FATAL_ERROR "Since both Slicer3_USE_QT and Slicer3_USE_PYTHONQT are ON, VTK_WRAP_PYTHON is expected to be ON. Disable Slicer3_USE_PYTHONQT or re-configure VTK with python wrapping.")
    ENDIF(NOT VTK_WRAP_PYTHON)
    # Enable qMRMLWidgets python wrapping
    SET(qMRMLWidgets_WRAP_PYTHON ON)
  ENDIF(Slicer3_USE_PYTHONQT)
ENDIF(Slicer3_USE_QT)  

#-----------------------------------------------------------------------------
## Teem
find_package(Teem REQUIRED)
if(NOT Teem_FOUND)
  message(FATAL_ERROR "Need Teem to build")
endif(NOT Teem_FOUND)
include(${Teem_USE_FILE})

#-----------------------------------------------------------------------------
## SlicerLibCurl
find_package(SLICERLIBCURL REQUIRED)
if(SLICERLIBCURL_FOUND)
  include(${SLICERLIBCURL_USE_FILE})
endif(SLICERLIBCURL_FOUND)

#-----------------------------------------------------------------------------
## BatchMake
option(Slicer3_USE_BatchMake "Build Slicer3 with BatchMake support." OFF)
if(Slicer3_USE_BatchMake)
  find_package(BatchMake REQUIRED)
  include(${BatchMake_USE_FILE})
endif(Slicer3_USE_BatchMake)

#-----------------------------------------------------------------------------
## MIDAS
option(Slicer3_USE_MIDAS "Build Slicer3 with MIDAS support." OFF)

#-----------------------------------------------------------------------------
## OpenIGTLink (Built in Slicer3-lib)
option(Slicer3_USE_OPENIGTLINK "Build Slicer3 with OpenIGTLink support." OFF)
if(Slicer3_USE_OPENIGTLINK)
  find_package(OpenIGTLink REQUIRED)
  include(${OpenIGTLink_USE_FILE})
endif(Slicer3_USE_OPENIGTLINK)

#-----------------------------------------------------------------------------
## OpenCV (Built in Slicer3-lib)
option(Slicer3_USE_OPENCV "Build Slicer3 with OpenCV support." OFF)
if(Slicer3_USE_OPENCV)
  find_package(OpenCV REQUIRED)

  IF(OPENCV_BUILD_SETTINGS_FILE AND NOT SKIP_OPENCV_BUILD_SETTINGS_FILE)
    INCLUDE(${CMAKE_ROOT}/Modules/CMakeImportBuildSettings.cmake)
    CMAKE_IMPORT_BUILD_SETTINGS(${OPENCV_BUILD_SETTINGS_FILE})
  ENDIF(OPENCV_BUILD_SETTINGS_FILE AND NOT SKIP_OPENCV_BUILD_SETTINGS_FILE)
  
  # Add compiler flags needed to use OPENCV.
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OPENCV_REQUIRED_C_FLAGS}")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPENCV_REQUIRED_CXX_FLAGS}")
  SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OPENCV_REQUIRED_LINK_FLAGS}")
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${OPENCV_REQUIRED_LINK_FLAGS}")
  SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${OPENCV_REQUIRED_LINK_FLAGS}")
  
  # Add include directories needed to use OPENCV.
  INCLUDE_DIRECTORIES(BEFORE ${OPENCV_INCLUDE_DIRS})
  
  # Add link directories needed to use OPENCV.
  LINK_DIRECTORIES(${OPENCV_LIBRARY_DIRS})
  #include(${OPENCV_USE_FILE})
endif(Slicer3_USE_OPENCV)


#-----------------------------------------------------------------------------
# Need some Tcl/Tk info
IF (VTK_WRAP_TCL)
  include("${VTK_CMAKE_DIR}/vtkTclTkMacros.cmake")
  VTK_GET_TCL_TK_VERSION(TCL_TK_MAJOR_VERSION TCL_TK_MINOR_VERSION)
  set(TCL_TK_VERSION_DOT "${TCL_TK_MAJOR_VERSION}.${TCL_TK_MINOR_VERSION}")
  set(TCL_TK_VERSION "${TCL_TK_MAJOR_VERSION}${TCL_TK_MINOR_VERSION}")
  IF(Slicer3_USE_KWWIDGETS)
    include("${KWWidgets_CMAKE_DIR}/KWWidgetsTclTkMacros.cmake")
    incr_tcl_get_version(INCR_TCL_MAJOR_VERSION INCR_TCL_MINOR_VERSION)
    set(INCR_TCL_VERSION_DOT "${INCR_TCL_MAJOR_VERSION}.${INCR_TCL_MINOR_VERSION}")
    set(INCR_TCL_VERSION "${INCR_TCL_MAJOR_VERSION}${INCR_TCL_MINOR_VERSION}")
  ENDIF(Slicer3_USE_KWWIDGETS)
ENDIF (VTK_WRAP_TCL)

#-----------------------------------------------------------------------------
# Generate the default setup paths scripts.
# Generate some useful scripts that can be used to setup various environment 
# variables and launch the executable properly against external packages.
#
IF (Slicer3_USE_KWWIDGETS)
  include("${KWWidgets_CMAKE_DIR}/KWWidgetsPathsMacros.cmake")
  set(extra_dirs
    "${Teem_DIR}/bin"
    "${BatchMake_DIR}/bin"
    "${OpenIGTLink_DIR}/bin"
    "${Slicer3_HOME}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}"
    "${Slicer3_HOME}/${Slicer3_INSTALL_PLUGINS_LIB_DIR}"
    "${Slicer3_HOME}/${Slicer3_INSTALL_MODULES_BIN_DIR}"
    "${Slicer3_HOME}/${Slicer3_INSTALL_MODULES_LIB_DIR}"
    )
  kwwidgets_generate_setup_paths_scripts(
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" "Slicer3SetupPaths"
    "${extra_dirs}"
  )
ENDIF (Slicer3_USE_KWWIDGETS)

#-----------------------------------------------------------------------------
# VTK 5.0 does not have VTK_CMAKE_DIR
# Alternatively you could use KWWidget's wrapping, which encapsulate
# the differences between various VTK versions:
#  INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsWrappingMacros.cmake")
#  KWWidgets_WRAP_TCL(${LIB_NAME} LIB_TCL_SRCS "${LIB_SRCS}" "")
#
IF (Slicer3_USE_KWWIDGETS)
  if(NOT VTK_CMAKE_DIR)
    set(VTK_CMAKE_DIR "${VTK_SOURCE_DIR}/CMake")
  endif(NOT VTK_CMAKE_DIR)
  include("${VTK_CMAKE_DIR}/vtkWrapTcl.cmake")
ENDIF (Slicer3_USE_KWWIDGETS)

#-----------------------------------------------------------------------------
# Check versions of VTK to track CVS HEAD
#
IF (Slicer3_USE_KWWIDGETS)
  find_package(VTK REQUIRED)
  if(EXISTS ${VTK_CMAKE_DIR}/VTKVersionMacros.cmake)
    include(${VTK_CMAKE_DIR}/VTKVersionMacros.cmake)
  else(EXISTS ${VTK_CMAKE_DIR}/VTKVersionMacros.cmake)
    #MESSAGE("Using local version of VTKVersionMacros for compatibility.")
    include(${Slicer3_SOURCE_DIR}/Scripts/VTKVersionMacros.cmake)
  endif(EXISTS ${VTK_CMAKE_DIR}/VTKVersionMacros.cmake)

  vtk_get_source_revision_and_date(VTKsource_revision VTKsource_date)
  if(VTKsource_revision LESS 1.2196)
    message(FATAL_ERROR "Sorry, your VTK library was last updated on ${VTKsource_date}. Its source revision, according to vtkKWWidgetsVersion.h, is ${VTKsource_revision}. Please update to a newer revision.")
  endif(VTKsource_revision LESS 1.2196)
ENDIF (Slicer3_USE_KWWIDGETS)

#-----------------------------------------------------------------------------
# These packages may exist within the Slicer3 source tree
#
set(TCLAP_DIR ${Slicer3_BINARY_DIR}/Libs/SlicerExecutionModel/tclap)
set(ModuleDescriptionParser_DIR ${Slicer3_BINARY_DIR}/Libs/SlicerExecutionModel/ModuleDescriptionParser)
set(GenerateCLP_DIR ${Slicer3_BINARY_DIR}/Libs/SlicerExecutionModel/GenerateCLP)
set(GenerateLM_DIR ${Slicer3_BINARY_DIR}/Libs/GenerateLM)
set(LoadableModule_DIR ${Slicer3_BINARY_DIR}/Libs/LoadableModule)
set(vtkITK_DIR ${Slicer3_BINARY_DIR}/Libs/vtkITK)

#-----------------------------------------------------------------------------
# Qt Loadable Modules
#
IF(Slicer3_USE_QT)
  OPTION(Slicer3_BUILD_QTLOADABLEMODULES "Build Slicer3 QT Loadable Modules" ON)
  IF(Slicer3_BUILD_QTLOADABLEMODULES)
    ADD_DEFINITIONS(-DSlicer3_BUILD_QTLOADABLEMODULES)
    set(Slicer3_INSTALL_QTLOADABLEMODULES_BIN_DIR "${Slicer3_INSTALL_LIB_DIR}/QTLoadableModules")
    set(Slicer3_INSTALL_QTLOADABLEMODULES_LIB_DIR "${Slicer3_INSTALL_LIB_DIR}/QTLoadableModules")
    set(Slicer3_INSTALL_QTLOADABLEMODULES_INCLUDE_DIR "${Slicer3_INSTALL_INCLUDE_DIR}/QTLoadableModules")
    set(Slicer3_INSTALL_QTLOADABLEMODULES_SHARE_DIR "${Slicer3_INSTALL_SHARE_DIR}/QTLoadableModules")
  ENDIF(Slicer3_BUILD_QTLOADABLEMODULES)
ENDIF(Slicer3_USE_QT)

# --------------------------------------------------------------------------
# Configure and install headers

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerConfigure.h
  )

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerVersionConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerVersionConfigure.h
  )

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/runValgrind.sh.in 
  ${CMAKE_CURRENT_BINARY_DIR}/Scripts/runValgrind.sh
  )

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/runValgrindForOneModule.sh.in 
  ${CMAKE_CURRENT_BINARY_DIR}/Scripts/runValgrindForOneModule.sh
  )

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/computeCodeCoverageLocally.sh.in 
  ${CMAKE_CURRENT_BINARY_DIR}/Scripts/computeCodeCoverageLocally.sh
  )

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/computeCodeCoverageForOneTest.sh.in 
  ${CMAKE_CURRENT_BINARY_DIR}/Scripts/computeCodeCoverageForOneTest.sh
  )

CONFIGURE_FILE(
  ${CMAKE_CURRENT_SOURCE_DIR}/Scripts/runExperimentalOffScreen.sh.in 
  ${CMAKE_CURRENT_BINARY_DIR}/Scripts/runExperimentalOffScreen.sh
  )

INCLUDE_DIRECTORIES(
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  )

INSTALL(FILES 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerConfigure.h"
  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR} COMPONENT Development
  )

INSTALL(FILES 
  "${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerVersionConfigure.h"
  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR} COMPONENT Development
  )

#-----------------------------------------------------------------------------
# Generate Slicer3Version.txt

INCLUDE(${Slicer3_CMAKE_DIR}/Slicer3TodayMacro.cmake)

# Get build time
TODAY(Slicer3_BUILDDATE)

# Note that Slicer_BUILD is set above

SET(Slicer3_WC_URL NA)
SET(Slicer3_WC_REVISION 0)

# Get SVN info
FIND_PACKAGE(Subversion)
IF(NOT Subversion_FOUND)
  MESSAGE(FATAL_ERROR "error: Install SVN and try to re-configure")
ENDIF(NOT Subversion_FOUND)

# Case where Slicer is checkout using SVN
IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.svn)
  Subversion_WC_INFO(${CMAKE_CURRENT_SOURCE_DIR} Slicer3)
  CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/Slicer3Version.txt.in 
    ${Slicer3_HOME}/lib/Slicer3/Slicer3Version.txt
    )
ENDIF()

# Case where Slicer is checkout using Git-SVN
IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git)
  FIND_PACKAGE(Git)
  IF(NOT Git_FOUND)
    MESSAGE(FATAL_ERROR "error: Failed to locate Git")
  ENDIF(NOT Git_FOUND)
  Git_WC_INFO(${CMAKE_CURRENT_SOURCE_DIR} Slicer3)
  CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/Slicer3Version.txt.in
    ${Slicer3_HOME}/lib/Slicer3/Slicer3Version.txt
    )
ENDIF()


#-----------------------------------------------------------------------------
# Subdirectories
#
add_subdirectory(Libs)
add_subdirectory(Base)
add_subdirectory(Resources)

#-----------------------------------------------------------------------------
# At this point, let's not have the applications and modules guess which
# include paths or libraries they need, but use Slicer3_*_LIBRARIES and
# Slicer3_*_INCLUDE_DIRS instead. As far as modules are concerned, both
# variables are actually set automatically by the import script when building
# against a Slicer3 build or installed tree. When in-source, we need to
# set both variables now. Retrieve them from the persistent property cache
# since they were collected in each subdir (which makes it impossible to
# use normal variables, since a new scope is created in each subdir).
#
slicer3_get_persistent_property(Slicer3_Libs_LIBRARIES Slicer3_Libs_LIBRARIES)
slicer3_get_persistent_property(Slicer3_Base_LIBRARIES Slicer3_Base_LIBRARIES)

slicer3_get_persistent_property(Slicer3_Libs_INCLUDE_DIRS Slicer3_Libs_INCLUDE_DIRS)
slicer3_get_persistent_property(Slicer3_Base_INCLUDE_DIRS Slicer3_Base_INCLUDE_DIRS)

#-----------------------------------------------------------------------------
# Module Logic

INCLUDE("${Slicer3_CMAKE_DIR}/Slicer3ModuleLogicMacros.cmake")

IF (NOT Slicer3_USE_KWWIDGETS)
  # KWWidgets and Qt modules share the same logics typically located in the
  # kwwdigets modules. When KWWidgets is not build, the logic are missing for Qt
  # we need to manually add them:
  # Loop through Modules subdirectory and try to include Logic subdirectory if it exists
  FILE(GLOB modules_subdirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} Modules/*)
  FOREACH(subdir ${modules_subdirs})
    IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${subdir}/Logic)
      ADD_SUBDIRECTORY(${subdir}/Logic)
    ENDIF()
    FILE(GLOB subsubdirs RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${subdir}/*)
    FOREACH(subsubdir ${subsubdirs})
      IF(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${subsubdir}/Logic)
        ADD_SUBDIRECTORY(${subsubdir}/Logic)
      ENDIF()
    ENDFOREACH()
  ENDFOREACH()
ENDIF()


#-----------------------------------------------------------------------------
# Modules and CLI
#
option(Slicer3_BUILD_CLI "Build Slicer3 CLI Plugins" ON)
IF (Slicer3_USE_KWWIDGETS)
  option(Slicer3_BUILD_MODULES "Build Slicer3 Modules" ON)
ENDIF (Slicer3_USE_KWWIDGETS)

include("${Slicer3_CMAKE_DIR}/Slicer3PluginsMacros.cmake")

IF (Slicer3_USE_KWWIDGETS)
  include("${Slicer3_CMAKE_DIR}/Slicer3ModulesMacros.cmake")
  
  if(Slicer3_BUILD_MODULES)
    add_subdirectory(Modules)
  else(Slicer3_BUILD_MODULES)
    # if Slicer3_BUILD_CLI is ON, we still need at least the CommandLineModule
    # so that we can have CLI, without all the other modules
    if(Slicer3_BUILD_CLI)
      add_subdirectory(Modules/CommandLineModule)
    endif(Slicer3_BUILD_CLI)
  endif(Slicer3_BUILD_MODULES)
ENDIF (Slicer3_USE_KWWIDGETS)

#-----------------------------------------------------------------------------
# Module EMSegment libraries
# The EMSegment{Registration,Graph,Algorithm,MRML} libraries are built from the top-level
# CMakeLists.txt. Doing so allow to build these libraries also if Slicer3_USE_KWWIDGETS is OFF

if(Slicer3_BUILD_MODULES)
  ADD_SUBDIRECTORY(Modules/EMSegment/Algorithm)
  ADD_SUBDIRECTORY(Modules/EMSegment/MRML)
  ADD_SUBDIRECTORY(Modules/EMSegment/Registration)
  ADD_SUBDIRECTORY(Modules/EMSegment/Graph)
endif()

# Since EMSegment module expects Volume module and its associated library to be
# built beforehand,let's enfore that by building EMSegment here.
# Note that EMSegment has been commented in Modules/CMakeLists.txt
IF (Slicer3_USE_KWWIDGETS)
  if(Slicer3_BUILD_MODULES)
    ADD_SUBDIRECTORY(Modules/EMSegment)
  endif()
ENDIF()

#-----------------------------------------------------------------------------
# Qt Loadable Modules
#
IF(Slicer3_USE_QT AND Slicer3_BUILD_QTLOADABLEMODULES)
  add_subdirectory(QTModules)
ENDIF(Slicer3_USE_QT AND Slicer3_BUILD_QTLOADABLEMODULES)

#-----------------------------------------------------------------------------
# Applications need to be built after Modules to capture the TractTracing in
# the correct ordering.
#
add_subdirectory(Applications)

#-----------------------------------------------------------------------------
# Testing
#
if(Slicer3_BUILD_TESTING)
  add_subdirectory(Testing)
endif(Slicer3_BUILD_TESTING)

# Install testing data

install(DIRECTORY
  Testing
  DESTINATION ${Slicer3_INSTALL_SHARE_DIR} COMPONENT Development
  PATTERN ".svn" EXCLUDE
  PATTERN "CMakeLists.txt" EXCLUDE
  )

#-----------------------------------------------------------------------------
# Launcher for slicer
#

# let's build the launcher for Kww, KwwAndQt and also QtOnly slicer ...
ADD_SUBDIRECTORY(Utilities/Launcher)

# Compute Slicer3_QT_PLUGIN_PATH used to configure launch.tcl.in
SET(Slicer3_QT_PLUGIN_PATH "")
IF(Slicer3_USE_QT)
  SET(designer_launcher_project_name SlicerQT)
  GET_TARGET_PROPERTY(output_directory ${designer_launcher_project_name} LIBRARY_OUTPUT_DIRECTORY)
  SET(Slicer3_QT_PLUGIN_PATH ${output_directory})
ENDIF(Slicer3_USE_QT)

CONFIGURE_FILE(
  ${Slicer3_SOURCE_DIR}/launch.tcl.in
  "${Slicer3_BINARY_DIR}/${Slicer3_INSTALL_LIB_DIR}/launch.tcl"
  )
INSTALL(
  FILES "${Slicer3_BINARY_DIR}/${Slicer3_INSTALL_LIB_DIR}/launch.tcl"
  DESTINATION ${Slicer3_INSTALL_LIB_DIR}
  )

#-----------------------------------------------------------------------------
# Launcher for QTDesigner
#

IF (Slicer3_USE_QT)
  add_subdirectory(Utilities/Designer)
ENDIF (Slicer3_USE_QT)

#-----------------------------------------------------------------------------
# Package slicer
#
add_subdirectory(Utilities/Package)

#-----------------------------------------------------------------------------
# Documentation
#
option(BUILD_DOCUMENTATION "Build Slicer3 Documentation" OFF)
if(BUILD_DOCUMENTATION)
  add_subdirectory(Utilities/Doxygen)
endif(BUILD_DOCUMENTATION)

#-----------------------------------------------------------------------------
# Setup file for setting custom ctest vars
CONFIGURE_FILE(
  ${Slicer3_SOURCE_DIR}/CTestCustom.cmake.in
  ${Slicer3_BINARY_DIR}/CTestCustom.cmake
  @ONLY
  )


#-----------------------------------------------------------------------------
# Set up for building against a Slicer3 build tree and a Slicer3 installation 
#
include(${Slicer3_SOURCE_DIR}/GenerateSlicer3Config.cmake)

configure_file(${Slicer3_SOURCE_DIR}/CTestConfig.cmake
  ${Slicer3_BINARY_DIR}/${Slicer3_INSTALL_LIB_DIR}/CTestConfig.cmake COPYONLY IMMEDIATE)

configure_file(${Slicer3_SOURCE_DIR}/UseSlicer3.cmake.in
  ${Slicer3_BINARY_DIR}/UseSlicer3.cmake COPYONLY IMMEDIATE)

configure_file(${Slicer3_SOURCE_DIR}/CMake/RemoveTemporaryFiles.cmake.in
  ${Slicer3_BINARY_DIR}/CMake/RemoveTemporaryFiles.cmake @ONLY IMMEDIATE)
configure_file(${Slicer3_SOURCE_DIR}/CMake/Slicer3SampleBuildTest.cmake.in
  ${Slicer3_BINARY_DIR}/CMake/Slicer3SampleBuildTest.cmake @ONLY)

install(FILES
  #   ${Slicer3_BINARY_DIR}/Slicer3BuildSettings.cmake
  ${Slicer3_SOURCE_DIR}/CTestConfig.cmake
  ${Slicer3_BINARY_DIR}/UseSlicer3.cmake
  ${Slicer3_BINARY_DIR}/Utilities/Slicer3Config.cmake
  ${Slicer3_BINARY_DIR}/lib/Slicer3/Slicer3Version.txt
  DESTINATION ${Slicer3_INSTALL_LIB_DIR}
  COMPONENT Development
  )
