# --------------------------------------------------------------------------
# Add subdirectories

ADD_SUBDIRECTORY(Logic)


# Module name (usually identique to the directory name)
SET(qt_module_name "EMSegment")

SET(qt_module_export_directive "Q_SLICER_QTMODULES_EMSEGMENT_EXPORT")

# Additional includes - Current_{source,binary} and Slicer_{Libs,Base} already included
SET(qt_module_include_directories
  ${EMSegmentMRML_SOURCE_DIR}
  ${EMSegmentMRML_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
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
  Workflow/qSlicerEMSegmentDefineTaskStep_p.h
  Workflow/qSlicerEMSegmentEditNodeBasedParametersStep.cxx        # Step 8
  Workflow/qSlicerEMSegmentEditNodeBasedParametersStep.h
  Workflow/qSlicerEMSegmentEditRegistrationParametersStep.cxx     # Step 5
  Workflow/qSlicerEMSegmentEditRegistrationParametersStep.h
  Workflow/qSlicerEMSegmentEditRegistrationParametersStep_p.h
  Workflow/qSlicerEMSegmentSegmentationLogicStep.cxx              # Step 10
  Workflow/qSlicerEMSegmentSegmentationLogicStep.h
  Workflow/qSlicerEMSegmentSpecifyIntensityDistributionStep.cxx   # Step 7
  Workflow/qSlicerEMSegmentSpecifyIntensityDistributionStep.h
  Workflow/qSlicerEMSegmentSpecifyIntensityDistributionStep_p.h
  Workflow/qSlicerEMSegmentRunSegmentationStep.cxx                # Step 9
  Workflow/qSlicerEMSegmentRunSegmentationStep.h
  Workflow/qSlicerEMSegmentRunSegmentationStep_p.h

  Workflow/qSlicerEMSegmentWorkflowWidgetStep.cxx
  Workflow/qSlicerEMSegmentWorkflowWidgetStep.h
  )

SET(workflow_MOC_SRCS
  Workflow/qSlicerEMSegmentDefineAnatomicalTreeStep.h           # Step 3
  Workflow/qSlicerEMSegmentDefineAtlasStep.h                    # Step 4
  Workflow/qSlicerEMSegmentDefineInputChannelsStep.h            # Step 2
  Workflow/qSlicerEMSegmentDefinePreprocessingStep.h            # Step 6
  Workflow/qSlicerEMSegmentDefineTaskStep.h                     # Step 1
  Workflow/qSlicerEMSegmentDefineTaskStep_p.h
  Workflow/qSlicerEMSegmentEditNodeBasedParametersStep.h        # Step 8
  Workflow/qSlicerEMSegmentEditRegistrationParametersStep.h     # Step 5
  Workflow/qSlicerEMSegmentEditRegistrationParametersStep_p.h
  Workflow/qSlicerEMSegmentSegmentationLogicStep.h              # Step 10
  Workflow/qSlicerEMSegmentSpecifyIntensityDistributionStep.h   # Step 7
  Workflow/qSlicerEMSegmentSpecifyIntensityDistributionStep_p.h
  Workflow/qSlicerEMSegmentRunSegmentationStep.h                # Step 9
  Workflow/qSlicerEMSegmentRunSegmentationStep_p.h

  Workflow/qSlicerEMSegmentWorkflowWidgetStep.h
  )

# Source files
SET(qt_module_SRCS
  ${widget_SRCS}
  ${workflow_SRCS}
  qSlicerEMSegmentModule.cxx
  qSlicerEMSegmentModule.h
  qSlicerEMSegmentModuleWidget.cxx
  qSlicerEMSegmentModuleWidget.h
  )

# Headers that should run through moc
SET(qt_module_MOC_SRCS
  ${widget_MOC_SRCS}
  ${workflow_MOC_SRCS}
  qSlicerEMSegmentModule.h
  qSlicerEMSegmentModuleWidget.h
  )

# UI files
SET(qt_module_UI_SRCS
  Resources/UI/qSlicerEMSegmentModule.ui

  # Widgets
  Resources/UI/qSlicerEMSegmentAnatomicalTreeWidget.ui
  Resources/UI/qSlicerEMSegmentGraphWidget.ui

  # Workflow: MRI Human brain
  Resources/UI/qSlicerEMSegmentDefineAnatomicalTreeStep.ui           # Step 3
  Resources/UI/qSlicerEMSegmentDefineAtlasStep.ui                    # Step 4
  Resources/UI/qSlicerEMSegmentDefineInputChannelsStep.ui            # Step 2
  Resources/UI/qSlicerEMSegmentDefinePreprocessingStep.ui            # Step 6
  Resources/UI/qSlicerEMSegmentDefineTaskStep.ui                     # Step 1
  Resources/UI/qSlicerEMSegmentEditNodeBasedParametersStep.ui        # Step 8
  Resources/UI/qSlicerEMSegmentEditRegistrationParametersStep.ui     # Step 5
  Resources/UI/qSlicerEMSegmentSpecifyIntensityDistributionStep.ui   # Step 7
  Resources/UI/qSlicerEMSegmentRunSegmentationStep.ui                # Step 9
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

slicerMacroBuildQtModule(
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
ADD_SUBDIRECTORY(Workflow/Plugins)

if(BUILD_TESTING)
  SET(EMSegment_TESTING_DATA_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Testing/Data)
  ADD_SUBDIRECTORY(Widgets/Testing)
  #ADD_SUBDIRECTORY(Testing)
endif(BUILD_TESTING)
