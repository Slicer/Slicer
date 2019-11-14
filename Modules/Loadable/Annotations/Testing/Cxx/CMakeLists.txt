set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(INPUT "${CMAKE_CURRENT_SOURCE_DIR}/../")
set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input/Fiducials\ List.acsv
  ${TEMP}/Fiducials\ List.acsv
  COPYONLY
  )

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  #qMRMLSceneAnnotationModelAndAnnotationTreeWidgetTest1
  qMRMLAnnotationROIWidgetTest1.cxx
  vtkMRMLAnnotationNodeTest1.cxx
  vtkMRMLAnnotationNodesUndoTest1.cxx
  vtkMRMLAnnotationStorageNodeTest1.cxx
  vtkMRMLAnnotationDisplayNodeTest1.cxx
  vtkMRMLAnnotationDisplayNodeTest2.cxx
  vtkMRMLAnnotationControlPointsNodeTest1.cxx
  vtkMRMLAnnotationPointDisplayNodeTest1.cxx
  vtkMRMLAnnotationControlPointsStorageNodeTest1.cxx
  vtkMRMLAnnotationTextDisplayNodeTest1.cxx
  vtkMRMLAnnotationLinesNodeTest1.cxx
  vtkMRMLAnnotationLineDisplayNodeTest1.cxx
  vtkMRMLAnnotationLinesStorageNodeTest1.cxx
  vtkMRMLAnnotationAngleNodeTest1.cxx
  vtkMRMLAnnotationAngleStorageNodeTest1.cxx
  vtkMRMLAnnotationRulerNodeTest1.cxx
  vtkMRMLAnnotationRulerStorageNodeTest1.cxx
  vtkMRMLAnnotationFiducialNodeTest1.cxx
  vtkMRMLAnnotationFiducialsStorageNodeTest1.cxx
  vtkMRMLAnnotationROINodeTest1.cxx
  vtkMRMLAnnotationDisplayableManagerTest1.cxx
  vtkMRMLAnnotationDisplayableManagerTest2.cxx
  #TestAnnotationWidgetsStandalone.cxx
  #vtkMRMLAnnotationTextNodeAndWidgetTest1.cxx
  #vtkMRMLAnnotationTextDisplayableManagerTest1.cxx
  #vtkMRMLAnnotationRulerDisplayableManagerTest1.cxx
  #vtkMRMLAnnotationTextNodeAndWidgetTest1.cxx
  vtkSlicerAnnotationModuleLogicTest1.cxx
  vtkSlicerAnnotationModuleLogicImportSceneTest.cxx
  #qSlicerAnnotationModuleAnnotationPropertyDialogTest1.cxx
  #TestAnnAngleWidget3D.cxx
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  TARGET_LIBRARIES qSlicer${MODULE_NAME}ModuleWidgets
  TESTS_TO_RUN_VAR KIT_TESTS_TO_RUN
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
# Add all the tests
foreach(test ${KIT_TESTS_TO_RUN})
  get_filename_component(testname ${test} NAME_WE)
  simple_test(${testname}
      -D ${INPUT}
      -T ${TEMP}
      #-V Baseline/${testname}.png
    )
endforeach()

