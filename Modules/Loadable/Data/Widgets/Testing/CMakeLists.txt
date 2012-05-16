set(KIT ${PROJECT_NAME})
set(CMAKE_TESTDRIVER_AFTER_TESTMAIN  "DEBUG_LEAKS_ENABLE_EXIT_ERROR();")
create_test_sourcelist(Tests ${KIT}CxxTests.cxx
  qMRMLNodeAttributeTableViewTest.cxx
  qMRMLNodeAttributeTableWidgetTest.cxx
  EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
  )

QT4_GENERATE_MOCS(
  qMRMLNodeAttributeTableViewTest.cxx
  qMRMLNodeAttributeTableWidgetTest.cxx
  )

include_directories( ${CMAKE_CURRENT_BINARY_DIR})

add_executable(${KIT}CxxTests ${Tests} ${Tests_MOC_CXX} ${Tests_UtilityFiles})
target_link_libraries(${KIT}CxxTests ${KIT})

SIMPLE_TEST( qMRMLNodeAttributeTableViewTest )
SIMPLE_TEST( qMRMLNodeAttributeTableWidgetTest )
