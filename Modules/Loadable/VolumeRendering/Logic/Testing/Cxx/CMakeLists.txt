set(KIT vtkSlicer${MODULE_NAME}ModuleLogic)

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  vtkSlicerVolumeRenderingLogicTest.cxx
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )

add_executable(${KIT}CxxTests ${Tests})
target_link_libraries(${KIT}CxxTests ${KIT})

set(VOLUME_FILE "${MRMLCore_SOURCE_DIR}/Testing/TestData/fixed.nrrd")

simple_test( vtkSlicerVolumeRenderingLogicTest ${Slicer_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_SHARE_DIR}/${MODULE_NAME})

