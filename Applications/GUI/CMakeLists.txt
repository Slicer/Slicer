project(GUI)

# --------------------------------------------------------------------------
# Enable shared link forwarding support if needed.
#
get_filename_component(TCL_LIBRARY_PATH "${TCL_LIBRARY}" PATH)
get_filename_component(TCL_DIR "${TCL_LIBRARY_PATH}" PATH)
get_filename_component(INCR_TCL_LIBRARY_PATH "${INCR_TCL_LIBRARY}" PATH)
get_filename_component(INCR_TK_LIBRARY_PATH "${INCR_TK_LIBRARY}" PATH)

# Some people use Mesa instead of the default system OpenGL.
# Try to incorporate that in the launcher, but NOT for installation.

if(NOT WIN32)
  foreach(dependency ${vtkftgl_LIB_DEPENDS})
    get_filename_component(dependency_lib "${dependency}" NAME_WE)
    if(dependency_lib STREQUAL "libGL")
      get_filename_component(OPENGL_gl_LIBRARY_PATH "${dependency}" PATH)
    endif(dependency_lib STREQUAL "libGL")
  endforeach(dependency)
endif(NOT WIN32)

set(Slicer_EXE_SUFFIX)
set(Slicer_EXE_INSTALL ${Slicer_INSTALL_BIN_DIR})
if(BUILD_SHARED_LIBS)
  set(Slicer_NEED_SHARED_FORWARD 1)
  set(Slicer_EXE_SUFFIX -real)
  set(Slicer_EXE_INSTALL ".")
  set(Slicer_FORWARD_DIR_BUILD "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
  set(Slicer_FORWARD_DIR_INSTALL "${Slicer_EXE_INSTALL}")
  if(WIN32)
    set(Slicer_FORWARD_PATH_BUILD 
      "\"${Slicer_FORWARD_DIR_BUILD}\" CONFIG_DIR_POST,\"${VTK_DIR}/bin\" CONFIG_DIR_POST,\"${KWWidgets_RUNTIME_DIRS}\" CONFIG_DIR_POST,\"${ITK_DIR}/bin\" CONFIG_DIR_POST,\"${Teem_DIR}/bin\" CONFIG_DIR_POST,\"${TCL_DIR}/bin\",\"${INCR_TCL_LIBRARY_PATH}\",\"${INCR_TK_LIBRARY_PATH}\"")
    set(Slicer_FORWARD_PATH_INSTALL
      "\"../lib/TclTk/bin\",\"../lib/TclTk/lib/itcl${INCR_TCL_VERSION_DOT}\",\"../lib/TclTk/lib/itk${INCR_TCL_VERSION_DOT}\"")
    if(Slicer_BUILD_MODULES)
      set(Slicer_FORWARD_PATH_BUILD 
        "${Slicer_FORWARD_PATH_BUILD},\"${Slicer_HOME}/${Slicer_INSTALL_MODULES_BIN_DIR}\" CONFIG_DIR_POST")
    endif(Slicer_BUILD_MODULES)
    if(Slicer_BUILD_CLI)
      set(Slicer_FORWARD_PATH_BUILD 
        "${Slicer_FORWARD_PATH_BUILD},\"${Slicer_HOME}/${Slicer_INSTALL_PLUGINS_BIN_DIR}\" CONFIG_DIR_POST")
    endif(Slicer_BUILD_CLI)
  else(WIN32)
    set(Slicer_FORWARD_PATH_BUILD 
      "\"${Slicer_FORWARD_DIR_BUILD}\",\"${VTK_DIR}/bin\",\"${KWWidgets_RUNTIME_DIRS}\",\"${ITK_DIR}/bin\",\"${Teem_DIR}/bin\",\"${TCL_DIR}/bin\",\"${INCR_TCL_LIBRARY_PATH}\",\"${INCR_TK_LIBRARY_PATH}\",\"${OPENGL_gl_LIBRARY_PATH}\"")
    set(Slicer_FORWARD_PATH_INSTALL
      "\"../${Slicer_INSTALL_LIB_DIR}\",\"../lib/vtk-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION}\",\"../lib/KWWidgets\",\"../lib/InsightToolkit\",\"../lib/Teem-${Teem_VERSION_MAJOR}.${Teem_VERSION_MINOR}\",\"../lib/TclTk/lib\",\"../lib/TclTk/lib/itcl${INCR_TCL_VERSION_DOT}\",\"../lib/TclTk/lib/itk${INCR_TCL_VERSION_DOT}\",\"../lib/FreeSurfer\",\"../lib/IGT\",\"../lib/ITKCommandIO\",\"../lib/LoadableModule\",\"../lib/MGHImageIO\",\"../lib/MRML\",\"../lib/ModuleDescriptionParser\",\"../lib/MRMLIDImageIO\",\"../lib/Qdec\",\"../lib/RemoteIO\",\"../lib/SlicerIO\",\"../lib/tclap\",\"../lib/vtkITK\",\"../lib/vtkTeem\"")
    if(Slicer_BUILD_MODULES)
      set(Slicer_FORWARD_PATH_BUILD 
        "${Slicer_FORWARD_PATH_BUILD},\"${Slicer_HOME}/${Slicer_INSTALL_MODULES_BIN_DIR}\"")
    endif(Slicer_BUILD_MODULES)
    if(Slicer_BUILD_CLI)
      set(Slicer_FORWARD_PATH_BUILD 
        "${Slicer_FORWARD_PATH_BUILD},\"${Slicer_HOME}/${Slicer_INSTALL_PLUGINS_BIN_DIR}\"")
    endif(Slicer_BUILD_CLI)
  endif(WIN32)
  if(Slicer_BUILD_MODULES)
    set(Slicer_FORWARD_PATH_INSTALL
      "${Slicer_FORWARD_PATH_INSTALL},\"../${Slicer_INSTALL_MODULES_BIN_DIR}\"")
  endif(Slicer_BUILD_MODULES)
  if(Slicer_BUILD_CLI)
    set(Slicer_FORWARD_PATH_INSTALL
      "${Slicer_FORWARD_PATH_INSTALL},\"../${Slicer_INSTALL_PLUGINS_BIN_DIR}\"")
  endif(Slicer_BUILD_CLI)
endif(BUILD_SHARED_LIBS)

# Add main server executables.
set(Slicer_EXE_LIST Slicer3)

# Add shared link forwarding executables if necessary.
if(Slicer_NEED_SHARED_FORWARD)
  foreach(slicer_exe ${Slicer_EXE_LIST})
    set(Slicer_FORWARD_EXE ${slicer_exe}${Slicer_EXE_SUFFIX})
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/Slicer3-forward.c.in
      ${CMAKE_CURRENT_BINARY_DIR}/${slicer_exe}-forward.c
      @ONLY)
    add_executable(${slicer_exe} WIN32
      ${CMAKE_CURRENT_BINARY_DIR}/${slicer_exe}-forward.c)
    add_dependencies(${slicer_exe} ${Slicer_FORWARD_EXE})
    install(TARGETS ${slicer_exe}
      DESTINATION ${Slicer_INSTALL_BIN_DIR}
      COMPONENT Runtime)
  endforeach(slicer_exe)
