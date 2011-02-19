
PROJECT( RegisterImagesMultiResApplications )

  SET(CLP RegisterImagesMultiRes)
  
  # Find ITK
  FIND_PACKAGE( ITK REQUIRED )
  INCLUDE( ${ITK_USE_FILE} )
    
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/vtkRegisterImagesModuleMultiResApplicationsConfigure.h.in 
    ${CMAKE_CURRENT_BINARY_DIR}/vtkRegisterImagesModuleMultiResApplicationsConfigure.h
    )

  # Find GenerateCLP
  FIND_PACKAGE( GenerateCLP REQUIRED )
  INCLUDE( ${GenerateCLP_USE_FILE} )

  # Setup the source and paths
  SET( RegisterImagesMultiRes_SOURCE 
    RegisterImagesMultiRes.cxx 
    ) 
  GENERATECLP( RegisterImagesMultiRes_SOURCE
               RegisterImagesMultiRes.xml 
               ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h)
  INCLUDE_DIRECTORIES( ${RegisterImagesMultiResApplications_BINARY_DIR} )
  INCLUDE_DIRECTORIES( ${RegisterImagesMultiResApplications_SOURCE_DIR} )

  SET( RegisterImagesMultiRes_SOURCE
    ${RegisterImagesMultiRes_SOURCE}
    ScalingHeuristics.cxx
    ConvertSlicerROIToRegion.cxx)

  # Build the library / plugin
  ADD_LIBRARY( RegisterImagesMultiResModule
               SHARED 
               ${RegisterImagesMultiRes_SOURCE} )
  SET_TARGET_PROPERTIES( RegisterImagesMultiResModule
                         PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint" )
  TARGET_LINK_LIBRARIES( RegisterImagesMultiResModule ${ITK_LIBRARIES} )
  SET_TARGET_PROPERTIES(${CLP}Module PROPERTIES LABELS ${CLP})
  
  # Build the command line 
  ADD_EXECUTABLE( RegisterImagesMultiRes ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx )
  TARGET_LINK_LIBRARIES( RegisterImagesMultiRes RegisterImagesMultiResModule )
  SET_TARGET_PROPERTIES(${CLP} PROPERTIES LABELS ${CLP})
  
  IF( Slicer_SOURCE_DIR )
    slicer3_set_plugins_output_path( RegisterImagesMultiResModule )
    slicer3_set_plugins_output_path( RegisterImagesMultiRes )

    SET(TARGETS
         RegisterImagesMultiResModule
         RegisterImagesMultiRes
         )
    slicer3_install_plugins( ${TARGETS} )
  ENDIF( Slicer_SOURCE_DIR )
  
  #
  # Setup testing
  #
  IF( BUILD_TESTING )
    ADD_SUBDIRECTORY( Testing )
  ENDIF( BUILD_TESTING )
