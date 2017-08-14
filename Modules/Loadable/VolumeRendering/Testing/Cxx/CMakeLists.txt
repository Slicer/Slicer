set(KIT qSlicer${MODULE_NAME}Module)

#-----------------------------------------------------------------------------

#
# ITK
#
set(${KIT}Testing_ITK_COMPONENTS
  ITKCommon
  )
find_package(ITK 4.6 COMPONENTS ${${KIT}Testing_ITK_COMPONENTS} REQUIRED)
set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
list(APPEND ITK_LIBRARIES ITKFactoryRegistration)
list(APPEND ITK_INCLUDE_DIRS
  ${ITKFactoryRegistration_INCLUDE_DIRS}
  )
include(${ITK_USE_FILE})

#-----------------------------------------------------------------------------
set(MRML_CORE_INPUT "${MRMLCore_SOURCE_DIR}/Testing/TestData/")
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../Data/Input)

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  qMRMLVolumePropertyNodeWidgetTest1.cxx
  qSlicerPresetComboBoxTest.cxx
  qSlicer${MODULE_NAME}ModuleWidgetTest1.cxx
  qSlicer${MODULE_NAME}ModuleWidgetTest2.cxx
  vtkMRMLVolumePropertyNodeTest1.cxx
  vtkMRMLVolumePropertyStorageNodeTest1.cxx
  vtkMRMLVolumeRenderingDisplayableManagerTest1.cxx
  vtkMRMLVolumeRenderingMultiVolumeTest.cxx
  )

#-----------------------------------------------------------------------------
set(KIT_TEST_GENERATE_MOC_SRCS
  qSlicerPresetComboBoxTest.cxx
  )
if(CTK_QT_VERSION VERSION_LESS "5")
  QT4_GENERATE_MOCS(${KIT_TEST_GENERATE_MOC_SRCS})
else()
  QT5_GENERATE_MOCS(${KIT_TEST_GENERATE_MOC_SRCS})
endif()
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  TARGET_LIBRARIES vtkSlicerVolumesModuleLogic
  WITH_VTK_DEBUG_LEAKS_CHECK
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
simple_test(qMRMLVolumePropertyNodeWidgetTest1)
simple_test(qSlicerPresetComboBoxTest)
simple_test(qSlicer${MODULE_NAME}ModuleWidgetTest1)
simple_test(qSlicer${MODULE_NAME}ModuleWidgetTest2 ${MRML_CORE_INPUT}/fixed.nrrd)
simple_test(vtkMRMLVolumePropertyNodeTest1 ${INPUT}/volRender.mrml)
simple_test(vtkMRMLVolumePropertyStorageNodeTest1)
simple_test(vtkMRMLVolumeRenderingDisplayableManagerTest1)
simple_test(vtkMRMLVolumeRenderingMultiVolumeTest)
