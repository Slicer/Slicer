
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1) # Do not add CTest default targets
include(CTest)
include(SlicerMacroSimpleTest)
include(SlicerMacroPythonTesting)
include(SlicerMacroConfigureGenericCxxModuleTests)
include(SlicerMacroConfigureGenericPythonModuleTests)

include(SlicerBlockUploadExtensionPrerequisites)
set(EXTENSION_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
if(NOT DEFINED EXTENSION_SUPERBUILD_BINARY_DIR)
  set(EXTENSION_SUPERBUILD_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
endif()
if(NOT DEFINED EXTENSION_BUILD_SUBDIRECTORY)
  set(EXTENSION_BUILD_SUBDIRECTORY ".")
endif()
if(NOT DEFINED EXTENSION_ENABLED)
  set(EXTENSION_ENABLED 1)
endif()
message(STATUS "Setting EXTENSION_SOURCE_DIR ..........: ${EXTENSION_SOURCE_DIR}")
message(STATUS "Setting EXTENSION_SUPERBUILD_BINARY_DIR: ${EXTENSION_SUPERBUILD_BINARY_DIR}")
message(STATUS "Setting EXTENSION_BUILD_SUBDIRECTORY ..: ${EXTENSION_BUILD_SUBDIRECTORY}")
message(STATUS "Setting EXTENSION_BINARY_DIR ..........: ${EXTENSION_SUPERBUILD_BINARY_DIR}/${EXTENSION_BUILD_SUBDIRECTORY}")

set(CTEST_MODEL "Experimental")
include(SlicerBlockUploadExtension)
add_custom_target(Experimental
  COMMAND ${EXTENSION_TEST_COMMAND}
  COMMENT "Build and test extension"
  )
add_custom_target(ExperimentalUpload
  COMMAND ${EXTENSION_UPLOAD_COMMAND}
  COMMENT "Build, test, package and upload extension"
  )

set(CTEST_MODEL "Nightly")
include(SlicerBlockUploadExtension)
add_custom_target(Nightly
  COMMAND ${EXTENSION_TEST_COMMAND}
  COMMENT "Build and test extension"
  )
add_custom_target(NightlyUpload
  COMMAND ${EXTENSION_UPLOAD_COMMAND}
  COMMENT "Build, test, package and upload extension"
  )



#
# In case the extension is build using a multi-configuration system, the build
# type will be know at build. For that reason, a script allowing to configure
# the "additonal launcher settings" at build time will be generated.
#

set(Slicer_ADDITIONAL_LAUNCHER_SETTINGS ${CMAKE_CURRENT_BINARY_DIR}/AdditionalLauncherSettings.ini)

# Add '--launcher-additional-settings' to launch command
list(FIND Slicer_LAUNCH_COMMAND "--launch" launch_index)
list(INSERT Slicer_LAUNCH_COMMAND ${launch_index} "--launcher-additional-settings" ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS})

# Configure script
set(_additonal_settings_configure_script ${CMAKE_CURRENT_BINARY_DIR}/AdditionalLauncherSettings-configure.cmake)
file(WRITE ${_additonal_settings_configure_script}
"
file(WRITE ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS}
\"[LibraryPaths]
1\\\\path=${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_LIB_DIR}/\${CMAKE_CFG_INTDIR}
2\\\\path=${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_LIB_DIR}/\${CMAKE_CFG_INTDIR}
size=2

[Paths]
1\\\\path=${CMAKE_BINARY_DIR}/${Slicer_CLIMODULES_BIN_DIR}/\${CMAKE_CFG_INTDIR}
size=1

[EnvironmentVariables]
PYTHONPATH=${CMAKE_BINARY_DIR}/${Slicer_QTSCRIPTEDMODULES_LIB_DIR}<PATHSEP>${CMAKE_BINARY_DIR}/${Slicer_QTLOADABLEMODULES_PYTHON_LIB_DIR}<PATHSEP><env:PYTHONPATH>
\")
")

add_custom_command(
  DEPENDS
    ${CMAKE_CURRENT_LIST_FILE}
  OUTPUT
    ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS}
  COMMAND ${CMAKE_COMMAND}
    -DCMAKE_CFG_INTDIR:STRING=${CMAKE_CFG_INTDIR}
    -P ${_additonal_settings_configure_script}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Configuring: AdditionalLauncherSettings.ini"
  )

add_custom_target(ConfigureAdditonalLauncherSettings ALL
  DEPENDS
    ${Slicer_ADDITIONAL_LAUNCHER_SETTINGS}
  )
