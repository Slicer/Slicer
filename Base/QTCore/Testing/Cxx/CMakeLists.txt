
if(BUILD_TESTING)

  # Code specfic to Slicer testing is added here.
  set(KIT QTCore)
  set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
  set(KIT_TEST_SRCS
    qSlicerAbstractCoreModuleTest1
    qSlicerCoreApplicationTest1.cxx
    qSlicerCoreIOManagerTest1.cxx
    qSlicerLoadableModuleFactoryTest1.cxx
    qSlicerUtilsTest1.cxx
    )
if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND KIT_TEST_SRCS
    qSlicerExtensionsManagerModelTest.cxx
    )
endif()
  create_test_sourcelist(Tests ${KIT}CxxTests.cxx
    ${KIT_TEST_SRCS}
    EXTRA_INCLUDE TestingMacros.h
    )

  set(TestsToRun ${Tests})
  list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

  set(LIBRARY_NAME ${PROJECT_NAME})

if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  QT4_GENERATE_MOCS(
    qSlicerExtensionsManagerModelTest.cxx
    )
endif()

  QT4_ADD_RESOURCES(Tests Resources/qSlicerBaseQTCoreTesting.qrc)

  add_executable(${KIT}CxxTests ${Tests})
  target_link_libraries(${KIT}CxxTests ${LIBRARY_NAME})
  set_target_properties(${KIT}CxxTests PROPERTIES LABELS ${LIBRARY_NAME})

  macro(SIMPLE_TEST TESTNAME)
    add_test(NAME ${TESTNAME} COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${KIT}CxxTests> ${TESTNAME} ${ARGN})
    set_property(TEST ${TESTNAME} PROPERTY LABELS ${LIBRARY_NAME})
  endmacro()

  #
  # Add Tests
  #

  # Remark: qSlicerModuleFactoryManager class is tested within Applications/SlicerQT/Testing

  SIMPLE_TEST( qSlicerCoreApplicationTest1)
  set_property(TEST qSlicerCoreApplicationTest1 PROPERTY LABELS ${LIBRARY_NAME})
  SIMPLE_TEST( qSlicerCoreIOManagerTest1 ${CMAKE_SOURCE_DIR}/Libs/MRML/Core/Testing/name.mrml)
  set_property(TEST qSlicerCoreIOManagerTest1 PROPERTY LABELS ${LIBRARY_NAME})
  SIMPLE_TEST( qSlicerAbstractCoreModuleTest1 )
if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  SIMPLE_TEST( qSlicerExtensionsManagerModelTest )
endif()
  SIMPLE_TEST( qSlicerLoadableModuleFactoryTest1 )
  SIMPLE_TEST( qSlicerUtilsTest1 )

endif()


#
# Test templates - Common to both Slicer and the extensions
#