endif(Slicer_NEED_SHARED_FORWARD)

# --------------------------------------------------------------------------
# Sources
#
set(SRC
  Slicer3.cxx
  )

# --------------------------------------------------------------------------
# Find SLICERLIBCURL (slicercurl)
#
find_package(SLICERLIBCURL REQUIRED)
include(${SLICERLIBCURL_USE_FILE})

# --------------------------------------------------------------------------
# Include dirs
#
set(dir_list
  ${Slicer_SOURCE_DIR}
  ${Slicer_BINARY_DIR}
  ${Slicer_Libs_INCLUDE_DIRS}
  ${Slicer_Base_INCLUDE_DIRS}
  ${NAVITRACK_INC}
  ${IGSTK_INCLUDE_DIRS}
  ${Volumes_SOURCE_DIR}
  ${Volumes_BINARY_DIR}
  ${Volumes_Init_SOURCE_DIR}
  ${Volumes_Init_BINARY_DIR}
  ${GradientAnisotropicDiffusionFilter_SOURCE_DIR}
  ${GradientAnisotropicDiffusionFilter_BINARY_DIR}
  ${GradientAnisotropicDiffusionFilter_Init_SOURCE_DIR}
  ${GradientAnisotropicDiffusionFilter_Init_BINARY_DIR}
  ${SlicerTractographyDisplay_SOURCE_DIR}
  ${SlicerTractographyDisplay_BINARY_DIR}
  ${SlicerTractographyDisplay_Init_SOURCE_DIR}
  ${SlicerTractographyDisplay_Init_BINARY_DIR}
  ${SlicerTractographyFiducialSeeding_SOURCE_DIR}
  ${SlicerTractographyFiducialSeeding_BINARY_DIR}
  ${SlicerTractographyFiducialSeeding_Init_SOURCE_DIR}
  ${SlicerTractographyFiducialSeeding_Init_BINARY_DIR}
  ${EMSegment_SOURCE_DIR}
  ${EMSegment_BINARY_DIR}
  ${EMSegment_Init_SOURCE_DIR}
  ${EMSegment_Init_BINARY_DIR}
  ${RealTimeImaging_SOURCE_DIR}
  ${RealTimeImaging_BINARY_DIR}
  ${RealTimeImaging_Init_SOURCE_DIR}
  ${RealTimeImaging_Init_BINARY_DIR}
  ${MRAblation_SOURCE_DIR}
  ${MRAblation_BINARY_DIR}
  ${MRAblation_Init_SOURCE_DIR}
  ${MRAblation_Init_BINARY_DIR}
  ${LabelStatistics_SOURCE_DIR}       
  ${LabelStatistics_BINARY_DIR}
  ${LabelStatistics_Init_SOURCE_DIR}
  ${LabelStatistics_Init_BINARY_DIR}
  ${WFEngineModule_SOURCE_DIR}
  ${WFEngineModule_BINARY_DIR}
  ${WFEngineModule_Init_SOURCE_DIR}
  ${WFEngineModule_Init_BINARY_DIR}
  ${CommandLineModule_SOURCE_DIR}
  ${CommandLineModule_BINARY_DIR}
  ${CommandLineModule_Init_SOURCE_DIR}
  ${CommandLineModule_Init_BINARY_DIR}
  ${ScriptedModule_SOURCE_DIR}
  ${ScriptedModule_BINARY_DIR}
  ${SlicerDaemon_SOURCE_DIR}
  ${SlicerDaemon_BINARY_DIR}
  ${SlicerDaemon_Init_SOURCE_DIR}
  ${SlicerDaemon_Init_BINARY_DIR}
  )

