project(SlicerBaseGUI)

# If we are not in the KWWidgets source tree, make sure we can find KWWidgets
# as an external package, and use it.

find_package(KWWidgets REQUIRED)
include(${KWWidgets_USE_FILE})

# The name of our targets (executable or libraries) will simply be based
# on the project name, with an extra prefix and suffix.

set(TARGET_BASE_NAME "Slicer3${PROJECT_NAME}Test")

# We actually define a class in this example, and we want to be able to
# use its callbacks from our user interface. To do so, we need to create
# a library and wrap it automatically for the Tcl language.

set(LIB_NAME "SlicerBaseGUI")
set(LIB_SRCS 
  vtkSlicerStyle.cxx
  vtkSlicerWindow.cxx
  vtkSlicerApplicationSettingsInterface.cxx
  vtkSlicerTheme.cxx
  vtkSlicerColor.cxx
  vtkSlicerFont.cxx
  vtkSlicerIcons.cxx
  vtkSlicerImageCloseUp2D.cxx
  vtkSlicerImageCrossHair2D.cxx
  vtkSlicerLogoIcons.cxx
  vtkSlicerHelpIcons.cxx
  vtkSlicerPopUpHelpWidget.cxx
  vtkSlicerBaseAcknowledgementLogoIcons.cxx
  vtkSlicerVisibilityIcons.cxx
  vtkSlicerModuleNavigationIcons.cxx
  vtkSlicerToolbarIcons.cxx
  vtkSlicerFoundationIcons.cxx
  vtkSlicerViewControlIcons.cxx
  vtkSlicerSlicesControlIcons.cxx
  vtkSlicerCheckRadioButtonIcons.cxx
  vtkSlicerApplication.cxx
  vtkSlicerComponentGUI.cxx
  vtkSlicerModuleGUI.cxx
  vtkSlicerGUICollection.cxx
  vtkSlicerModuleNavigator.cxx
  #vtkSlicerTabbedNotebookModuleExampleGUI.cxx
  vtkSlicerSliceGUI.cxx
  vtkSlicerSliceGUICollection.cxx
  vtkSlicerSliceViewer.cxx
  vtkSlicerSliceControllerWidget.cxx
  vtkSlicerSlicesGUI.cxx
  vtkSlicerModelsGUI.cxx
  vtkSlicerFiducialsGUI.cxx
  vtkSlicerColorGUI.cxx
  vtkSlicerCamerasGUI.cxx
  vtkSlicerTransformsGUI.cxx
  vtkSlicerDataGUI.cxx
  vtkSlicerGUILayout.cxx    
  vtkSlicerToolbarGUI.cxx
  vtkSlicerViewControlGUI.cxx
  vtkSlicerSlicesControlGUI.cxx
  vtkSlicerModuleChooseGUI.cxx
  vtkSlicerLogoDisplayGUI.cxx
  vtkSlicerApplicationGUI.cxx
  vtkSlicerDataTransferWidget.cxx
  vtkSlicerCacheAndDataIOManagerGUI.cxx
  # KW Widgets
  vtkKWTextWithHyperlinksWithScrollbars.cxx
  vtkSlicerModuleCollapsibleFrame.cxx
  vtkSlicerNodeSelectorWidget.cxx
  vtkSlicerWidget.cxx
  vtkKWMatrix4x4.cxx
  vtkKWScriptedWidget.cxx
  vtkKWProgressDialog.cxx
  vtkSlicerVolumeHeaderWidget.cxx
  vtkSlicerVolumeFileHeaderWidget.cxx
  vtkSlicerTransformManagerWidget.cxx
  vtkSlicerTransformEditorWidget.cxx
  vtkSlicerMRMLTreeWidget.cxx
  vtkSlicerViewerWidget.cxx
  vtkSlicerFiducialListWidget.cxx
  vtkSlicerModelDisplayWidget.cxx
  vtkSlicerColorDisplayWidget.cxx
  vtkSlicerMRMLSaveDataWidget.cxx
  vtkSlicerClipModelsWidget.cxx
  vtkSlicerParameterWidget.cxx
  vtkSlicerSceneSnapshotWidget.cxx
  vtkSlicerRecordSnapshotWidget.cxx
  vtkSlicerModelHierarchyWidget.cxx
  vtkSlicerDiffusionTensorGlyphDisplayWidget.cxx
  vtkSlicerPermissionPrompterWidget.cxx
  vtkSlicerXNATPermissionPrompterWidget.cxx
  # VTK Replacement classes
  vtkSlicerInteractorStyle.cxx
  vtkSlicerViewerInteractorStyle.cxx
  vtkSlicerROIGUI.cxx
  )

