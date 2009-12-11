#include <vector>
#include <string>
#include "vtkMRMLScene.h"
#include "vtkEMSegmentLogic.h"
#include "vtkEMSegmentTestUtilities.h"

int main(int argc, char** argv)
{
  std::cerr << "Starting EM black box test..." << std::endl;
  std::vector<std::string> targetImageFilenames;
  std::string mrmlSceneFilename;
  std::string sceneRootDirectory;
  std::string parametersNodeName;
  std::string correctSegmentationFilename;
  enum TestType { ReadScene, CopyScene, ManualCopyScene, SetScene };

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

#ifdef WIN32
  //
  // strip backslashes from parameter node name (present if spaces were used)
  std::string tmpNodeName = parametersNodeName;
  parametersNodeName.clear();
  for (unsigned int i = 0; i < tmpNodeName.size(); ++i)
    {
      if (tmpNodeName[i] != '\\')
        {
        parametersNodeName.push_back(tmpNodeName[i]);
        }
      else if (i > 0 && tmpNodeName[i-1] == '\\')
        {
        parametersNodeName.push_back(tmpNodeName[i]);
        }
    }
#endif

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
  vtkIntArray *emsEvents                 = vtkIntArray::New();
  emsEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  emsEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  emLogic->SetAndObserveMRMLSceneEvents(mrmlScene, emsEvents);
  emsEvents->Delete();

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
  int numParameterSets = emLogic->GetMRMLManager()->GetNumberOfParameterSets();
  std::cerr << "Found " << numParameterSets << " EM top level nodes."
            << std::endl;
  bool foundParameters = false;
  std::cerr << "Searching for an EM parameter node named: " 
            << parametersNodeName << std::endl;

  for (int i = 0; i < numParameterSets; ++i)
  {
    std::string currentNodeName(emLogic->GetMRMLManager()->GetNthParameterSetName(i)); 
    std::cerr << "Node " << i << " name: " << currentNodeName << std::endl;
    if (parametersNodeName == currentNodeName)
    {
      try
      {
        emLogic->GetMRMLManager()->SetLoadedParameterSetIndex(i);
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
    else
      {
      std::cerr << "Found non-matching EM parameters node: " 
                << currentNodeName << std::endl;
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
    std::cerr << "Starting segmentation..." << std::endl;
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
  std::string resultMRMLID = emLogic->GetMRMLManager()->GetOutputVolumeMRMLID();
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

  std::cerr << "All clean..." << std::endl;

  return EXIT_SUCCESS;
}

