
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
    qSlicerSslTest.cxx
    )
  if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
    list(APPEND KIT_TEST_SRCS
      qSlicerExtensionsManagerModelTest.cxx
      qSlicerPersistentCookieJarTest.cxx
      )
  endif()
  if(Slicer_USE_PYTHONQT)
    list(APPEND KIT_TEST_SRCS
      qSlicerCorePythonManagerWithoutApplicationTest.cxx
      )
  endif()
  create_test_sourcelist(Tests ${KIT}CxxTests.cxx
    ${KIT_TEST_SRCS}
    EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
    )

  QT4_GENERATE_MOCS(
    qSlicerSslTest.cxx
    )

  if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
    QT4_GENERATE_MOCS(
      qSlicerExtensionsManagerModelTest.cxx
      qSlicerPersistentCookieJarTest.cxx
      )
  endif()

  if(Slicer_USE_PYTHONQT)
    QT4_GENERATE_MOCS(
      qSlicerCorePythonManagerWithoutApplicationTest.cxx
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

  simple_test( qSlicerCoreApplicationTest1)
  set_property(TEST qSlicerCoreApplicationTest1 PROPERTY LABELS ${LIBRARY_NAME})
  simple_test( qSlicerCoreIOManagerTest1 ${CMAKE_SOURCE_DIR}/Libs/MRML/Core/Testing/name.mrml)
  set_property(TEST qSlicerCoreIOManagerTest1 PROPERTY LABELS ${LIBRARY_NAME})
  simple_test( qSlicerAbstractCoreModuleTest1 )
  simple_test( qSlicerLoadableModuleFactoryTest1 )
  simple_test( qSlicerUtilsTest1 )
  simple_test( qSlicerSslTest )

  if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
    simple_test( qSlicerExtensionsManagerModelTest )
    simple_test( qSlicerPersistentCookieJarTest )
  endif()

  if(Slicer_USE_PYTHONQT)
    simple_test( qSlicerCorePythonManagerWithoutApplicationTest )
  endif()

endif()


#
# Test templates - Common to both Slicer and the extensions
#