# Helper classes

#SET_SOURCE_FILES_PROPERTIES(
# none yet
# vtkSlicerGUIUpdate.cxx
# vtkSlicerLogicUpdate.cxx
# vtkSlicerMRMLUpdate.cxx
# WRAP_EXCLUDE
# )

configure_file(
  ${SlicerBaseGUI_SOURCE_DIR}/vtkSlicerBaseGUIConfigure.h.in 
  ${SlicerBaseGUI_BINARY_DIR}/vtkSlicerBaseGUIConfigure.h
  )

set(include_dirs
  ${SlicerBaseGUI_SOURCE_DIR}
  ${SlicerBaseGUI_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  ${FreeSurfer_SOURCE_DIR}
  ${FreeSurfer_BINARY_DIR}
  ${OPENTRACKER_INC_DIR}
  )

include_directories(${include_dirs})

GET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS tmp)
SET_PERSISTENT_PROPERTY(Slicer3_INCLUDE_DIRS ${tmp} ${include_dirs})

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h") 
install(FILES 
  ${files}
  ${SlicerBaseGUI_BINARY_DIR}/vtkSlicerBaseGUIConfigure.h
  DESTINATION include/Slicer3 COMPONENT Development
  )
file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.h") 
install(FILES 
  ${files}
  DESTINATION include/Slicer3/Resources COMPONENT Development
  )

include("${KWWidgets_CMAKE_DIR}/KWWidgetsWrappingMacros.cmake")
kwwidgets_wrap_tcl(${LIB_NAME} LIB_TCL_SRCS "${LIB_SRCS}" "")

# Create the library.

add_library(${LIB_NAME} ${LIB_TCL_SRCS} ${LIB_SRCS})
GET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES tmp)
SET_PERSISTENT_PROPERTY(Slicer3_LIBRARIES ${tmp} ${LIB_NAME})

target_link_libraries(${LIB_NAME} 
  ${KWWidgets_LIBRARIES}
  ModuleDescriptionParser
  SlicerBaseLogic
  FreeSurfer
  )