if(Slicer_BUILD_MODULES)
  add_definitions(-DSlicer_BUILD_MODULES)
endif(Slicer_BUILD_MODULES)

if(Slicer_BUILD_CLI)
  add_definitions(-DSlicer_BUILD_CLI)
endif(Slicer_BUILD_CLI)

if(Slicer_USE_PYTHON AND PYTHON_INCLUDE_PATH)
  set(dir_list ${dir_list} ${PYTHON_INCLUDE_PATH})
  if(WIN32)
    set(dir_list ${dir_list} ${PYTHON_INCLUDE_PATH}/../PC)
  endif(WIN32)
endif(Slicer_USE_PYTHON AND PYTHON_INCLUDE_PATH)

include_directories(${dir_list})

# --------------------------------------------------------------------------
# with the WIN32_CONSOLE you can see diagnostic output on in a DOS-style
# cmd window
#
option(Slicer_BUILD_WIN32_CONSOLE 
  "Build Slicer3 executable as a console app on windows (allows debug output)" 
  OFF
  )

if(Slicer_BUILD_WIN32_CONSOLE)
  add_executable(Slicer3${Slicer_EXE_SUFFIX} 
    ${SRC} 
    ${RC_FILENAME}
    )
else(Slicer_BUILD_WIN32_CONSOLE)
  add_executable(
    Slicer3${Slicer_EXE_SUFFIX} WIN32 
    ${SRC} 
    ${RC_FILENAME}
    )
