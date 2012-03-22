
if(BUILD_TESTING)

  # Code specfic to Slicer testing is added here.
  set(KIT ${PROJECT_NAME})
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
    EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
    )

  if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
    QT4_GENERATE_MOCS(
      qSlicerExtensionsManagerModelTest.cxx
      )
  endif()

  QT4_ADD_RESOURCES(Tests Resources/${KIT}Testing.qrc)

  add_executable(${KIT}CxxTests ${Tests})
  target_link_libraries(${KIT}CxxTests ${KIT})
  set_target_properties(${KIT}CxxTests PROPERTIES LABELS ${KIT})

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
