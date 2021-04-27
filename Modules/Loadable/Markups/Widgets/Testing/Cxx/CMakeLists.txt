set(KIT ${PROJECT_NAME})

#-----------------------------------------------------------------------------
set(INPUT ${MRMLCore_SOURCE_DIR}/Testing)

#-----------------------------------------------------------------------------
set(KIT_TEST_SOURCES
  qSlicerMarkupsAdditionalOptionsWidgetsFactoryTest.cxx
  qSlicerSimpleMarkupsWidgetTest1.cxx
  )

create_test_sourcelist(Tests ${KIT}CppTests.cxx
  ${KIT_TEST_SOURCES}
  )

set(Tests_MOC_SRCS
  qSlicerMarkupsMalformedWidget.h
  )
set(Tests_UtilityFiles
  qSlicerMarkupsMalformedWidget.h
  qSlicerMarkupsMalformedWidget.cxx
  )

set(KIT_TEST_GENERATE_MOC_SRCS
  qSlicerMarkupsAdditionalOptionsWidgetsFactoryTest.cxx
  )

set(_moc_options OPTIONS -DMRML_WIDGETS_HAVE_QT5)
QT5_WRAP_CPP(Tests_MOC_CXX ${Tests_MOC_SRCS} ${_moc_options})
QT5_GENERATE_MOCS(${KIT_TEST_GENERATE_MOC_SRCS})

include_directories( ${CMAKE_CURRENT_BINARY_DIR})
include_directories( ${CMAKE_CURRENT_SOURCE_DIR})

ctk_add_executable_utf8(${KIT}CxxTests ${Tests} ${Tests_MOC_CXX} ${Tests_UtilityFiles})
target_link_libraries(${KIT}CxxTests ${KIT})

set_target_properties(${KIT}CxxTests PROPERTIES FOLDER "Module-${KIT}")

#-----------------------------------------------------------------------------
simple_test( qSlicerMarkupsAdditionalOptionsWidgetsFactoryTest )
simple_test( qSlicerSimpleMarkupsWidgetTest1 )
