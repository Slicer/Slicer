PROJECT(qSlicerBaseQTCore)

#
# See CMake/SlicerQTBaseLibraryMacros.cmake for details
#

SET(KIT_export_directive "Q_SLICER_BASE_QTCORE_EXPORT")

# Additional directories to include
SET(KIT_include_directories
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  ${MRMLCLI_SOURCE_DIR}
  ${MRMLCLI_BINARY_DIR}
  ${MRMLLogic_SOURCE_DIR}
  ${MRMLLogic_BINARY_DIR}
  ${FreeSurfer_SOURCE_DIR} # for qSlicerXcedeCatalogIO
  ${FreeSurfer_BINARY_DIR} # for qSlicerXcedeCatalogIO
  )

# Source files
SET(KIT_SRCS
  qSlicerAbstractCoreModule.cxx
  qSlicerAbstractCoreModule.h
  qSlicerAbstractModuleRepresentation.cxx
  qSlicerAbstractModuleRepresentation.h
  qSlicerCoreApplication.cxx
  qSlicerCoreApplication.h
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
SET(KIT_MOC_SRCS
  qSlicerAbstractCoreModule.h
  qSlicerCoreApplication.h
  qSlicerCoreIOManager.h
  qSlicerIO.h
  qSlicerModuleManager.h
  qSlicerSceneIO.h
  qSlicerSlicer2SceneReader.h
  qSlicerXcedeCatalogIO.h
  )

# Additional Target libraries (QT libs already included)
SET(KIT_target_libraries
  ModuleDescriptionParser
  SlicerBaseLogic
  FreeSurfer # for qSlicerXcedeCatalogIO
  CTKCore
  CTKVisualizationVTKCore
  )

# Resources
SET(KIT_resources
  Resources/qSlicerBaseQTCore.qrc
)

# Python wrap
IF(Slicer_USE_PYTHONQT)

  SET(KIT_PYTHONQT_SRCS) # Clear variable
  ctkMacroWrapPythonQt("org.slicer.base" ${PROJECT_NAME}
    KIT_PYTHONQT_SRCS "${KIT_SRCS}" FALSE)
  ADD_LIBRARY(${PROJECT_NAME}PythonQt STATIC ${KIT_PYTHONQT_SRCS})
  TARGET_LINK_LIBRARIES(${PROJECT_NAME}PythonQt ${PROJECT_NAME})
  IF(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    SET_TARGET_PROPERTIES(${PROJECT_NAME}PythonQt PROPERTIES COMPILE_FLAGS "-fPIC")
  ENDIF()

  LIST(APPEND KIT_target_libraries
    CTKCorePythonQt
    CTKScriptingPythonCorePythonQt
    CTKVisualizationVTKCorePythonQt
    CTKScriptingPythonCore
    vtkCommonPythonD # For vtkPythonUtils
    PythonQt
    )
  IF(WIN32)
    # TODO: this resolves link errors on windows, but shouldn't be
    # needed - issue may lie with ctk external library macros?
    LIST(APPEND KIT_target_libraries
      ${PYTHONQT_LIBRARY}
      )
  ENDIF(WIN32)

  # Sources
  LIST(APPEND KIT_SRCS
    qSlicerCorePythonManager.cxx
    qSlicerCorePythonManager.h
    )

  # Headers that should run through moc
  LIST(APPEND KIT_MOC_SRCS
    qSlicerCorePythonManager.h
    qSlicerBaseQTCorePythonQtDecorators.h
    )

ENDIF(Slicer_USE_PYTHONQT)

Slicer_build_slicer_qtbase_library(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${KIT_export_directive}
  INCLUDE_DIRECTORIES ${KIT_include_directories}
  SRCS ${KIT_SRCS}
  MOC_SRCS ${KIT_MOC_SRCS}
  TARGET_LIBRARIES ${KIT_target_libraries}
  RESOURCES ${KIT_resources}
  )

# Plugins
#ADD_SUBDIRECTORY(Plugins)

# Testing
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
