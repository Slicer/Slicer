project(qSlicerBaseQTCLI)

#
# Dependencies
#

#
# ModuleDescriptionParser
#
find_package(SlicerExecutionModel REQUIRED ModuleDescriptionParser)

#
# ITK - Import ITK targets required by ModuleDescriptionParser
#
set(${PROJECT_NAME}_ITK_COMPONENTS
  ${ModuleDescriptionParser_ITK_COMPONENTS}
  )
find_package(ITK 4.6 COMPONENTS ${${PROJECT_NAME}_ITK_COMPONENTS} REQUIRED)

#
# VTK
#
if(Slicer_USE_PYTHONQT AND NOT VTK_WRAP_PYTHON)
  message(FATAL_ERROR "Since Slicer_USE_PYTHONQT is ON, VTK_WRAP_PYTHON is expected to be ON. "
                      "Re-configure VTK with python wrapping.")
endif()

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
  qSlicerCLIProgressBar.cxx
  qSlicerCLIProgressBar.h
  )

# Headers that should run through moc
set(KIT_MOC_SRCS
  qSlicerCLIModule.h
  qSlicerCLIModuleWidget.h
  qSlicerCLIModuleWidget_p.h
  qSlicerCLIModuleUIHelper.h
  qSlicerCLIProgressBar.h
  )

# UI files
set(KIT_UI_SRCS
  Resources/UI/qSlicerCLIModuleWidget.ui
  )

# Resources
set(KIT_resources
  )

# Additional Target libraries (Qt libs already included)
set(KIT_target_libraries
  # ${QT_LIBRARIES} # Not needed: All dependencies are transitively satisfied by other targets
  qSlicerBaseQTCore
  qSlicerBaseQTGUI
  ModuleDescriptionParser ${ITK_LIBRARIES}
  MRMLCLI
  )

if(Slicer_USE_QtTesting)
  list(APPEND KIT_SRCS
    qSlicerCLIModuleWidgetEventPlayer.cxx
    qSlicerCLIModuleWidgetEventPlayer.h
    )
  list(APPEND KIT_MOC_SRCS
    qSlicerCLIModuleWidgetEventPlayer.h
    )
endif()

SlicerMacroBuildBaseQtLibrary(
  NAME ${PROJECT_NAME}
  EXPORT_DIRECTIVE ${KIT_export_directive}
  INCLUDE_DIRECTORIES ${KIT_include_directories}
  SRCS ${KIT_SRCS}
  MOC_SRCS ${KIT_MOC_SRCS}
  UI_SRCS ${KIT_UI_SRCS}
  TARGET_LIBRARIES ${KIT_target_libraries}
  RESOURCES ${KIT_resources}
  WRAP_PYTHONQT
  )

if(Slicer_BUILD_QT_DESIGNER_PLUGINS)
  add_subdirectory(DesignerPlugins)
endif()

# --------------------------------------------------------------------------
# Python Wrapping
# --------------------------------------------------------------------------
if(VTK_WRAP_PYTHON)
  include(vtkMacroKitPythonWrap)

  vtkMacroKitPythonWrap(
    KIT_NAME ${PROJECT_NAME}
    KIT_SRCS ${KIT_VTK_SRCS}
    KIT_INSTALL_BIN_DIR ${Slicer_INSTALL_BIN_DIR}
    KIT_INSTALL_LIB_DIR ${Slicer_INSTALL_LIB_DIR}
    KIT_PYTHON_LIBRARIES SlicerBaseLogicPythonD
    )
  set_property(GLOBAL APPEND PROPERTY Slicer_TARGETS ${PROJECT_NAME}Python ${PROJECT_NAME}PythonD)
  # Folder
  set_target_properties(${PROJECT_NAME}Python PROPERTIES FOLDER "Core-Base")
  set_target_properties(${PROJECT_NAME}PythonD PROPERTIES FOLDER "Core-Base")
  if(TARGET ${PROJECT_NAME}Hierarchy)
    set_target_properties(${PROJECT_NAME}Hierarchy PROPERTIES FOLDER "Core-Base")
  endif()
endif()

# --------------------------------------------------------------------------
# Testing
# --------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
