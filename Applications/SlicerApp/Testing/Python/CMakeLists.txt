
#-----------------------------------------------------------------------------
set(INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../../Data/Input)

#
# Check if '--python-code' / 'slicer.testing' works as expected
#

add_test(
  NAME py_nomainwindow_SlicerPythonCodeExitSuccessTest
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${APP_TARGET_NAME}>
  --disable-modules --ignore-slicerrc --no-main-window
  --python-code "exit(slicer.util.EXIT_SUCCESS)"
  )
add_test(
  NAME py_nomainwindow_testing_SlicerPythonCodeExitSuccessTest
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${APP_TARGET_NAME}>
  --testing --disable-modules --no-main-window
  --python-code "exit(slicer.util.EXIT_SUCCESS)"
  )

add_test(
  NAME py_nomainwindow_SlicerPythonCodeExitFailureTest
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${APP_TARGET_NAME}>
  --disable-modules --ignore-slicerr --no-main-windowc
  --python-code "exit(slicer.util.EXIT_FAILURE)"
  )
add_test(
  NAME py_nomainwindow_testing_SlicerPythonCodeExitFailureTest
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${APP_TARGET_NAME}>
  --testing --disable-modules --no-main-window
  --python-code "exit(slicer.util.EXIT_FAILURE)"
  )
set_tests_properties(
    py_nomainwindow_SlicerPythonCodeExitFailureTest
    py_nomainwindow_testing_SlicerPythonCodeExitFailureTest
  PROPERTIES WILL_FAIL TRUE
  )

add_test(
  NAME py_nomainwindow_SlicerPythonCodeTest3
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${APP_TARGET_NAME}>
  --testing --disable-modules --no-main-window
  --python-code "import slicer.testing; slicer.testing.exitSuccess()"
  )

add_test(
  NAME py_nomainwindow_SlicerPythonCodeTest4
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${APP_TARGET_NAME}>
  --testing --disable-modules --no-main-window
  --python-code "import slicer.testing; slicer.testing.exitFailure('Should fail !')"
  )
set_tests_properties(py_nomainwindow_SlicerPythonCodeTest4 PROPERTIES WILL_FAIL TRUE)

add_test(
  NAME py_nomainwindow_SlicerPythonCodeTest5
  COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${APP_TARGET_NAME}>
  --testing --disable-modules --no-main-window
  --python-code "raise Exception('Should fail !')"
  )
set_tests_properties(py_nomainwindow_SlicerPythonCodeTest5 PROPERTIES WILL_FAIL TRUE)

#
# Check if '--python-scripts' / 'testing' works as expected
#

slicer_add_python_test(
  SCRIPT SlicerTestingExitSuccessTest.py
  SCRIPT_ARGS --no-main-window TESTNAME_PREFIX nomainwindow_
  )

if (WIN32)
  # Test script execution with native path separator character (backslash)
  string(REPLACE "/" "\\\\" nativeFilePath "${CMAKE_CURRENT_SOURCE_DIR}/bSlicerTestingExitSuccessTest.py")
  slicer_add_python_test(
    SCRIPT ${nativeFilePath}
    SCRIPT_ARGS --no-main-window TESTNAME_PREFIX nomainwindow_nativePath_
    )
endif()

slicer_add_python_test(
  SCRIPT SlicerTestingExitFailureTest.py
  SCRIPT_ARGS --no-main-window TESTNAME_PREFIX nomainwindow_
  )
set_tests_properties(py_nomainwindow_SlicerTestingExitFailureTest PROPERTIES WILL_FAIL TRUE)

slicer_add_python_test(SCRIPT SlicerTestingExitSuccessTest.py)

slicer_add_python_test(SCRIPT SlicerTestingExitFailureTest.py)
set_tests_properties(py_SlicerTestingExitFailureTest PROPERTIES WILL_FAIL TRUE)

#
# Check if 'slicer.testing.runUnitTest()' works as expected
#

slicer_add_python_unittest(
  SCRIPT SlicerUnitTestTest.py
  SLICER_ARGS --no-main-window --disable-modules
  TESTNAME_PREFIX nomainwindow_
  )

slicer_add_python_unittest(
  SCRIPT SlicerUnitTestWithErrorsTest.py
  SLICER_ARGS --no-main-window --disable-modules
  TESTNAME_PREFIX nomainwindow_
  )
set_tests_properties(py_nomainwindow_SlicerUnitTestWithErrorsTest PROPERTIES WILL_FAIL TRUE)