endif(Slicer_BUILD_WIN32_CONSOLE)

# --------------------------------------------------------------------------
# Options
#
if(WFENGINEMODULE)
  set(WFENGINE_LINK_LIB ${WFENGINE_LINK_LIB} WFEngineModule)
else(WFENGINEMODULE)
  add_definitions(-DWFENGINE_DEBUG)
endif(WFENGINEMODULE)

if(LABELSTATISTICS)
  set(LABELSTATISTICS_LINK_LIB LabelStatistics)
else(LABELSTATISTICS)
  add_definitions(-DLABELSTATISTICS_DEBUG)
endif(LABELSTATISTICS)

# --------------------------------------------------------------------------
# Link
# Important: remove RemoteIO when using CMake 2.6. Couldn't be part of 
# Slicer_LIBRARIES right now because of the Curl lib RemoteIO is using is
# STATIC, but that's fixed in 2.6.
#
target_link_libraries(Slicer3${Slicer_EXE_SUFFIX}
  ${Slicer_Libs_LIBRARIES}
  ${Slicer_Base_LIBRARIES}
  RemoteIO
  ${KWWidgets_LIBRARIES}
#  ${SLICERLIBCURL_LIBRARIES}
  )

if(Slicer_BUILD_MODULES)
  target_link_libraries(Slicer3${Slicer_EXE_SUFFIX}
    Volumes
    CommandLineModule
    ScriptedModule
    SlicerDaemon
    SlicerTractographyDisplay
    SlicerTractographyFiducialSeeding
    )
else(Slicer_BUILD_MODULES)
  # if Slicer_BUILD_CLI is ON, we still need at least the CommandLineModule
  # so that we can have CLI, without all the other modules
  if(Slicer_BUILD_CLI)
    target_link_libraries(Slicer3${Slicer_EXE_SUFFIX}
      CommandLineModule
      )
  endif(Slicer_BUILD_CLI)
endif(Slicer_BUILD_MODULES)

if(Slicer_USE_PYTHON AND PYTHON_LIBRARIES)
  target_link_libraries(Slicer3${Slicer_EXE_SUFFIX} SlicerBaseGUIPython)
  if(WIN32 AND PYTHON_DEBUG_LIBRARIES)
    target_link_libraries(Slicer3${Slicer_EXE_SUFFIX}
      debug ${PYTHON_DEBUG_LIBRARIES}
      optimized ${PYTHON_LIBRARIES})
  else(WIN32 AND PYTHON_DEBUG_LIBRARIES)
    target_link_libraries(Slicer3${Slicer_EXE_SUFFIX} ${PYTHON_LIBRARIES})
  endif(WIN32 AND PYTHON_DEBUG_LIBRARIES)
  if(NOT WIN32)
    # Linux/Unix requires util library, but mac doesn't
    if(APPLE OR UNIX)
      target_link_libraries(Slicer3${Slicer_EXE_SUFFIX})
    else(APPLE OR UNIX)
      target_link_libraries(Slicer3${Slicer_EXE_SUFFIX} util)
    endif(APPLE OR UNIX)
  endif(NOT WIN32)
endif(Slicer_USE_PYTHON AND PYTHON_LIBRARIES)

# --------------------------------------------------------------------------
# Install
#
install(TARGETS Slicer3${Slicer_EXE_SUFFIX}
  DESTINATION ${Slicer_INSTALL_BIN_DIR}
  COMPONENT Runtime)

# --------------------------------------------------------------------------
# If needed, copy the Tcl/Tk support files required at run-time 
# to initialize Tcl/Tk. This is only triggered if VTK was built
# against a Tcl/Tk static library.

include("${KWWidgets_CMAKE_DIR}/KWWidgetsTclTkMacros.cmake")
kwwidgets_copy_tcl_tk_support_files("${Slicer_BINARY_DIR}/lib")
kwwidgets_install_tcl_tk_support_files("/lib")

# --------------------------------------------------------------------------
# Use the command line utilities
#
find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

generateclp(${SRC} Slicer3.xml)

# --------------------------------------------------------------------------
# Testing
#
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
