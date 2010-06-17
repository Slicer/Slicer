PROJECT(SlicerBaseGUI)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
IF(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)
MARK_AS_ADVANCED(CMAKE_BACKWARDS_COMPATIBILITY)


# --------------------------------------------------------------------------
# Include dirs

set(include_dirs
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${SlicerBaseLogic_SOURCE_DIR}
  ${SlicerBaseLogic_BINARY_DIR}
  ${qSlicerBaseQTBase_SOURCE_DIR}
  ${qSlicerBaseQTBase_BINARY_DIR}
  ${qSlicerBaseQTCore_SOURCE_DIR}
  ${qSlicerBaseQTCore_BINARY_DIR}
  ${qSlicerBaseQTGUI_SOURCE_DIR}
  ${qSlicerBaseQTGUI_BINARY_DIR}
  ${qMRMLWidgets_SOURCE_DIR}
  ${qMRMLWidgets_BINARY_DIR}
  ${MRML_SOURCE_DIR}
  ${MRML_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  ${FreeSurfer_SOURCE_DIR}
  ${FreeSurfer_BINARY_DIR}
  ${OPENTRACKER_INC_DIR}
  ${RemoteIO_SOURCE_DIR}
  ${RemoteIO_BINARY_DIR}
  )

include_directories(${include_dirs})

slicer3_get_persistent_property(Slicer3_Base_INCLUDE_DIRS tmp)
slicer3_set_persistent_property(Slicer3_Base_INCLUDE_DIRS ${tmp} ${include_dirs})

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/vtkSlicerBaseGUIConfigure.h.in 
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerBaseGUIConfigure.h
  )

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/*.h") 
install(FILES 
  ${files}
  ${CMAKE_CURRENT_BINARY_DIR}/vtkSlicerBaseGUIConfigure.h
  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME} COMPONENT Development
  )

file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.h") 
install(FILES 
  ${files}
  DESTINATION ${Slicer3_INSTALL_INCLUDE_DIR}/${PROJECT_NAME}/Resources COMPONENT Development
  )

# --------------------------------------------------------------------------
# Sources

set(SlicerBaseGUI_SRCS 
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
  vtkSlicerSaveDataWidgetIcons.cxx
  vtkSlicerColorLUTIcons.cxx
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
  vtkSlicerModelInfoWidget.cxx
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
  vtkSlicerEmptyModuleGUI.cxx
  # KW Widgets
  vtkKWTextWithHyperlinksWithScrollbars.cxx
  vtkSlicerModuleCollapsibleFrame.cxx
  vtkSlicerNodeSelectorWidget.cxx
  vtkSlicerContextMenuHelper.cxx
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
  vtkSlicerSeedWidgetClass.cxx
  vtkSlicerFiducialListWidget.cxx
  vtkSlicerModelDisplayWidget.cxx
  vtkSlicerColorDisplayWidget.cxx
  vtkSlicerColorEditWidget.cxx
  vtkSlicerMRMLSaveDataWidget.cxx
  vtkSlicerClipModelsWidget.cxx
  vtkSlicerParameterWidget.cxx
  vtkSlicerSceneSnapshotWidget.cxx
  vtkSlicerRecordSnapshotWidget.cxx
  vtkSlicerModelHierarchyWidget.cxx
  vtkSlicerDiffusionTensorGlyphDisplayWidget.cxx
  vtkSlicerPermissionPrompterWidget.cxx
  vtkSlicerXNATPermissionPrompterWidget.cxx
  vtkSlicerROIGUI.cxx
  vtkKWCheckBoxSelectionDialog.cxx
  vtkSlicerModulesWizardDialog.cxx
  vtkSlicerModulesConfigurationStep.cxx
  vtkSlicerModulesStep.cxx
  vtkSlicerModulesResultStep.cxx
  vtkSlicerROIDisplayWidget.cxx
  vtkSlicerROIViewerWidget.cxx
  # VTK Replacement classes
  vtkSlicerInteractorStyle.cxx
  vtkSlicerViewerInteractorStyle.cxx
  #vtkSlicerRenderWidget.cxx - Not used
  vtkSlicerBoxRepresentation.cxx
  vtkSlicerBoxWidget2.cxx
  vtkSlicerWaitMessageWidget.cxx
  vtkSlicerXYPlotWidget.cxx
  ${SRCS}
  )


# --------------------------------------------------------------------------
# Wrapping

set(lib_name "SlicerBaseGUI")

include("${KWWidgets_CMAKE_DIR}/KWWidgetsWrappingMacros.cmake")
kwwidgets_wrap_tcl(${lib_name} SlicerBaseGUI_TCL_SRCS "${SlicerBaseGUI_SRCS}" "")

# --------------------------------------------------------------------------
# Build and install the library

add_library(${lib_name} 
  ${SlicerBaseGUI_TCL_SRCS} 
  ${SlicerBaseGUI_SRCS}
)

set(libraries 
  ${KWWidgets_LIBRARIES}
  ModuleDescriptionParser
  SlicerBaseLogic
  FreeSurfer
  RemoteIO
  )

IF(Slicer3_USE_QT)
  set(libraries 
    ${libraries} 
    ${QT_LIBRARIES}
    qSlicerBaseQTGUI
    )
ENDIF(Slicer3_USE_QT)

target_link_libraries(${lib_name} ${libraries})

# Apply user-defined properties to the library target.
IF(Slicer3_LIBRARY_PROPERTIES)
  SET_TARGET_PROPERTIES(${lib_name} PROPERTIES ${Slicer3_LIBRARY_PROPERTIES})
ENDIF(Slicer3_LIBRARY_PROPERTIES)


slicer3_get_persistent_property(Slicer3_Base_LIBRARIES tmp)
slicer3_set_persistent_property(Slicer3_Base_LIBRARIES ${tmp} ${lib_name})

install(TARGETS ${lib_name}
  RUNTIME DESTINATION ${Slicer3_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries 
  LIBRARY DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
  ARCHIVE DESTINATION ${Slicer3_INSTALL_LIB_DIR} COMPONENT Development
  )

# Python settings

if(Slicer3_USE_PYTHON AND PYTHON_INCLUDE_PATH)
  include_directories(${PYTHON_INCLUDE_PATH})
  if(WIN32)
    include_directories(${PYTHON_INCLUDE_PATH}/../PC)
  endif(WIN32)
  if(WIN32 AND PYTHON_DEBUG_LIBRARIES)
    target_link_libraries(${lib_name}
      debug ${PYTHON_DEBUG_LIBRARIES}
      optimized ${PYTHON_LIBRARIES})
  else(WIN32 AND PYTHON_DEBUG_LIBRARIES)
    target_link_libraries(${lib_name} ${PYTHON_LIBRARIES})
  endif(WIN32 AND PYTHON_DEBUG_LIBRARIES)
  if(NOT WIN32)
    if(APPLE OR UNIX)
      target_link_libraries(${lib_name} ${PYTHON_LIBRARIES})
    else(APPLE OR UNIX)
      target_link_libraries(${lib_name} ${PYTHON_LIBRARIES} util)
    endif(APPLE OR UNIX)
  endif(NOT WIN32)
endif(Slicer3_USE_PYTHON AND PYTHON_INCLUDE_PATH)

# --------------------------------------------------------------------------
# Install support files

file(GLOB TCLFILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "Tcl/*.tcl")
foreach(tclfile ${TCLFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${tclfile}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_LIB_DIR}/${PROJECT_NAME}/${tclfile}
    COPYONLY)
endforeach(tclfile)

install(
  FILES ${TCLFILES}
  DESTINATION ${Slicer3_INSTALL_LIB_DIR}/${PROJECT_NAME}/Tcl
  )

file(GLOB PNGFILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "Resources/*.png")
foreach(pngfile ${PNGFILES})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${pngfile}
    ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_SHARE_DIR}/${PROJECT_NAME}/${pngfile}
    COPYONLY
    )
endforeach(pngfile)

install(
  FILES ${PNGFILES}
  DESTINATION ${Slicer3_INSTALL_SHARE_DIR}/${PROJECT_NAME}/Resources
  )

# --------------------------------------------------------------------------
# Python support

IF(Slicer3_USE_PYTHON AND PYTHON_INCLUDE_PATH)
  ADD_SUBDIRECTORY(Python)
ENDIF(Slicer3_USE_PYTHON AND PYTHON_INCLUDE_PATH)

# --------------------------------------------------------------------------
# Testing

IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
