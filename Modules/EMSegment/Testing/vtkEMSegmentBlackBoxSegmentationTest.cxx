#include <vector>
#include <string>
#include "vtkMRMLScene.h"
#include "vtkEMSegmentLogic.h"
#include "vtkEMSegmentTestUtilities.h"

int main(int argc, char** argv)
{
  std::vector<std::string> targetImageFilenames;
  std::string mrmlSceneFilename;
  std::string sceneRootDirectory;
  std::string parametersNodeName;
  std::string correctSegmentationFilename;
  enum TestType { ReadScene, CopyScene, ManualCopyScene, SetScene };
  TestType testType = ReadScene;

  //
  // parse command line
  if (argc < 4)
  {
    std::cerr 
      << "Usage: vtkEMSegmentBlackBoxSegmentationTest"  << std::endl
      <<         "<mrmlSceneFilename>"                  << std::endl
      <<         "<scene root directory>"               << std::endl
      <<         "<EMSegmentParametersNodeName>"        << std::endl
      <<         "<correctSegmentationFilename>"        << std::endl
      << std::endl;
    return EXIT_FAILURE;
  }

  mrmlSceneFilename           = argv[1];
  sceneRootDirectory          = argv[2];
  parametersNodeName          = argv[3];
  correctSegmentationFilename = argv[4];

  //
  // create a mrml scene that will hold the data parameters
  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  vtkMRMLScene::SetActiveScene(mrmlScene);
  mrmlScene->SetRootDirectory(sceneRootDirectory.c_str());
  mrmlScene->SetURL(mrmlSceneFilename.c_str());

  //
  // create an instance of vtkEMSegmentLogic and connect it with the
  // MRML scene
  vtkEMSegmentLogic* emLogic = vtkEMSegmentLogic::New();
  emLogic->SetAndObserveMRMLScene(mrmlScene);
  emLogic->RegisterMRMLNodesWithScene();

  try 
  {
    mrmlScene->Import();
  }
  catch (...)
  {
    std::cerr << "Error reading/setting mrml scene: " << std::endl;

    // clean up
    mrmlScene->Clear(true);
    mrmlScene->Delete();
    emLogic->SetAndObserveMRMLScene(NULL);
    emLogic->Delete();

    return EXIT_FAILURE;
  }
  std::cerr << "Imported: " << mrmlScene->GetNumberOfNodes()
            << " nodes." << std::endl;

  //
  // populate the logic class with testing data
  int numParameterSets = emLogic->GetNumberOfParameterSets();
  std::cerr << "Found " << numParameterSets << " EM top level nodes."
            << std::endl;
  bool foundParameters = false;
  for (int i = 0; i < numParameterSets; ++i)
  {
    std::string currentNodeName(emLogic->GetNthParameterSetName(i)); 
    std::cerr << "Node " << i << " name: " << currentNodeName << std::endl;
    if (parametersNodeName == currentNodeName)
    {
      try
      {
        emLogic->SetLoadedParameterSetIndex(i);
      }
      catch (...)
      {
        std::cerr << "Error setting parameter set: " << std::endl;

        // clean up
        mrmlScene->Clear(true);
        mrmlScene->Delete();
        emLogic->SetAndObserveMRMLScene(NULL);
        emLogic->Delete();

        return EXIT_FAILURE;
      }
      foundParameters = true;
      break;
    }
  }

  if (!foundParameters)
  {
    std::cerr << "Error: parameters not found in scene" << std::endl;
    
    // clean up
    mrmlScene->Clear(true);
    mrmlScene->Delete();
    emLogic->SetAndObserveMRMLScene(NULL);
    emLogic->Delete();

    return EXIT_FAILURE;    
  }
  
  //
  // run the segmentation
  try
  {
    emLogic->StartSegmentation();
  }
  catch (...)
  {
    std::cerr << "Error running segmentation: " << std::endl;

    // clean up
    mrmlScene->Clear(true);
    mrmlScene->Delete();
    emLogic->SetAndObserveMRMLScene(NULL);
    emLogic->Delete();

    return EXIT_FAILURE;
  }

  //
  // get a pointer to the results
  std::string resultMRMLID = emLogic->GetOutputVolumeMRMLID();
  std::cerr << "Results stored in mrml node: " << resultMRMLID << std::endl;
  vtkImageData* resultImage = NULL;
  try 
  {
    vtkMRMLVolumeNode* node   = vtkMRMLVolumeNode::
      SafeDownCast(mrmlScene->GetNodeByID(resultMRMLID.c_str()));
    resultImage = node->GetImageData();
    resultImage->SetSpacing(node->GetSpacing());
    resultImage->SetOrigin(node->GetOrigin());
  }
  catch (...)
  {
    std::cerr << "Error geting result from scene: " << std::endl;

    // clean up
    mrmlScene->Clear(true);
    mrmlScene->Delete();
    emLogic->SetAndObserveMRMLScene(NULL);
    emLogic->Delete();

    return EXIT_FAILURE;
  }

  //
  // compare results with standard segmentation results
  bool imagesDiffer = ImageDiff(resultImage, correctSegmentationFilename);
  if (imagesDiffer)
  {
    std::cerr << "Result does not match standard!" << std::endl;

    // clean up
    mrmlScene->Clear(true);
    mrmlScene->Delete();
    emLogic->SetAndObserveMRMLScene(NULL);
    emLogic->Delete();

    return EXIT_FAILURE;    
  }
  else
  {
    std::cerr << "Segmentation matches standard." << std::endl;    
  }

  //
  // clean up
  mrmlScene->Clear(true);
  mrmlScene->Delete();
  emLogic->SetAndObserveMRMLScene(NULL);
  emLogic->Delete();
  
  return EXIT_SUCCESS;
}

