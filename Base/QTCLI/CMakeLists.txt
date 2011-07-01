project(qSlicerBaseQTCLI)

#
# Third party library
#

# Required to define ModuleDescriptionParser_INCLUDE_DIRS
find_package(SlicerExecutionModel REQUIRED ModuleDescriptionParser)

#
# See CMake/SlicerMacroBuildBaseQtLibrary.cmake for details
#

set(KIT_export_directive "Q_SLICER_BASE_QTCLI_EXPORT")

# Additional directories to include
set(KIT_include_directories
  ${qSlicerBaseQTCore_SOURCE_DIR}
  ${qSlicerBaseQTCore_BINARY_DIR}
  ${qSlicerBaseQTGUI_SOURCE_DIR}
  ${qSlicerBaseQTGUI_BINARY_DIR}
  ${ModuleDescriptionParser_INCLUDE_DIRS}
  ${MRMLCLI_INCLUDE_DIRS}
  ${MRMLLogic_INCLUDE_DIRS}
  )

# Source files
set(KIT_VTK_SRCS
  vtkSlicerCLIModuleLogic.cxx
  vtkSlicerCLIModuleLogic.h
  )

# Source files
set(KIT_SRCS
  ${KIT_VTK_SRCS}
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
  )

# Headers that should run through moc
set(KIT_MOC_SRCS
  qSlicerCLIModule.h
  qSlicerCLIModuleWidget.h
  qSlicerCLIModuleWidget_p.h
  qSlicerCLIModuleUIHelper.h
  )

# UI files
set(KIT_UI_SRCS
  Resources/UI/qSlicerCLIModuleWidget.ui
  )

# Resources
set(KIT_resources
  )

# Additional Target libraries (QT libs already included)
set(KIT_target_libraries
  qSlicerBaseQTCore
  qSlicerBaseQTGUI
  ModuleDescriptionParser
  MRMLCLI
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
#add_subdirectory(Plugins)

# --------------------------------------------------------------------------
# Python Wrapping
# --------------------------------------------------------------------------
if(VTK_WRAP_PYTHON)
  vtkMacroKitPythonWrap(
    KIT_NAME ${PROJECT_NAME}
    KIT_SRCS ${KIT_VTK_SRCS}
    KIT_INSTALL_BIN_DIR ${Slicer_INSTALL_BIN_DIR}
    KIT_INSTALL_LIB_DIR ${Slicer_INSTALL_LIB_DIR}
    KIT_PYTHON_LIBRARIES SlicerBaseLogicPythonD
    )
endif()

# --------------------------------------------------------------------------
# Testing
# --------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
