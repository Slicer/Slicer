set(KIT qSlicer${MODULE_NAME}Module)

set(KIT_TEST_SRCS)
set(KIT_TEST_NAMES)
set(KIT_TEST_NAMES_CXX)
SlicerMacroConfigureGenericCxxModuleTests(${MODULE_NAME} KIT_TEST_SRCS KIT_TEST_NAMES KIT_TEST_NAMES_CXX)

set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  ${KIT_TEST_NAMES_CXX}
  qMRMLVolumePropertyNodeWidgetTest1.cxx
  qSlicerPresetComboBoxTest.cxx
  qSlicerVolumeRenderingModuleWidgetTest1.cxx
  qSlicerVolumeRenderingModuleWidgetTest2.cxx
  vtkMRMLVolumePropertyNodeTest1.cxx
  vtkMRMLVolumePropertyStorageNodeTest1.cxx
  vtkMRMLVolumeRenderingDisplayableManagerTest1.cxx
  vtkMRMLVolumeRenderingMultiVolumeTest.cxx
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )
list(REMOVE_ITEM Tests ${KIT_TEST_NAMES_CXX})
list(APPEND Tests ${KIT_TEST_SRCS})

QT4_GENERATE_MOCS(
  qSlicerPresetComboBoxTest.cxx
  )
include_directories( ${CMAKE_CURRENT_BINARY_DIR})

set(LIBRARY_NAME ${PROJECT_NAME})

add_executable(${KIT}CxxTests ${Tests})
set_target_properties(${KIT}CxxTests PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${Slicer_BIN_DIR})
target_link_libraries(${KIT}CxxTests ${KIT} vtkSlicerVolumesModuleLogic)

set(VOLUME_FILE "${MRMLCore_SOURCE_DIR}/Testing/TestData/fixed.nrrd")

foreach(testname ${KIT_TEST_NAMES})
  simple_test( ${testname} ${VOLUME_FILE})
endforeach()
simple_test( qMRMLVolumePropertyNodeWidgetTest1 )
simple_test( qSlicerPresetComboBoxTest )
simple_test( qSlicerVolumeRenderingModuleWidgetTest1 )
simple_test( qSlicerVolumeRenderingModuleWidgetTest2 ${VOLUME_FILE} )
simple_test( vtkMRMLVolumePropertyNodeTest1 )
simple_test( vtkMRMLVolumePropertyStorageNodeTest1 )
simple_test( vtkMRMLVolumeRenderingDisplayableManagerTest1 )
simple_test( vtkMRMLVolumeRenderingMultiVolumeTest )
