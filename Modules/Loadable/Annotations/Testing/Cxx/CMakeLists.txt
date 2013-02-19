set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(DATA "${CMAKE_CURRENT_SOURCE_DIR}/../")
set(TEMP "${Slicer_BINARY_DIR}/Testing/Temporary")

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input/Fiducials\ List.acsv
  ${TEMP}/Fiducials\ List.acsv
  COPYONLY
  )

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS)
set(KIT_TEST_NAMES)
set(KIT_TEST_NAMES_CXX)
SlicerMacroConfigureGenericCxxModuleTests(${MODULE_NAME} KIT_TEST_SRCS KIT_TEST_NAMES KIT_TEST_NAMES_CXX)

#-----------------------------------------------------------------------------
set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_NAMES_CXX}
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
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )

list(REMOVE_ITEM Tests ${KIT_TEST_NAMES_CXX})
list(APPEND Tests ${KIT_TEST_SRCS})

set(TestsToRun ${Tests})
list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

add_executable(${KIT}CxxTests ${Tests})
set_target_properties(${KIT}CxxTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${Slicer_BIN_DIR})

set(KIT_TEST_TARGET_LIBRARIES
  qSlicerAnnotationsModuleWidgets
  )

target_link_libraries(${KIT}CxxTests ${KIT}
  ${KIT_TEST_TARGET_LIBRARIES}
  )

#-----------------------------------------------------------------------------
# Add all the tests
foreach(test ${TestsToRun})
  get_filename_component(TESTNAME ${test} NAME_WE)
  add_test(
    NAME ${TESTNAME}
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME}
      -D ${DATA}
      -T ${TEMP}
      #-V Baseline/${TESTNAME}.png
    )
endforeach()

