#include <iostream>
#include <vector>
#include <string>

#include "vtkMRMLScene.h"
#include "vtkEMSegmentLogic.h"
#include "vtkEMSegmentTestUtilities.h"
#include "EMSegmentCommandLineCLP.h"

int main(int argc, char** argv)
{
  PARSE_ARGS;

//   //
//   // basic arguments
//   std::string                  mrmlSceneFilename;
//   std::vector<std::string>     targetImageFilenames;

//   //
//   // vars for options
//   std::string                  parametersMRMLNodeName;
//   std::string                  parametersMRMLNodeID;

//   std::string                  resultMRMLNodeName;
//   std::string                  resultMRMLNodeID;
//   std::string                  resultVolumeFilename;

//   std::string                  mrmlDataRoot;

//   std::string                  correctSegmentationFilename;

//   bool                         noThreads        = false;
//   bool                         verbose          = false;
//   bool                         help             = false;

//   //
//   // internal options          
//   bool                         verboseArgParsing = false;

//   static int                   EXIT_ARGUMENT_FAILURE = -10;

  if (verbose)
  {
    std::cerr << "parametersMRMLNodeName: " << parametersMRMLNodeName 
              << std::endl;
    std::cerr << "resultMRMLNodeName: " << resultMRMLNodeName << std::endl;
    std::cerr << "resultVolumeFileName: " << resultVolumeFileName << std::endl;
    std::cerr << "mrmlDataRoot: " << mrmlDataRoot << std::endl;
    std::cerr << "mrmlSceneFileName: " << mrmlSceneFileName << std::endl;
    std::cerr << "regionOfInterestBegin: [" 
              << regionOfInterestBegin[0] << ", "
              << regionOfInterestBegin[1] << ", "
              << regionOfInterestBegin[2] << "]" << std::endl;
    std::cerr << "regionOfInterestEnd: [" 
              << regionOfInterestEnd[0] << ", "
              << regionOfInterestEnd[1] << ", "
              << regionOfInterestEnd[2] << "]" << std::endl;
    std::cerr << "noThreads: " << noThreads << std::endl;    
    std::cerr << "verbose: " << verbose << std::endl;    
    std::cerr << "correctSegmentationFileName: " << correctSegmentationFileName << std::endl;
  }

  //
  // make sure arguments are sufficient
  bool argsSufficient = true;
  if (mrmlSceneFileName.empty())
  {
    std::cerr << "Error: mrmlSceneFileName must be specified." << std::endl;
    argsSufficient = false;
  }
  if (mrmlDataRoot.empty())
  {
    std::cerr << "Error: mrmlDataRoot must be specified." << std::endl;
    argsSufficient = false;
  }
  if (!argsSufficient)
  {
    std::cerr << "Try --help for usage..." << std::endl;
    exit(EXIT_FAILURE);
  }
  
  //
  // create a mrml scene that will hold the data parameters
  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  vtkMRMLScene::SetActiveScene(mrmlScene);
  mrmlScene->SetRootDirectory(mrmlDataRoot.c_str());
  mrmlScene->SetURL(mrmlSceneFileName.c_str());

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
  std::cerr << "Searching for an EM parameter node named: " 
            << parametersMRMLNodeName << std::endl;

  for (int i = 0; i < numParameterSets; ++i)
  {
    std::string currentNodeName(emLogic->GetNthParameterSetName(i)); 
    std::cerr << "Node " << i << " name: " << currentNodeName << std::endl;
    if (parametersMRMLNodeName == currentNodeName)
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
  bool imagesDiffer = ImageDiff(resultImage, correctSegmentationFileName);
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
