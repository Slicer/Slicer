if(BUILD_TESTING)
  # Code specific to Slicer testing is added here.
  set(KIT ${PROJECT_NAME})




  #-----------------------------------------------------------------------------
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
  set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN  "
    DEBUG_LEAKS_ENABLE_EXIT_ERROR();
    TESTING_OUTPUT_INIT();
    itk::itkFactoryRegistration();
    ")

  set(CMAKE_TESTDRIVER_AFTER_TESTMAIN  "
    TESTING_OUTPUT_ASSERT_WARNINGS_ERRORS(0);
    ")

  set(EXTRA_INCLUDE "vtkMRMLDebugLeaksMacro.h\"\n\#include <itkConfigure.h>\n\#include <itkFactoryRegistration.h>\n\#include \"vtkTestingOutputWindow.h")

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
    EXTRA_INCLUDE ${EXTRA_INCLUDE}
    )

  set(KIT_TEST_GENERATE_MOC_SRCS
    )
  if(Slicer_USE_PYTHONQT)
    list(APPEND KIT_TEST_GENERATE_MOC_SRCS
      qSlicerLoadableModuleWithPythonTest.cxx
      qSlicerPythonManagerWithoutApplicationTest.cxx
      qSlicerScriptedLoadableModuleTest.cxx
      qSlicerScriptedLoadableModuleWidgetTest.cxx
      )
  endif()

    if(KIT_TEST_GENERATE_MOC_SRCS)
      QT5_GENERATE_MOCS(${KIT_TEST_GENERATE_MOC_SRCS})
    endif()
    QT5_ADD_RESOURCES(Tests Resources/${KIT}Testing.qrc)

  ctk_add_executable_utf8(${KIT}CxxTests ${Tests})
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
  simple_test( qSlicerWidgetTest2 DATA{${CMAKE_SOURCE_DIR}/Libs/MRML/Core/Testing/TestData/fixed.nrrd} )

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

