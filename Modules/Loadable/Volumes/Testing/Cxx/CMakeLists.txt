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
set(TEMP ${Slicer_BINARY_DIR}/Testing/Temporary)
set(MRML_CORE_INPUT ${MRMLCore_SOURCE_DIR}/Testing/TestData/)
set(SLICERAPP_INPUT ${Slicer_SOURCE_DIR}/Applications/SlicerApp/Data/Input/)

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
  WITH_VTK_ERROR_OUTPUT_CHECK
  )

#-----------------------------------------------------------------------------
simple_test(qSlicerVolumesIOOptionsWidgetTest1)
simple_test(qSlicerVolumesModuleWidgetTest1 DATA{${MRML_CORE_INPUT}/fixed.nrrd})
simple_test(vtkSlicerVolumesLogicTest1 DATA{${MRML_CORE_INPUT}/fixed.nrrd})
simple_test(vtkSlicerVolumesLogicTest1_TestNAN
  DRIVER_TESTNAME vtkSlicer${MODULE_NAME}LogicTest1 DATA{${SLICERAPP_INPUT}/testNANInVolume.nrrd}
  )

#-----------------------------------------------------------------------------
ctk_add_executable_utf8(vtkSlicer${MODULE_NAME}LogicTest2 vtkSlicer${MODULE_NAME}LogicTest2.cxx)
target_link_libraries(vtkSlicer${MODULE_NAME}LogicTest2 ${KIT})
set_target_properties(vtkSlicer${MODULE_NAME}LogicTest2 PROPERTIES FOLDER "Module-${MODULE_NAME}")

add_test(NAME vtkSlicer${MODULE_NAME}LogicTest2
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:vtkSlicer${MODULE_NAME}LogicTest2>
  vtkSlicer${MODULE_NAME}LogicTest2 ${TEMP}
  )
