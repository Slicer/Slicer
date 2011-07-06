project( RegisterImagesModuleApplications )

  set(CLP RegisterImages)

  # Find ITK
  find_package( ITK REQUIRED )
  include( ${ITK_USE_FILE} )

  # Find RegisterImagesModule
  find_path( RegisterImagesModule_INCLUDE_PATH
             itkImageToImageRegistrationHelper.h
             DOC "Directory with RegisterImages helper classes" )
  include_directories( ${RegisterImagesModule_INCLUDE_PATH} )

  # SlicerExecutionModel
  find_package(SlicerExecutionModel REQUIRED GenerateCLP)
  include(${GenerateCLP_USE_FILE})

  # Setup the source and paths
  set( RegisterImagesSlicer_SOURCE RegisterImagesSlicer.cxx )
  GENERATECLP( RegisterImagesSlicer_SOURCE
               RegisterImagesSlicer.xml
               ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h )
  include_directories( ${RegisterImagesModuleApplications_BINARY_DIR} )

  # Build the library / plugin
  add_library( ${CLP}Module
               SHARED
               ${RegisterImagesSlicer_SOURCE} )
  if( SUPPORT_2D_IMAGES )
    set_target_properties( ${CLP}Module
                           PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint -DSUPPORT_2D_IMAGES" )
  else( SUPPORT_2D_IMAGES )
    set_target_properties( ${CLP}Module
                           PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint" )
  endif( SUPPORT_2D_IMAGES )

  set_target_properties(${CLP}Module PROPERTIES LABELS ${CLP})
  target_link_libraries(${CLP}Module ${ITK_LIBRARIES})

  # Build the command line
  add_executable( RegisterImages ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx )
  target_link_libraries( RegisterImages RegisterImagesModule )

  # Setup the source and paths
  set( TransformFromFiducials_SOURCE TransformFromFiducials.cxx )
  GENERATECLP( TransformFromFiducials_SOURCE
               TransformFromFiducials.xml
               ${Slicer_SOURCE_DIR}/Resources/NAMICLogo.h )
  include_directories( ${RegisterImagesModuleApplications_BINARY_DIR} )

  # Build the library / plugin
  add_library( TransformFromFiducialsModule
               SHARED
               ${TransformFromFiducials_SOURCE} )
  set_target_properties( TransformFromFiducialsModule
                         PROPERTIES COMPILE_FLAGS "-Dmain=ModuleEntryPoint" )
  target_link_libraries( TransformFromFiducialsModule ${ITK_LIBRARIES} )
  set_target_properties(TransformFromFiducialsModule PROPERTIES LABELS ${CLP})

  # Build the command line
  add_executable( TransformFromFiducials ${CLI_SOURCE_DIR}/Templates/CommandLineSharedLibraryWrapper.cxx)
  target_link_libraries( TransformFromFiducials TransformFromFiducialsModule )
  set_target_properties(TransformFromFiducials PROPERTIES LABELS ${CLP})

  if( Slicer_SOURCE_DIR )
    slicer3_set_plugins_output_path( RegisterImagesModule )
    slicer3_set_plugins_output_path( RegisterImages )

    slicer3_set_plugins_output_path( TransformFromFiducialsModule )
    slicer3_set_plugins_output_path( TransformFromFiducials )

    set(TARGETS
         RegisterImagesModule
         RegisterImages
         TransformFromFiducialsModule
         TransformFromFiducials
         )
    slicer3_install_plugins( ${TARGETS} )
  endif( Slicer_SOURCE_DIR )

  if( Maverick_SOURCE_DIR )

    MOVE_TO_TOP_BINARY( RegisterImagesModule
                        RegisterImagesModule_LIB )
    MOVE_TO_TOP_BINARY( RegisterImages
                        RegisterImages_EXE )

    add_executable( InitialImageToImageRegistrationMethod InitialImageToImageRegistrationMethod.cxx )
    target_link_libraries( InitialImageToImageRegistrationMethod ${ITK_LIBRARIES} )
    set_target_properties(InitialImageToImageRegistrationMethod PROPERTIES LABELS ${CLP})

    add_executable( RigidImageToImageRegistrationMethod RigidImageToImageRegistrationMethod.cxx )
    target_link_libraries( RigidImageToImageRegistrationMethod ${ITK_LIBRARIES} )
    set_target_properties(RigidImageToImageRegistrationMethod PROPERTIES LABELS ${CLP})

    add_executable( AffineImageToImageRegistrationMethod AffineImageToImageRegistrationMethod.cxx )
    target_link_libraries( AffineImageToImageRegistrationMethod ${ITK_LIBRARIES} )
    set_target_properties(AffineImageToImageRegistrationMethod PROPERTIES LABELS ${CLP})

    add_executable( BSplineImageToImageRegistrationMethod BSplineImageToImageRegistrationMethod.cxx )
    target_link_libraries( BSplineImageToImageRegistrationMethod ${ITK_LIBRARIES} )
    set_target_properties(BSplineImageToImageRegistrationMethod PROPERTIES LABELS ${CLP})

    MOVE_TO_TOP_BINARY( InitialImageToImageRegistrationMethod
                        InitialImageToImageRegistrationMethod_EXE )
    MOVE_TO_TOP_BINARY( RigidImageToImageRegistrationMethod
                        RigidImageToImageRegistrationMethod_EXE )
    MOVE_TO_TOP_BINARY( AffineImageToImageRegistrationMethod
                        AffineImageToImageRegistrationMethod_EXE )
    MOVE_TO_TOP_BINARY( BSplineImageToImageRegistrationMethod
                        BSplineImageToImageRegistrationMethod_EXE )

    add_library( RegisterImagesModuleLogic RegisterImagesModuleLogic.cxx )
    target_link_libraries( RegisterImagesModuleLogic
                           MaverickModule ${ITK_LIBRARIES} SlicerBaseLogic
                           ${ITK_LIBRARIES} )
    set_target_properties(RegisterImagesModuleLogic PROPERTIES LABELS ${CLP})
    MOVE_TO_TOP_BINARY( RegisterImagesModuleLogic
                        RegisterImagesModuleLogic_LIB )

    #
    # Setup testing
    #
    if( BUILD_TESTING )

      set( REGISTER_IMAGES_TEST_DATA_ROOT ${CMAKE_CURRENT_BINARY_DIR}/Testing/Data
           CACHE PATH "Location of registration testing data" )
      mark_as_advanced( REGISTER_IMAGES_TEST_DATA_ROOT )
      file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Testing )
      file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Testing/Temporary )
      set( TEST_TEMP ${CMAKE_CURRENT_BINARY_DIR}/Testing/Temporary )

      set( RI_TEST_INPUT_DIR "${REGISTER_IMAGES_TEST_DATA_ROOT}/RegisterImagesTestData-Inputs" )
      set( RI_TEST_BASELINE_DIR "${REGISTER_IMAGES_TEST_DATA_ROOT}/RegisterImagesTestData-RegisterImagesModule" )

      set( UNC_TEST_INPUT_DIR "${REGISTER_IMAGES_TEST_DATA_ROOT}/UNCHealthyBrainMRI-Inputs" )
      set( UNC_TEST_BASELINE_DIR "${REGISTER_IMAGES_TEST_DATA_ROOT}/UNCHealthyBrainMRI-RegisterImagesModule" )

      add_executable( GenerateRegisterImagesTestData GenerateRegisterImagesTestData.cxx )
      target_link_libraries( GenerateRegisterImagesTestData ${ITK_LIBRARIES} )
      set_target_properties(GenerateRegisterImagesTestData PROPERTIES LABELS ${CLP})

      add_test(GenerateFixedImageTestData
        ${CXX_TEST_PATH}/GenerateRegisterImagesTestData
        50 ${TEST_TEMP}/RI_TestData.mha
        )
      set_property(TEST GenerateFixedImageTestData PROPERTY LABELS ${CLP})

      add_test(GenerateRigidTestData
        ${CXX_TEST_PATH}/GenerateRegisterImagesTestData
        50 ${TEST_TEMP}/RI_RigidTestData.mha
        -t ${RegisterImagesModuleApplications_SOURCE_DIR}/Data/RI_RigidUNC24.tfm
        )
      set_property(TEST GenerateRigidTestData PROPERTY LABELS ${CLP})

      add_test(GenerateAffineTestData
        ${CXX_TEST_PATH}/GenerateRegisterImagesTestData
        50 ${TEST_TEMP}/RI_AffineTestData.mha
        -t ${RegisterImagesModuleApplications_SOURCE_DIR}/Data/RI_AffineUNC24.tfm
        )
      set_property(TEST GenerateAffineTestData PROPERTY LABELS ${CLP})

      add_test(GenerateBSplineTestData
        ${CXX_TEST_PATH}/GenerateRegisterImagesTestData
        50 ${TEST_TEMP}/RI_BSplineTestData.mha
        -t ${RegisterImagesModuleApplications_SOURCE_DIR}/Data/RI_BSplineUNC24.tfm
        )
      set_property(TEST GenerateBSplineTestData PROPERTY LABELS ${CLP})

      add_test(RegisterImages_RigidTestData
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
      set_property(TEST RegisterImages PROPERTY LABELS ${CLP})

      add_test(RegisterImages_AffineTestData
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
      set_property(TEST RegisterImages_AffineTestData PROPERTY LABELS ${CLP})

      add_test(RegisterImages_BSplineTestData
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
      set_property(TEST RegisterImages_BSplineTestData PROPERTY LABELS ${CLP})

      option( BUILD_REGISTER_IMAGES_REAL_WORLD_TESTING
              "Build and download MRI data for testing the RegisterImagesModule" OFF )
      if( BUILD_REGISTER_IMAGES_REAL_WORLD_TESTING )
        set( msg "Attention: You have chosen to enable BUILD_REGISTER_IMAGES_REAL_WORLD_TESTING." )
        set( msg "${msg} This module will download MRI data from Kitware's MIDAS server" )
        set( msg "${msg} and run test that may require several hours of computation time." )
        message( "${msg}" )

        #
        # Fetch the data
        #
        macro( DOWNLOAD_IJ_DATA PUBID DIRNAME FILENAME )
          if( NOT EXISTS "${REGISTER_IMAGES_TEST_DATA_ROOT}/${DIRNAME}/${FILENAME}" )
            file(MAKE_DIRECTORY "${REGISTER_IMAGES_TEST_DATA_ROOT}" )
            file(MAKE_DIRECTORY "${REGISTER_IMAGES_TEST_DATA_ROOT}/${DIRNAME}" )
            if( NOT SVNCOMMAND )
              if( Subversion_SVN_EXECUTABLE )
                set( SVNCOMMAND ${Subversion_SVN_EXECUTABLE} )
              else( Subversion_SVN_EXECUTABLE )
                find_package( Subversion REQUIRED )
                if( Subversion_FOUND )
                  set( SVNCOMMAND ${Subversion_SVN_EXECUTABLE} )
                else( Subversion_FOUND )
                  message( FATAL_ERROR "Subversion is required to download testing data." )
                endif( Subversion_FOUND )
              endif( Subversion_SVN_EXECUTABLE )
            endif( NOT SVNCOMMAND )
            set( svnUser "--username=aylward@unc.edu" )
            set( svnPassword "--password=testing" )
            set( svnCmd "export" )
            set( svnSite "http://www.insight-journal.org/svn/IJ/${PUBID}/${FILENAME}" )
            set( svnDir "${REGISTER_IMAGES_TEST_DATA_ROOT}/${DIRNAME}/${FILENAME}" )
            message( STATUS "Downloading ${FILENAME}" )
            execute_process(
                  COMMAND "${SVNCOMMAND}" ${svnUser} ${svnPassword} ${svnCmd} ${svnSite} ${svnDir}
                  OUTPUT_VARIABLE svnOutput
                  ERROR_VARIABLE svnError
                  RESULT_VARIABLE svnResult
                  )
            # Result should be 0 on success
            if(svnResult)
              message(FATAL_ERROR "SVN checkout of ${FILENAME} failed with error = ${svnError}")
            endif(svnResult)
          endif( NOT EXISTS "${REGISTER_IMAGES_TEST_DATA_ROOT}/${DIRNAME}/${FILENAME}" )
        endmacro( DOWNLOAD_IJ_DATA PUBID DIRNAME FILENAME )

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
        add_test(InitialRegistrationTestUNC24
          ${CXX_TEST_PATH}/RegisterImages
          --registration Initial
          --resampledImage ${TEST_TEMP}/InitialUNC24.mha
          --baselineImage ${UNC_TEST_BASELINE_DIR}/InitialUNC24.mha
          --baselineDifferenceImage ${TEST_TEMP}/InitialUNC24_Diff.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash.mha
          )
        set_property(TEST InitialRegistrationTestUNC24 PROPERTY LABELS ${CLP})

        add_test(InitialRegistrationTestUNC24Stripped
          ${CXX_TEST_PATH}/RegisterImages
          --registration Initial
          --resampledImage ${TEST_TEMP}/InitialUNC24Stripped.mha
          --baselineImage ${UNC_TEST_BASELINE_DIR}/InitialUNC24Stripped.mha
          --baselineDifferenceImage ${TEST_TEMP}/InitialUNC24Stripped_Diff.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash-Stripped.mha
          )
        set_property(TEST InitialRegistrationTestUNC24Stripped PROPERTY LABELS ${CLP})

        ###################
        # Rigid
        ###################
        add_test(RigidRegistrationTestUNC24
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
        set_property(TEST RigidRegistrationTestUNC24 PROPERTY LABELS ${CLP})

        add_test(RigidRegistrationTestUNC24Stripped
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
        set_property(TEST RigidRegistrationTestUNC24Stripped PROPERTY LABELS ${CLP})

        ###################
        # Affine
        ###################
        add_test(AffineRegistrationTestUNC24
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
        set_property(TEST AffineRegistrationTestUNC24 PROPERTY LABELS ${CLP})

        add_test(AffineRegistrationTestUNC24Stripped
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
        set_property(TEST AffineRegistrationTestUNC24Stripped PROPERTY LABELS ${CLP})

        ###################
        # BSpline
        ###################
        add_test(BSplineRegistrationTestUNC24
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
        set_property(TEST BSplineRegistrationTestUNC24 PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC24StrippedC5
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
        set_property(TEST BSplineRegistrationTestUNC24StrippedC5 PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC24StrippedC10
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
        set_property(TEST BSplineRegistrationTestUNC24StrippedC10 PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC24StrippedC15
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
        set_property(TEST BSplineRegistrationTestUNC24StrippedC15 PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC25
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
        set_property(TEST BSplineRegistrationTestUNC25 PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC25Stripped
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
        set_property(TEST BSplineRegistrationTestUNC25Stripped PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC42
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
        set_property(TEST BSplineRegistrationTestUNC42 PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC42Stripped
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
        set_property(TEST BSplineRegistrationTestUNC42Stripped PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC45
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
        set_property(TEST BSplineRegistrationTestUNC45 PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC45Stripped
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
        set_property(TEST BSplineRegistrationTestUNC45Stripped PROPERTY LABELS ${CLP})

        ###################
        # Initial
        ###################
        add_test(InitialRegistrationTestUNC24
          ${CXX_TEST_PATH}/InitialImageToImageRegistrationMethod
          --ResampledMovingImage ${TEST_TEMP}/InitialUNC24.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/InitialUNC24.mha
          --DifferenceImage ${TEST_TEMP}/InitialUNC24_Diff.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash.mha
          )
        set_property(TEST InitialRegistrationTestUNC24 PROPERTY LABELS ${CLP})

        add_test(InitialRegistrationTestUNC24Stripped
          ${CXX_TEST_PATH}/InitialImageToImageRegistrationMethod
          --ResampledMovingImage ${TEST_TEMP}/InitialUNC24Stripped.mha
          --BaselineResampledMovingImage ${UNC_TEST_BASELINE_DIR}/InitialUNC24Stripped.mha
          --DifferenceImage ${TEST_TEMP}/InitialUNC24Stripped_Diff.mha
          ${UNC_TEST_INPUT_DIR}/Normal002-T1-Flash-Stripped.mha
          ${UNC_TEST_INPUT_DIR}/Normal004-T1-Flash-Stripped.mha
          )
        set_property(TEST InitialRegistrationTestUNC24Stripped PROPERTY LABELS ${CLP})

        ###################
        # Rigid
        ###################
        add_test(RigidRegistrationTestUNC24
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
        set_property(TEST RigidRegistrationTestUNC24 PROPERTY LABELS ${CLP})

        add_test(RigidRegistrationTestUNC24Stripped
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
        set_property(TEST RigidRegistrationTestUNC24Stripped PROPERTY LABELS ${CLP})

        ###################
        # Affine
        ###################
        add_test(AffineRegistrationTestUNC24
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
        set_property(TEST AffineRegistrationTestUNC24 PROPERTY LABELS ${CLP})

        add_test(AffineRegistrationTestUNC24Stripped
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
        set_property(TEST AffineRegistrationTestUNC24Stripped PROPERTY LABELS ${CLP})

        ###################
        # BSpline
        ###################
        add_test(BSplineRegistrationTestUNC24
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
        set_property(TEST BSplineRegistrationTestUNC24 PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC24StrippedC5
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
        set_property(TEST BSplineRegistrationTestUNC24StrippedC5 PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC24Stripped
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
        set_property(TEST BSplineRegistrationTestUNC24Stripped PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC24StrippedC15
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
        set_property(TEST BSplineRegistrationTestUNC24StrippedC15 PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC24StrippedC20
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
        set_property(TEST BSplineRegistrationTestUNC24StrippedC20 PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC25
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
        set_property(TEST BSplineRegistrationTestUNC25 PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC25Stripped
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
        set_property(TEST BSplineRegistrationTestUNC25Stripped PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC42
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
        set_property(TEST BSplineRegistrationTestUNC42 PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC42Stripped
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
        set_property(TEST BSplineRegistrationTestUNC42Stripped PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC45
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
        set_property(TEST BSplineRegistrationTestUNC45 PROPERTY LABELS ${CLP})

        add_test(BSplineRegistrationTestUNC45Stripped
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
        set_property(TEST BSplineRegistrationTestUNC45Stripped PROPERTY LABELS ${CLP})

      endif( BUILD_REGISTER_IMAGES_REAL_WORLD_TESTING )

    endif( BUILD_TESTING )

  endif( Maverick_SOURCE_DIR )

