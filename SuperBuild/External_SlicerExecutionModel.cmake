
set(proj SlicerExecutionModel)

# Set dependency list
set(${proj}_DEPENDENCIES ${ITK_EXTERNAL_NAME})

if(Slicer_BUILD_PARAMETERSERIALIZER_SUPPORT)
  set(${proj}_DEPENDENCIES ${${proj}_DEPENDENCIES} JsonCpp ParameterSerializer)
endif()

# Include dependent projects if any
ExternalProject_Include_Dependencies(${proj} PROJECT_VAR proj DEPENDS_VAR ${proj}_DEPENDENCIES)

if(${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})
  message(FATAL_ERROR "Enabling ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj} is not supported !")
endif()

# Sanity checks
if(DEFINED SlicerExecutionModel_DIR AND NOT EXISTS ${SlicerExecutionModel_DIR})
  message(FATAL_ERROR "SlicerExecutionModel_DIR variable is defined but corresponds to nonexistent directory")
endif()

if(NOT DEFINED SlicerExecutionModel_DIR AND NOT ${CMAKE_PROJECT_NAME}_USE_SYSTEM_${proj})

  set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS)
  set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS)

  if(APPLE)
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -DSlicerExecutionModel_DEFAULT_CLI_EXECUTABLE_LINK_FLAGS:STRING=-Wl,-rpath,@loader_path/../../../
      )
  endif()

  if(NOT DEFINED git_protocol)
    set(git_protocol "git")
  endif()

  macro(_set var type value)
    set(${var} ${value})
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS
      -D${var}:${type}=${value}
      )
  endmacro()

  _set(SlicerExecutionModel_DEFAULT_CLI_RUNTIME_OUTPUT_DIRECTORY PATH
    ${CMAKE_BINARY_DIR}/${Slicer_BINARY_INNER_SUBDIR}/${Slicer_CLIMODULES_BIN_DIR})
  _set(SlicerExecutionModel_DEFAULT_CLI_LIBRARY_OUTPUT_DIRECTORY PATH
    ${CMAKE_BINARY_DIR}/${Slicer_BINARY_INNER_SUBDIR}/${Slicer_CLIMODULES_LIB_DIR})
  _set(SlicerExecutionModel_DEFAULT_CLI_ARCHIVE_OUTPUT_DIRECTORY PATH
    ${CMAKE_BINARY_DIR}/${Slicer_BINARY_INNER_SUBDIR}/${Slicer_CLIMODULES_LIB_DIR})

  _set(SlicerExecutionModel_DEFAULT_CLI_INSTALL_RUNTIME_DESTINATION STRING
    ${Slicer_INSTALL_CLIMODULES_BIN_DIR})
  _set(SlicerExecutionModel_DEFAULT_CLI_INSTALL_LIBRARY_DESTINATION STRING
    ${Slicer_INSTALL_CLIMODULES_LIB_DIR})
  _set(SlicerExecutionModel_DEFAULT_CLI_INSTALL_ARCHIVE_DESTINATION STRING
    ${Slicer_INSTALL_CLIMODULES_LIB_DIR})

  if(Slicer_BUILD_PARAMETERSERIALIZER_SUPPORT)
    _set(ParameterSerializer_DIR PATH ${ParameterSerializer_DIR})
    _set(JsonCpp_INCLUDE_DIR PATH ${JsonCpp_INCLUDE_DIR})

    # JsoncCpp_LIBRARY needs to be added as a CMAKE_ARGS because it contains an
    # expression that needs to be evaluated
    list(APPEND EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS
      -DJsonCpp_LIBRARY:PATH=${JsonCpp_LIBRARY}
      )
  endif()

  if(NOT EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS STREQUAL "")
    set(EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS
      CMAKE_ARGS
      ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS})
  endif()

  ExternalProject_Add(${proj}
    ${${proj}_EP_ARGS}
    GIT_REPOSITORY "${git_protocol}://github.com/Slicer/SlicerExecutionModel.git"
    GIT_TAG "983d2112ec431af20cafa49ca6cf7bd1a777869a"
    SOURCE_DIR ${CMAKE_BINARY_DIR}/${proj}
    BINARY_DIR ${proj}-build
    CMAKE_CACHE_ARGS
      -DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}
      -DCMAKE_CXX_FLAGS:STRING=${ep_common_cxx_flags}
      -DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}
      -DCMAKE_C_FLAGS:STRING=${ep_common_c_flags} # Unused
      -DBUILD_TESTING:BOOL=OFF
      -DITK_DIR:PATH=${ITK_DIR}
      -DGenerateCLP_USE_SERIALIZER:BOOL=${Slicer_BUILD_PARAMETERSERIALIZER_SUPPORT}
      -DModuleDescriptionParser_USE_SERIALIZER:BOOL=${Slicer_BUILD_PARAMETERSERIALIZER_SUPPORT}
      -DSlicerExecutionModel_USE_JSONCPP:BOOL=${Slicer_BUILD_PARAMETERSERIALIZER_SUPPORT}
      -DSlicerExecutionModel_LIBRARY_PROPERTIES:STRING=${Slicer_LIBRARY_PROPERTIES}
      -DSlicerExecutionModel_INSTALL_BIN_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      -DSlicerExecutionModel_INSTALL_LIB_DIR:PATH=${Slicer_INSTALL_LIB_DIR}
      #-DSlicerExecutionModel_INSTALL_SHARE_DIR:PATH=${Slicer_INSTALL_ROOT}share/${SlicerExecutionModel}
      -DSlicerExecutionModel_INSTALL_NO_DEVELOPMENT:BOOL=${Slicer_INSTALL_NO_DEVELOPMENT}
      -DSlicerExecutionModel_DEFAULT_CLI_TARGETS_FOLDER_PREFIX:STRING=Module-
      ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_CACHE_ARGS}
      ${EXTERNAL_PROJECT_OPTIONAL_CMAKE_ARGS}
    INSTALL_COMMAND ""
    DEPENDS
      ${${proj}_DEPENDENCIES}
    )
  set(SlicerExecutionModel_DIR ${CMAKE_BINARY_DIR}/${proj}-build)

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