#
# Exercise different Slicer command line option and check that no warnings are displayed.
#
macro(nowarning_test test_name)
  add_test(
    NAME py_nowarning_${test_name}Test
    COMMAND ${Slicer_LAUNCH_COMMAND} $<TARGET_FILE:${APP_TARGET_NAME}>
    ${ARGN} --testing --python-code "exit(slicer.util.EXIT_SUCCESS)"
    )
  set(fail_regexp)
  list(APPEND fail_regexp "Failed to obtain reference to")
  list(APPEND fail_regexp "Exception AttributeError: .* instance has no attribute")
  set_tests_properties(py_nowarning_${test_name}Test PROPERTIES FAIL_REGULAR_EXPRESSION "${fail_regexp}")
endmacro()
nowarning_test(mainwindow --no-splash)
nowarning_test(mainwindow_application_info --application-information)
nowarning_test(mainwindow_python_interactor --show-python-interactor)
nowarning_test(mainwindow_nocli --disable-cli-modules)
nowarning_test(mainwindow_noloadable --disable-loadable-modules)
nowarning_test(mainwindow_noscripted --disable-scripted-loadable-modules)
nowarning_test(mainwindow_nocli_noloadable --disable-cli-modules --disable-loadable-modules)
nowarning_test(mainwindow_nocli_noscripted --disable-cli-modules --disable-scripted-loadable-modules)
nowarning_test(mainwindow_noloadable_noscripted --disable-loadable-modules --disable-scripted-loadable-modules)
nowarning_test(mainwindow_nocli_noloadable_noscripted --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules)
nowarning_test(mainwindow_nomodules --disable-modules)
nowarning_test(nomainwindow --no-main-window)
nowarning_test(nomainwindow_application_info --no-main-window --application-information)
nowarning_test(nomainwindow_python_interactor --no-main-window --show-python-interactor)
nowarning_test(nomainwindow_nocli --no-main-window --disable-cli-modules)
nowarning_test(nomainwindow_noloadable --no-main-window --disable-loadable-modules)
nowarning_test(nomainwindow_noscripted --no-main-window --disable-scripted-loadable-modules)
nowarning_test(nomainwindow_nocli_noloadable --no-main-window --disable-cli-modules --disable-loadable-modules)
nowarning_test(nomainwindow_nocli_noscripted --no-main-window --disable-cli-modules --disable-scripted-loadable-modules)
nowarning_test(nomainwindow_noloadable_noscripted --no-main-window --disable-loadable-modules --disable-scripted-loadable-modules)
nowarning_test(nomainwindow_nocli_noloadable_noscripted --no-main-window --disable-cli-modules --disable-loadable-modules --disable-scripted-loadable-modules)
nowarning_test(nomainwindow_nomodules --no-main-window --disable-modules)
nowarning_test(nomainwindow_ignoreslicerrc --no-main-window --ignore-slicerrc)

#
# Test Slicer command line options
#

