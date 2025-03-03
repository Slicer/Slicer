
#-----------------------------------------------------------------------------
# Dependencies

if(Slicer_BUILD_CLI_SUPPORT)
  # Needed to support include of "qSlicerCLIModule.h" depending on "ModuleDescription"

  # ModuleDescriptionParser
  find_package(SlicerExecutionModel REQUIRED ModuleDescriptionParser)

  # ITK - Import ITK targets required by ModuleDescriptionParser
  set(${PROJECT_NAME}_ITK_COMPONENTS
    ${ModuleDescriptionParser_ITK_COMPONENTS}
    )
  find_package(ITK 4.6 COMPONENTS ${${PROJECT_NAME}_ITK_COMPONENTS} REQUIRED)
endif()

#-----------------------------------------------------------------------------
set(MODULE_NAME GeneralizedReformat)

string(TOUPPER ${MODULE_NAME} MODULE_NAME_UPPER)

#-----------------------------------------------------------------------------
add_subdirectory(MRML)
add_subdirectory(Logic)

#-----------------------------------------------------------------------------
set(MODULE_EXPORT_DIRECTIVE "Q_SLICER_QTMODULES_${MODULE_NAME_UPPER}_EXPORT")

# Current_{source,binary} and Slicer_{Libs,Base} already included
set(MODULE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/MRML
  ${CMAKE_CURRENT_BINARY_DIR}/MRML
  ${CMAKE_CURRENT_SOURCE_DIR}/Logic
  ${CMAKE_CURRENT_BINARY_DIR}/Logic
  )
if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND MODULE_INCLUDE_DIRECTORIES
    ${ModuleDescriptionParser_INCLUDE_DIRS}
    )
endif()

set(MODULE_SRCS
  qSlicer${MODULE_NAME}Module.cxx
  qSlicer${MODULE_NAME}Module.h
  )
if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND MODULE_SRCS
    vtkMRMLScalarVectorDWIVolumeResampler.cxx
    vtkMRMLScalarVectorDWIVolumeResampler.h
    )
endif()

set(MODULE_MOC_SRCS
  qSlicer${MODULE_NAME}Module.h
  )

set(MODULE_UI_SRCS
  )

set(MODULE_TARGET_LIBRARIES
  vtkSlicer${MODULE_NAME}ModuleMRML
  vtkSlicer${MODULE_NAME}ModuleLogic
  )
if(Slicer_BUILD_CLI_SUPPORT)
  list(APPEND MODULE_TARGET_LIBRARIES
    qSlicerBaseQTCLI
    )
endif()

set(MODULE_RESOURCES
  )

#-----------------------------------------------------------------------------
slicerMacroBuildLoadableModule(
  NAME ${MODULE_NAME}
  EXPORT_DIRECTIVE ${MODULE_EXPORT_DIRECTIVE}
  INCLUDE_DIRECTORIES ${MODULE_INCLUDE_DIRECTORIES}
  SRCS ${MODULE_SRCS}
  MOC_SRCS ${MODULE_MOC_SRCS}
  UI_SRCS ${MODULE_UI_SRCS}
  TARGET_LIBRARIES ${MODULE_TARGET_LIBRARIES}
  RESOURCES ${MODULE_RESOURCES}
  WITH_GENERIC_TESTS
  )

#-----------------------------------------------------------------------------
if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()
