
project( RegisterImagesMultiResApplications )

  set(CLP RegisterImagesMultiRes)

  # Find ITK
  find_package( ITK REQUIRED )
  include( ${ITK_USE_FILE} )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/vtkRegisterImagesModuleMultiResApplicationsConfigure.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/vtkRegisterImagesModuleMultiResApplicationsConfigure.h
    )

  # Find GenerateCLP
  find_package( GenerateCLP REQUIRED )
  include( ${GenerateCLP_USE_FILE} )

  # Setup the source and paths
  set( RegisterImagesMultiRes_SOURCE
    RegisterImagesMultiRes.cxx
    )
  GENERATECLP( RegisterImagesMultiRes_SOURCE
               RegisterImagesMultiRes.xml
               ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)
  include_directories( ${RegisterImagesMultiResApplications_BINARY_DIR} )
  include_directories( ${RegisterImagesMultiResApplications_SOURCE_DIR} )

  set( RegisterImagesMultiRes_SOURCE
    ${RegisterImagesMultiRes_SOURCE}
    ScalingHeuristics.cxx
    ConvertSlicerROIToRegion.cxx)

  # Build the library / plugin
  add_library( RegisterImagesMultiResModule
               SHARED
               ${RegisterImagesMultiRes_SOURCE} )
  set_target_properties( RegisterImagesMultiResModule
                         PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint" )
  target_link_libraries( RegisterImagesMultiResModule ${ITK_LIBRARIES} )
  set_target_properties(${CLP}Module PROPERTIES LABELS ${CLP})

  # Build the command line
  add_executable( RegisterImagesMultiRes ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx )
  target_link_libraries( RegisterImagesMultiRes RegisterImagesMultiResModule )
  set_target_properties(${CLP} PROPERTIES LABELS ${CLP})

  if( Slicer_SOURCE_DIR )
    slicer3_set_plugins_output_path( RegisterImagesMultiResModule )
    slicer3_set_plugins_output_path( RegisterImagesMultiRes )

    set(TARGETS
         RegisterImagesMultiResModule
         RegisterImagesMultiRes
         )
    slicer3_install_plugins( ${TARGETS} )
  endif( Slicer_SOURCE_DIR )

  #
  # Setup testing
  #
  if( BUILD_TESTING )
    add_subdirectory( Testing )
  endif( BUILD_TESTING )
