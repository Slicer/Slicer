set(KIT ${PROJECT_NAME})

set(TEST_SOURCES
  qMRMLColorListViewTest1.cxx
  qMRMLColorPickerWidgetTest1.cxx
  qMRMLColorPickerWidgetTest2.cxx
  qMRMLColorPickerWidgetTest3.cxx
  qMRMLColorTableViewTest1.cxx
  )

if(Slicer_USE_QtTesting)
  list(APPEND TEST_SOURCES
    qMRMLColorListViewEventTranslatorPlayerTest1.cxx
    qMRMLColorPickerWidgetEventTranslatorPlayerTest1.cxx
    qMRMLColorTableViewEventTranslatorPlayerTest1.cxx
    )
endif()

create_test_sourcelist(Tests ${KIT}CppTests.cxx
  ${TEST_SOURCES}
  )

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

ctk_add_executable_utf8(${KIT}CxxTests ${Tests} ${Tests_MOC_CXX})
target_link_libraries(${KIT}CxxTests ${KIT})

#-----------------------------------------------------------------------------
simple_test( qMRMLColorListViewTest1 )
simple_test( qMRMLColorPickerWidgetTest1 )
simple_test( qMRMLColorPickerWidgetTest2 )
simple_test( qMRMLColorPickerWidgetTest3 )
simple_test( qMRMLColorTableViewTest1 )

if(Slicer_USE_QtTesting)
  function(QtTesting_TEST testname)
    if(${ARGC} GREATER 1)
      set(SCENEFILENAME ${ARGV1})

      # Extract list of external files to download. Note that the ${_externalfiles} variable
      # is only specified to trigger download of data files used in the scene, the arguments
      # are NOT explicitly used in ${KIT}CxxTests test executable.
      string(REPLACE "|" ";" scenefilename_and_externalfiles ${SCENEFILENAME})
      list(GET scenefilename_and_externalfiles 0 SCENEFILENAME)
      list(LENGTH scenefilename_and_externalfiles _length)
      if(_length EQUAL 2)
        configure_file(
          ${MRMLCore_SOURCE_DIR}/Testing/${SCENEFILENAME}
          ${MRMLCore_BINARY_DIR}/Testing/${SCENEFILENAME}
          COPYONLY
          )
        set(_scenefile ${MRMLCore_BINARY_DIR}/Testing/${SCENEFILENAME})
        list(GET scenefilename_and_externalfiles 1 _externalfiles)
      else()
        set(_scenefile ${MRMLCore_SOURCE_DIR}/Testing/${SCENEFILENAME})
        set(_externalfiles "")
      endif()

    else()
      set(_scenefile "")
    endif()
    simple_test(${testname} ${Slicer_SOURCE_DIR} ${_scenefile})
    set_tests_properties(${testname} PROPERTIES RUN_SERIAL ON)
  endfunction()
  QtTesting_TEST(qMRMLColorListViewEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLColorPickerWidgetEventTranslatorPlayerTest1)
  QtTesting_TEST(qMRMLColorTableViewEventTranslatorPlayerTest1)
endif()
