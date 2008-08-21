
PROJECT( RegisterImagesModuleApplications )

  # Find ITK
  FIND_PACKAGE( ITK REQUIRED )
  INCLUDE( ${ITK_USE_FILE} )
    
  # Find RegisterImagesModule
  FIND_PATH( RegisterImagesModule_INCLUDE_PATH
             itkImageToImageRegistrationHelper.h
             DOC "Directory with RegisterImages helper classes" )
  INCLUDE_DIRECTORIES( ${RegisterImagesModule_INCLUDE_PATH} )
  
  # Find GenerateCLP
  FIND_PACKAGE( GenerateCLP REQUIRED )
  INCLUDE( ${GenerateCLP_USE_FILE} )

  # Setup the source and paths
  SET( RegisterImagesSlicer_SOURCE RegisterImagesSlicer.cxx )
  GENERATECLP( RegisterImagesSlicer_SOURCE 
               RegisterImagesSlicer.xml
               ${RegisterImagesModuleApplications_SOURCE_DIR}/logo.h )
  INCLUDE_DIRECTORIES( ${RegisterImagesModuleApplications_BINARY_DIR} )

  # Build the library / plugin
  ADD_LIBRARY( RegisterImagesModule 
               SHARED 
               ${RegisterImagesSlicer_SOURCE} )
  SET_TARGET_PROPERTIES( RegisterImagesModule 
                         PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint" )
  TARGET_LINK_LIBRARIES( RegisterImagesModule ${ITK_LIBRARIES} MRML )
  slicer3_set_plugins_output_path( RegisterImagesModule )

  # Build the command line
  ADD_EXECUTABLE( RegisterImages ${RegisterImagesSlicer_SOURCE} )
  TARGET_LINK_LIBRARIES( RegisterImages ${ITK_LIBRARIES} MRML )
  slicer3_set_plugins_output_path( RegisterImages )

  IF( Slicer3_SOURCE_DIR )
     SET(TARGETS
         RegisterImagesModule
         RegisterImages
         )
     slicer3_install_plugins( ${TARGETS} )
  ENDIF( Slicer3_SOURCE_DIR )

