project (GaussianBlurBatchMakeModule)

# Set the name of the Slicer3 execution module to be run using BatchMake
set(Slicer3ExeModule GaussianBlurImageFilter)

# Set the name of the Slicer3 BatchMake module to be created
set(Slicer3BatchModule GaussianBlurBatchMakeModule)

# Set the source code for the Slicer3 BatchMake module
set(Slicer3BatchModule_SOURCE GaussianBlurBatchMakeModule.cxx)

# Set the logo for the module, or comment out the following line
set(Slicer3BatchModule_LOGO GaussianBlurBatchMakeModuleLogo.h)

# Disable MSVC 8 warnings
if(WIN32)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
endif(WIN32)

find_package(BatchMake REQUIRED)
include(${BatchMake_USE_FILE})

if(BatchMake_DIR)

  find_path(Slicer_BINARY_DIR NAMES Base/Logic/SlicerBaseLogicInit.cxx)
  if(Slicer_BINARY_DIR)

    find_program(${Slicer3ExeModule}
      NAMES ${Slicer3ExeModule}.exe ${Slicer3ExeModule}
      PATHS ${Slicer_BINARY_DIR}/bin/relwithdebinfo
      ${Slicer_BINARY_DIR}/bin/release
      ${Slicer_BINARY_DIR}/bin/debug
      ${Slicer_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}
      ${Slicer_BINARY_DIR}/bin
      ${Slicer_INSTALL_CLIMODULES_BIN_DIR}/relwithdebinfo
      ${Slicer_INSTALL_CLIMODULES_BIN_DIR}/release
      ${Slicer_INSTALL_CLIMODULES_BIN_DIR}/debug
      ${Slicer_INSTALL_CLIMODULES_BIN_DIR}/${CMAKE_BUILD_TYPE}
      ${Slicer_INSTALL_CLIMODULES_BIN_DIR}
      ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_CLIMODULES_BIN_DIR}/relwithdebinfo
      ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_CLIMODULES_BIN_DIR}/release
      ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_CLIMODULES_BIN_DIR}/debug
      ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_CLIMODULES_BIN_DIR}/${CMAKE_BUILD_TYPE}
      ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_CLIMODULES_BIN_DIR})

    #
    # TODO The following code will have to be updated to copy the file at build time
    # using a command like:
    #  execute_process(${CMAKE_COMMAND} -E copy ...) and using
    # CMAKE_CFG_INTDIR instead of CMAKE_BUILD_TYPE
    #

    if(WIN32)
      set(BatchMake_WRAPPED_APPLICATION_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE})
    else(WIN32)
      set(BatchMake_WRAPPED_APPLICATION_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    endif(WIN32)

    # Copy the wrapped application for BatchMake.
    configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/${Slicer3ExeModule}.bmm.in"
      "${BatchMake_WRAPPED_APPLICATION_DIR}/${Slicer3ExeModule}.bmm")

    # Configure a header
    configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/SlicerBatchMakeConfig.h.in"
      "${CMAKE_CURRENT_BINARY_DIR}/SlicerBatchMakeConfig.h")

  endif(Slicer_BINARY_DIR)

  # SlicerExecutionModel
  find_package(SlicerExecutionModel REQUIRED GenerateCLP)
  if(GenerateCLP_FOUND)
    include(${GenerateCLP_USE_FILE})
  endif(GenerateCLP_FOUND)

  if(${Slicer3BatchModule_LOGO})
    generateclp(Slicer3BatchModule_SOURCE
      ${Slicer3BatchModule}.xml
      ${CMAKE_CURRENT_SOURCE_DIR}/${Slicer3BatchModule_LOGO})
  else(${Slicer3BatchModule_LOGO})
    generateclp(Slicer3BatchModule_SOURCE
      ${Slicer3BatchModule}.xml)
  endif(${Slicer3BatchModule_LOGO})

  add_executable(${Slicer3BatchModule} ${Slicer3BatchModule_SOURCE})
  slicer3_set_plugins_output_path(${Slicer3BatchModule})
  target_link_libraries(${Slicer3BatchModule} BatchMakeLib)
  add_dependencies(${Slicer3BatchModule} ${Slicer3ExeModule})
  set_target_properties(${Slicer3BatchModule} PROPERTIES LABELS ${Slicer3BatchModule})

  if (Slicer_SOURCE_DIR)
    # install each target in the production area (where it would appear in an
    # installation) and install each target in the developer area (for running
    # from a build)
    set(TARGETS ${Slicer3BatchModule})
    slicer3_install_plugins(${TARGETS})

    set(RUNTIME_SUPPORT_FILES
      "${CMAKE_CURRENT_SOURCE_DIR}/${Slicer3BatchModule}.xml"
      "${BatchMake_WRAPPED_APPLICATION_DIR}/${Slicer3ExeModule}.bmm")
    install(FILES ${RUNTIME_SUPPORT_FILES}
      DESTINATION ${Slicer_INSTALL_CLIMODULES_SHARE_DIR}/${PROJECT_NAME})

  endif(Slicer_SOURCE_DIR)

endif(BatchMake_DIR)
