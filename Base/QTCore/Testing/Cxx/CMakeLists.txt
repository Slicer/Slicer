
if(BUILD_TESTING)

  set(KIT ${PROJECT_NAME})

  set(resources_dir ${CMAKE_CURRENT_BINARY_DIR}/Resources)
  file(MAKE_DIRECTORY ${resources_dir})

  # Download pre-built test extensions into build directory
  foreach(hash_and_filename IN ITEMS
      "5eb10b69fe164d9998be8cfd2a18a868afe0d9ac052f79fea9564ec3de6a8a07:30987-linux-amd64-ImageMaker-git6293f99-2017-07-03.tar.gz"
      "cd4434157d4846ee211ebe76ebc109e4ebf5dea6de06c146bfdd1366a7e32793:30987-linux-amd64-MarkupsToModel-git835453f-2022-05-11.tar.gz"
      "aa0c39c22027777f27205b671eff272466c75cb020d7c264a44b271b23b3e91a:30987-linux-amd64-CurveMaker-git5619cf5-2019-09-09.tar.gz"
      "66b6dfa2fd76b5d47f0a0eebabde1b61a7779b3c5c475243ead2e84fb5718d16:30987-macosx-amd64-ImageMaker-git6293f99-2017-07-03.tar.gz"
      "505556ddfac14d69017146663e18360c03afd972c5b906e14fe92c17917a1f45:30987-macosx-amd64-MarkupsToModel-git835453f-2022-05-11.tar.gz"
      "cd65433867aa1bfa9c7b57b193f3d6c01c9adb8eca8fb644147ec234fe1bcecb:30987-macosx-amd64-CurveMaker-git5619cf5-2019-09-09.tar.gz"
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
      "30987_imagemaker_linux_amd64_6293f99.json"
      "30987_markupstomodel_linux_amd64_835453f.json"
      "30987_curvemaker_linux_amd64_5619cf5.json"
      "30987_imagemaker_macosx_amd64_6293f99.json"
      "30987_markupstomodel_macosx_amd64_835453f.json"
      "30987_curvemaker_macosx_amd64_5619cf5.json"
      "30338_slicerheart_win_amd64_1db3627.json"
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
    qSlicerAbstractCoreModuleTest1.cxx
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
