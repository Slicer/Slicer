set(KIT ${PROJECT_NAME})

#-----------------------------------------------------------------------------
set(INPUT ${MRMLCore_SOURCE_DIR}/Testing)

#-----------------------------------------------------------------------------
set(KIT_TEST_SOURCES
  qMRMLMarkupsOptionsWidgetsFactoryTest.cxx
  qMRMLSimpleMarkupsWidgetTest1.cxx
  )

create_test_sourcelist(Tests ${KIT}CppTests.cxx
  ${KIT_TEST_SOURCES}
  )

set(Tests_UtilityFiles
  qMRMLMarkupsMalformedWidget.h
  qMRMLMarkupsMalformedWidget.cxx
  )

include_directories( ${CMAKE_CURRENT_BINARY_DIR})
include_directories( ${CMAKE_CURRENT_SOURCE_DIR})

ctk_add_executable_utf8(${KIT}CxxTests ${Tests} ${Tests_UtilityFiles})
target_link_libraries(${KIT}CxxTests ${KIT})

set_target_properties(${KIT}CxxTests PROPERTIES FOLDER "Module-${KIT}")

target_compile_definitions(${KIT}CxxTests PRIVATE
  $<$<BOOL:${MRML_WIDGETS_HAVE_QT5}>:MRML_WIDGETS_HAVE_QT5>
  )

# Configure CMake Qt automatic code generation
set_target_properties(${KIT}CxxTests PROPERTIES
  AUTOMOC ON
  )

#-----------------------------------------------------------------------------
simple_test( qMRMLMarkupsOptionsWidgetsFactoryTest )
simple_test( qMRMLSimpleMarkupsWidgetTest1 )
