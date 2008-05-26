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
  option(DISABLE_MSVC8_DEPRECATED_WARNINGS 
    "Disable Visual Studio 8 deprecated warnings" ON)
  mark_as_advanced(FORCE DISABLE_MSVC8_DEPRECATED_WARNINGS)  
  if(DISABLE_MSVC8_DEPRECATED_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  endif(DISABLE_MSVC8_DEPRECATED_WARNINGS)
endif(WIN32)

find_package(BatchMake REQUIRED)
include(${BatchMake_USE_FILE})

if(BatchMake_DIR)
  find_program(bmGridSend
    NAMES bmGridSend.exe bmGridSend
    PATHS ${BatchMake_DIR}/bin/relwithdebinfo
    ${BatchMake_DIR}/bin/release
    ${BatchMake_DIR}/bin/debug
    ${BatchMake_DIR}/bin/${CMAKE_BUILD_TYPE}
    ${BatchMake_DIR}/bin)
  
  find_program(bmGridStore
    NAMES bmGridStore.exe bmGridStore
    PATHS ${BatchMake_DIR}/bin/relwithdebinfo
    ${BatchMake_DIR}/bin/release
    ${BatchMake_DIR}/bin/debug
    ${BatchMake_DIR}/bin/${CMAKE_BUILD_TYPE}
    ${BatchMake_DIR}/bin)
  
  find_program(bmSliceExtractor
    NAMES bmSliceExtractor.exe bmSliceExtractor
    PATHS ${BatchMake_DIR}/bin/relwithdebinfo
    ${BatchMake_DIR}/bin/release
    ${BatchMake_DIR}/bin/debug
    ${BatchMake_DIR}/bin/${CMAKE_BUILD_TYPE}
    ${BatchMake_DIR}/bin)

  find_path(Slicer3_BINARY_DIR NAMES Base/Logic/SlicerBaseLogicInit.cxx)
  if(Slicer3_BINARY_DIR)

    find_program(${Slicer3ExeModule}
      NAMES ${Slicer3ExeModule}.exe ${Slicer3ExeModule}
      PATHS ${Slicer3_BINARY_DIR}/bin/relwithdebinfo
      ${Slicer3_BINARY_DIR}/bin/release
      ${Slicer3_BINARY_DIR}/bin/debug
      ${Slicer3_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}
      ${Slicer3_BINARY_DIR}/bin
      ${Slicer3_INSTALL_PLUGINS_BIN_DIR}/relwithdebinfo
      ${Slicer3_INSTALL_PLUGINS_BIN_DIR}/release
      ${Slicer3_INSTALL_PLUGINS_BIN_DIR}/debug
      ${Slicer3_INSTALL_PLUGINS_BIN_DIR}/${CMAKE_BUILD_TYPE}
      ${Slicer3_INSTALL_PLUGINS_BIN_DIR}
      ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/relwithdebinfo
      ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/release
      ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/debug
      ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR}/${CMAKE_BUILD_TYPE}
      ${CMAKE_BINARY_DIR}/${Slicer3_INSTALL_PLUGINS_BIN_DIR})
    
    slicer3_set_default_plugins_output_path()

    if(WIN32)
      set(BatchMake_WRAPPED_APPLICATION_DIR ${EXECUTABLE_OUTPUT_PATH}/${CMAKE_BUILD_TYPE})
    else(WIN32)
      set(BatchMake_WRAPPED_APPLICATION_DIR ${EXECUTABLE_OUTPUT_PATH})
    endif(WIN32)

    # Copy the wrapped application for BatchMake.
    configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/${Slicer3ExeModule}.bmm.in"
      "${BatchMake_WRAPPED_APPLICATION_DIR}/${Slicer3ExeModule}.bmm")

    configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/bmSliceExtractor.bmm.in"
      "${BatchMake_WRAPPED_APPLICATION_DIR}/bmSliceExtractor.bmm")
    configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/bmGridSend.bmm.in"
      "${BatchMake_WRAPPED_APPLICATION_DIR}/bmGridSend.bmm")
    configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/bmGridStore.bmm.in"
      "${BatchMake_WRAPPED_APPLICATION_DIR}/bmGridStore.bmm")

    # Configure a header
    configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/SlicerBatchMakeConfig.h.in"
      "${CMAKE_CURRENT_BINARY_DIR}/SlicerBatchMakeConfig.h")

  endif(Slicer3_BINARY_DIR)

  # Find GenerateCLP
  find_package(GenerateCLP REQUIRED)
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
  target_link_libraries(${Slicer3BatchModule} BatchMakeLib)
  add_dependencies(${Slicer3BatchModule} ${Slicer3ExeModule})

  if (Slicer3_SOURCE_DIR)
    # install each target in the production area (where it would appear in an 
    # installation) and install each target in the developer area (for running 
    # from a build)
    set(TARGETS ${Slicer3BatchModule})
    slicer3_install_plugins(${TARGETS})

    set(RUNTIME_SUPPORT_FILES 
      "${CMAKE_CURRENT_SOURCE_DIR}/${Slicer3BatchModule}.xml"
      "${BatchMake_WRAPPED_APPLICATION_DIR}/bmSliceExtractor.bmm"
      "${BatchMake_WRAPPED_APPLICATION_DIR}/bmGridSend.bmm"
      "${BatchMake_WRAPPED_APPLICATION_DIR}/bmGridStore.bmm"
      "${BatchMake_WRAPPED_APPLICATION_DIR}/${Slicer3BatchMakeModule}.bmm")
    install(FILES ${RUNTIME_SUPPORT_FILES} 
      DESTINATION ${Slicer3_INSTALL_PLUGINS_SHARE_DIR}/${PROJECT_NAME})

  endif(Slicer3_SOURCE_DIR)

endif(BatchMake_DIR)
