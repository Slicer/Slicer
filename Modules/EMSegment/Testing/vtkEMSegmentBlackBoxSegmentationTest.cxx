#include <vector>
#include <string>
#include "vtkMRMLScene.h"
#include "vtkEMSegmentLogic.h"
#include "vtkImageMathematics.h"
#include "vtkImageAccumulate.h"
#include "vtkITKArchetypeImageSeriesReader.h"
#include "vtkITKArchetypeImageSeriesScalarReader.h"

/**
 *
 * Black box test for EMSegmenter.  This test runs the EMSegment
 * algorithm on a data set described in a MRML scene file.  The
 * segmentation results are compared to a known "correct" answer
 * segmentation image file.
 */

//
// This function checks to see if the image stored in standardFilename
// differs from resultData.  True is returned if the images differ,
// false is returned if they are identical.
bool ImageDiff(vtkImageData* resultData, std::string standardFilename)
{
  bool imagesDiffer = false;

  //
  // read segmentation result standard
  vtkITKArchetypeImageSeriesReader* standardReader = 
    vtkITKArchetypeImageSeriesScalarReader::New();
  standardReader->SetArchetype(standardFilename.c_str());
  standardReader->SetOutputScalarTypeToNative();
  standardReader->SetDesiredCoordinateOrientationToNative();
  standardReader->SetUseNativeOriginOn();
  try
  {
    standardReader->Update();
  }
  catch (...)
  {
    std::cerr << "Error reading standard image: " << std::endl;
    standardReader->Delete();
    return true;
  }

  //
  // compare image origin and spacing
  for (unsigned int i = 0; i < 3; ++i)
  {
    if (resultData->GetSpacing()[i] != 
        standardReader->GetOutput()->GetSpacing()[i] ||
        resultData->GetOrigin()[i] != 
        standardReader->GetOutput()->GetOrigin()[i])
    {
      //
      // display spacing and origin info for resultData
      std::cerr << "Image spacing and/or origin does not match standard!" 
                << std::endl;
      std::cerr << "result origin: " 
                << resultData->GetOrigin()[0] << " "
                << resultData->GetOrigin()[1] << " "
                << resultData->GetOrigin()[2] << std::endl;
      std::cerr << "result spacing: " 
                << resultData->GetSpacing()[0] << " "
                << resultData->GetSpacing()[1] << " "
                << resultData->GetSpacing()[2] << std::endl;

      std::cerr << "Standard origin: " 
                << standardReader->GetOutput()->GetOrigin()[0] << " "
                << standardReader->GetOutput()->GetOrigin()[1] << " "
                << standardReader->GetOutput()->GetOrigin()[2] << std::endl;
      std::cerr << "Standard spacing: " 
                << standardReader->GetOutput()->GetSpacing()[0] << " "
                << standardReader->GetOutput()->GetSpacing()[1] << " "
                << standardReader->GetOutput()->GetSpacing()[2] << std::endl;
      imagesDiffer = true;
    }
  }
  if (!imagesDiffer)
  {
    std::cerr << "Result image origin and spacing match." << std::endl;
  }

  //
  // compare image voxels
  vtkImageMathematics* imageDifference = vtkImageMathematics::New();
  imageDifference->SetOperationToSubtract();
  imageDifference->SetInput1(resultData);
  imageDifference->SetInput2(standardReader->GetOutput());

  vtkImageAccumulate* differenceAccumulator = vtkImageAccumulate::New();
  differenceAccumulator->SetInputConnection(imageDifference->GetOutputPort());
  differenceAccumulator->SetIgnoreZero(1);
  differenceAccumulator->Update();
  
  imagesDiffer = differenceAccumulator->GetVoxelCount() > 0;
  if (imagesDiffer)
  {
    std::cerr << "(ignoring zero) Num / Min / Max / Mean difference = " 
              << differenceAccumulator->GetVoxelCount()  << " / "
              << differenceAccumulator->GetMin()[0]      << " / "
              << differenceAccumulator->GetMax()[0]      << " / "
              << differenceAccumulator->GetMean()[0]     << std::endl;
  }
  else
  {
    std::cerr << "Result image voxels match." << std::endl;
  }

  standardReader->Delete();
  imageDifference->Delete();
  differenceAccumulator->Delete();

  return imagesDiffer;
}

int main(int argc, char** argv)
{
  std::vector<std::string> targetImageFilenames;
  std::string mrmlSceneFilename;
  std::string sceneRootDirectory;
  std::string parametersNodeName;
  std::string correctSegmentationFilename;

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

  //
  // read the scene
  try 
  {
    mrmlScene->Import();
  }
  catch (...)
  {
    std::cerr << "Error parsing mrml scene: " << std::endl;
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
        return EXIT_FAILURE;
      }
      foundParameters = true;
      break;
    }
  }

  if (!foundParameters)
  {
    std::cerr << "Error: parameters not found in scene" << std::endl;
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
    return EXIT_FAILURE;
  }

  //
  // compare results with standard segmentation results
  bool imagesDiffer = ImageDiff(resultImage, correctSegmentationFilename);
  if (imagesDiffer)
  {
    std::cerr << "Result does not match standard!" << std::endl;
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
}