if(BUILD_SHARED_LIBS)
  install(TARGETS ${LIB_NAME}
    RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
    LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
endif(BUILD_SHARED_LIBS)

# The name of our executable and the corresponding source file.

set(EXE_NAME "${TARGET_BASE_NAME}")
set(EXE_SRCS "${EXE_NAME}.cxx")

# On Win32 platforms, let's configure the KWWidgets sample resource file
# to get a nice application icon and some additional information.

if(WIN32 AND NOT BORLAND AND NOT CYGWIN)
  include_directories(${VTK_TK_RESOURCES_DIR})
  set(RES_FILE "${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.rc")
  set(KWWidgets_RESOURCE_APPLICATION_NAME "${EXE_NAME}")
  set(KWWidgets_RESOURCE_FILE_NAME "${EXE_NAME}")
  configure_file(${KWWidgets_RESOURCES_DIR}/KWWidgets.rc.in ${RES_FILE})
endif(WIN32 AND NOT BORLAND AND NOT CYGWIN)

# Create the executable, and link it against the KWWidgets libraries and our
# own library.

#### TODO: move the test application to Testing
#### ADD_EXECUTABLE(${EXE_NAME} WIN32 ${EXE_SRCS} ${RES_FILE})
#### TARGET_LINK_LIBRARIES(${EXE_NAME} SlicerBaseLogic SlicerBaseGUI MRML ${KWWidgets_LIBRARIES} ${LIB_NAME})
#### IF(BUILD_TESTING)
####   ADD_TEST(Test${EXE_NAME} ${EXECUTABLE_OUTPUT_PATH}/${EXE_NAME} --test)
#### ENDIF(BUILD_TESTING)

if(BUILD_TESTING)
  add_test(Slicer3GUITest1 ${Slicer3_BINARY_DIR}/Slicer3 --no-splash --script ${Slicer3_SOURCE_DIR}/Base/GUI/Testing/TestSliceController.tcl)
  add_test(Slicer3GUITest2 ${Slicer3_BINARY_DIR}/Slicer3 --no-splash --script ${Slicer3_SOURCE_DIR}/Base/GUI/Testing/TestProgressDialog.tcl)
  add_test(Slicer3FiducialsTest1 ${Slicer3_BINARY_DIR}/Slicer3 --no-splash --script ${Slicer3_SOURCE_DIR}/Base/GUI/Testing/TestFiducials.tcl)
endif(BUILD_TESTING)

# If needed, copy the Tcl/Tk support files required at run-time 
# to initialize Tcl/Tk. This is only triggered if VTK was built
# against a Tcl/Tk static library.

#INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsTclTkMacros.cmake")
#KWWidgets_COPY_TCL_TK_SUPPORT_FILES("${PROJECT_BINARY_DIR}/lib")

# Install the example target. If you are using this CMakeLists.txt file
# to create your own application based on KWWidgets, you can most likely 
# omit this section or change the installation directory, unless you want
# to install your application right where KWWidgets is already installed.

#INSTALL_TARGETS(${KWWidgets_INSTALL_BIN_DIR} ${EXE_NAME})

# Register this example as a test. Our executable supports a --test
# configuration option so that it can be run non-interactively as a test.
# If you are using this CMakeLists.txt file to create your own application
# based on KWWidgets, you can most likely omit this section too, unless
# your application supports that feature too.

#
# install the tcl package for the gui
#
set(SlicerBaseGUITcl_DIR "${SLICER_LIBRARIES_DIR}/SlicerBaseGUITcl")

file(MAKE_DIRECTORY ${SlicerBaseGUITcl_DIR})

set(TCLFILES
  pkgIndex.tcl SliceViewerInteractor.tcl
  SWidget.tcl SliceSWidget.tcl SeedSWidget.tcl CsysSWidget.tcl
  FiducialsSWidget.tcl GridSWidget.tcl CrosshairSWidget.tcl RegionsSWidget.tcl 
  Slicer2Import.tcl XcedeCatalogImport.tcl XcatalogImport.tcl
  XnatXcatImport.tcl Loader.tcl LoadVolume.tcl FilteredDirectoryDialog.tcl
  XnatPublishScene.tcl MRMLWatcher.tcl
  )

foreach(tclfile ${TCLFILES})
  configure_file(
    ${Slicer3_SOURCE_DIR}/Base/GUI/Tcl/${tclfile}
    ${SlicerBaseGUITcl_DIR}/${tclfile}
    COPYONLY
    )
endforeach(tclfile)


# Python settings
if(USE_PYTHON AND PYTHON_INCLUDE_PATH)
  subdirs(Python)
  add_definitions(-DUSE_PYTHON)
  include_directories(${PYTHON_INCLUDE_PATH})
  if (WIN32 AND PYTHON_DEBUG_LIBRARIES)
    target_link_libraries(${LIB_NAME}
      debug ${PYTHON_DEBUG_LIBRARIES}
      optimized ${PYTHON_LIBRARIES})
  else (WIN32 AND PYTHON_DEBUG_LIBRARIES)
    target_link_libraries(${LIB_NAME} ${PYTHON_LIBRARIES})
  endif (WIN32 AND PYTHON_DEBUG_LIBRARIES)
  if(NOT WIN32)
    if (APPLE)
      target_link_libraries(${LIB_NAME} ${PYTHON_LIBRARIES})
    else (APPLE)
      target_link_libraries(${LIB_NAME} ${PYTHON_LIBRARIES} util)
    endif (APPLE)
  endif(NOT WIN32)
endif(USE_PYTHON AND PYTHON_INCLUDE_PATH)

# CMAke 2.4 does not have INSTALL(DIRECTORY ...) do it manually
#INSTALL(DIRECTORY "${Slicer3_BINARY_DIR}/SlicerBaseGUITcl"
#  DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR})
install(CODE "EXECUTE_PROCESS\(COMMAND \"${CMAKE_COMMAND}\" -E copy_directory \"${SLICER_LIBRARIES_DIR}/SlicerBaseGUITcl\" \"\${CMAKE_INSTALL_PREFIX}/${SLICER_INSTALL_LIBRARIES_DIR}/SlicerBaseGUITcl\"\)")
