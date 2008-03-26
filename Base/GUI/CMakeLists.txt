
PROJECT(SlicerBaseGUI)

# If we are not in the KWWidgets source tree, make sure we can find KWWidgets
# as an external package, and use it.

FIND_PACKAGE(KWWidgets REQUIRED)
INCLUDE(${KWWidgets_USE_FILE})

# The name of our targets (executable or libraries) will simply be based
# on the project name, with an extra prefix and suffix.

SET(TARGET_BASE_NAME "Slicer3${PROJECT_NAME}Test")

# We actually define a class in this example, and we want to be able to
# use its callbacks from our user interface. To do so, we need to create
# a library and wrap it automatically for the Tcl language.

SET(LIB_NAME "SlicerBaseGUI")
SET(LIB_SRCS 
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
  vtkSlicerDataTransferIcons.cxx
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

INCLUDE_DIRECTORIES(
  ${SlicerBase_SOURCE_DIR}
  ${SlicerBase_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${VTKITK_SOURCE_DIR}/Libs/vtkITK
  ${VTK_INCLUDE_DIR}
  ${TEEM_INCLUDE_DIR}
  ${vtkTeem_SOURCE_DIR}
  ${vtkTeem_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  ${LoadableModule_SOURCE_DIR}
  ${LoadableModule_BINARY_DIR}
  ${FreeSurfer_SOURCE_DIR}
  ${FreeSurfer_BINARY_DIR}
  )

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE("${KWWidgets_CMAKE_DIR}/KWWidgetsWrappingMacros.cmake")
KWWidgets_WRAP_TCL(${LIB_NAME} LIB_TCL_SRCS "${LIB_SRCS}" "")


# Create the library.

ADD_LIBRARY(${LIB_NAME} ${LIB_TCL_SRCS} ${LIB_SRCS})
TARGET_LINK_LIBRARIES(${LIB_NAME} 
  ${KWWidgets_LIBRARIES}
  ModuleDescriptionParser
  SlicerBaseLogic
  FreeSurfer
  )

IF(BUILD_SHARED_LIBS)
  INSTALL(TARGETS ${LIB_NAME}
      RUNTIME DESTINATION bin COMPONENT RuntimeLibraries 
      LIBRARY DESTINATION lib/Slicer3 COMPONENT RuntimeLibraries
      ARCHIVE DESTINATION lib/Slicer3 COMPONENT Development)
ENDIF(BUILD_SHARED_LIBS)

# The name of our executable and the corresponding source file.

SET(EXE_NAME "${TARGET_BASE_NAME}")
SET(EXE_SRCS "${EXE_NAME}.cxx")

# On Win32 platforms, let's configure the KWWidgets sample resource file
# to get a nice application icon and some additional information.

IF(WIN32 AND NOT BORLAND AND NOT CYGWIN)
  INCLUDE_DIRECTORIES(${VTK_TK_RESOURCES_DIR})
  SET(RES_FILE "${CMAKE_CURRENT_BINARY_DIR}/${EXE_NAME}.rc")
  SET(KWWidgets_RESOURCE_APPLICATION_NAME "${EXE_NAME}")
  SET(KWWidgets_RESOURCE_FILE_NAME "${EXE_NAME}")
  CONFIGURE_FILE(${KWWidgets_RESOURCES_DIR}/KWWidgets.rc.in ${RES_FILE})
ENDIF(WIN32 AND NOT BORLAND AND NOT CYGWIN)

# Create the executable, and link it against the KWWidgets libraries and our
# own library.

#### TODO: move the test application to Testing
#### ADD_EXECUTABLE(${EXE_NAME} WIN32 ${EXE_SRCS} ${RES_FILE})
#### TARGET_LINK_LIBRARIES(${EXE_NAME} SlicerBaseLogic SlicerBaseGUI MRML ${KWWidgets_LIBRARIES} ${LIB_NAME})
#### IF(BUILD_TESTING)
####   ADD_TEST(Test${EXE_NAME} ${EXECUTABLE_OUTPUT_PATH}/${EXE_NAME} --test)
#### ENDIF(BUILD_TESTING)

IF(BUILD_TESTING)
  ADD_TEST(Slicer3GUITest1 ${Slicer3_BINARY_DIR}/Slicer3 --no-splash --script ${Slicer3_SOURCE_DIR}/Base/GUI/Testing/TestSliceController.tcl)
  ADD_TEST(Slicer3GUITest2 ${Slicer3_BINARY_DIR}/Slicer3 --no-splash --script ${Slicer3_SOURCE_DIR}/Base/GUI/Testing/TestProgressDialog.tcl)
  ADD_TEST(Slicer3FiducialsTest1 ${Slicer3_BINARY_DIR}/Slicer3 --no-splash --script ${Slicer3_SOURCE_DIR}/Base/GUI/Testing/TestFiducials.tcl)
ENDIF(BUILD_TESTING)

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

INCLUDE_DIRECTORIES(
  ${SlicerBaseGUI_BINARY_DIR}
  ${OPENTRACKER_INC_DIR}
)
CONFIGURE_FILE(
  ${SlicerBaseGUI_SOURCE_DIR}/vtkSlicerBaseGUIConfigure.h.in 
  ${SlicerBaseGUI_BINARY_DIR}/vtkSlicerBaseGUIConfigure.h
)

#
# install the tcl package for the gui
#
SET(SlicerBaseGUITcl_DIR "${SLICER_LIBRARIES_DIR}/SlicerBaseGUITcl")

FILE(MAKE_DIRECTORY ${SlicerBaseGUITcl_DIR})

SET(TCLFILES
  pkgIndex.tcl SliceViewerInteractor.tcl
  SWidget.tcl SliceSWidget.tcl SeedSWidget.tcl CsysSWidget.tcl
  FiducialsSWidget.tcl GridSWidget.tcl CrosshairSWidget.tcl RegionsSWidget.tcl 
  Slicer2Import.tcl XcedeCatalogImport.tcl
  Loader.tcl LoadVolume.tcl FilteredDirectoryDialog.tcl
  MRMLWatcher.tcl
)

FOREACH(tclfile ${TCLFILES})
  CONFIGURE_FILE(
    ${Slicer3_SOURCE_DIR}/Base/GUI/Tcl/${tclfile}
    ${SlicerBaseGUITcl_DIR}/${tclfile}
    COPYONLY
  )
ENDFOREACH(tclfile)


# Python settings
IF(USE_PYTHON AND PYTHON_INCLUDE_PATH)
  SUBDIRS(Python)
  ADD_DEFINITIONS(-DUSE_PYTHON)
  INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
  IF (WIN32 AND PYTHON_DEBUG_LIBRARIES)
     TARGET_LINK_LIBRARIES( ${LIB_NAME}
                            debug ${PYTHON_DEBUG_LIBRARIES}
                            optimized ${PYTHON_LIBRARIES})
  ELSE (WIN32 AND PYTHON_DEBUG_LIBRARIES)
     TARGET_LINK_LIBRARIES( ${LIB_NAME} ${PYTHON_LIBRARIES})
  ENDIF (WIN32 AND PYTHON_DEBUG_LIBRARIES)
  IF(NOT WIN32)
    IF (APPLE)
     TARGET_LINK_LIBRARIES( ${LIB_NAME} ${PYTHON_LIBRARIES} )
    ELSE (APPLE)
     TARGET_LINK_LIBRARIES( ${LIB_NAME} ${PYTHON_LIBRARIES} util)
    ENDIF (APPLE)
  ENDIF(NOT WIN32)
ENDIF(USE_PYTHON AND PYTHON_INCLUDE_PATH)

# CMAke 2.4 does not have INSTALL(DIRECTORY ...) do it manually
#INSTALL(DIRECTORY "${Slicer3_BINARY_DIR}/SlicerBaseGUITcl"
#  DESTINATION ${SLICER_INSTALL_LIBRARIES_DIR})
INSTALL(CODE "EXECUTE_PROCESS\(COMMAND \"${CMAKE_COMMAND}\" -E copy_directory \"${SLICER_LIBRARIES_DIR}/SlicerBaseGUITcl\" \"\${CMAKE_INSTALL_PREFIX}/${SLICER_INSTALL_LIBRARIES_DIR}/SlicerBaseGUITcl\"\)")
