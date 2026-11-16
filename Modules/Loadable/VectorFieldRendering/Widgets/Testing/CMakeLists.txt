set(KIT ${PROJECT_NAME})

set(TEST_SOURCES
  qMRMLColorLegendGroupBoxTest1.cxx
  qMRMLVectorFieldDisplayWidgetTest1.cxx
  )

create_test_sourcelist(Tests ${KIT}CppTests.cxx
  ${TEST_SOURCES}
  )

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
# Fixtures shared by the vector field tests
include_directories(${vtkSlicerVectorFieldRenderingModuleMRML_SOURCE_DIR}/Testing/Cxx)

ctk_add_executable_utf8(${KIT}CxxTests ${Tests} ${Tests_MOC_CXX})
target_link_libraries(${KIT}CxxTests ${KIT})

#-----------------------------------------------------------------------------
simple_test( qMRMLColorLegendGroupBoxTest1 )
simple_test( qMRMLVectorFieldDisplayWidgetTest1 )
