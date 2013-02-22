set(KIT ${PROJECT_NAME})

#-----------------------------------------------------------------------------
set(KIT_TEST_SRCS
  qMRMLNodeAttributeTableViewTest.cxx
  qMRMLNodeAttributeTableWidgetTest.cxx
  )

#-----------------------------------------------------------------------------
QT4_GENERATE_MOCS(
  qMRMLNodeAttributeTableViewTest.cxx
  qMRMLNodeAttributeTableWidgetTest.cxx
  )

include_directories(${CMAKE_CURRENT_BINARY_DIR})

#-----------------------------------------------------------------------------
slicerMacroConfigureModuleCxxTestDriver(
  NAME ${KIT}
  SOURCES ${KIT_TEST_SRCS}
  WITH_VTK_DEBUG_LEAKS_CHECK
  )

#-----------------------------------------------------------------------------
simple_test( qMRMLNodeAttributeTableViewTest )
simple_test( qMRMLNodeAttributeTableWidgetTest )
