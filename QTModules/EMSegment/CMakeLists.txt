# --------------------------------------------------------------------------
# Add subdirectories

ADD_SUBDIRECTORY(Logic)


# Module name (usually identique to the directory name)
SET(qt_module_name "EMSegment")

SET(qt_module_export_directive "Q_SLICER_QTMODULES_EMSEGMENT_EXPORT")

# Additional includes - Current_{source,binary} and Slicer3_{Libs,Base} already included
SET(qt_module_include_directories
  ${EMSegmentMRML_SOURCE_DIR}
  ${EMSegmentMRML_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  ${CMAKE_CURRENT_SOURCE_DIR}/Panels
  ${CMAKE_CURRENT_BINARY_DIR}/Panels
  ${CMAKE_CURRENT_SOURCE_DIR}/Widgets
  ${CMAKE_CURRENT_BINARY_DIR}/Widgets
  ${CMAKE_CURRENT_SOURCE_DIR}/Workflow
  ${CMAKE_CURRENT_BINARY_FIR}/Workflow
  )

# Widget source files
SET(widget_SRCS
  Widgets/qSlicerEMSegmentAnatomicalTreeWidget.cxx
  Widgets/qSlicerEMSegmentAnatomicalTreeWidget.h
  Widgets/qSlicerEMSegmentAnatomicalTreeWidget_p.h
  Widgets/qSlicerEMSegmentGraphWidget.cxx
  Widgets/qSlicerEMSegmentGraphWidget.h
  Widgets/qSlicerEMSegmentInputChannelListWidget.cxx
  Widgets/qSlicerEMSegmentInputChannelListWidget.h
  Widgets/vtkPlotGaussian.cxx
  Widgets/vtkPlotGaussian.h

  Widgets/qSlicerEMSegmentWidget.cxx
  Widgets/qSlicerEMSegmentWidget.h
  )

SET(widget_MOC_SRCS
  Widgets/qSlicerEMSegmentAnatomicalTreeWidget.h
  Widgets/qSlicerEMSegmentAnatomicalTreeWidget_p.h
  Widgets/qSlicerEMSegmentGraphWidget.h
  Widgets/qSlicerEMSegmentInputChannelListWidget.h

  Widgets/qSlicerEMSegmentWidget.h
  )

# Panel source files
SET(panel_SRCS
  Panels/qSlicerEMSegmentDefineAnatomicalTreePanel.cxx           # Step 3
  Panels/qSlicerEMSegmentDefineAnatomicalTreePanel.h
  Panels/qSlicerEMSegmentDefineAtlasPanel.cxx                    # Step 4
  Panels/qSlicerEMSegmentDefineAtlasPanel.h
  Panels/qSlicerEMSegmentDefineInputChannelsPanel.cxx            # Step 2
  Panels/qSlicerEMSegmentDefineInputChannelsPanel.h
  Panels/qSlicerEMSegmentDefinePreprocessingPanel.cxx            # Step 6
  Panels/qSlicerEMSegmentDefinePreprocessingPanel.h
  Panels/qSlicerEMSegmentDefineTaskPanel.cxx                     # Step 1
  Panels/qSlicerEMSegmentDefineTaskPanel.h
  Panels/qSlicerEMSegmentEditNodeBasedParametersPanel.cxx        # Step 8
  Panels/qSlicerEMSegmentEditNodeBasedParametersPanel.h
  Panels/qSlicerEMSegmentEditRegistrationParametersPanel.cxx     # Step 5
  Panels/qSlicerEMSegmentEditRegistrationParametersPanel.h
  Panels/qSlicerEMSegmentSpecifyIntensityDistributionPanel.cxx   # Step 7
  Panels/qSlicerEMSegmentSpecifyIntensityDistributionPanel.h
  Panels/qSlicerEMSegmentRunSegmentationPanel.cxx                # Step 9
  Panels/qSlicerEMSegmentRunSegmentationPanel.h
  )

SET(panel_MOC_SRCS
  Panels/qSlicerEMSegmentDefineAnatomicalTreePanel.h           # Step 3
  Panels/qSlicerEMSegmentDefineAtlasPanel.h                    # Step 4
  Panels/qSlicerEMSegmentDefineInputChannelsPanel.h            # Step 2
  Panels/qSlicerEMSegmentDefinePreprocessingPanel.h            # Step 6
  Panels/qSlicerEMSegmentDefineTaskPanel.h                     # Step 1
  Panels/qSlicerEMSegmentEditNodeBasedParametersPanel.h        # Step 8
  Panels/qSlicerEMSegmentEditRegistrationParametersPanel.h     # Step 5
  Panels/qSlicerEMSegmentSpecifyIntensityDistributionPanel.h   # Step 7
  Panels/qSlicerEMSegmentRunSegmentationPanel.h                # Step 9
  )

# Workflow source files
SET(workflow_SRCS
  Workflow/qSlicerEMSegmentDefineAnatomicalTreeStep.cxx           # Step 3
  Workflow/qSlicerEMSegmentDefineAnatomicalTreeStep.h
  Workflow/qSlicerEMSegmentDefineAtlasStep.cxx                    # Step 4
  Workflow/qSlicerEMSegmentDefineAtlasStep.h
  Workflow/qSlicerEMSegmentDefineInputChannelsStep.cxx            # Step 2
  Workflow/qSlicerEMSegmentDefineInputChannelsStep.h
  Workflow/qSlicerEMSegmentDefinePreprocessingStep.cxx            # Step 6
  Workflow/qSlicerEMSegmentDefinePreprocessingStep.h
  Workflow/qSlicerEMSegmentDefineTaskStep.cxx                     # Step 1
  Workflow/qSlicerEMSegmentDefineTaskStep.h
  Workflow/qSlicerEMSegmentEditNodeBasedParametersStep.cxx        # Step 8
  Workflow/qSlicerEMSegmentEditNodeBasedParametersStep.h
  Workflow/qSlicerEMSegmentEditRegistrationParametersStep.cxx     # Step 5
  Workflow/qSlicerEMSegmentEditRegistrationParametersStep.h
  Workflow/qSlicerEMSegmentSegmentationLogicStep.cxx              # Step 10
  Workflow/qSlicerEMSegmentSegmentationLogicStep.h
  Workflow/qSlicerEMSegmentSpecifyIntensityDistributionStep.cxx   # Step 7
  Workflow/qSlicerEMSegmentSpecifyIntensityDistributionStep.h
  Workflow/qSlicerEMSegmentRunSegmentationStep.cxx                # Step 9
  Workflow/qSlicerEMSegmentRunSegmentationStep.h

  Workflow/qSlicerEMSegmentWorkflowWidgetStep.cxx
  Workflow/qSlicerEMSegmentWorkflowWidgetStep.h
  )

SET(workflow_MOC_SRCS
  Workflow/qSlicerEMSegmentDefineAnatomicalTreeStep.h           # Step 3
  Workflow/qSlicerEMSegmentDefineAtlasStep.h                    # Step 4
  Workflow/qSlicerEMSegmentDefineInputChannelsStep.h            # Step 2
  Workflow/qSlicerEMSegmentDefinePreprocessingStep.h            # Step 6
  Workflow/qSlicerEMSegmentDefineTaskStep.h                     # Step 1
  Workflow/qSlicerEMSegmentEditNodeBasedParametersStep.h        # Step 8
  Workflow/qSlicerEMSegmentEditRegistrationParametersStep.h     # Step 5
  Workflow/qSlicerEMSegmentSegmentationLogicStep.h              # Step 10
  Workflow/qSlicerEMSegmentSpecifyIntensityDistributionStep.h   # Step 7
  Workflow/qSlicerEMSegmentRunSegmentationStep.h                # Step 9

  Workflow/qSlicerEMSegmentWorkflowWidgetStep.h
  )

# Source files
SET(qt_module_SRCS
  ${panel_SRCS}
  ${widget_SRCS}
  ${workflow_SRCS}
  qSlicerEMSegmentModule.cxx
  qSlicerEMSegmentModule.h
  qSlicerEMSegmentModuleWidget.cxx
  qSlicerEMSegmentModuleWidget.h
  )

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  ${panel_MOC_SRCS}
  ${widget_MOC_SRCS}
  ${workflow_MOC_SRCS}
  qSlicerEMSegmentModule.h
  qSlicerEMSegmentModuleWidget.h
  )