add_test(
  NAME py_nomainwindow_SlicerOptionDisableSettingsTest
  COMMAND ${PYTHON_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/SlicerOptionDisableSettingsTest.py
    ${Slicer_LAUNCHER_EXECUTABLE}
  )

add_test(
  NAME py_nomainwindow_SlicerOptionIgnoreSlicerRCTest
  COMMAND ${PYTHON_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/SlicerOptionIgnoreSlicerRCTest.py
    ${Slicer_LAUNCHER_EXECUTABLE}
  )

if(UNIX)
  add_test(
    NAME py_nomainwindow_SlicerOptionModulesToIgnoreTest
    COMMAND ${PYTHON_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/SlicerOptionModulesToIgnoreTest.py
      ${Slicer_LAUNCHER_EXECUTABLE}
      ${Slicer_BINARY_DIR}/bin/slicerExtensionWizard
    )
endif()

if(Slicer_BUILD_PARAMETERSERIALIZER_SUPPORT)
  ExternalData_add_test(${Slicer_ExternalData_DATA_MANAGEMENT_TARGET}
    NAME py_nomainwindow_CLISerializationTest
    COMMAND ${PYTHON_EXECUTABLE}
      ${CMAKE_CURRENT_SOURCE_DIR}/CLISerializationTest.py
      ${Slicer_LAUNCHER_EXECUTABLE}
      ${Slicer_SOURCE_DIR}/Testing/Data/Input
      DATA{${INPUT}/MRHeadResampled.nhdr,MRHeadResampled.raw.gz}
      DATA{${INPUT}/CTHeadAxial.nhdr,CTHeadAxial.raw.gz}
      ${Slicer_BINARY_DIR}/Testing/Temporary
    )
endif()

#
# Check if scripted module import works as expected
#

set(testname ScriptedModuleDiscoveryTest)
slicer_add_python_test(
  SCRIPT ${testname}.py
  SLICER_ARGS --no-main-window --disable-builtin-modules --additional-module-path ${CMAKE_CURRENT_SOURCE_DIR}/ScriptedModuleDiscoveryTest
  TESTNAME_PREFIX nomainwindow_
  )
if(UNIX)
set_tests_properties(py_nomainwindow_${testname} PROPERTIES
  PASS_REGULAR_EXPRESSION "ModuleAWidget finalized"
  )
endif()

#
# Check that an exception raised during scripted module cleanup sets exit code
#
set(testname ScriptedModuleCleanupTest_with_testing)
add_test(
  NAME py_${testname}
  COMMAND ${PYTHON_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/ScriptedModuleCleanupTest.py ${Slicer_LAUNCHER_EXECUTABLE} --with-testing
  )

set(testname ScriptedModuleCleanupTest_without_testing)
add_test(
  NAME py_${testname}
  COMMAND ${PYTHON_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/ScriptedModuleCleanupTest.py ${Slicer_LAUNCHER_EXECUTABLE}
  )

#
# Application tests
#
slicer_add_python_unittest(
  SCRIPT SlicerSceneObserverTest.py
  SLICER_ARGS --no-main-window --disable-modules --ignore-slicerrc
  TESTNAME_PREFIX nomainwindow_
  )

if(VTK_DEBUG_LEAKS AND Slicer_HAS_CONSOLE_IO_SUPPORT)
  set(testname MRMLCreateNodeByClassWithoutSetReferenceCount)
  slicer_add_python_test(
    SCRIPT ${testname}.py
    SCRIPT_ARGS --no-main-window --disable-modules
    TESTNAME_PREFIX nomainwindow_
    )
  set_tests_properties(py_nomainwindow_${testname} PROPERTIES
    PASS_REGULAR_EXPRESSION "instances? still around"
    )
endif()

set(testname MRMLCreateNodeByClassWithSetReferenceCountMinusOne)
slicer_add_python_test(
  SCRIPT ${testname}.py
  SCRIPT_ARGS --no-main-window --disable-modules
  TESTNAME_PREFIX nomainwindow_
  )

set(testname MRMLSceneImportAndExport)
slicer_add_python_test(
  SCRIPT ${testname}.py
  TESTNAME_PREFIX mainwindow_
  )

if(Slicer_USE_SimpleITK)
  slicer_add_python_unittest(
    SCRIPT ${Slicer_SOURCE_DIR}/Base/Python/tests/test_sitkUtils.py
    SLICER_ARGS --no-main-window
    TESTNAME_PREFIX nomainwindow_
    )
endif()

slicer_add_python_unittest(
  SCRIPT ${Slicer_SOURCE_DIR}/Base/Python/tests/test_PythonManager.py
  SLICER_ARGS --no-main-window --disable-modules
  TESTNAME_PREFIX nomainwindow_
  )

slicer_add_python_unittest(
  SCRIPT ${Slicer_SOURCE_DIR}/Base/Python/slicer/tests/test_slicer_util_without_modules.py
  SLICER_ARGS --no-main-window  --disable-modules
  TESTNAME_PREFIX nomainwindow_
  )

slicer_add_python_unittest(
  SCRIPT ${Slicer_SOURCE_DIR}/Base/Python/slicer/tests/test_slicer_util_save.py
  SLICER_ARGS --no-main-window --disable-cli-modules --disable-scripted-loadable-modules DATA{${INPUT}/MR-head.nrrd}
  TESTNAME_PREFIX nomainwindow_
  )

slicer_add_python_unittest(
  SCRIPT ${Slicer_SOURCE_DIR}/Base/Python/slicer/tests/test_slicer_util_getNodes.py
  SLICER_ARGS --no-main-window --disable-modules
  TESTNAME_PREFIX nomainwindow_
  )

## Test reading MGH file format types.
slicer_add_python_unittest(
  SCRIPT ${Slicer_SOURCE_DIR}/Base/Python/slicer/tests/test_slicer_mgh.py
  SLICER_ARGS --no-main-window --disable-cli-modules --disable-scripted-loadable-modules
              DATA{${INPUT}/T1.mgz}
              DATA{${INPUT}/T1_longname.mgh.gz}
              DATA{${INPUT}/T1_uncompressed.mgh}
  TESTNAME_PREFIX nomainwindow_
  )

## Test reading MINC file format types.
slicer_add_python_unittest(
  SCRIPT ${Slicer_SOURCE_DIR}/Base/Python/slicer/tests/test_slicer_minc.py
  SLICER_ARGS --no-main-window --disable-cli-modules --disable-scripted-loadable-modules
              DATA{${INPUT}/pd_z-_float_xyz.mnc}
  TESTNAME_PREFIX nomainwindow_
  )

slicer_add_python_unittest(
  SCRIPT ${Slicer_SOURCE_DIR}/Base/Python/slicer/tests/test_slicer_python_sqlite3.py
  SLICER_ARGS --no-main-window  --disable-modules
  TESTNAME_PREFIX nomainwindow_
  )

slicer_add_python_unittest(
  SCRIPT ${Slicer_SOURCE_DIR}/Base/Python/slicer/tests/test_slicer_python_lzma.py
  SLICER_ARGS --no-main-window  --disable-modules
  TESTNAME_PREFIX nomainwindow_
  )

slicer_add_python_unittest(
  SCRIPT ${Slicer_SOURCE_DIR}/Base/Python/slicer/tests/test_slicer_util_VTKObservationMixin.py
  SLICER_ARGS --no-main-window --disable-modules
  TESTNAME_PREFIX nomainwindow_
  )

slicer_add_python_unittest(
  SCRIPT ${Slicer_SOURCE_DIR}/Base/Python/slicer/tests/test_slicer_environment.py
  SLICER_ARGS --no-main-window --disable-modules
  TESTNAME_PREFIX nomainwindow_
  )

# Test multi-shell DWI I/O
slicer_add_python_test(
  SCRIPT DWMRIMultishellIOTests.py
  SLICER_ARGS --no-main-window --disable-modules --testing
  SCRIPT_ARGS
    ${Slicer_SOURCE_DIR}/
    ${Slicer_BINARY_DIR}/Testing/Temporary/
  TESTNAME_PREFIX nomainwindow_
  )

# DCMTKPrivateDictionary test
slicer_add_python_test(
  SCRIPT DCMTKPrivateDictTest.py
  SLICER_ARGS --no-main-window --disable-modules
  SCRIPT_ARGS ${Slicer_SOURCE_DIR}/Testing/Data/Input/philips_ingenia_4_1_3_dwi.dcm
  TESTNAME_PREFIX nomainwindow_
  )

if(Slicer_USE_PYTHONQT_WITH_OPENSSL)
  add_test(
    NAME py_SlicerSslTest
    COMMAND ${Slicer_LAUNCHER_EXECUTABLE}
    --ignore-slicerrc -c "from urllib.request import urlopen; urlopen('https://www.eff.org/https-everywhere')"
    )
  add_test(
    NAME py_PythonSslTest
    COMMAND ${PYTHON_EXECUTABLE}
    -c "from urllib.request import urlopen; urlopen('https://www.eff.org/https-everywhere')"
    )
endif()

# Check that python standard library modules can be imported
foreach(stdlib_module IN ITEMS
    "bz2"
    )
  add_test(
    NAME py_test_stdlib_import_${stdlib_module}_using_slicer
    COMMAND ${Slicer_LAUNCHER_EXECUTABLE}
    -c "import ${stdlib_module}" --no-splash --ignore-slicerrc --disable-settings
    )
  add_test(
    NAME py_test_stdlib_import_${stdlib_module}_using_python
    COMMAND ${PYTHON_EXECUTABLE}
    -c "import ${stdlib_module}"
    )
endforeach()

# Check that application signal "startupCompleted()" is emitted
add_test(
  NAME py_startupcompleted_signal_emitted
  COMMAND ${PYTHON_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/SlicerStartupCompletedTest.py
    ${Slicer_LAUNCHER_EXECUTABLE}
  )

# Check that stand-alone Python interpreter, without PythonQt support, can `import slicer` safely (#945)
add_test(
  NAME py_standalonepython_import_slicer
  COMMAND ${Slicer_LAUNCHER_EXECUTABLE} --launch
    ${PYTHON} ${CMAKE_CURRENT_SOURCE_DIR}/StandalonePythonImportSlicerTest.py
  )

#
# SelfTests
# see http://wiki.slicer.org/slicerWiki/index.php/Documentation/Nightly/Developers/Tutorials/SelfTestModule
#

if(Slicer_USE_QtTesting AND Slicer_USE_PYTHONQT)

  # add as unit test for use at build/test time
  slicer_add_python_unittest(SCRIPT AtlasTests.py)
  slicer_add_python_unittest(SCRIPT DICOMReaders.py)
  slicer_add_python_unittest(SCRIPT KneeAtlasTest.py)
  slicer_add_python_unittest(SCRIPT sceneImport2428.py)
  slicer_add_python_unittest(SCRIPT SlicerMRBMultipleSaveRestoreLoopTest.py)
  slicer_add_python_unittest(SCRIPT SlicerMRBMultipleSaveRestoreTest.py)
  slicer_add_python_unittest(SCRIPT SlicerMRBSaveRestoreCheckPathsTest.py)
  slicer_add_python_unittest(SCRIPT Slicer4Minute.py)
  slicer_add_python_unittest(SCRIPT SlicerBoundsTest.py)
  slicer_add_python_unittest(SCRIPT Charting.py)
  if(Slicer_BUILD_WEBENGINE_SUPPORT)
    slicer_add_python_unittest(SCRIPT WebEngine.py)
  endif()
  slicer_add_python_unittest(SCRIPT SliceLinkLogic.py)
  slicer_add_python_unittest(SCRIPT ScenePerformance.py)
  slicer_add_python_unittest(SCRIPT SlicerCreateRulerCrashIssue4199.py)
  slicer_add_python_unittest(SCRIPT SlicerRestoreSceneViewCrashIssue3445.py)
  slicer_add_python_unittest(SCRIPT RSNAVisTutorial.py)
  slicer_add_python_unittest(SCRIPT RSNAQuantTutorial.py)
  slicer_add_python_unittest(SCRIPT slicerCloseCrashBug2590.py)
  slicer_add_python_unittest(SCRIPT SlicerOrientationSelectorTest.py)
  slicer_add_python_unittest(SCRIPT SlicerTransformInteractionTest1.py)
  slicer_add_python_unittest(SCRIPT UtilTest.py)
  slicer_add_python_unittest(SCRIPT ViewControllersSliceInterpolationBug1926.py)
  slicer_add_python_unittest(SCRIPT RSNA2012ProstateDemo.py)
  slicer_add_python_unittest(SCRIPT JRC2013Vis.py)
  slicer_add_python_unittest(SCRIPT FiducialLayoutSwitchBug1914.py)
  slicer_add_python_unittest(SCRIPT ShaderProperties.py)
  slicer_add_python_unittest(SCRIPT SlicerScriptedFileReaderWriterTest.py)

  # add as hidden module for use at run time
  set(KIT_PYTHON_SCRIPTS
    AtlasTests.py
    sceneImport2428.py
    SlicerMRBMultipleSaveRestoreLoopTest.py
    SlicerMRBMultipleSaveRestoreTest.py
    SlicerMRBSaveRestoreCheckPathsTest.py
    Slicer4Minute.py
    SlicerBoundsTest.py
    Charting.py
    WebEngine.py
    SliceLinkLogic.py
    ScenePerformance.py
    RSNAVisTutorial.py
    RSNAQuantTutorial.py
    slicerCloseCrashBug2590.py
    SlicerOrientationSelectorTest.py
    SlicerTransformInteractionTest1.py
    ViewControllersSliceInterpolationBug1926.py
    RSNA2012ProstateDemo.py
    JRC2013Vis.py
    FiducialLayoutSwitchBug1914.py
    ShaderProperties.py
    UtilTest.py
    SlicerScriptedFileReaderWriterTest.py
    )

  if(Slicer_BUILD_CLI_SUPPORT)
    slicer_add_python_unittest(SCRIPT CLIEventTest.py SLICER_ARGS --no-main-window)
    slicer_add_python_unittest(SCRIPT TwoCLIsInARowTest.py)
    slicer_add_python_unittest(SCRIPT TwoCLIsInParallelTest.py)

    if(Slicer_BUILD_BRAINSTOOLS)
      slicer_add_python_unittest(SCRIPT BRAINSFitRigidRegistrationCrashIssue4139.py)

      list(APPEND KIT_PYTHON_SCRIPTS
        BRAINSFitRigidRegistrationCrashIssue4139.py
        )
    endif()
  endif()

  set(KIT_PYTHON_RESOURCES
    Resources/UI/ScenePerformance.ui
    Resources/UI/UtilTest.ui
    )

  ctkMacroCompilePythonScript(
    TARGET_NAME ApplicationSelfTests
    SCRIPTS "${KIT_PYTHON_SCRIPTS}"
    RESOURCES "${KIT_PYTHON_RESOURCES}"
    DESTINATION_DIR ${Slicer_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}
    INSTALL_DIR ${Slicer_INSTALL_QTSCRIPTEDMODULES_LIB_DIR}
    NO_INSTALL_SUBDIR
    )

endif()

