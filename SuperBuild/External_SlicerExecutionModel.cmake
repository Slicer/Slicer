
set(proj SlicerExecutionModel)

# Set dependency list
set(${proj}_DEPENDENCIES ${ITK_EXTERNAL_NAME})

if(Slicer_USE_TBB)
  list(APPEND ${proj}_DEPENDENCIES tbb)
endif()

if(Slicer_BUILD_PARAMETERSERIALIZER_SUPPORT)
  set(${proj}_DEPENDENCIES ${${proj}_DEPENDENCIES} JsonCpp ParameterSerializer)
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(Slicer_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling Slicer_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED SlicerExecutionModel_DIR AND NOT EXISTS ${SlicerExecutionModel_DIR})
  message(FATAL_ERROR "SlicerExecutionModel_DIR variable is defined but corresponds to nonexistent directory")
endif()

if(NOT DEFINED SlicerExecutionModel_DIR AND NOT Slicer_USE_SYSTEM_${proj})

  set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)
  set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS)
  set(EXTERNAL_PROJECT_ADDITIONAL_FORWARD_PATHS_BUILD)
  set(EXTERNAL_PROJECT_ADDITIONAL_FORWARD_PATHS_INSTALL)

  if(APPLE)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DSlicerExecutionModel_DEFAULT_CLI_EXECUTABLE_LINK_FLAGS:STRING=-Wl,-rpath,@loader_path/../../../
      )
  endif()

  if(Slicer_BUILD_PARAMETERSERIALIZER_SUPPORT)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DJsonCpp_INCLUDE_DIR:PATH=${JsonCpp_INCLUDE_DIR}
      -DParameterSerializer_DIR:PATH=${ParameterSerializer_DIR}
      )
    # JsoncCpp_LIBRARY needs to be added as a CMAKE_ARGS because it contains an
    # expression that needs to be evaluated
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS
      -DJsonCpp_LIBRARY:PATH=${JsonCpp_LIBRARY}
      )
  endif()

  if(Slicer_USE_TBB)
    list(APPEND EXTERNAL_PROJECT_ADDITIONAL_FORWARD_PATHS_BUILD
      ${TBB_BIN_DIR}
      )
    list(APPEND EXTERNAL_PROJECT_ADDITIONAL_FORWARD_PATHS_INSTALL
      ${TBB_BIN_DIR}
      )
  endif()

  # If it applies, prepend "CMAKE_ARGS"
  if(NOT EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS STREQUAL "")
    set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS
      CMAKE_ARGS
      ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS})
  endif()

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_REPOSITORY
    "${EP_GIT_PROTOCOL}://github.com/Slicer/SlicerExecutionModel.git"
    QUIET
    )

  ExternalProject_SetIfNotDefined(
    Slicer_${proj}_GIT_TAG
    "f19d6e88a94ba8f31ddafcff4adf185fe90d7e72"
    QUIET
    )

  set(EP_SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj})
  set(EP_BINARY_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${Slicer_${proj}_GIT_REPOSITORY}"
    GIT_TAG "${Slicer_${proj}_GIT_TAG}"
    SOURCE_DIR ${EP_SOURCE_DIR}
    BINARY_DIR ${EP_BINARY_DIR}
    CMAKE_CACHE_ARGS
      # Compiler settings
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags} # Unused
      -DCMAKE_CXX_STANDARD:STRING=${CMAKE_CXX_STANDARD}
      -DCMAKE_CXX_STANDARD_REQUIRED:BOOL=${CMAKE_CXX_STANDARD_REQUIRED}
      -DCMAKE_CXX_EXTENSIONS:BOOL=${CMAKE_CXX_EXTENSIONS}
      # Options
      -DBUILD_TESTING:BOOL=OFF
      -DSlicerExecutionModel_USE_UTF8:BOOL=ON
      -DITK_DIR:PATH=${ITK_DIR}
      -DSlicerExecutionModel_USE_SERIALIZER:BOOL=${Slicer_BUILD_PARAMETERSERIALIZER_SUPPORT}
      -DSlicerExecutionModel_USE_JSONCPP:BOOL=${Slicer_BUILD_PARAMETERSERIALIZER_SUPPORT}
      -DSlicerExecutionModel_LIBRARY_PROPERTIES:STRING=${Slicer_LIBRARY_PROPERTIES}
      # Output directories
      -DSlicerExecutionModel_DEFAULT_CLI_RUNTIME_OUTPUT_DIRECTORY:PATH=${SlicerExecutionModel_DEFAULT_CLI_RUNTIME_OUTPUT_DIRECTORY}
      -DSlicerExecutionModel_DEFAULT_CLI_LIBRARY_OUTPUT_DIRECTORY:PATH=${SlicerExecutionModel_DEFAULT_CLI_LIBRARY_OUTPUT_DIRECTORY}
      -DSlicerExecutionModel_DEFAULT_CLI_ARCHIVE_OUTPUT_DIRECTORY:PATH=${SlicerExecutionModel_DEFAULT_CLI_ARCHIVE_OUTPUT_DIRECTORY}
      # Install directories
      -DSlicerExecutionModel_INSTALL_BIN_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DSlicerExecutionModel_INSTALL_LIB_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      #-DSlicerExecutionModel_INSTALL_SHARE_DIR:PATH=${Slicer_INSTALL_ROOT}share/${SlicerExecutionModel}
      -DSlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION:STRING=${Slicer_INSTALL_CLIMODULES_BIN_DIR}
      -DSlicerExecutionModel_DEFAULT_CLI_INSTALL_LIBRARY_DESTINATION:STRING=${Slicer_INSTALL_CLIMODULES_LIB_DIR}
      -DSlicerExecutionModel_DEFAULT_CLI_INSTALL_ARCHIVE_DESTINATION:STRING=${Slicer_INSTALL_CLIMODULES_LIB_DIR}
      # Options
      -DSlicerExecutionModel_INSTALL_NO_DEVELOPMENT:BOOL=${Slicer_INSTALL_NO_DEVELOPMENT}
      -DSlicerExecutionModel_DEFAULT_CLI_TARGETS_FOLDER_PREFIX:STRING=Module-
      -DSlicerExecutionModel_ADDITIONAL_FORWARD_PATHS_BUILD:PATH=${EXTERNAL_PROJECT_ADDITIONAL_FORWARD_PATHS_BUILD}
      -DSlicerExecutionModel_ADDITIONAL_FORWARD_PATHS_INSTALL:PATH=${EXTERNAL_PROJECT_ADDITIONAL_FORWARD_PATHS_INSTALL}
      ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
    ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS}
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )

  ExternalProject_GenerateProjectDescription_Step(${proj})

  set(SlicerExecutionModel_DIR ${EP_BINARY_DIR})

  #-----------------------------------------------------------------------------
  # Launcher setting specific to build tree

  set(${proj}_LIBRARY_PATHS_LAUNCHER_BUILD ${SlicerExecutionModel_DIR}/ModuleDescriptionParser/bin/<CMAKE_CFG_INTDIR>)
  mark_as_superbuild(
    VARS ${proj}_LIBRARY_PATHS_LAUNCHER_BUILD
    LABELS "LIBRARY_PATHS_LAUNCHER_BUILD"
    )

else()
  ExternalProject_Add_Empty(${proj} DEPENDS ${${proj}_DEPENDENCIES})
endif()

mark_as_superbuild(
  VARS SlicerExecutionModel_DIR:PATH
  LABELS "FIND_PACKAGE"
  )
