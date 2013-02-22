set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------
set(TEMP ${Slicer_BINARY_DIR}/Testing/Temporary)
set(INPUT ${MRMLCore_SOURCE_DIR}/Testing/TestData/)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  qSlicer${MODULE_NAME}IOOptionsWidgetTest1.cxx
  qSlicer${MODULE_NAME}ModuleWidgetTest1.cxx
  vtkSlicer${MODULE_NAME}LogicTest1.cxx
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  TARGET_LIBRARIES vtkSlicerVolumesModuleLogic
  WITH_VTK_DEBUG_LEAKS_CHECK
  )

#-----------------------------------------------------------------------------
simple_test(qSlicerVolumesIOOptionsWidgetTest1)
simple_test(qSlicerVolumesModuleWidgetTest1 ${INPUT}/fixed.nrrd)
simple_test(vtkSlicerVolumesLogicTest1 ${INPUT}/fixed.nrrd)
  
#-----------------------------------------------------------------------------
add_executable(vtkSlicer${MODULE_NAME}LogicTest2 vtkSlicer${MODULE_NAME}LogicTest2.cxx)
target_link_libraries(vtkSlicer${MODULE_NAME}LogicTest2 ${KIT})

add_test(NAME vtkSlicer${MODULE_NAME}LogicTest2
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:vtkSlicer${MODULE_NAME}LogicTest2>
  vtkSlicer${MODULE_NAME}LogicTest2 ${TEMP}
  )
