PROJECT(qSlicerBaseQTCLI)

#
# See CMake/SlicerMacroBuildBaseQtLibrary.cmake for details
#

SET(KIT_export_directive "Q_SLICER_BASE_QTCLI_EXPORT")

# Additional directories to include
SET(KIT_include_directories
  ${qSlicerBaseQTCore_SOURCE_DIR}
  ${qSlicerBaseQTCore_BINARY_DIR}
  ${qSlicerBaseQTGUI_SOURCE_DIR}
  ${qSlicerBaseQTGUI_BINARY_DIR}
  ${ModuleDescriptionParser_SOURCE_DIR}
  ${ModuleDescriptionParser_BINARY_DIR}
  ${MRMLCLI_SOURCE_DIR}
  ${MRMLCLI_BINARY_DIR}
  ${MRMLLogic_SOURCE_DIR}
  ${MRMLLogic_BINARY_DIR}
  )

# Source files
SET(KIT_SRCS
  qSlicerCLIExecutableModuleFactory.cxx
  qSlicerCLIExecutableModuleFactory.h
  qSlicerCLILoadableModuleFactory.cxx
  qSlicerCLILoadableModuleFactory.h
  qSlicerCLIModule.cxx
  qSlicerCLIModule.h
  qSlicerCLIModuleFactoryHelper.cxx
  qSlicerCLIModuleFactoryHelper.h
  qSlicerCLIModuleUIHelper.cxx
  qSlicerCLIModuleUIHelper.h
  qSlicerCLIModuleWidget.cxx
  qSlicerCLIModuleWidget.h
  qSlicerCLIModuleWidget_p.h
  vtkSlicerCLIModuleLogic.cxx
  vtkSlicerCLIModuleLogic.h
  )

# Headers that should run through moc
SET(KIT_MOC_SRCS
  qSlicerCLIModule.h
  qSlicerCLIModuleWidget.h
  qSlicerCLIModuleWidget_p.h
  qSlicerCLIModuleUIHelper.h
  )

# UI files
SET(KIT_UI_SRCS
  Resources/UI/qSlicerCLIModuleWidget.ui
)

# Resources
SET(KIT_resources
)

# Additional Target libraries (QT libs already included)
SET(KIT_target_libraries
  qSlicerBaseQTCore
  qSlicerBaseQTGUI
  #SlicerBaseLogic
  ModuleDescriptionParser
  MRMLCLI
  )

SET(KIT_ToPythonWrap_SRCS
  vtkSlicerCLIModuleLogic.cxx
  vtkSlicerCLIModuleLogic.h
)

SlicerMacroBuildBaseQtLibrary(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${KIT_export_directive}
  INCLUDE_DIRECTORIES ${KIT_include_directories}
  SRCS ${KIT_SRCS}
  MOC_SRCS ${KIT_MOC_SRCS}
  UI_SRCS ${KIT_UI_SRCS}
  TARGET_LIBRARIES ${KIT_target_libraries}
  RESOURCES ${KIT_resources}
  )

# Plugin
#ADD_SUBDIRECTORY(Plugins)

# --------------------------------------------------------------------------
# Create Python wrappers if VTK is wrapped in Python
#

IF(VTK_WRAP_PYTHON AND BUILD_SHARED_LIBS)

  SET(KIT_PYTHON_LIBRARIES ${VTK_LIBRARIES})

  INCLUDE(${VTK_CMAKE_DIR}/vtkWrapPython.cmake)
  VTK_WRAP_PYTHON3(${lib_name}Python PYTHON_SRCS "${KIT_ToPythonWrap_SRCS}")
  ADD_LIBRARY(${lib_name}PythonD ${PYTHON_SRCS})
  SET_TARGET_PROPERTIES(${lib_name}PythonD PROPERTIES LABELS ${lib_name})
  ADD_LIBRARY(${lib_name}Python MODULE ${lib_name}PythonInit.cxx)
  SET_TARGET_PROPERTIES(${lib_name}Python PROPERTIES LABELS ${lib_name})
  
  TARGET_LINK_LIBRARIES(${lib_name}PythonD ${lib_name})
  #  TARGET_LINK_LIBRARIES(${lib_name}PythonD MRMLLogicPythonD)
  
  FOREACH(c ${KIT_PYTHON_LIBRARIES})
    TARGET_LINK_LIBRARIES(${lib_name}PythonD ${c}PythonD)
  ENDFOREACH(c)

  TARGET_LINK_LIBRARIES(${lib_name}Python ${lib_name}PythonD)
  #  TARGET_LINK_LIBRARIES(${lib_name}Python MRMLLogicPythonD)

  ## Python modules on Windows must have the extension ".pyd"
  IF(WIN32 AND NOT CYGWIN)
    SET_TARGET_PROPERTIES(${lib_name}Python PROPERTIES SUFFIX ".pyd")
  ENDIF(WIN32 AND NOT CYGWIN)

  INSTALL(TARGETS ${lib_name}PythonD ${lib_name}Python
    RUNTIME DESTINATION ${Slicer_INSTALL_BIN_DIR} COMPONENT RuntimeLibraries
    LIBRARY DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT RuntimeLibraries
    ARCHIVE DESTINATION ${Slicer_INSTALL_LIB_DIR} COMPONENT Development)

ENDIF(VTK_WRAP_PYTHON AND BUILD_SHARED_LIBS)


# Testing
IF(BUILD_TESTING)
  ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
