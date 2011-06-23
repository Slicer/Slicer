
if(BUILD_TESTING)
  # Code specfic to Slicer testing is added here.
  set(KIT QTCore)
  set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "DEBUG_LEAKS_ENABLE_EXIT_ERROR();" )
  create_test_sourcelist(Tests ${KIT}CxxTests.cxx
    qSlicerAbstractCoreModuleTest1
    qSlicerCoreApplicationTest1.cxx
    qSlicerCoreIOManagerTest1.cxx
    qSlicerLoadableModuleFactoryTest1.cxx
    qSlicerUtilsTest1.cxx
    EXTRA_INCLUDE TestingMacros.h
    )

  set(TestsToRun ${Tests})
  list(REMOVE_ITEM TestsToRun ${KIT}CxxTests.cxx)

  set(LIBRARY_NAME ${PROJECT_NAME})

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
  SIMPLE_TEST( qSlicerCoreIOManagerTest1 ${CMAKE_SOURCE_DIR}/Libs/MRML/Testing/name.mrml)
  set_property(TEST qSlicerCoreIOManagerTest1 PROPERTY LABELS ${LIBRARY_NAME})
  SIMPLE_TEST( qSlicerAbstractCoreModuleTest1 )
  SIMPLE_TEST( qSlicerLoadableModuleFactoryTest1 )
  SIMPLE_TEST( qSlicerUtilsTest1 )

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
