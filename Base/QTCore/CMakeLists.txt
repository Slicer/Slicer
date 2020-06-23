project(qSlicerBaseQTCore)

#
# Dependencies
#

#
# DCMTK
#
if(Slicer_BUILD_DICOM_SUPPORT)
  find_package(DCMTK REQUIRED)
endif()

#
# ModuleDescriptionParser - Required to define ModuleDescriptionParser_INCLUDE_DIRS
#
if(Slicer_BUILD_CLI_SUPPORT)
  find_package(SlicerExecutionModel REQUIRED ModuleDescriptionParser)

  #
  # ITK
  #
  set(${PROJECT_NAME}_ITK_COMPONENTS
    # Import ITK targets required by ModuleDescriptionParser
    ${ModuleDescriptionParser_ITK_COMPONENTS}
    # Import ITK targets required by CTKImageProcessingITKCore
    ITKCommon
    )
  find_package(ITK 4.6 COMPONENTS ${${PROJECT_NAME}_ITK_COMPONENTS} REQUIRED)
  set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 1) # See Libs/ITKFactoryRegistration/CMakeLists.txt
  include(${ITK_USE_FILE})
endif()

#
# qRestAPI
#
if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  find_package(qRestAPI REQUIRED)
  include(${qRestAPI_USE_FILE})
endif()

#
# CTKAppLauncherLib
#
find_package(CTKAppLauncherLib REQUIRED)

#
# See CMake/SlicerMacroBuildBaseQtLibrary.cmake for details
#

set(KIT_export_directive "Q_SLICER_BASE_QTCORE_EXPORT")

# Additional directories to include
set(KIT_include_directories
  ${MRMLLogic_INCLUDE_DIRS}
  ${MRMLDisplayableManager_INCLUDE_DIRS}
  )

if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND KIT_include_directories
    ${ModuleDescriptionParser_INCLUDE_DIRS}
    ${MRMLCLI_INCLUDE_DIRS}
    )
endif()

# Source files
set(KIT_SRCS
  qSlicerAbstractCoreModule.cxx
  qSlicerAbstractCoreModule.h
  qSlicerAbstractModuleFactoryManager.cxx
  qSlicerAbstractModuleFactoryManager.h
  qSlicerAbstractModuleRepresentation.cxx
  qSlicerAbstractModuleRepresentation.h
  qSlicerCoreApplication.cxx
  qSlicerCoreApplication.h
  qSlicerCoreApplication_p.h
  qSlicerCoreCommandOptions.cxx
  qSlicerCoreCommandOptions.h
  qSlicerCoreIOManager.cxx
  qSlicerCoreIOManager.h
  qSlicerFileReader.cxx
  qSlicerFileReader.h
  qSlicerFileWriter.cxx
  qSlicerFileWriter.h
  qSlicerIO.cxx
  qSlicerIO.h
  qSlicerIOOptions.cxx
  qSlicerIOOptions.h
  qSlicerIOOptions_p.h
  qSlicerLoadableModuleFactory.cxx
  qSlicerLoadableModuleFactory.h
  qSlicerModuleFactoryManager.cxx
  qSlicerModuleFactoryManager.h
  qSlicerModuleManager.cxx
  qSlicerModuleManager.h
  qSlicerObject.cxx
  qSlicerObject.h
  qSlicerPersistentCookieJar.cxx
  qSlicerPersistentCookieJar.h
  qSlicerSceneBundleReader.cxx
  qSlicerSceneBundleReader.h
  qSlicerSlicer2SceneReader.cxx
  qSlicerSlicer2SceneReader.h
  qSlicerUtils.cxx
  qSlicerUtils.h
  )

if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND KIT_SRCS
    qSlicerExtensionDownloadTask.cxx
    qSlicerExtensionDownloadTask.h
    qSlicerExtensionsManagerModel.cxx
    qSlicerExtensionsManagerModel.h
    )
endif()

if(Slicer_USE_PYTHONQT)
  list(APPEND KIT_SRCS
    qSlicerScriptedFileReader.cxx
    qSlicerScriptedFileReader.h
    qSlicerScriptedFileWriter.cxx
    qSlicerScriptedFileWriter.h
    qSlicerScriptedUtils.cxx
    qSlicerScriptedUtils_p.h
    )
endif()

# Since the wrapper doesn't consider hierarchy of classes, let's exclude
# classes deriving from pure abstract class and remaining abstract pure
set_source_files_properties(
  qSlicerFileReader.h
  qSlicerFileWriter.h
  WRAP_EXCLUDE
  )

