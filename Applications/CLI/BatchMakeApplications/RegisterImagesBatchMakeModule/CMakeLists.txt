project (RegisterImagesBatchMakeModule)

# Set the name of the Slicer3 execution module to be run using BatchMake
set(Slicer3ExeModule RegisterImages)

# Set the name of the Slicer3 BatchMake module to be created
set(Slicer3BatchModule RegisterImagesBatchMakeModule)

# Set the source code for the Slicer3 BatchMake module
set(Slicer3BatchModule_SOURCE ../GenericBatchMakeModule/BatchMakeModule.cxx)

# Set the logo for the module, or comment out the following line
#set(Slicer3BatchModule_LOGO GaussianBlurBatchMakeModuleLogo.h)

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
  
  find_path(Slicer_BINARY_DIR NAMES Base/Logic/SlicerBaseLogicInit.cxx)
  if(Slicer_BINARY_DIR)

    find_program( ${Slicer3ExeModule}Path 
      NAMES ${Slicer3ExeModule}.exe ${Slicer3ExeModule}
      PATHS ${Slicer_BINARY_DIR}/bin/relwithdebinfo
      ${Slicer_BINARY_DIR}/bin/release
      ${Slicer_BINARY_DIR}/bin/debug
      ${Slicer_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}
      ${Slicer_BINARY_DIR}/bin
      ${Slicer_INSTALL_PLUGINS_BIN_DIR}/relwithdebinfo
      ${Slicer_INSTALL_PLUGINS_BIN_DIR}/release
      ${Slicer_INSTALL_PLUGINS_BIN_DIR}/debug
      ${Slicer_INSTALL_PLUGINS_BIN_DIR}/${CMAKE_BUILD_TYPE}
      ${Slicer_INSTALL_PLUGINS_BIN_DIR}
      ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_PLUGINS_BIN_DIR}/relwithdebinfo
      ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_PLUGINS_BIN_DIR}/release
      ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_PLUGINS_BIN_DIR}/debug
      ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_PLUGINS_BIN_DIR}/${CMAKE_BUILD_TYPE}
      ${CMAKE_BINARY_DIR}/${Slicer_INSTALL_PLUGINS_BIN_DIR})
    #message( >>>>>>> Slicer3ExeModulePath ${AppPath} )
    set( Slicer3ExeModulePath ${${Slicer3ExeModule}Path} )

    # Configure a header
    #configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/SlicerBatchMakeConfig.h.in"
    #  "${CMAKE_CURRENT_BINARY_DIR}/SlicerBatchMakeConfig.h")
    configure_file( "${CMAKE_CURRENT_SOURCE_DIR}/../GenericBatchMakeModule/BatchMakeModuleConfig.h.in"
      "${CMAKE_CURRENT_BINARY_DIR}/BatchMakeModuleConfig.h")

  endif(Slicer_BINARY_DIR)

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
  slicer3_set_plugins_output_path(${Slicer3BatchModule})
  target_link_libraries(${Slicer3BatchModule} BatchMakeLib)
  add_dependencies(${Slicer3BatchModule} ${Slicer3ExeModule})


  if (Slicer_SOURCE_DIR)
    # install each target in the production area (where it would appear in an 
    # installation) and install each target in the developer area (for running 
    # from a build)
    set(TARGETS ${Slicer3BatchModule})
    slicer3_install_plugins(${TARGETS})
    
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
    
    set(RUNTIME_SUPPORT_FILES 
      "${CMAKE_CURRENT_SOURCE_DIR}/${Slicer3BatchModule}.xml" )
#      "${BatchMake_WRAPPED_APPLICATION_DIR}/bmSliceExtractor.bmm"
#      "${BatchMake_WRAPPED_APPLICATION_DIR}/bmGridSend.bmm"
#      "${BatchMake_WRAPPED_APPLICATION_DIR}/bmGridStore.bmm"
#      "${BatchMake_WRAPPED_APPLICATION_DIR}/${Slicer3ExeModule}.bmm")
    install(FILES ${RUNTIME_SUPPORT_FILES} 
      DESTINATION ${Slicer_INSTALL_PLUGINS_SHARE_DIR}/${PROJECT_NAME})

  endif(Slicer_SOURCE_DIR)

endif(BatchMake_DIR)
