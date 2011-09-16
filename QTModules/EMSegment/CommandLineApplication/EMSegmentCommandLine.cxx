#include "EMSegmentCommandLineCLP.h"
#include "EMSegmentAPIHelper.h"
#include "EMSegmentCommandLineFct.h"
#include "vtkSlicerCommonInterface.h"

#ifdef Slicer3_USE_KWWIDGETS
extern "C" int Atlascreatorcxxmodule_Init(Tcl_Interp *interp);
extern "C" int Slicerbasegui_Init(Tcl_Interp *interp);
#endif

// =======================================================================
//  MAIN
// =======================================================================
int main(int argc, char** argv)
{

  // =======================================================================
  //
  //  Read and check Input
  //
  // =======================================================================

  // parse arguments using the CLP system; this creates variables.
  PARSE_ARGS;

  ProgressReporter progressReporter;
  float currentStep = 0.0f;
  float totalSteps  = 6.0f;

  progressReporter.ReportProgress("Parsing Arguments...",
                                  currentStep++ / totalSteps);

  bool useDefaultParametersNode = parametersMRMLNodeName.empty();
  bool useDefaultTarget         = targetVolumeFileNames.empty();
  bool useDefaultAtlas          = atlasVolumeFileNames.empty();
  bool useDefaultOutput         = resultVolumeFileName.empty();
  bool writeIntermediateResults = !intermediateResultsDirectory.empty();
  bool segmentationSucceeded    = true;

  if (verbose) std::cout << "Starting EMSegment Command Line." << std::endl;

  if (!generateEmptyMRMLSceneAndQuit.empty())
    {
      GenerateEmptyMRMLScene(generateEmptyMRMLSceneAndQuit.c_str());
      return EXIT_SUCCESS;
    }

#ifdef _WIN32
  //
  // strip backslashes from parameter node name (present if spaces were used)
  parametersMRMLNodeName = StripBackslashes(parametersMRMLNodeName);
#endif

  if (verbose) std::cout << "Setting taskPreProcessingSetting: " << taskPreProcessingSetting << std::endl;

  //
  // make sure arguments are sufficient and unique
  bool argsOK = true;
  if (mrmlSceneFileName.empty())
    {
      std::cerr << "Error: mrmlSceneFileName must be specified." << std::endl;
      argsOK = false;
    }
  if (!argsOK)
    {
      std::cerr << "Try --help for usage..." << std::endl;
      return EXIT_FAILURE;
    }

  //
  // make sure files exist
  if (writeIntermediateResults &&
      !vtksys::SystemTools::FileExists(intermediateResultsDirectory.c_str()))
    {
      std::cout << "Warning: intermediate results directory does not exist. "
                << "We will try to create it for you."
                << std::endl;
      std::cout << intermediateResultsDirectory << std::endl;
    }

  if (!vtksys::SystemTools::FileExists(mrmlSceneFileName.c_str()))
    {
      std::cerr << "Error: MRML scene file does not exist." << std::endl;
      std::cerr << mrmlSceneFileName << std::endl;
      return EXIT_FAILURE;
    }

  if (!resultStandardVolumeFileName.empty() &&
      !vtksys::SystemTools::FileExists(resultStandardVolumeFileName.c_str()))
    {
      std::cerr << "Error: result standard volume file does not exist."
                << std::endl;
      std::cerr << resultStandardVolumeFileName << std::endl;
      return EXIT_FAILURE;
    }

  // the gui uses <image>, the command line uses actual files
  for (unsigned int i = 0; i < targetVolumeFileNames.size(); ++i)
    {
      if (!vtksys::SystemTools::
          FileExists(targetVolumeFileNames[i].c_str()))
        {
          std::cerr << "Error: target volume file " << i << " does not exist."
                    << std::endl;
          std::cerr << targetVolumeFileNames[i] << std::endl;
          return EXIT_FAILURE;
        }
    }

  for (unsigned int i = 0; i < atlasVolumeFileNames.size(); ++i)
    {
      if (!vtksys::SystemTools::
          FileExists(atlasVolumeFileNames[i].c_str()))
        {
          std::cerr << "Error: atlas volume file " << i << " does not exist."
                    << std::endl;
          std::cerr << atlasVolumeFileNames[i] << std::endl;
          return EXIT_FAILURE;
        }
    }

  // =======================================================================
  //
  //  Initialize Enviornment
  //
  // =======================================================================

  // SLICER COMMON INTERFACE
  vtkSlicerCommonInterface *slicerCommon = vtkSlicerCommonInterface::New();

  // ================== Tcl  ==================
  Tcl_Interp *interp =CreateTclInterp(argc,argv,slicerCommon);
#ifdef Slicer3_USE_KWWIDGETS
  if (!interp)
    {
      return EXIT_FAILURE;
    }
#endif
   
#ifdef Slicer3_USE_KWWIDGETS
  Atlascreatorcxxmodule_Init(interp);
  Slicerbasegui_Init(interp);
#endif

  // ================== Application  ==================
  vtkSlicerApplicationLogic* appLogic = InitializeApplication(slicerCommon,argc,argv);
  if (!appLogic)
    {
      CleanUp(appLogic,slicerCommon);
      return EXIT_FAILURE;
    }


  // ================== MRMLScene  ==================

  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  vtkMRMLScene::SetActiveScene(mrmlScene);

  // We don't need to observe the scene in the applogic
  //appLogic->SetAndObserveMRMLScene(mrmlScene);

  vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New ( );
  colorLogic->SetMRMLScene(mrmlScene);
  colorLogic->AddDefaultColorNodes();
  colorLogic->SetMRMLScene(NULL);
  colorLogic->Delete();

  // ================== Python via Tcl  ==================
  slicerCommon->RegisterObjectWithTcl(appLogic,"ApplicationLogic");
  slicerCommon->RegisterObjectWithTcl(mrmlScene,"MRMLScene");

  slicerCommon->InitializePythonViaTcl(interp,argc,argv);



  // ================== EMSegmenter  ==================

  // create an instance of vtkEMSegmentLogic and connect it with theMRML scene
  vtkEMSegmentLogic* EMSLogic = vtkEMSegmentLogic::New();
  EMSLogic->SetModuleName("EMSegment");
  EMSLogic->SetAndObserveMRMLScene(mrmlScene);
  EMSLogic->SetMRMLScene(mrmlScene);
  EMSLogic->InitializeEventListeners();

  EMSLogic->RegisterMRMLNodesWithScene();
  std::string EMSLogicTcl = vtksys::SystemTools::DuplicateString(slicerCommon->GetTclNameFromPointer(EMSLogic));

  // For the EMSegment logic, getting and setting of parameters in the
  // MRML scene is delegated to the EMSegment MRML manager.  Get a
  // shortcut to the manager.
  vtkEMSegmentMRMLManager* emMRMLManager = EMSLogic->GetMRMLManager();
  std::string emMRMLManagerTcl = vtksys::SystemTools::DuplicateString(slicerCommon->GetTclNameFromPointer(emMRMLManager));
  emMRMLManager->SetMRMLScene( mrmlScene );

  // ================== Data IO  ==================
  vtkDataIOManagerLogic *dataIOManagerLogic = vtkDataIOManagerLogic::New();

  slicerCommon->AddDataIOToScene(mrmlScene,appLogic,dataIOManagerLogic);

  //
  // global try block makes sure data is cleaned up if anything goes  wrong
  //
  try
    {

      // =======================================================================
      //
      //  Loading Data
      //
      // =======================================================================

      progressReporter.ReportProgress("Loading Data...", currentStep++ / totalSteps, 0.2f);

      // ================== MRML ==============
      mrmlScene->SetURL(mrmlSceneFileName.c_str());
      if (LoadMRMLScene(emMRMLManager, mrmlScene,  verbose))
        {
          throw std::runtime_error("ERROR: failed to import mrml scene.");
        }
      progressReporter.ReportProgress("Loading Data...", currentStep / totalSteps, 0.4f);

      // ================== EMS Template ==================
      if (DefineEMSTemplate(useDefaultParametersNode, parametersMRMLNodeName, emMRMLManager, verbose) )
        {
          throw std::runtime_error("ERROR: failed to set EMSegment parameters.");
        }
      else {
        // don't use manual sampling because the target images might change this is a hack; do better !!!
        emMRMLManager->ChangeTreeNodeDistributionsFromManualSamplingToManual();

        // change the default tcl preprocessing parameter
        if (taskPreProcessingSetting.size())
          {
            emMRMLManager->GetGlobalParametersNode()->SetTaskPreProcessingSetting(taskPreProcessingSetting.c_str());
          }
      }
      progressReporter.ReportProgress("Loading Data...", currentStep / totalSteps,0.6f);

      // ================== Target Images ================
      if (DefineTargetVolume( useDefaultTarget, targetVolumeFileNames, EMSLogic , mrmlScene, appLogic, verbose, loadTargetCentered))
        {
          throw std::runtime_error("ERROR: failed to define target image ");
        }

      progressReporter.ReportProgress("Loading Data...", currentStep / totalSteps, 0.8f);

      // =================== Atlas ====================
      if (!useDefaultAtlas)
        {
          if (LoadUserDefinedAtlas (atlasVolumeFileNames, EMSLogic, mrmlScene, appLogic, verbose, !loadAtlasNonCentered))
            {
              throw std::runtime_error("ERROR: failed to load user specified atlas ");
            }
        }

      // ================== Final Result ================
      if (DefineFinalOutput(useDefaultOutput, resultVolumeFileName, emMRMLManager, mrmlScene, verbose))
        {
          throw std::runtime_error("ERROR: failed to define volume node for final output.");
        }

      // =======================================================================
      //
      //  Update Misc. Parameters
      //
      // =======================================================================
      progressReporter.ReportProgress("Updating Parameters...", currentStep++ / totalSteps, 0.0f);

      // ================== Registration  ==================

      if (!registrationPackage.empty()) {

        if (registrationPackage == "CMTK")
          {
            emMRMLManager->SetRegistrationPackageType(0); //CMTK
          }
        else if (registrationPackage == "BRAINS")
          {
            emMRMLManager->SetRegistrationPackageType(1); // BRAINS
          }
        else if (registrationPackage == "PLASTIMATCH")
          {
            emMRMLManager->SetRegistrationPackageType(2); // PLASTIMATCH
          }
        else if (registrationPackage == "DEMONS")
          { 
            emMRMLManager->SetRegistrationPackageType(3); // DEMONS
          }
        else if (registrationPackage == "DRAMMS")
          { 
            emMRMLManager->SetRegistrationPackageType(4); // DRAMMS
          }
        else if (registrationPackage == "ANTS")
          { 
            emMRMLManager->SetRegistrationPackageType(5); // ANTS
          }
        else
          {
            throw std::runtime_error("ERROR: registration package not known.");
          }
      }

      if (verbose)
        std::cout << "Registration Package is " << registrationPackage << std::endl;

      if ( registrationAffineType != -2) {
        emMRMLManager->SetRegistrationAffineType(registrationAffineType);
      }

      if ( registrationDeformableType != -2) {
        emMRMLManager->SetRegistrationDeformableType(registrationDeformableType);
      }


      // ================== Multithreading  ==================
      if (disableMultithreading != -1) {
        emMRMLManager->SetEnableMultithreading(!disableMultithreading);
      }
      if (verbose)
        std::cout << "Multithreading is "
                  << (disableMultithreading ? "disabled." : "enabled.")
                  << std::endl;

      // ================== Intermediate Results  ==================
      if (dontUpdateIntermediateData != -1) {
        emMRMLManager->SetUpdateIntermediateData(!dontUpdateIntermediateData);
        if (verbose)
           std::cout << "Update intermediate data: "
                  << (dontUpdateIntermediateData ? "disabled." : "enabled.")
                  << std::endl;
      }

      // set intermediate results directory
      if (writeIntermediateResults)
        {
          emMRMLManager->SetSaveIntermediateResults(true);
          vtkstd::string absolutePath = vtksys::SystemTools::
            CollapseFullPath(intermediateResultsDirectory.c_str());
          emMRMLManager->
            SetSaveWorkingDirectory(absolutePath.c_str());
          std::cout << "Intermediate results will be written to: "
                    << absolutePath << std::endl;
        }

      // ================== Segmentation Boundary  ==================
      int segmentationBoundaryMin[3];
      int segmentationBoundaryMax[3];
      emMRMLManager->GetSegmentationBoundaryMin(segmentationBoundaryMin);
      emMRMLManager->GetSegmentationBoundaryMax(segmentationBoundaryMax);
      if (verbose) std::cout
                     << "Default ROI is ["
                     << segmentationBoundaryMin[0] << ", "
                     << segmentationBoundaryMin[1] << ", "
                     << segmentationBoundaryMin[2] << "] -->> ["
                     << segmentationBoundaryMax[0] << ", "
                     << segmentationBoundaryMax[1] << ", "
                     << segmentationBoundaryMax[2] << "]" << std::endl;

      if (verbose) {
        std::cout << "=============== Print EMSegmentMRMLManager" << std::endl;
        // emMRMLManager->PrintInfo(std::cout);
      }

      // ================== Check Frinal Parameter Definition  ==================
      if (!emMRMLManager->CheckMRMLNodeStructureForProcessing())
        {
          throw std::runtime_error("ERROR: EMSegment invalid parameter node structure");
        }

      // =======================================================================
      //
      //  Process Data
      //
      // =======================================================================

      progressReporter.ReportProgress("Running Segmentation...",
                                      currentStep++ / totalSteps);

      try
        {
          // ================== Preprocessing ==================
          if (RunPreprocessing( EMSLogic, EMSLogicTcl,  emMRMLManagerTcl, slicerCommon, emMRMLManager, verbose) ) {
            throw std::runtime_error("");
          }

          // ================== Segmentation ==================
          if (verbose) std::cout << "EMSEG: Start Segmentation." << std::endl;
          if ( EMSLogic->StartSegmentationWithoutPreprocessing(appLogic) == 1 )
            {
              std::cerr << "ERROR: StartSegmentationWithoutPreprocessing failed." << std::endl;
              throw std::runtime_error("");
            }
          if (verbose) std::cout << "Segmentation complete." << std::endl;
        }
      catch (...)
        {
          throw std::runtime_error("ERROR: failed to run preprocessing,segmentation, or postprocessing.");
        }

      //
      // End of global try block
      //
    }
  catch (std::runtime_error& e)
    {
      std::cerr << "EMSegmentCommandline.cxx: Segmentation failed: " << e.what() << std::endl;
      segmentationSucceeded = false;
    }
  catch (...)
    {
      std::cerr << "EMSegmentCommandline.cxx: Unknown error detected.  Segmentation failed." << std::endl;
      segmentationSucceeded = false;
    }

  progressReporter.ReportProgress("Updating Results...",
                                  currentStep++ / totalSteps);

  // =======================================================================
  //
  //  Write out results and clean up
  //
  // =======================================================================

  // ================== Write Out Results  ==================
  if (segmentationSucceeded && !dontWriteResults)
    {
      segmentationSucceeded = WriteResultsToFile(disableCompression,  emMRMLManager, verbose);
    }
  else
    {
      if (verbose) std::cout << "Skipping over saving segmentation results." << std::endl;
    }

  // ================== Compare To Standard==================
  if (segmentationSucceeded && !resultStandardVolumeFileName.empty())
    {
      segmentationSucceeded =  CompareResultsToStandard(resultStandardVolumeFileName,  disableCompression, emMRMLManager, mrmlScene, verbose);
    }

  // ==================Write Out MRML ==================
  if (segmentationSucceeded && !resultMRMLSceneFileName.empty())
    {
      if (verbose) std::cout << "Writing mrml scene...";
      mrmlScene->Commit(resultMRMLSceneFileName.c_str());
      if (verbose) std::cout << "DONE" << std::endl;
    }

  progressReporter.ReportProgress("Cleaning Up...", currentStep++ / totalSteps);


  // ================== Clean Up ==================
  if (verbose) std::cout << "Cleaning up...";


  slicerCommon->RemoveDataIOFromScene(mrmlScene,dataIOManagerLogic);

  dataIOManagerLogic->Delete();
  dataIOManagerLogic = NULL;

  EMSLogic->SetAndObserveMRMLScene(NULL);
  EMSLogic->Delete();

  mrmlScene->Clear(true);
  mrmlScene->Delete();

  CleanUp(appLogic,slicerCommon);

  if (verbose) std::cout << "DONE" << std::endl;

  return segmentationSucceeded ? EXIT_SUCCESS : EXIT_FAILURE;
}
