set(KIT qSlicer${MODULE_NAME}Module)

#
# ITK
#
set(${KIT}Testing_ITK_COMPONENTS
  ITKCommon
  )
find_package(ITK 4.6 COMPONENTS ${${KIT}Testing_ITK_COMPONENTS} REQUIRED)
if(ITK_VERSION VERSION_GREATER_EQUAL "5.3")
  foreach(factory_uc IN ITEMS "IMAGEIO")
    set(ITK_NO_${factory_uc}_FACTORY_REGISTER_MANAGER 1)
  endforeach()
else()
  set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
endif()
list(APPEND ITK_LIBRARIES ITKFactoryRegistration)
list(APPEND ITK_INCLUDE_DIRS
  ${ITKFactoryRegistration_INCLUDE_DIRS}
  )
include(${ITK_USE_FILE})

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  vtkMRMLSequenceBrowserNodeTest1.cxx
  vtkMRMLSequenceNodeTest1.cxx
  vtkSlicerSequencesLogicTest1.cxx
  vtkMRMLSequenceStorageNodeTest1.cxx
  )

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  )

set(TEMP "${CMAKE_BINARY_DIR}/Testing/Temporary")

#-----------------------------------------------------------------------------
simple_test(vtkMRMLSequenceBrowserNodeTest1)
simple_test(vtkMRMLSequenceNodeTest1)
simple_test(vtkSlicerSequencesLogicTest1)
simple_test(vtkMRMLSequenceStorageNodeTest1 ${TEMP})
