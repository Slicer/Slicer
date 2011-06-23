project(qSlicerBaseQTCore)

#
# See CMake/SlicerMacroBuildBaseQtLibrary.cmake for details
#

set(KIT_export_directive "Q_SLICER_BASE_QTCORE_EXPORT")

# Additional directories to include
set(KIT_include_directories
  ${ModuleDescriptionParser_INCLUDE_DIRS}
  ${MRMLCLI_INCLUDE_DIRS}
  ${MRMLLogic_INCLUDE_DIRS}
  ${FreeSurfer_INCLUDE_DIRS} # for qSlicerXcedeCatalogIO
  )

# Source files
set(KIT_SRCS
  qSlicerAbstractCoreModule.cxx
  qSlicerAbstractCoreModule.h
  qSlicerAbstractModuleRepresentation.cxx
  qSlicerAbstractModuleRepresentation.h
  qSlicerCoreApplication.cxx
  qSlicerCoreApplication.h
  qSlicerCoreApplication_p.h
  qSlicerCoreCommandOptions.cxx
  qSlicerCoreCommandOptions.h
  qSlicerCoreIOManager.cxx
  qSlicerCoreIOManager.h
  qSlicerIO.cxx
  qSlicerIO.h
  qSlicerIOOptions.cxx
  qSlicerIOOptions.h
  qSlicerLoadableModuleFactory.cxx
  qSlicerLoadableModuleFactory.h
  qSlicerModuleFactoryManager.cxx
  qSlicerModuleFactoryManager.h
  qSlicerModuleManager.cxx
  qSlicerModuleManager.h
  qSlicerObject.cxx
  qSlicerObject.h
  qSlicerSceneIO.cxx
  qSlicerSceneIO.h
  qSlicerSlicer2SceneReader.cxx
  qSlicerSlicer2SceneReader.h
  qSlicerUtils.cxx
  qSlicerUtils.h
  qSlicerXcedeCatalogIO.cxx
  qSlicerXcedeCatalogIO.h
  )

# Headers that should run through moc
set(KIT_MOC_SRCS
  qSlicerAbstractCoreModule.h
  qSlicerCoreCommandOptions.h
  qSlicerCoreApplication.h
  qSlicerCoreIOManager.h
  qSlicerIO.h
  qSlicerModuleFactoryManager.h
  qSlicerModuleManager.h
  qSlicerSceneIO.h
  qSlicerSlicer2SceneReader.h
  qSlicerXcedeCatalogIO.h
  )

# Additional Target libraries (QT libs already included)
set(KIT_target_libraries
  ModuleDescriptionParser
  SlicerBaseLogic
  FreeSurfer # for qSlicerXcedeCatalogIO
  CTKCore
  CTKDICOMCore
  CTKImageProcessingITKCore
  CTKVisualizationVTKCore
  )

# Resources
set(KIT_resources
  Resources/qSlicerBaseQTCore.qrc
)

# Python wrap
if(Slicer_USE_PYTHONQT)

  list(APPEND KIT_target_libraries
    CTKCorePythonQt
    CTKDICOMCorePythonQt
    CTKVisualizationVTKCorePythonQt
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
    qSlicerBaseQTCorePythonQtDecorators.h
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
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