# UI files
SET(qt_module_UI_SRCS
  Resources/UI/qSlicerEMSegmentModule.ui

  # Widgets
  #Resources/UI/qMRMLEMSegmentAnatomicalTreeWidget.ui
  Resources/UI/qSlicerEMSegmentGraphWidget.ui

  # Workflow: MRI Human brain
  Resources/UI/qSlicerEMSegmentDefineAnatomicalTreePanel.ui           # Step 3
  Resources/UI/qSlicerEMSegmentDefineAtlasPanel.ui                    # Step 4
  Resources/UI/qSlicerEMSegmentDefineInputChannelsPanel.ui            # Step 2
  Resources/UI/qSlicerEMSegmentDefinePreprocessingPanel.ui            # Step 6
  Resources/UI/qSlicerEMSegmentDefineTaskPanel.ui                     # Step 1
  Resources/UI/qSlicerEMSegmentEditNodeBasedParametersPanel.ui        # Step 8
  Resources/UI/qSlicerEMSegmentEditRegistrationParametersPanel.ui     # Step 5
  Resources/UI/qSlicerEMSegmentSpecifyIntensityDistributionPanel.ui   # Step 7
  Resources/UI/qSlicerEMSegmentRunSegmentationPanel.ui                # Step 9
)

# Additional Target libraries
SET(qt_module_target_libraries
  vtkSlicerEMSegmentModuleLogic
  vtkCharts
  )

# Resources
SET(qt_module_resources
  Resources/qSlicerEMSegmentModule.qrc
)

Slicer3_build_qtmodule(
  NAME ${qt_module_name}
  EXPORT_DIRECTIVE ${qt_module_export_directive}
  INCLUDE_DIRECTORIES ${qt_module_include_directories}
  SRCS ${qt_module_SRCS}
  MOC_SRCS ${qt_module_MOC_SRCS}
  UI_SRCS ${qt_module_UI_SRCS}
  TARGET_LIBRARIES ${qt_module_target_libraries}
  RESOURCES ${qt_module_resources}
  )

# --------------------------------------------------------------------------
# Sub directories
#
ADD_SUBDIRECTORY(Widgets/Plugins)

if(BUILD_TESTING)
  SET(EMSegment_TESTING_DATA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Testing/Data)
  ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)
