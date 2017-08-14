
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
      qSlicerPersistentCookieJarTest.cxx
      )
  endif()
  if(Slicer_USE_PYTHONQT)
    list(APPEND KIT_TEST_SRCS
      qSlicerCorePythonManagerWithoutApplicationTest.cxx
      qSlicerScriptedUtilsTest1.cxx
      )
  endif()
  if(Slicer_USE_PYTHONQT_WITH_OPENSSL)
    list(APPEND KIT_TEST_SRCS
      qSlicerSslTest.cxx
      )
  endif()
  create_test_sourcelist(Tests ${KIT}CxxTests.cxx
    ${KIT_TEST_SRCS}
    EXTRA_INCLUDE vtkMRMLDebugLeaksMacro.h
    )

  set(KIT_TEST_GENERATE_MOC_SRCS
    qSlicerSslTest.cxx
    )

  if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
    list(APPEND KIT_TEST_GENERATE_MOC_SRCS
      qSlicerExtensionsManagerModelTest.cxx
      qSlicerPersistentCookieJarTest.cxx
      )
  endif()

  if(Slicer_USE_PYTHONQT)
    list(APPEND KIT_TEST_GENERATE_MOC_SRCS
      qSlicerCorePythonManagerWithoutApplicationTest.cxx
      )
  endif()

  set(KIT_TEST_TARGET_LIBRARIES ${KIT})
  if(CTK_QT_VERSION VERSION_LESS "5")
    QT4_GENERATE_MOCS(${KIT_TEST_GENERATE_MOC_SRCS})
    QT4_ADD_RESOURCES(Tests Resources/${KIT}Testing.qrc)
    list(APPEND KIT_TEST_TARGET_LIBRARIES Qt4::QtTest)
  else()
    QT5_GENERATE_MOCS(${KIT_TEST_GENERATE_MOC_SRCS})
    QT5_ADD_RESOURCES(Tests Resources/${KIT}Testing.qrc)
    list(APPEND KIT_TEST_TARGET_LIBRARIES Qt5::Test)
  endif()

  if(Slicer_USE_PYTHONQT)
  list(APPEND KIT_TEST_TARGET_LIBRARIES
    CTKScriptingPythonWidgets
    )
  endif()

  add_executable(${KIT}CxxTests ${Tests})
  target_link_libraries(${KIT}CxxTests ${KIT_TEST_TARGET_LIBRARIES})
  set_target_properties(${KIT}CxxTests PROPERTIES LABELS ${KIT})
  set_target_properties(${KIT}CxxTests PROPERTIES FOLDER "Core-Base")

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

  if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
    simple_test( qSlicerExtensionsManagerModelTest )
    simple_test( qSlicerPersistentCookieJarTest )
  endif()

  if(Slicer_USE_PYTHONQT)
    simple_test( qSlicerCorePythonManagerWithoutApplicationTest )
    simple_test( qSlicerScriptedUtilsTest1 )
  endif()

  if(Slicer_USE_PYTHONQT_WITH_OPENSSL)
    simple_test( qSlicerSslTest )
  endif()

endif()


#
# Test templates - Common to both Slicer and the extensions
#
