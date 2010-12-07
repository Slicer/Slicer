PROJECT( RegisterImagesModuleApplications )

  SET(CLP RegisterImages)
  
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
               ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h )
  INCLUDE_DIRECTORIES( ${RegisterImagesModuleApplications_BINARY_DIR} )

  # Build the library / plugin
  ADD_LIBRARY( ${CLP}Module 
               SHARED 
               ${RegisterImagesSlicer_SOURCE} )
  IF( SUPPORT_2D_IMAGES )
    SET_TARGET_PROPERTIES( ${CLP}Module 
                           PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint -DSUPPORT_2D_IMAGES" )
  ELSE( SUPPORT_2D_IMAGES )
    SET_TARGET_PROPERTIES( ${CLP}Module 
                           PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint" )
  ENDIF( SUPPORT_2D_IMAGES )

  SET_TARGET_PROPERTIES(${CLP}Module PROPERTIES LABELS ${CLP})
  TARGET_LINK_LIBRARIES(${CLP}Module ${ITK_LIBRARIES})

  # Build the command line
  ADD_EXECUTABLE( RegisterImages ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx )
  TARGET_LINK_LIBRARIES( RegisterImages RegisterImagesModule )

  # Setup the source and paths
  SET( TransformFromFiducials_SOURCE TransformFromFiducials.cxx )
  GENERATECLP( TransformFromFiducials_SOURCE 
               TransformFromFiducials.xml
               ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h )
  INCLUDE_DIRECTORIES( ${RegisterImagesModuleApplications_BINARY_DIR} )

  # Build the library / plugin
  ADD_LIBRARY( TransformFromFiducialsModule 
               SHARED 
               ${TransformFromFiducials_SOURCE} )
  SET_TARGET_PROPERTIES( TransformFromFiducialsModule 
                         PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint" )
  TARGET_LINK_LIBRARIES( TransformFromFiducialsModule ${ITK_LIBRARIES} )
  SET_TARGET_PROPERTIES(TransformFromFiducialsModule PROPERTIES LABELS ${CLP})

  # Build the command line
  ADD_EXECUTABLE( TransformFromFiducials ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
  TARGET_LINK_LIBRARIES( TransformFromFiducials TransformFromFiducialsModule )
  SET_TARGET_PROPERTIES(TransformFromFiducials PROPERTIES LABELS ${CLP})
  
  IF( Slicer_SOURCE_DIR )
    slicer3_set_plugins_output_path( RegisterImagesModule )
    slicer3_set_plugins_output_path( RegisterImages )

    slicer3_set_plugins_output_path( TransformFromFiducialsModule )
    slicer3_set_plugins_output_path( TransformFromFiducials )

    SET(TARGETS
         RegisterImagesModule
         RegisterImages
         TransformFromFiducialsModule
         TransformFromFiducials
         )
    slicer3_install_plugins( ${TARGETS} )
  ENDIF( Slicer_SOURCE_DIR )

  IF( Maverick_SOURCE_DIR )

    MOVE_TO_TOP_BINARY( RegisterImagesModule 
                        RegisterImagesModule_LIB )
    MOVE_TO_TOP_BINARY( RegisterImages 
                        RegisterImages_EXE )

    ADD_EXECUTABLE( InitialImageToImageRegistrationMethod InitialImageToImageRegistrationMethod.cxx )
    TARGET_LINK_LIBRARIES( InitialImageToImageRegistrationMethod ITKStatistics ITKIO ITKNumerics )
    SET_TARGET_PROPERTIES(InitialImageToImageRegistrationMethod PROPERTIES LABELS ${CLP})
 
    ADD_EXECUTABLE( RigidImageToImageRegistrationMethod RigidImageToImageRegistrationMethod.cxx )
    TARGET_LINK_LIBRARIES( RigidImageToImageRegistrationMethod ITKStatistics ITKIO ITKNumerics )
    SET_TARGET_PROPERTIES(RigidImageToImageRegistrationMethod PROPERTIES LABELS ${CLP})
  
    ADD_EXECUTABLE( AffineImageToImageRegistrationMethod AffineImageToImageRegistrationMethod.cxx )
    TARGET_LINK_LIBRARIES( AffineImageToImageRegistrationMethod ITKStatistics ITKIO ITKNumerics )
    SET_TARGET_PROPERTIES(AffineImageToImageRegistrationMethod PROPERTIES LABELS ${CLP})
  
    ADD_EXECUTABLE( BSplineImageToImageRegistrationMethod BSplineImageToImageRegistrationMethod.cxx )
    TARGET_LINK_LIBRARIES( BSplineImageToImageRegistrationMethod ITKStatistics ITKIO ITKNumerics )
    SET_TARGET_PROPERTIES(BSplineImageToImageRegistrationMethod PROPERTIES LABELS ${CLP})
    
    MOVE_TO_TOP_BINARY( InitialImageToImageRegistrationMethod
                        InitialImageToImageRegistrationMethod_EXE )
    MOVE_TO_TOP_BINARY( RigidImageToImageRegistrationMethod
                        RigidImageToImageRegistrationMethod_EXE )
    MOVE_TO_TOP_BINARY( AffineImageToImageRegistrationMethod
                        AffineImageToImageRegistrationMethod_EXE )
    MOVE_TO_TOP_BINARY( BSplineImageToImageRegistrationMethod
                        BSplineImageToImageRegistrationMethod_EXE )

    ADD_LIBRARY( RegisterImagesModuleLogic RegisterImagesModuleLogic.cxx )
    TARGET_LINK_LIBRARIES( RegisterImagesModuleLogic
                           MaverickModule ${ITK_LIBRARIES} SlicerBaseLogic
                           ITKIO ITKCommon ITKStatistics ITKNumerics )
    SET_TARGET_PROPERTIES(RegisterImagesModuleLogic PROPERTIES LABELS ${CLP})
    MOVE_TO_TOP_BINARY( RegisterImagesModuleLogic
                        RegisterImagesModuleLogic_LIB )

    #
    # Setup testing
    #
    IF( BUILD_TESTING )
  
      SET( REGISTER_IMAGES_TEST_DATA_ROOT ${CMAKE_CURRENT_BINARY_DIR}/Testing/Data 
           CACHE PATH "Location of registration testing data" )
      MARK_AS_ADVANCED( REGISTER_IMAGES_TEST_DATA_ROOT )
      FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Testing )
      FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Testing/Temporary )
      SET( TEST_TEMP ${CMAKE_CURRENT_BINARY_DIR}/Testing/Temporary )
  
      SET( RI_TEST_INPUT_DIR "${REGISTER_IMAGES_TEST_DATA_ROOT}/RegisterImagesTestData-Inputs" )
      SET( RI_TEST_BASELINE_DIR "${REGISTER_IMAGES_TEST_DATA_ROOT}/RegisterImagesTestData-RegisterImagesModule" )
  
      SET( UNC_TEST_INPUT_DIR "${REGISTER_IMAGES_TEST_DATA_ROOT}/UNCHealthyBrainMRI-Inputs" )
      SET( UNC_TEST_BASELINE_DIR "${REGISTER_IMAGES_TEST_DATA_ROOT}/UNCHealthyBrainMRI-RegisterImagesModule" )
  
      ADD_EXECUTABLE( GenerateRegisterImagesTestData GenerateRegisterImagesTestData.cxx )
      TARGET_LINK_LIBRARIES( GenerateRegisterImagesTestData ITKStatistics ITKIO ITKNumerics )
      SET_TARGET_PROPERTIES(GenerateRegisterImagesTestData PROPERTIES LABELS ${CLP})
  
      ADD_TEST(GenerateFixedImageTestData
        ${CXX_TEST_PATH}/GenerateRegisterImagesTestData 
        50 ${TEST_TEMP}/RI_TestData.mha
        )
      SET_PROPERTY(TEST GenerateFixedImageTestData PROPERTY LABELS ${CLP})
      
      ADD_TEST(GenerateRigidTestData
        ${CXX_TEST_PATH}/GenerateRegisterImagesTestData 
        50 ${TEST_TEMP}/RI_RigidTestData.mha
        -t ${RegisterImagesModuleApplications_SOURCE_DIR}/Data/RI_RigidUNC24.tfm
        )
      SET_PROPERTY(TEST GenerateRigidTestData PROPERTY LABELS ${CLP})
      
      ADD_TEST(GenerateAffineTestData
        ${CXX_TEST_PATH}/GenerateRegisterImagesTestData 
        50 ${TEST_TEMP}/RI_AffineTestData.mha
        -t ${RegisterImagesModuleApplications_SOURCE_DIR}/Data/RI_AffineUNC24.tfm
        )
      SET_PROPERTY(TEST GenerateAffineTestData PROPERTY LABELS ${CLP})
      
      ADD_TEST(GenerateBSplineTestData
        ${CXX_TEST_PATH}/GenerateRegisterImagesTestData 
        50 ${TEST_TEMP}/RI_BSplineTestData.mha
        -t ${RegisterImagesModuleApplications_SOURCE_DIR}/Data/RI_BSplineUNC24.tfm
        )
      SET_PROPERTY(TEST GenerateBSplineTestData PROPERTY LABELS ${CLP})
 
      ADD_TEST(RegisterImages_RigidTestData
        ${CXX_TEST_PATH}/RegisterImages
        --baselineImage ${TEST_TEMP}/RI_RigidTestData.mha
        --baselineNumberOfFailedPixelsTolerance 4000
        --baselineIntensityTolerance 40
        --baselineRadiusTolerance 1
        --baselineDifferenceImage ${TEST_TEMP}/RI_RigidTestData_Diff.mha
        --saveTransform ${TEST_TEMP}/RI_RigidTestData_Transform.tfm
        --randomNumberSeed 121212
        --registration Rigid
        --rigidMaxIterations 10
        ${TEST_TEMP}/RI_RigidTestData.mha
        ${TEST_TEMP}/RI_TestData.mha
        --resampledImage ${TEST_TEMP}/RI_RigidTestData_Results.mha
        )
      SET_PROPERTY(TEST RegisterImages PROPERTY LABELS ${CLP})
      
      ADD_TEST(RegisterImages_AffineTestData
        ${CXX_TEST_PATH}/RegisterImages
        --baselineImage ${TEST_TEMP}/RI_AffineTestData.mha
        --baselineNumberOfFailedPixelsTolerance 25000
        --baselineIntensityTolerance 40
        --baselineRadiusTolerance 1
        --baselineDifferenceImage ${TEST_TEMP}/RI_AffineTestData_Diff.mha
        --saveTransform ${TEST_TEMP}/RI_AffineTestData_Transform.tfm
        --randomNumberSeed 121212
        --registration Affine
        --affineMaxIterations 10
        ${TEST_TEMP}/RI_AffineTestData.mha
        ${TEST_TEMP}/RI_TestData.mha
        --resampledImage ${TEST_TEMP}/RI_AffineTestData_Results.mha
        )
      SET_PROPERTY(TEST RegisterImages_AffineTestData PROPERTY LABELS ${CLP})
      
      ADD_TEST(RegisterImages_BSplineTestData
        ${CXX_TEST_PATH}/RegisterImages
        --baselineImage ${TEST_TEMP}/RI_BSplineTestData.mha
        --baselineNumberOfFailedPixelsTolerance 40000
        --baselineIntensityTolerance 40
        --baselineRadiusTolerance 1
        --baselineDifferenceImage ${TEST_TEMP}/RI_BSplineTestData_Diff.mha
        --saveTransform ${TEST_TEMP}/RI_BSplineTestData_Transform.tfm
        --randomNumberSeed 121212
        --registration BSpline
        --bsplineMaxIterations 5
        --bsplineSamplingRatio 0.05
        --initialization None
        ${TEST_TEMP}/RI_BSplineTestData.mha
        ${TEST_TEMP}/RI_TestData.mha
        --resampledImage ${TEST_TEMP}/RI_BSplineTestData_Results.mha
        )
      SET_PROPERTY(TEST RegisterImages_BSplineTestData PROPERTY LABELS ${CLP})

      OPTION( BUILD_REGISTER_IMAGES_REAL_WORLD_TESTING 
              "Build and download MRI data for testing the RegisterImagesModule" OFF )
      IF( BUILD_REGISTER_IMAGES_REAL_WORLD_TESTING )
        SET( msg "Attention: You have chosen to enable BUILD_REGISTER_IMAGES_REAL_WORLD_TESTING." )
        SET( msg "${msg} This module will download MRI data from Kitware's MIDAS server" )
        SET( msg "${msg} and run test that may require several hours of computation time." )
        MESSAGE( "${msg}" )
  
        # 
        # Fetch the data
        # 
        MACRO( DOWNLOAD_IJ_DATA PUBID DIRNAME FILENAME )
          IF( NOT EXISTS "${REGISTER_IMAGES_TEST_DATA_ROOT}/${DIRNAME}/${FILENAME}" )
            FILE(MAKE_DIRECTORY "${REGISTER_IMAGES_TEST_DATA_ROOT}" )
            FILE(MAKE_DIRECTORY "${REGISTER_IMAGES_TEST_DATA_ROOT}/${DIRNAME}" )
            IF( NOT SVNCOMMAND )
              IF( Subversion_SVN_EXECUTABLE )
                SET( SVNCOMMAND ${Subversion_SVN_EXECUTABLE} )
              ELSE( Subversion_SVN_EXECUTABLE )
                FIND_PACKAGE( Subversion REQUIRED )
                IF( Subversion_FOUND )
                  SET( SVNCOMMAND ${Subversion_SVN_EXECUTABLE} )
                ELSE( Subversion_FOUND )
                  MESSAGE( FATAL_ERROR "Subversion is required to download testing data." )
                ENDIF( Subversion_FOUND )
              ENDIF( Subversion_SVN_EXECUTABLE )
            ENDIF( NOT SVNCOMMAND )
            SET( svnUser "--username=aylward@unc.edu" )
            SET( svnPassword "--password=testing" )
            SET( svnCmd "export" )
            SET( svnSite "http://www.insight-journal.org/svn/IJ/${PUBID}/${FILENAME}" )
            SET( svnDir "${REGISTER_IMAGES_TEST_DATA_ROOT}/${DIRNAME}/${FILENAME}" )
            MESSAGE( STATUS "Downloading ${FILENAME}" )
            EXECUTE_PROCESS(
                  COMMAND "${SVNCOMMAND}" ${svnUser} ${svnPassword} ${svnCmd} ${svnSite} ${svnDir} 
                  OUTPUT_VARIABLE svnOutput
                  ERROR_VARIABLE svnError
                  RESULT_VARIABLE svnResult
                  )
            # Result should be 0 on success
            IF(svnResult)
              MESSAGE(FATAL_ERROR "SVN checkout of ${FILENAME} failed with error = ${svnError}")
            ENDIF(svnResult)
          ENDIF( NOT EXISTS "${REGISTER_IMAGES_TEST_DATA_ROOT}/${DIRNAME}/${FILENAME}" )
        ENDMACRO( DOWNLOAD_IJ_DATA PUBID DIRNAME FILENAME )
     
        # Standard Inputs
        DOWNLOAD_IJ_DATA( 242 "UNCHealthyBrainMRI-Inputs"
                          "Normal002-T1-Flash.mha" )
        DOWNLOAD_IJ_DATA( 242 "UNCHealthyBrainMRI-Inputs"
                          "Normal002-T1-Flash-Stripped.mha" )
    
        DOWNLOAD_IJ_DATA( 242 "UNCHealthyBrainMRI-Inputs"
                          "Normal004-T1-Flash.mha" )
        DOWNLOAD_IJ_DATA( 242 "UNCHealthyBrainMRI-Inputs"
                          "Normal004-T1-Flash-Stripped.mha" )
    
        DOWNLOAD_IJ_DATA( 242 "UNCHealthyBrainMRI-Inputs"
                          "Normal005-T1-Flash.mha" )
        DOWNLOAD_IJ_DATA( 242 "UNCHealthyBrainMRI-Inputs"
                          "Normal005-T1-Flash-Stripped.mha" )
    
        # Deformable Registration Inputs
        DOWNLOAD_IJ_DATA( 242 "UNCHealthyBrainMRI-Inputs"
                          "Normal002-Normal004-Affine.mha" )
        DOWNLOAD_IJ_DATA( 242 "UNCHealthyBrainMRI-Inputs"
                          "Normal002-Normal004-Stripped-Affine.mha" )
    
        DOWNLOAD_IJ_DATA( 242 "UNCHealthyBrainMRI-Inputs"
                          "Normal002-Normal005-Affine.mha" )
        DOWNLOAD_IJ_DATA( 242 "UNCHealthyBrainMRI-Inputs"
                          "Normal002-Normal005-Stripped-Affine.mha" )
    
        DOWNLOAD_IJ_DATA( 242 "UNCHealthyBrainMRI-Inputs"
                          "Normal004-Normal002-Affine.mha" )
        DOWNLOAD_IJ_DATA( 242 "UNCHealthyBrainMRI-Inputs"
                          "Normal004-Normal002-Stripped-Affine.mha" )
    
        DOWNLOAD_IJ_DATA( 242 "UNCHealthyBrainMRI-Inputs"
                          "Normal004-Normal005-Affine.mha" )
        DOWNLOAD_IJ_DATA( 242 "UNCHealthyBrainMRI-Inputs"
                          "Normal004-Normal005-Stripped-Affine.mha" )
    
   
        # Baselines
        DOWNLOAD_IJ_DATA( 247 "UNCHealthyBrainMRI-RegisterImagesModule" 
                               "InitialUNC24.mha" )
        DOWNLOAD_IJ_DATA( 247 "UNCHealthyBrainMRI-RegisterImagesModule" 
                              "InitialUNC24Stripped.mha" )
        DOWNLOAD_IJ_DATA( 247 "UNCHealthyBrainMRI-RegisterImagesModule" 
                              "RigidUNC24.mha" )
        DOWNLOAD_IJ_DATA( 247 "UNCHealthyBrainMRI-RegisterImagesModule" 
                              "RigidUNC24Stripped.mha" )
        DOWNLOAD_IJ_DATA( 247 "UNCHealthyBrainMRI-RegisterImagesModule" 
                              "AffineUNC24.mha" )
        DOWNLOAD_IJ_DATA( 247 "UNCHealthyBrainMRI-RegisterImagesModule" 
                              "AffineUNC24Stripped.mha" )
        DOWNLOAD_IJ_DATA( 247 "UNCHealthyBrainMRI-RegisterImagesModule" 
                              "BSplineUNC24.mha" )
        DOWNLOAD_IJ_DATA( 247 "UNCHealthyBrainMRI-RegisterImagesModule" 
                              "BSplineUNC24StrippedC5.mha" )
        DOWNLOAD_IJ_DATA( 247 "UNCHealthyBrainMRI-RegisterImagesModule" 
                              "BSplineUNC24Stripped.mha" )
        DOWNLOAD_IJ_DATA( 247 "UNCHealthyBrainMRI-RegisterImagesModule" 
                              "BSplineUNC24StrippedC15.mha" )
        DOWNLOAD_IJ_DATA( 247 "UNCHealthyBrainMRI-RegisterImagesModule" 
                              "BSplineUNC24StrippedC20.mha" )
        DOWNLOAD_IJ_DATA( 247 "UNCHealthyBrainMRI-RegisterImagesModule" 
                              "BSplineUNC25.mha" )
        DOWNLOAD_IJ_DATA( 247 "UNCHealthyBrainMRI-RegisterImagesModule" 
                              "BSplineUNC25Stripped.mha" )
        DOWNLOAD_IJ_DATA( 247 "UNCHealthyBrainMRI-RegisterImagesModule" 
                              "BSplineUNC42.mha" )
        DOWNLOAD_IJ_DATA( 247 "UNCHealthyBrainMRI-RegisterImagesModule" 
                              "BSplineUNC42Stripped.mha" )
        DOWNLOAD_IJ_DATA( 247 "UNCHealthyBrainMRI-RegisterImagesModule" 
                              "BSplineUNC45.mha" )
        DOWNLOAD_IJ_DATA( 247 "UNCHealthyBrainMRI-RegisterImagesModule" 
                              "BSplineUNC45Stripped.mha" )
  
        ###################
        ###################
        #
        #  Define Tests
        #
        ###################
        ###################
        
        ###################
        # Initial
        ###################
        ADD_TEST(InitialRegistrationTestUNC24
          ${CXX_TEST_PATH}/RegisterImages 
          --registration Initial
          --resampledImage ${TEST_TEMP}/InitialUNC24.mha
          --baselineImage ${UNC_TEST_BASELINE_DIR}/InitialUNC24.mha
          --baselineDifferenceImage ${TEST_TEMP}/InitialUNC24_Diff.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash.mha
          )
        SET_PROPERTY(TEST InitialRegistrationTestUNC24 PROPERTY LABELS ${CLP})
        
        ADD_TEST(InitialRegistrationTestUNC24Stripped
          ${CXX_TEST_PATH}/RegisterImages 
          --registration Initial
          --resampledImage ${TEST_TEMP}/InitialUNC24Stripped.mha
          --baselineImage ${UNC_TEST_BASELINE_DIR}/InitialUNC24Stripped.mha
          --baselineDifferenceImage ${TEST_TEMP}/InitialUNC24Stripped_Diff.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash-Stripped.mha
          )
        SET_PROPERTY(TEST InitialRegistrationTestUNC24Stripped PROPERTY LABELS ${CLP})
        
        ###################
        # Rigid
        ###################
        ADD_TEST(RigidRegistrationTestUNC24
          ${CXX_TEST_PATH}/RegisterImages 
          --registration Rigid
          --resampledImage ${TEST_TEMP}/RigidUNC24.mha
          --baselineImage ${UNC_TEST_BASELINE_DIR}/RigidUNC24.mha
          --baselineDifferenceImage ${TEST_TEMP}/RigidUNC24_Diff.mha
          --baselineIntensityTolerance 200
          --baselineRadiusTolerance 1
          --randomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash.mha
          )
        SET_PROPERTY(TEST RigidRegistrationTestUNC24 PROPERTY LABELS ${CLP})
        
        ADD_TEST(RigidRegistrationTestUNC24Stripped
          ${CXX_TEST_PATH}/RegisterImages
          --registration Rigid
          --resampledImage ${TEST_TEMP}/RigidUNC24Stripped.mha
          --baselineImage ${UNC_TEST_BASELINE_DIR}/RigidUNC24Stripped.mha
          --baselineDifferenceImage ${TEST_TEMP}/RigidUNC24Stripped_Diff.mha
          --baselineIntensityTolerance 200
          --baselineRadiusTolerance 1
          --randomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash-Stripped.mha
          )
        SET_PROPERTY(TEST RigidRegistrationTestUNC24Stripped PROPERTY LABELS ${CLP})
        
        ###################
        # Affine
        ###################
        ADD_TEST(AffineRegistrationTestUNC24
          ${CXX_TEST_PATH}/RegisterImages 
          --registration Affine
          --resampledImage ${TEST_TEMP}/AffineUNC24.mha
          --baselineImage ${UNC_TEST_BASELINE_DIR}/AffineUNC24.mha
          --baselineDifferenceImage ${TEST_TEMP}/AffineUNC24_Diff.mha
          --baselineIntensityTolerance 200
          --baselineRadiusTolerance 1
          --randomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash.mha
          )
        SET_PROPERTY(TEST AffineRegistrationTestUNC24 PROPERTY LABELS ${CLP})
        
        ADD_TEST(AffineRegistrationTestUNC24Stripped
          ${CXX_TEST_PATH}/RegisterImages
          --resampledImage ${TEST_TEMP}/AffineUNC24Stripped.mha
          --baselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/AffineUNC24Stripped.mha
          --baselineDifferenceImage ${TEST_TEMP}/AffineUNC24Stripped_Diff.mha
          --baselineIntensityTolerance 200
          --baselineRadiusTolerance 1
          --randomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash-Stripped.mha
          )
        SET_PROPERTY(TEST AffineRegistrationTestUNC24Stripped PROPERTY LABELS ${CLP})
        
        ###################
        # BSpline
        ###################
        ADD_TEST(BSplineRegistrationTestUNC24
          ${CXX_TEST_PATH}/RegisterImages 
          --resampledImage ${TEST_TEMP}/BSplineUNC24.mha
          --baselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC24.mha
          --baselineDifferenceImage ${TEST_TEMP}/BSplineUNC24_Diff.mha
          --minimizeMemory
          --baselineIntensityTolerance 200
          --baselineRadiusTolerance 1
          --randomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-Normal004-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC24 PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC24StrippedC5
          ${CXX_TEST_PATH}/RegisterImages 
          --resampledImage ${TEST_TEMP}/BSplineUNC24StrippedC5.mha
          --baselineImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC24StrippedC5.mha
          --baselineDifferenceImage ${TEST_TEMP}/BSplineUNC24StrippedC5_Diff.mha
          --minimizeMemory
          --controlPointSpacing 40
          --baselineIntensityTolerance 200
          --baselineRadiusTolerance 1
          --randomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-Normal004-Stripped-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC24StrippedC5 PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC24StrippedC10
          ${CXX_TEST_PATH}/RegisterImages 
          --resampledImage ${TEST_TEMP}/BSplineUNC24StrippedC10.mha
          --baselineImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC24StrippedC10.mha
          --baselineDifferenceImage ${TEST_TEMP}/BSplineUNC24StrippedC10_Diff.mha
          --minimizeMemory
          --controlPointSpacing 20
          --baselineIntensityTolerance 200
          --baselineRadiusTolerance 1
          --randomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-Normal004-Stripped-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC24StrippedC10 PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC24StrippedC15
          ${CXX_TEST_PATH}/RegisterImages 
          --resampledImage ${TEST_TEMP}/BSplineUNC24StrippedC15.mha
          --baselineImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC24StrippedC15.mha
          --baselineDifferenceImage ${TEST_TEMP}/BSplineUNC24StrippedC15_Diff.mha
          --minimizeMemory
          --controlPointSpacing 15
          --baselineIntensityTolerance 200
          --baselineRadiusTolerance 1
          --randomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-Normal004-Stripped-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC24StrippedC15 PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC25
          ${CXX_TEST_PATH}/RegisterImages 
          --resampledImage ${TEST_TEMP}/BSplineUNC25.mha
          --baselineImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC25.mha
          --baselineDifferenceImage ${TEST_TEMP}/BSplineUNC25_Diff.mha
          --minimizeMemory
          --baselineIntensityTolerance 200
          --baselineRadiusTolerance 1
          --randomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-Normal005-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC25 PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC25Stripped
          ${CXX_TEST_PATH}/RegisterImages 
          --resampledImage ${TEST_TEMP}/BSplineUNC25Stripped.mha
          --baselineImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC25Stripped.mha
          --baselineDifferenceImage ${TEST_TEMP}/BSplineUNC25Stripped_Diff.mha
          --minimizeMemory
          --baselineIntensityTolerance 200
          --baselineRadiusTolerance 1
          --randomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-Normal005-Stripped-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC25Stripped PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC42
          ${CXX_TEST_PATH}/RegisterImages 
          --resampledImage ${TEST_TEMP}/BSplineUNC42.mha
          --baselineImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC42.mha
          --baselineDifferenceImage ${TEST_TEMP}/BSplineUNC42_Diff.mha
          --minimizeMemory
          --baselineIntensityTolerance 200
          --baselineRadiusTolerance 1
          --randomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-Normal002-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC42 PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC42Stripped
          ${CXX_TEST_PATH}/RegisterImages 
          --resampledImage ${TEST_TEMP}/BSplineUNC42Stripped.mha
          --baselineImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC42Stripped.mha
          --baselineDifferenceImage ${TEST_TEMP}/BSplineUNC42Stripped_Diff.mha
          --minimizeMemory
          --baselineIntensityTolerance 200
          --baselineRadiusTolerance 1
          --randomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-Normal002-Stripped-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC42Stripped PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC45
          ${CXX_TEST_PATH}/RegisterImages
          --resampledImage ${TEST_TEMP}/BSplineUNC45.mha
          --baselineImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC45.mha
          --baselineDifferenceImage ${TEST_TEMP}/BSplineUNC45_Diff.mha
          --minimizeMemory
          --baselineIntensityTolerance 200
          --baselineRadiusTolerance 1
          --randomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-Normal005-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC45 PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC45Stripped
          ${CXX_TEST_PATH}/RegisterImages
          --resampledImage ${TEST_TEMP}/BSplineUNC45Stripped.mha
          --baselineImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC45Stripped.mha
          --baselineDifferenceImage ${TEST_TEMP}/BSplineUNC45Stripped_Diff.mha
          --minimizeMemory
          --baselineIntensityTolerance 200
          --baselineRadiusTolerance 1
          --randomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-Normal005-Stripped-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC45Stripped PROPERTY LABELS ${CLP})

        ###################
        # Initial
        ###################
        ADD_TEST(InitialRegistrationTestUNC24
          ${CXX_TEST_PATH}/InitialImageToImageRegistrationMethod 
          --ResampledMovingImage ${TEST_TEMP}/InitialUNC24.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/InitialUNC24.mha
          --DifferenceImage ${TEST_TEMP}/InitialUNC24_Diff.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash.mha
          )
        SET_PROPERTY(TEST InitialRegistrationTestUNC24 PROPERTY LABELS ${CLP})
        
        ADD_TEST(InitialRegistrationTestUNC24Stripped
          ${CXX_TEST_PATH}/InitialImageToImageRegistrationMethod 
          --ResampledMovingImage ${TEST_TEMP}/InitialUNC24Stripped.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/InitialUNC24Stripped.mha
          --DifferenceImage ${TEST_TEMP}/InitialUNC24Stripped_Diff.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash-Stripped.mha
          )
        SET_PROPERTY(TEST InitialRegistrationTestUNC24Stripped PROPERTY LABELS ${CLP})
        
        ###################
        # Rigid
        ###################
        ADD_TEST(RigidRegistrationTestUNC24
          ${CXX_TEST_PATH}/RigidImageToImageRegistrationMethod 
          --Mode DRAFT
          --ResampledMovingImage ${TEST_TEMP}/RigidUNC24.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/RigidUNC24.mha
          --DifferenceImage ${TEST_TEMP}/RigidUNC24_Diff.mha
          --FailureIntensityTolerance 200
          --FailureOffsetTolerance 1
          --RandomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash.mha
          )
        SET_PROPERTY(TEST RigidRegistrationTestUNC24 PROPERTY LABELS ${CLP})
        
        ADD_TEST(RigidRegistrationTestUNC24Stripped
          ${CXX_TEST_PATH}/RigidImageToImageRegistrationMethod 
          --Mode DRAFT
          --ResampledMovingImage ${TEST_TEMP}/RigidUNC24Stripped.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/RigidUNC24Stripped.mha
          --DifferenceImage ${TEST_TEMP}/RigidUNC24Stripped_Diff.mha
          --FailureIntensityTolerance 200
          --FailureOffsetTolerance 1
          --RandomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash-Stripped.mha
          )
        SET_PROPERTY(TEST RigidRegistrationTestUNC24Stripped PROPERTY LABELS ${CLP})
        
        ###################
        # Affine
        ###################
        ADD_TEST(AffineRegistrationTestUNC24
          ${CXX_TEST_PATH}/AffineImageToImageRegistrationMethod 
          --Mode DRAFT
          --ResampledMovingImage ${TEST_TEMP}/AffineUNC24.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/AffineUNC24.mha
          --DifferenceImage ${TEST_TEMP}/AffineUNC24_Diff.mha
          --FailureIntensityTolerance 200
          --FailureOffsetTolerance 1
          --RandomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash.mha
          )
        SET_PROPERTY(TEST AffineRegistrationTestUNC24 PROPERTY LABELS ${CLP})
        
        ADD_TEST(AffineRegistrationTestUNC24Stripped
          ${CXX_TEST_PATH}/AffineImageToImageRegistrationMethod 
          --Mode DRAFT
          --ResampledMovingImage ${TEST_TEMP}/AffineUNC24Stripped.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/AffineUNC24Stripped.mha
          --DifferenceImage ${TEST_TEMP}/AffineUNC24Stripped_Diff.mha
          --FailureIntensityTolerance 200
          --FailureOffsetTolerance 1
          --RandomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash-Stripped.mha
          )
        SET_PROPERTY(TEST AffineRegistrationTestUNC24Stripped PROPERTY LABELS ${CLP})
        
        ###################
        # BSpline
        ###################
        ADD_TEST(BSplineRegistrationTestUNC24
          ${CXX_TEST_PATH}/BSplineImageToImageRegistrationMethod 
          --Mode DRAFT
          --ResampledMovingImage ${TEST_TEMP}/BSplineUNC24.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC24.mha
          --DifferenceImage ${TEST_TEMP}/BSplineUNC24_Diff.mha
          --MinimizeMemory
          --FailureIntensityTolerance 200
          --FailureOffsetTolerance 1
          --RandomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-Normal004-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC24 PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC24StrippedC5
          ${CXX_TEST_PATH}/BSplineImageToImageRegistrationMethod 
          --Mode DRAFT
          --ResampledMovingImage ${TEST_TEMP}/BSplineUNC24StrippedC5.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC24StrippedC5.mha
          --DifferenceImage ${TEST_TEMP}/BSplineUNC24StrippedC5_Diff.mha
          --MinimizeMemory
          --NumberOfControlPoints 5
          --FailureIntensityTolerance 200
          --FailureOffsetTolerance 1
          --RandomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-Normal004-Stripped-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC24StrippedC5 PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC24Stripped
          ${CXX_TEST_PATH}/BSplineImageToImageRegistrationMethod 
          --Mode DRAFT
          --ResampledMovingImage ${TEST_TEMP}/BSplineUNC24Stripped.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC24Stripped.mha
          --DifferenceImage ${TEST_TEMP}/BSplineUNC24Stripped_Diff.mha
          --MinimizeMemory
          --FailureIntensityTolerance 200
          --FailureOffsetTolerance 1
          --RandomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-Normal004-Stripped-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC24Stripped PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC24StrippedC15
          ${CXX_TEST_PATH}/BSplineImageToImageRegistrationMethod 
          --Mode DRAFT
          --ResampledMovingImage ${TEST_TEMP}/BSplineUNC24StrippedC15.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC24StrippedC15.mha
          --DifferenceImage ${TEST_TEMP}/BSplineUNC24StrippedC15_Diff.mha
          --MinimizeMemory
          --NumberOfControlPoints 15
          --FailureIntensityTolerance 200
          --FailureOffsetTolerance 1
          --RandomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-Normal004-Stripped-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC24StrippedC15 PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC24StrippedC20
          ${CXX_TEST_PATH}/BSplineImageToImageRegistrationMethod 
          --Mode DRAFT
          --ResampledMovingImage ${TEST_TEMP}/BSplineUNC24StrippedC20.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC24StrippedC20.mha
          --DifferenceImage ${TEST_TEMP}/BSplineUNC24StrippedC20_Diff.mha
          --MinimizeMemory
          --NumberOfControlPoints 20
          --FailureIntensityTolerance 200
          --FailureOffsetTolerance 1
          --RandomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-Normal004-Stripped-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC24StrippedC20 PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC25
          ${CXX_TEST_PATH}/BSplineImageToImageRegistrationMethod 
          --Mode DRAFT
          --ResampledMovingImage ${TEST_TEMP}/BSplineUNC25.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC25.mha
          --DifferenceImage ${TEST_TEMP}/BSplineUNC25_Diff.mha
          --MinimizeMemory
          --FailureIntensityTolerance 200
          --FailureOffsetTolerance 1
          --RandomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-Normal005-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC25 PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC25Stripped
          ${CXX_TEST_PATH}/BSplineImageToImageRegistrationMethod 
          --Mode DRAFT
          --ResampledMovingImage ${TEST_TEMP}/BSplineUNC25Stripped.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC25Stripped.mha
          --DifferenceImage ${TEST_TEMP}/BSplineUNC25Stripped_Diff.mha
          --MinimizeMemory
          --FailureIntensityTolerance 200
          --FailureOffsetTolerance 1
          --RandomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-Normal005-Stripped-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC25Stripped PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC42
          ${CXX_TEST_PATH}/BSplineImageToImageRegistrationMethod 
          --Mode DRAFT
          --ResampledMovingImage ${TEST_TEMP}/BSplineUNC42.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC42.mha
          --DifferenceImage ${TEST_TEMP}/BSplineUNC42_Diff.mha
          --MinimizeMemory
          --FailureIntensityTolerance 200
          --FailureOffsetTolerance 1
          --RandomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-Normal002-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC42 PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC42Stripped
          ${CXX_TEST_PATH}/BSplineImageToImageRegistrationMethod 
          --Mode DRAFT
          --ResampledMovingImage ${TEST_TEMP}/BSplineUNC42Stripped.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC42Stripped.mha
          --DifferenceImage ${TEST_TEMP}/BSplineUNC42Stripped_Diff.mha
          --MinimizeMemory
          --FailureIntensityTolerance 200
          --FailureOffsetTolerance 1
          --RandomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-Normal002-Stripped-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC42Stripped PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC45
          ${CXX_TEST_PATH}/BSplineImageToImageRegistrationMethod 
          --Mode DRAFT
          --ResampledMovingImage ${TEST_TEMP}/BSplineUNC45.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC45.mha
          --DifferenceImage ${TEST_TEMP}/BSplineUNC45_Diff.mha
          --MinimizeMemory
          --FailureIntensityTolerance 200
          --FailureOffsetTolerance 1
          --RandomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-Normal005-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC45 PROPERTY LABELS ${CLP})
        
        ADD_TEST(BSplineRegistrationTestUNC45Stripped
          ${CXX_TEST_PATH}/BSplineImageToImageRegistrationMethod
          --Mode DRAFT
          --ResampledMovingImage ${TEST_TEMP}/BSplineUNC45Stripped.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/BSplineUNC45Stripped.mha
          --DifferenceImage ${TEST_TEMP}/BSplineUNC45Stripped_Diff.mha
          --MinimizeMemory
          --FailureIntensityTolerance 200
          --FailureOffsetTolerance 1
          --RandomNumberSeed 121212
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-Normal005-Stripped-Affine.mha
          )
        SET_PROPERTY(TEST BSplineRegistrationTestUNC45Stripped PROPERTY LABELS ${CLP})

      ENDIF( BUILD_REGISTER_IMAGES_REAL_WORLD_TESTING )
    
    ENDIF( BUILD_TESTING )

  ENDIF( Maverick_SOURCE_DIR )

