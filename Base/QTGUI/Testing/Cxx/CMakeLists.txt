if(BUILD_TESTING)
  # Code specfic to Slicer testing is added here.
  set(KIT ${PROJECT_NAME})
  set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  set(KIT_TEST_SRCS
    qSlicerAbstractModuleTest1.cxx
    qSlicerAbstractModuleWidgetTest1.cxx
    qSlicerApplicationTest1.cxx
    qSlicerDataDialogTest1.cxx
    qSlicerDirectoryListViewTest1.cxx
    qSlicerLayoutManagerTest1.cxx
    qSlicerModulePanelTest1.cxx
    qSlicerMouseModeToolBarTest1.cxx
    qSlicerSaveDataDialogCustomFileWriterTest.cxx
    qSlicerWidgetTest1.cxx
    qSlicerWidgetTest2.cxx
    )
  if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
    list(APPEND KIT_TEST_SRCS
      )
  endif()
  if(Slicer_USE_PYTHONQT)
    list(APPEND KIT_TEST_SRCS
      qSlicerLoadableModuleWithPythonTest.cxx
      qSlicerPythonManagerWithoutApplicationTest.cxx
      qSlicerScriptedLoadableModuleTest.cxx
      qSlicerScriptedLoadableModuleWidgetTest.cxx
      )
  endif()
  create_test_sourcelist(Tests ${KIT}CxxTests.cxx
    ${KIT_TEST_SRCS}
    EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
    )

  set(KIT_TEST_MOC_SRCS
    )
  if(Slicer_USE_PYTHONQT)
    list(APPEND KIT_TEST_MOC_SRCS
      qSlicerLoadableModuleWithPythonTest.cxx
      qSlicerPythonManagerWithoutApplicationTest.cxx
      qSlicerScriptedLoadableModuleTest.cxx
      qSlicerScriptedLoadableModuleWidgetTest.cxx
      )
    QT4_GENERATE_MOCS(${KIT_TEST_MOC_SRCS})
  endif()

  QT4_ADD_RESOURCES(Tests Resources/${KIT}Testing.qrc)

  add_executable(${KIT}CxxTests ${Tests})
  target_link_libraries(${KIT}CxxTests ${KIT})
  set_target_properties(${KIT}CxxTests PROPERTIES LABELS ${KIT})
  set_target_properties(${KIT}CxxTests PROPERTIES FOLDER "Core-Base")

  #
  # Add Tests
  #

  simple_test( qSlicerApplicationTest1 )
  simple_test( qSlicerAbstractModuleTest1 )
  simple_test( qSlicerAbstractModuleWidgetTest1 )
  simple_test( qSlicerDataDialogTest1 )
  simple_test( qSlicerDirectoryListViewTest1 )
  simple_test( qSlicerLayoutManagerTest1 )
  simple_test( qSlicerModulePanelTest1 )
  simple_test( qSlicerMouseModeToolBarTest1 )
  simple_test( qSlicerSaveDataDialogCustomFileWriterTest )
  simple_test( qSlicerWidgetTest1 )
  simple_test( qSlicerWidgetTest2 )

  if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  endif()

  if(Slicer_USE_PYTHONQT)
    simple_test( qSlicerLoadableModuleWithPythonTest )
    simple_test( qSlicerPythonManagerWithoutApplicationTest )
    simple_test( qSlicerScriptedLoadableModuleTest )
    simple_test( qSlicerScriptedLoadableModuleWidgetTest )
  endif()

endif()

#
# Test templates - Common to both Slicer and the extensions
#

set(Slicer_CXX_MODULE_TEST_TEMPLATES_DIR ${CMAKE_CURRENT_SOURCE_DIR}
  CACHE INTERNAL "Path to cxx test templates directory.")

# Install the file so that project building against the 'installed tree' can reference it.
if(NOT Slicer_INSTALL_NO_DEVELOPMENT)
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/qSlicerUtilsTest1.cxx.in
    DESTINATION ${Slicer_INSTALL_SHARE_DIR} COMPONENT Development
    )
endif()