# Headers that should run through moc
set(KIT_MOC_SRCS
  qSlicerAbstractCoreModule.h
  qSlicerAbstractModuleFactoryManager.h
  qSlicerCoreCommandOptions.h
  qSlicerCoreApplication.h
  qSlicerCoreIOManager.h
  qSlicerFileReader.h
  qSlicerFileWriter.h
  qSlicerIO.h
  qSlicerModuleFactoryManager.h
  qSlicerModuleManager.h
  qSlicerSceneBundleReader.h
  qSlicerSlicer2SceneReader.h
  )

if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND KIT_MOC_SRCS
    qSlicerExtensionDownloadTask.h
    qSlicerExtensionsManagerModel.h
    )
endif()

if(Slicer_USE_PYTHONQT)
  list(APPEND KIT_MOC_SRCS
    qSlicerScriptedFileReader.h
    qSlicerScriptedFileWriter.h
    )
endif()

# Additional Target libraries
set(KIT_target_libraries
  ${QT_LIBRARIES}
  SlicerBaseLogic
  MRMLDisplayableManager
  ${CTKAppLauncherLib_LIBRARIES}
  CTKCore
  CTKImageProcessingITKCore
  CTKVisualizationVTKCore
  ${ITK_LIBRARIES}
  )

if(Slicer_BUILD_DICOM_SUPPORT)
  list(APPEND KIT_target_libraries
    ${DCMTK_LIBRARIES}
    CTKDICOMCore
    )
endif()

if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND KIT_target_libraries
    qRestAPI
    )
endif()

if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND KIT_target_libraries
    ${ITK_LIBRARIES}
    ModuleDescriptionParser
    )
endif()

# Resources
set(KIT_resources
  Resources/qSlicerBaseQTCore.qrc
  )

if(APPLE)
  set(QT_CONF_FILEPATH "${CMAKE_CURRENT_BINARY_DIR}/Resources/qt.conf")
  file(WRITE ${QT_CONF_FILEPATH}
"[Paths]
Plugins = ${Slicer_QtPlugins_DIR}
")

  configure_file(
    Resources/qSlicerBaseQTCoreExtra.qrc.in
    ${CMAKE_CURRENT_BINARY_DIR}/Resources/qSlicerBaseQTCoreExtra.qrc
    )

  list(APPEND KIT_resources
    ${CMAKE_CURRENT_BINARY_DIR}/Resources/qSlicerBaseQTCoreExtra.qrc
    )
endif()

# Python wrap
if(Slicer_USE_PYTHONQT)
  set(VTK_COMMON_PYTHON_LIBRARY
    vtkWrappingPythonCore # For vtkPythonUtil
    )
  list(APPEND KIT_target_libraries
    CTKScriptingPythonCore
    ${VTK_COMMON_PYTHON_LIBRARY} # For vtkPythonUtils
    )
  if(WIN32)
    # TODO: this resolves link errors on windows, but shouldn't be
    # needed - issue may lie with ctk external library macros?
    list(APPEND KIT_target_libraries
      ${PYTHONQT_LIBRARY}
      )
  endif()

  # Sources
  list(APPEND KIT_SRCS
    qSlicerCorePythonManager.cxx
    qSlicerCorePythonManager.h
    )

  # Headers that should run through moc
  list(APPEND KIT_MOC_SRCS
    qSlicerCorePythonManager.h
    )

endif()

SlicerMacroBuildBaseQtLibrary(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${KIT_export_directive}
  INCLUDE_DIRECTORIES ${KIT_include_directories}
  SRCS ${KIT_SRCS}
  MOC_SRCS ${KIT_MOC_SRCS}
  TARGET_LIBRARIES ${KIT_target_libraries}
  RESOURCES ${KIT_resources}
  WRAP_PYTHONQT
  )

if(Slicer_USE_PYTHONQT_WITH_OPENSSL)
  configure_file(
    Resources/Certs/Slicer.crt
    ${Slicer_BINARY_DIR}/${Slicer_SHARE_DIR}/Slicer.crt
    COPYONLY
    )
  install(
    FILES Resources/Certs/Slicer.crt
    DESTINATION ${Slicer_INSTALL_SHARE_DIR} COMPONENT Runtime
    )
endif()

# Plugins
#add_subdirectory(Plugins)

# Testing
# Note: Within both Testing/Cxx and Testing/Python, some code is useful to both Slicer and the
#       Extensions. For that reason, the Testing directory is added inconditionnally.
#       The code specific to Slicer will be excluded directly in either Testing/Python
#       or Testing/Cxx.
add_subdirectory(Testing)
