#include "EMSegmentAPIHelper.h"
#include "EMSegmentTransformToNewFormatCLP.h"
#include "vtkSlicerColorLogic.h"
#include "vtkEMSegmentLogic.h"

int main(int argc, char** argv)
{
  // parse arguments using the CLP system; this creates variables.
  PARSE_ARGS;

  // make sure arguments are sufficient and unique
  bool argsOK = true;
  if (inputMRMLFileName.empty() || !vtksys::SystemTools::FileExists(inputMRMLFileName.c_str()) || !vtksys::SystemTools::FileIsFullPath(inputMRMLFileName.c_str()))
    {
      std::cerr << "Error: inputMRMLFileName must be specified or file does not exists or it is not specified via the absolute path." << std::endl;
      argsOK = false;
    }

  if (outputMRMLFileName.empty())
    {
      std::cerr << "Error: outputMRMLFileName must be specified." << std::endl;
      argsOK = false;
    }

  if (vtksys::SystemTools::GetParentDirectory(inputMRMLFileName.c_str()) != vtksys::SystemTools::GetParentDirectory(outputMRMLFileName.c_str()) )
    {
      std::cerr << "Error: inputMRMLFileName and outputMRMLFileName must be in the same directory." << std::endl;
      argsOK = false;
    }

  if (!argsOK)
    {
      std::cerr << "Try --help for usage..." << std::endl;
      return EXIT_FAILURE;
    }

  // =======================================================================
  //
  //  Initialize
  //
  // =======================================================================

  // SLICER COMMON INTERFACE
  vtkSlicerCommonInterface *slicerCommon = vtkSlicerCommonInterface::New();

  // interp has to be set to initialize vtkSlicer
  CreateTclInterp(argc,argv,slicerCommon);

  vtkSlicerApplicationLogic* appLogic = InitializeApplication(slicerCommon,argc,argv);
  if (!appLogic)
    {
      CleanUp(appLogic,slicerCommon);
      return EXIT_FAILURE;
    }

  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  vtkMRMLScene::SetActiveScene(mrmlScene);
  appLogic->SetAndObserveMRMLScene(mrmlScene);
  std::string appLogicTcl = vtksys::SystemTools::DuplicateString(slicerCommon->GetTclNameFromPointer(appLogic));

  std::ostringstream os2;
  os2 << "namespace eval slicer3 set ApplicationLogic ";
  os2 << appLogicTcl;
  slicerCommon->EvaluateTcl(os2.str().c_str());

  vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New ( );
  colorLogic->SetMRMLScene(mrmlScene);
  colorLogic->AddDefaultColorNodes();
  colorLogic->SetMRMLScene(NULL);
  colorLogic->Delete();

  vtkEMSegmentLogic* emLogic = vtkEMSegmentLogic::New();
  emLogic->SetModuleName("EMSegment");
  emLogic->SetAndObserveMRMLScene(mrmlScene);
  emLogic->RegisterMRMLNodesWithScene();

  vtkIntArray* emsEvents                 = vtkIntArray::New();
  emsEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  emsEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  emLogic->SetAndObserveMRMLSceneEvents(mrmlScene, emsEvents);
  emsEvents->Delete();

  vtkDataIOManagerLogic *dataIOManagerLogic = vtkDataIOManagerLogic::New();
  slicerCommon->AddDataIOToScene(mrmlScene,appLogic,dataIOManagerLogic);

  vtkEMSegmentMRMLManager* emMRMLManager = emLogic->GetMRMLManager();

  // =======================================================================
  //
  //  Start Processing
  //
  // =======================================================================
  bool success = true;
  try
    {
      try
        {
          //
          // read the mrml scene
          mrmlScene->SetURL(inputMRMLFileName.c_str());
          std::cout << "Reading MRML scene..." << endl;
          //
          mrmlScene->Connect();
          std::cout << "DONE" << std::endl;
        }
      catch (...)
        {
          throw std::runtime_error("ERROR: failed to import mrml scene.");
        }

      //
      //  Transform to new style
      //  GetNumberOfParameterSets calls RemoveLegacyNodes , which removes the old nodes 
      //
      int numParameterSets = emMRMLManager->GetNumberOfParameterSets();
      std::cerr << "Imported: " << mrmlScene->GetNumberOfNodes()
                << (mrmlScene->GetNumberOfNodes() == 1
                    ? " node" : " nodes")
                << ", including " << numParameterSets
                << " EM parameter "
                << (numParameterSets == 1 ? "node." : "nodes.")
                << std::endl;

      // make sure there is at least one parameter set
      if (numParameterSets < 1)
        {
          throw std::runtime_error("ERROR: no EMSegment parameter nodes in scene.");
        }

      // Look at all path if they are correct

      //
      // Write results back
      //
      if (templateFlag)
        {
          // As a Template
          std::cout << "Using default parameter set named: "
                    << emMRMLManager->GetNthParameterSetName(0)
                    << std::endl;

          //
          // populate the mrmlManager with the parameters
          try
            {
              emMRMLManager->SetLoadedParameterSetIndex(0);
            }
          catch (...)
            {
              throw std::runtime_error("ERROR: failed to set EMSegment parameters.");
            }

          emMRMLManager->SetSaveTemplateFilename(outputMRMLFileName.c_str());
          emMRMLManager->CreateTemplateFile();
        }
      else
        {
          // Just save it back
          mrmlScene->Commit(outputMRMLFileName.c_str());
        }
      //
      // Done
      //
    }
  catch (...)
    {
      std::cerr << "Error: " << std::endl;
      success = false;
    }

  //
  // clean up
  //
  std::cout << "Cleaning up...";

  emLogic->SetAndObserveMRMLScene(NULL);
  slicerCommon->RemoveDataIOFromScene(mrmlScene,dataIOManagerLogic);

  CleanUp(appLogic,slicerCommon);

  emLogic->Delete();

  dataIOManagerLogic->Delete();
  dataIOManagerLogic = NULL;

  mrmlScene->Clear(true);
  mrmlScene->Delete();

  std::cout << "DONE" << std::endl;

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

