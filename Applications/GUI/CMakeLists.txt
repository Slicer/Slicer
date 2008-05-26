project(GUI)

#########################################################################
# Enable shared link forwarding support if needed.
set(Slicer3_EXE_SUFFIX)
set(Slicer3_EXE_INSTALL ${Slicer3_INSTALL_BIN_DIR})
if(BUILD_SHARED_LIBS)
  set(Slicer3_NEED_SHARED_FORWARD 1)
  set(Slicer3_EXE_SUFFIX -real)
  set(Slicer3_EXE_INSTALL ".")
  set(Slicer3_FORWARD_DIR_BUILD "${EXECUTABLE_OUTPUT_PATH}")
  set(Slicer3_FORWARD_DIR_INSTALL "${Slicer3_EXE_INSTALL}")
  set(Slicer3_FORWARD_PATH_BUILD "\"${Slicer3_FORWARD_DIR_BUILD}\",\"${KWWidgets_RUNTIME_DIRS}\",\"${VTK_DIR}/bin\"")
  if(WIN32)
    set(Slicer3_FORWARD_PATH_INSTALL
      "\"../${Slicer3_INSTALL_LIB_DIR}/tcl/bin\"")
  else(WIN32)
    set(Slicer3_FORWARD_PATH_INSTALL
      "\"../lib\",\"../lib/KWWidgets\",\"../lib/InsightToolkit\",\"..//lib/TclTk\"")
  endif(WIN32)
endif(BUILD_SHARED_LIBS)
# Add main server executables.
set(Slicer3_EXE_LIST Slicer3)

# Add shared link forwarding executables if necessary.
if(Slicer3_NEED_SHARED_FORWARD)
  foreach(slicer_exe ${Slicer3_EXE_LIST})
    set(Slicer3_FORWARD_EXE ${slicer_exe}${Slicer3_EXE_SUFFIX})
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/Slicer3-forward.c.in
      ${CMAKE_CURRENT_BINARY_DIR}/${slicer_exe}-forward.c
      @ONLY IMMEDIATE)
    add_executable(${slicer_exe} WIN32
      ${CMAKE_CURRENT_BINARY_DIR}/${slicer_exe}-forward.c)
    add_dependencies(${slicer_exe} ${slicer_exe}${Slicer3_EXE_SUFFIX})
    install(TARGETS ${slicer_exe}
      DESTINATION bin
      COMPONENT Runtime)
  endforeach(slicer_exe)
endif(Slicer3_NEED_SHARED_FORWARD)

set(SRC
  Slicer3.cxx
  )

# --------------------------------------------------------------------------
# Find SLICERLIBCURL (slicercurl)
find_package(SLICERLIBCURL REQUIRED)
include(${SLICERLIBCURL_USE_FILE})

# --------------------------------------------------------------------------
# Include dirs
# MESSAGE(STATUS "Setting up dir_list, Qdec bin dir = ${Qdec_BINARY_DIR}, Qdec src dir = ${Qdec_SOURCE_DIR}")

