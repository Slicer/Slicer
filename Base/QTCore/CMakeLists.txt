project(qSlicerBaseQTCore)

#
# Third party library
#

if(Slicer_BUILD_CLI_SUPPORT)
  # Required to define ModuleDescriptionParser_INCLUDE_DIRS
  find_package(SlicerExecutionModel REQUIRED ModuleDescriptionParser)
endif()

#
# See CMake/SlicerMacroBuildBaseQtLibrary.cmake for details
#

set(KIT_export_directive "Q_SLICER_BASE_QTCORE_EXPORT")

# Additional directories to include
set(KIT_include_directories
  ${MRMLLogic_INCLUDE_DIRS}
  ${MRMLDisplayableManager_INCLUDE_DIRS}
  ${FreeSurfer_INCLUDE_DIRS} # for qSlicerXcedeCatalogIO
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
  qSlicerSceneBundleIO.cxx
  qSlicerSceneBundleIO.h
  qSlicerSlicer2SceneReader.cxx
  qSlicerSlicer2SceneReader.h
  qSlicerUtils.cxx
  qSlicerUtils.h
  qSlicerXcedeCatalogIO.cxx
  qSlicerXcedeCatalogIO.h
  )

if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND KIT_SRCS
    qSlicerExtensionsManagerModel.cxx
    qSlicerExtensionsManagerModel.h
    qSlicerPersistentCookieJar.cxx
    qSlicerPersistentCookieJar.h
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
  qSlicerSceneBundleIO.h
  qSlicerSlicer2SceneReader.h
  qSlicerXcedeCatalogIO.h
  )

if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND KIT_MOC_SRCS
    qSlicerExtensionsManagerModel.h
    )
endif()

# Additional Target libraries (QT libs already included)
set(KIT_target_libraries
  SlicerBaseLogic
  MRMLDisplayableManager
  CTKCore
  CTKImageProcessingITKCore
  CTKVisualizationVTKCore
  )

if(Slicer_BUILD_EXTENSIONMANAGER_SUPPORT)
  list(APPEND KIT_target_libraries
    qRestAPI
    )
endif()

if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND KIT_target_libraries
    ModuleDescriptionParser
    )
endif()

# Resources
set(KIT_resources
  Resources/qSlicerBaseQTCore.qrc
  )

# Python wrap
if(Slicer_USE_PYTHONQT)

  list(APPEND KIT_target_libraries
    CTKScriptingPythonCore
    vtkCommonPythonD # For vtkPythonUtils
    )
  if(WIN32)
    # TODO: this resolves link errors on windows, but shouldn't be
    # needed - issue may lie with ctk external library macros?
    list(APPEND KIT_target_libraries
      ${PYTHONQT_LIBRARY}
      )
  endif(WIN32)

  # Sources
  list(APPEND KIT_SRCS
    qSlicerCorePythonManager.cxx
    qSlicerCorePythonManager.h
    )

  # Headers that should run through moc
  list(APPEND KIT_MOC_SRCS
    qSlicerCorePythonManager.h
    )

endif(Slicer_USE_PYTHONQT)

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

# Plugins
#add_subdirectory(Plugins)

# Testing
# Note: Within both Testing/Cxx and Testing/Python, some code is useful to both Slicer and the
#       Extensions. For that reason, the Testing directory is added inconditionnally.
#       The code specific to Slicer will be excluded directly in either Testing/Python
#       or Testing/Cxx.
add_subdirectory(Testing)
