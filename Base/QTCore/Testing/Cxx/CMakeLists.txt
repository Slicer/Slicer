
if(BUILD_TESTING)

  set(KIT ${PROJECT_NAME})

  set(resources_dir ${CMAKE_CURRENT_BINARY_DIR}/Resources)
  file(MAKE_DIRECTORY ${resources_dir})

  # Download pre-built test extensions into build directory
  foreach(hash_and_filename IN ITEMS
      "01fd0b39e96da9e85b841273173001958a5feb219875deee0719a74e36b02652:19354-linux-amd64-CLIExtensionTemplate-svn19354-2012-02-23.tar.gz"
      "87105840a5b6a5efee1e682df07957d880e23e131791c3b53746e0a4e6c2dfc8:19354-linux-amd64-LoadableExtensionTemplate-svn19354-2012-02-23.tar.gz"
      "dd619c9bbd8f058f85c124d775fe5fec1883f05ce4c1025c0f245db1ec088b51:19354-linux-amd64-ScriptedLoadableExtensionTemplate-svn19354-2012-02-23.tar.gz"
      "8e047014e0d5addf1c8c5d92c20069e2c6d5c9893a670c4731b4c4e45cac7b21:19354-linux-amd64-SuperBuildLoadableExtensionTemplate-svn19354-2012-02-23.tar.gz"
      "57d76d876cb96bde01d30a16d42316941209c8b6c22ad311d623d918597141c0:19615-macosx-amd64-CLIExtensionTemplate-svn19615-2012-03-18.tar.gz"
      "3ce4a2f0485c9f5db56fb48727421fbe9ccef213c345175e8defb573ce2113da:19615-macosx-amd64-LoadableExtensionTemplate-svn19615-2012-03-18.tar.gz"
      "d0d9ee8477a9e29c7b2823c2b7ece97183209e14375aa44ae4cc1f2f3a18b8c1:19615-macosx-amd64-ScriptedLoadableExtensionTemplate-svn19615-2012-03-18.tar.gz"
      "15b83e8e00f530f2fea3ed436587ad248b5b008ca04d0af13ef595e5574ac36f:19615-macosx-amd64-SuperBuildLoadableExtensionTemplate-svn19615-2012-03-18.tar.gz"
      )
    string(REPLACE ":" ";" hash_and_filename ${hash_and_filename})
    list(GET hash_and_filename 0 _hash)
    list(GET hash_and_filename 1 _filename)
    file(DOWNLOAD
      https://github.com/Slicer/SlicerTestingData/releases/download/SHA256/${_hash}
      ${resources_dir}/${_filename}
      EXPECTED_HASH SHA256=${_hash}
      )
  endforeach()

  # Copy json files into build directory
  foreach(filename IN ITEMS
      "19354-linux-amd64-CLIExtensionTemplate-svn19354-2012-02-23.json"
      "19354-linux-amd64-LoadableExtensionTemplate-svn19354-2012-02-23.json"
      "19354-linux-amd64-ScriptedLoadableExtensionTemplate-svn19354-2012-02-23.json"
      "19354-linux-amd64-SuperBuildLoadableExtensionTemplate-svn19354-2012-02-23.json"
      "19615-macosx-amd64-CLIExtensionTemplate-svn19615-2012-03-18.json"
      "19615-macosx-amd64-LoadableExtensionTemplate-svn19615-2012-03-18.json"
      "19615-macosx-amd64-ScriptedLoadableExtensionTemplate-svn19615-2012-03-18.json"
      "19615-macosx-amd64-SuperBuildLoadableExtensionTemplate-svn19615-2012-03-18.json"
      )
    configure_file(
      ../Data/Input/${filename}
      ${resources_dir}/${filename}
      COPYONLY
      )
  endforeach()

  # Configure testing resources file
  configure_file(
    Resources/${KIT}Testing.qrc
    ${resources_dir}/${KIT}Testing.qrc
    )

  # Code specific to Slicer testing is added here.
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
    QT5_GENERATE_MOCS(${KIT_TEST_GENERATE_MOC_SRCS})
    QT5_ADD_RESOURCES(Tests ${resources_dir}/${KIT}Testing.qrc)
    list(APPEND KIT_TEST_TARGET_LIBRARIES Qt5::Test)

  if(Slicer_USE_PYTHONQT)
  list(APPEND KIT_TEST_TARGET_LIBRARIES
    CTKScriptingPythonWidgets
    )
  endif()

  ctk_add_executable_utf8(${KIT}CxxTests ${Tests})
  target_link_libraries(${KIT}CxxTests ${KIT_TEST_TARGET_LIBRARIES})
  set_target_properties(${KIT}CxxTests PROPERTIES LABELS ${KIT})
  set_target_properties(${KIT}CxxTests PROPERTIES FOLDER "Core-Base")

  #
  # Add Tests
  #

  # Remark: qSlicerModuleFactoryManager class is tested within Applications/SlicerApp/Testing

  simple_test( qSlicerCoreApplicationTest1)
  set_property(TEST qSlicerCoreApplicationTest1 PROPERTY LABELS ${LIBRARY_NAME})
  simple_test( qSlicerCoreIOManagerTest1 )
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