set(dir_list
  ${Slicer3_Libs_INCLUDE_DIRS}
  ${Slicer3_Base_INCLUDE_DIRS}
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

if(Slicer3_BUILD_MODULES)
  add_definitions(-DSlicer3_BUILD_MODULES)
endif(Slicer3_BUILD_MODULES)

if(Slicer3_BUILD_CLI)
  add_definitions(-DSlicer3_BUILD_CLI)
endif(Slicer3_BUILD_CLI)

if(Slicer3_USE_PYTHON AND PYTHON_INCLUDE_PATH)
  set(dir_list ${dir_list} ${PYTHON_INCLUDE_PATH})
endif(Slicer3_USE_PYTHON AND PYTHON_INCLUDE_PATH)

include_directories(${dir_list})

# On Win32 platforms, let's configure the KWWidgets sample resource file
# to get a nice application icon and some additional information.
# This is *mandatory* for a static build otherwise the static Tk resources
# won't be included and images like checkboxes will not appear.

# NOTE: this is commented for now until we get rid of extra include directives.
# We have 90 (!) at this point and rc.exe can *not* handle such number.

#IF(WIN32 AND NOT BORLAND AND NOT CYGWIN)
#  INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsResourceMacros.cmake")
#  SET(RC_FILENAME "${CMAKE_CURRENT_BINARY_DIR}/Slicer3.rc")
#  KWWidgets_CREATE_RC_FILE(
#    RC_FILENAME "${RC_FILENAME}"
#    RC_APPLICATION_NAME "Slicer3"
#    RC_COMPANY_NAME "NA-MIC")
#ENDIF(WIN32 AND NOT BORLAND AND NOT CYGWIN)

#
# with the WIN32_CONSOLE you can see diagnostic output on in a DOS-style
# cmd window
#
option(Slicer3_BUILD_WIN32_CONSOLE "Build Slicer3 executable as a console app on windows (allows debug output)" OFF)
if(Slicer3_BUILD_WIN32_CONSOLE)
  add_executable(Slicer3${Slicer3_EXE_SUFFIX} ${SRC} ${RC_FILENAME})
else(Slicer3_BUILD_WIN32_CONSOLE)
  add_executable(Slicer3${Slicer3_EXE_SUFFIX} WIN32 ${SRC} ${RC_FILENAME})
endif(Slicer3_BUILD_WIN32_CONSOLE)

if(WFENGINEMODULE)
  set(WFENGINE_LINK_LIB ${WFENGINE_LINK_LIB} WFEngineModule)
else(WFENGINEMODULE)
  add_definitions(-DWFENGINE_DEBUG)
endif(WFENGINEMODULE)

# Option for LabelStatistics
if(LABELSTATISTICS)
  set(LABELSTATISTICS_LINK_LIB LabelStatistics)
else(LABELSTATISTICS)
  add_definitions(-DLABELSTATISTICS_DEBUG)
endif(LABELSTATISTICS)

# Important: remove RemoteIO when using CMake 2.6. Couldn't be part of 
# Slicer3_LIBRARIES right now because of the Curl lib RemoteIO is using is
# STATIC, but that's fixed in 2.6.

target_link_libraries(Slicer3${Slicer3_EXE_SUFFIX}
  ${Slicer3_Libs_LIBRARIES}
  ${Slicer3_Base_LIBRARIES}
  RemoteIO
  ${KWWidgets_LIBRARIES}
#  ${SLICERLIBCURL_LIBRARIES}
  )

# MESSAGE(STATUS "Slicer lib curl libs = ${SLICERLIBCURL_LIBRARIES}")
# MESSAGE(STATUS "Slicer lib curl library dirs = ${SLICERLIBCURL_LIBRARY_DIRS}")

if(Slicer3_BUILD_MODULES)
  target_link_libraries(Slicer3${Slicer3_EXE_SUFFIX}
    Volumes
    CommandLineModule
    ScriptedModule
    SlicerDaemon
    SlicerTractographyDisplay
    SlicerTractographyFiducialSeeding
    )
else(Slicer3_BUILD_MODULES)
  # if Slicer3_BUILD_CLI is ON, we still need at least the CommandLineModule
  # so that we can have CLI, without all the other modules
  if(Slicer3_BUILD_CLI)
    target_link_libraries(Slicer3${Slicer3_EXE_SUFFIX}
      CommandLineModule
      )
  endif(Slicer3_BUILD_CLI)
endif(Slicer3_BUILD_MODULES)

if(Slicer3_USE_PYTHON AND PYTHON_LIBRARIES)
  target_link_libraries(Slicer3${Slicer3_EXE_SUFFIX} SlicerGUIPython)
  if(WIN32 AND PYTHON_DEBUG_LIBRARIES)
    target_link_libraries(Slicer3${Slicer3_EXE_SUFFIX}
      debug ${PYTHON_DEBUG_LIBRARIES}
      optimized ${PYTHON_LIBRARIES})
  else(WIN32 AND PYTHON_DEBUG_LIBRARIES)
    target_link_libraries(Slicer3${Slicer3_EXE_SUFFIX} ${PYTHON_LIBRARIES})
  endif(WIN32 AND PYTHON_DEBUG_LIBRARIES)
  if(NOT WIN32)
    # Linux/Unix requires util library, but mac doesn't
    if(APPLE)
      target_link_libraries(Slicer3${Slicer3_EXE_SUFFIX})
    else(APPLE)
      target_link_libraries(Slicer3${Slicer3_EXE_SUFFIX} util)
    endif(APPLE)
  endif(NOT WIN32)
endif(Slicer3_USE_PYTHON AND PYTHON_LIBRARIES)

install_targets(/bin Slicer3${Slicer3_EXE_SUFFIX})

# If needed, copy the Tcl/Tk support files required at run-time 
# to initialize Tcl/Tk. This is only triggered if VTK was built
# against a Tcl/Tk static library.

include("${KWWidgets_CMAKE_DIR}/KWWidgetsTclTkMacros.cmake")
kwwidgets_copy_tcl_tk_support_files("${Slicer3_BINARY_DIR}/lib")
kwwidgets_install_tcl_tk_support_files("/lib")

#
# use the command line utilities
#
find_package(GenerateCLP REQUIRED)
if(GenerateCLP_FOUND)
  include(${GenerateCLP_USE_FILE})
endif(GenerateCLP_FOUND)

generateclp(${SRC} Slicer3.xml)

#
# Command line parsing tests
#

add_test(Slicer3CLTest1 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --help)
add_test(Slicer3CLTest2 ${Slicer3_BINARY_DIR}/Slicer3 --xml --test-mode)
add_test(Slicer3CLTest3 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode -f ${Slicer3_SOURCE_DIR}/Applications/GUI/Testing/TestScript.tcl)
add_test(Slicer3CLTest4 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --eval "puts testing ,. exit 0")
add_test(Slicer3CLTest5 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --exec "puts testing ,. exit 0")
add_test(Slicer3CLTest6 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Applications/GUI/Testing/TestScript.tcl)
add_test(Slicer3CLTest7 ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Applications/GUI/Testing/LoadSceneTest.tcl)
add_test(Slicer3ScrollTest ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --script ${Slicer3_SOURCE_DIR}/Applications/GUI/Testing/ScrollTesting.tcl)

#
# MRML Tests
#

add_test(Slicer3MRMLUndo ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --no-splash -f ${Slicer3_HOME}/share/MRML/Testing/testUndo.tcl)
add_test(Slicer3MRMLVolume ${Slicer3_BINARY_DIR}/Slicer3 --test-mode --no-splash -f ${Slicer3_HOME}/share/MRML/Testing/testVolume.tcl)

#
# make the testing
#
if(Slicer3_BUILD_TESTING)
  subdirs(Testing)
endif(Slicer3_BUILD_TESTING)
