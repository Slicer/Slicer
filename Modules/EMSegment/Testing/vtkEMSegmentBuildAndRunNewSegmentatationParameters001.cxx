#include "vtkMRMLScene.h"
#include "vtkSlicerColorLogic.h"
#include "vtkSlicerVolumesLogic.h"
#include "vtkEMSegmentLogic.h"
#include "vtkEMSegmentMRMLManager.h"
#include <sstream>

int main(int argc, char** argv)
{
  //
  // read parameters
  std::vector<std::string> targetImageFilenames;
  targetImageFilenames.push_back(argv[1]);
  targetImageFilenames.push_back(argv[2]);
  targetImageFilenames.push_back(argv[3]);
  std::string outputImageFilename(argv[4]);

  //
  // create a mrml scene that will hold the parameters and data
  std::cerr << "Creating MRML scene..." << std::endl;
  vtkMRMLScene* mrmlScene = vtkMRMLScene::New();
  vtkMRMLScene::SetActiveScene(mrmlScene);
  mrmlScene->SetURL("DummyTestingScene.mrml");

  vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New ( );
  colorLogic->SetMRMLScene(mrmlScene);
  colorLogic->AddDefaultColorNodes();
  colorLogic->SetMRMLScene(NULL);
  colorLogic->Delete();

  //
  // create an instance of vtkEMSegmentLogic and connect it with the
  // MRML scene
  std::cerr << "Creating EMSegmenter logic..." << std::endl;
  vtkEMSegmentLogic* emLogic             = vtkEMSegmentLogic::New();
  emLogic->SetAndObserveMRMLScene(mrmlScene);
  emLogic->RegisterMRMLNodesWithScene();
  vtkIntArray *emsEvents                 = vtkIntArray::New();
  emsEvents->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  emsEvents->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  emLogic->SetAndObserveMRMLSceneEvents(mrmlScene, emsEvents);
  emsEvents->Delete();

  //
  // For the EMSegment logic, getting and setting of parameters in the
  // MRML scene is delegated to the EMSegment MRML manager.  Get a
  // shortcut to the manager.
  vtkEMSegmentMRMLManager* mrmlManager = emLogic->GetMRMLManager();
  
  //
  // load test images
  std::cerr << "Loading test images..." << std::endl;
  vtkSlicerVolumesLogic* volumeLogic = vtkSlicerVolumesLogic::New();
  volumeLogic->SetMRMLScene(mrmlScene);
  vtkMRMLScalarVolumeNode* targetVolumeNode0 = NULL;
  for (unsigned int i = 0; i < targetImageFilenames.size(); ++i)
  {
    std::stringstream ss;
    ss << "TargetImage" << i;
    std::string imageName;
    ss >> imageName;

    vtkMRMLScalarVolumeNode* volumeNode = 
      volumeLogic->AddArchetypeScalarVolume(targetImageFilenames[i].c_str(), 
                                            imageName.c_str(), 0);  
    if (volumeNode->GetImageData() == NULL)
    {
      std::cerr << "ERROR: can't load all input images!" << std::endl;
      return EXIT_FAILURE;      
    }
    if (i == 0)
    {
      targetVolumeNode0 = volumeNode;
    }
  }
  volumeLogic->Delete();

  //
  // create a new parameter set and add three subclasses
  std::cerr << "Creating EMSegmenter parameter set..." << std::endl;
  mrmlManager->CreateAndObserveNewParameterSet();
  vtkIdType rootID  = mrmlManager->GetTreeRootNodeID();
  vtkIdType redID   = mrmlManager->AddTreeNode(rootID);
  mrmlManager->SetTreeNodeLabel(redID, "Red");
  vtkIdType greenID = mrmlManager->AddTreeNode(rootID);
  mrmlManager->SetTreeNodeLabel(greenID, "Green");
  vtkIdType blueID  = mrmlManager->AddTreeNode(rootID);
  mrmlManager->SetTreeNodeLabel(blueID, "Blue");

  //
  // no atlas is used for this test
  // nothing to do here...

  //
  // assign 2 input channels
  std::cerr << "Assigning input channels..." << std::endl;
  mrmlManager->AddTargetSelectedVolume(mrmlManager->GetVolumeNthID(0));
  mrmlManager->AddTargetSelectedVolume(mrmlManager->GetVolumeNthID(1));

  //
  // don't align target images
  mrmlManager->SetEnableTargetToTargetRegistration(0);

  //
  // disable all normalization
  mrmlManager->SetNthTargetVolumeIntensityNormalizationEnabled(0, 0);
  mrmlManager->SetNthTargetVolumeIntensityNormalizationEnabled(1, 0);
  
  //
  // manually sample distributions
  std::cerr << "Sampling to build intensity distributions..." << std::endl;
  mrmlManager->SetTreeNodeDistributionSpecificationMethod
    (redID, 
     vtkEMSegmentMRMLManager::DistributionSpecificationManuallySample);
  mrmlManager->SetTreeNodeDistributionSpecificationMethod
    (greenID, 
     vtkEMSegmentMRMLManager::DistributionSpecificationManuallySample);
  mrmlManager->SetTreeNodeDistributionSpecificationMethod
    (blueID, 
     vtkEMSegmentMRMLManager::DistributionSpecificationManuallySample);

  // sample points...
  double ijkPoint[4]; 
  double rasPoint[4];
  vtkMatrix4x4* ijkToras = vtkMatrix4x4::New();
  //ijkToras->Identity();
  targetVolumeNode0->GetIJKToRASMatrix(ijkToras);
  ijkToras->PrintSelf(std::cerr, 0);

  std::cerr << "Adding point 1" << std::endl;
  ijkPoint[0] = 10.0; ijkPoint[1] = 10.0; ijkPoint[2] = 5.0; ijkPoint[3] = 1.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(redID, rasPoint);
  std::cerr << "Adding point 2" << std::endl;
  ijkPoint[0] = 20.0; ijkPoint[1] = 20.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(redID, rasPoint);
  std::cerr << "Adding point 3" << std::endl;
  ijkPoint[0] = 30.0; ijkPoint[1] = 30.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(redID, rasPoint);
  std::cerr << "Adding point 4" << std::endl;
  ijkPoint[0] = 40.0; ijkPoint[1] = 40.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(redID, rasPoint);
  std::cerr << "Adding point 5" << std::endl;
  ijkPoint[0] = 50.0; ijkPoint[1] = 50.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(redID, rasPoint);

  std::cerr << "Adding point 6" << std::endl;
  ijkPoint[0] = 15.0; ijkPoint[1] = 15.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(greenID, rasPoint);
  std::cerr << "Adding point 7" << std::endl;
  ijkPoint[0] = 25.0; ijkPoint[1] = 25.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(greenID, rasPoint);
  std::cerr << "Adding point 8" << std::endl;
  ijkPoint[0] = 35.0; ijkPoint[1] = 35.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(greenID, rasPoint);
  std::cerr << "Adding point 9" << std::endl;
  ijkPoint[0] = 45.0; ijkPoint[1] = 45.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(greenID, rasPoint);
  std::cerr << "Adding point 10" << std::endl;
  ijkPoint[0] = 55.0; ijkPoint[1] = 55.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(greenID, rasPoint);

  std::cerr << "Adding point 11" << std::endl;
  ijkPoint[0] = 18.0; ijkPoint[1] = 18.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(blueID, rasPoint);
  std::cerr << "Adding point 12" << std::endl;
  ijkPoint[0] = 28.0; ijkPoint[1] = 28.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(blueID, rasPoint);
  std::cerr << "Adding point 13" << std::endl;
  ijkPoint[0] = 38.0; ijkPoint[1] = 38.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(blueID, rasPoint);
  std::cerr << "Adding point 14" << std::endl;
  ijkPoint[0] = 48.0; ijkPoint[1] = 48.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(blueID, rasPoint);
  std::cerr << "Adding point 15" << std::endl;
  ijkPoint[0] = 58.0; ijkPoint[1] = 58.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(blueID, rasPoint);

  
  //
  // set tree-based parameters
  std::cerr << "Setting tree-based parameters..." << std::endl;
  mrmlManager->SetTreeNodeClassProbability(rootID, 1.0);
  mrmlManager->SetTreeNodeSpatialPriorWeight(rootID, 0.0);
  mrmlManager->SetTreeNodeClassProbability(redID, 0.25);
  mrmlManager->SetTreeNodeSpatialPriorWeight(redID, 0.0);
  mrmlManager->SetTreeNodeClassProbability(greenID, 0.25);
  mrmlManager->SetTreeNodeSpatialPriorWeight(greenID, 0.0);
  mrmlManager->SetTreeNodeClassProbability(blueID, 0.5);
  mrmlManager->SetTreeNodeSpatialPriorWeight(blueID, 0.0);

  //
  // registration should be off
  mrmlManager->SetRegistrationAffineType
    (vtkEMSegmentMRMLManager::AtlasToTargetAffineRegistrationOff);
  mrmlManager->SetRegistrationDeformableType
    (vtkEMSegmentMRMLManager::AtlasToTargetDeformableRegistrationOff);

  //
  // select an output labelmap
  std::cerr << "Creating output labelmap..." << outputImageFilename 
            << std::endl;
  std::string labelVolumeName = "Segmentation";
  vtkMRMLScalarVolumeNode* labelVolumeNode = 
    volumeLogic->CreateLabelVolume(mrmlScene, targetVolumeNode0, 
                                   (char*) labelVolumeName.c_str());    
  std::cerr << "Setting output image in parameters..." << std::endl;
  mrmlManager->SetOutputVolumeMRMLID(labelVolumeNode->GetID());

  // start segmentation
  std::cerr << "Starting segmentation with 2 channels..." << std::endl;
  emLogic->StartSegmentation();

  //
  // add a new anatomical structure
  std::cerr << "Add a new anatomical structure..." << std::endl;
  vtkIdType whiteID  = mrmlManager->AddTreeNode(rootID);
  mrmlManager->SetTreeNodeLabel(whiteID, "White");  
  mrmlManager->SetTreeNodeDistributionSpecificationMethod
    (whiteID, 
     vtkEMSegmentMRMLManager::DistributionSpecificationManuallySample);
  mrmlManager->SetTreeNodeClassProbability(blueID, 0.25);
  mrmlManager->SetTreeNodeClassProbability(whiteID, 0.25);
  mrmlManager->SetTreeNodeSpatialPriorWeight(whiteID, 0.0);

  std::cerr << "Adding point 16" << std::endl;
  ijkPoint[0] = 12.0; ijkPoint[1] = 12.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(whiteID, rasPoint);
  std::cerr << "Adding point 17" << std::endl;
  ijkPoint[0] = 22.0; ijkPoint[1] = 22.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(whiteID, rasPoint);
  std::cerr << "Adding point 18" << std::endl;
  ijkPoint[0] = 32.0; ijkPoint[1] = 32.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(whiteID, rasPoint);
  std::cerr << "Adding point 19" << std::endl;
  ijkPoint[0] = 42.0; ijkPoint[1] = 42.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(whiteID, rasPoint);
  std::cerr << "Adding point 20" << std::endl;
  ijkPoint[0] = 52.0; ijkPoint[1] = 52.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(whiteID, rasPoint);

  //
  // start segmentation
  std::cerr << "Starting segmentation with extra structure..." << std::endl;
  emLogic->StartSegmentation();

  //
  // remove an anatomical structure
  std::cerr << "Removing extra structure..." << std::endl;
  mrmlManager->RemoveTreeNode(whiteID);
  mrmlManager->SetTreeNodeClassProbability(blueID, 0.50);
  
  //
  // start segmentation
  std::cerr << "Starting segmentation after removing extra structure..." << std::endl;
  emLogic->StartSegmentation();

  //
  // now add a third channel
  std::cerr << "Add a 3rd channel..." << std::endl;
  std::vector<vtkIdType> vectorOfVolumeIDs;
  vectorOfVolumeIDs.push_back(mrmlManager->GetVolumeNthID(0));
  vectorOfVolumeIDs.push_back(mrmlManager->GetVolumeNthID(1));
  vectorOfVolumeIDs.push_back(mrmlManager->GetVolumeNthID(2));
  mrmlManager->ResetTargetSelectedVolumes(vectorOfVolumeIDs);
  //mrmlManager->AddTargetSelectedVolume(mrmlManager->GetVolumeNthID(2));
  mrmlManager->SetNthTargetVolumeIntensityNormalizationEnabled(2, 0);  
  
  //
  // print out covariance matrix for first structure
  bool foundZero = false;
  for (int r = 0; r < mrmlManager->GetTargetNumberOfSelectedVolumes(); ++r)
    {
    for (int c = 0; c < mrmlManager->GetTargetNumberOfSelectedVolumes(); ++c)
      {    
      double lc = mrmlManager->GetTreeNodeDistributionLogCovariance(redID, r, c);
      std::cerr << lc << " ";
      foundZero = foundZero || (lc == 0.0);
      }
    std::cerr << std::endl; 
   }
  if (foundZero)
    {
    std::cerr << "Error: Found a zero I wasn't expecting in the covariance matrix!" << std::endl;
    return EXIT_FAILURE;
    }
    
  // start segmentation
  std::cerr << "Starting segmentation with 3 channels..." << std::endl;
  emLogic->StartSegmentation();

  //
  // now remove two channels
  std::cerr << "Removing channels 1 and 2 of 3..." << std::endl;
  vectorOfVolumeIDs.clear();
  vectorOfVolumeIDs.push_back(mrmlManager->GetVolumeNthID(2));
  mrmlManager->ResetTargetSelectedVolumes(vectorOfVolumeIDs);
  //mrmlManager->RemoveTargetSelectedVolume(mrmlManager->GetVolumeNthID(0));
  //mrmlManager->RemoveTargetSelectedVolume(mrmlManager->GetVolumeNthID(1));
  
  // start segmentation
  std::cerr << "Starting segmentation with 1 channel..." << std::endl;
  emLogic->StartSegmentation();

  //
  // add a new anatomical structure
  std::cerr << "Add a new anatomical structure..." << std::endl;
  vtkIdType blackID  = mrmlManager->AddTreeNode(rootID);
  mrmlManager->SetTreeNodeLabel(blackID, "Black");  
  mrmlManager->SetTreeNodeDistributionSpecificationMethod
    (blackID, 
     vtkEMSegmentMRMLManager::DistributionSpecificationManuallySample);
  mrmlManager->SetTreeNodeClassProbability(blueID, 0.25);
  mrmlManager->SetTreeNodeClassProbability(blackID, 0.25);
  mrmlManager->SetTreeNodeSpatialPriorWeight(blackID, 0.0);

  std::cerr << "Adding point 16" << std::endl;
  ijkPoint[0] = 12.0; ijkPoint[1] = 12.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(blackID, rasPoint);
  std::cerr << "Adding point 17" << std::endl;
  ijkPoint[0] = 22.0; ijkPoint[1] = 22.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(blackID, rasPoint);
  std::cerr << "Adding point 18" << std::endl;
  ijkPoint[0] = 32.0; ijkPoint[1] = 32.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(blackID, rasPoint);
  std::cerr << "Adding point 19" << std::endl;
  ijkPoint[0] = 42.0; ijkPoint[1] = 42.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(blackID, rasPoint);
  std::cerr << "Adding point 20" << std::endl;
  ijkPoint[0] = 52.0; ijkPoint[1] = 52.0; ijkPoint[2] = 5.0;
  ijkToras->MultiplyPoint(ijkPoint, rasPoint);
  mrmlManager->AddTreeNodeDistributionSamplePoint(blackID, rasPoint);

  //
  // start segmentation
  std::cerr << "Starting segmentation with extra structure..." << std::endl;
  emLogic->StartSegmentation();

  //
  // remove an anatomical structure
  std::cerr << "Removing extra structure..." << std::endl;
  mrmlManager->RemoveTreeNode(blackID);
  mrmlManager->SetTreeNodeClassProbability(blueID, 0.50);
  
  //
  // start segmentation
  std::cerr << "Starting segmentation after removing extra structure..." << std::endl;
  emLogic->StartSegmentation();

  //
  // now add a second channel
  std::cerr << "Add a 2nd channel..." << std::endl;
  mrmlManager->AddTargetSelectedVolume(mrmlManager->GetVolumeNthID(0));
  mrmlManager->SetNthTargetVolumeIntensityNormalizationEnabled(1, 0);  

  //
  // start segmentation
  std::cerr << "Starting segmentation after adding another channel..." << std::endl;
  emLogic->StartSegmentation();

  //
  // clean up
  std::cerr << "Cleaning up..." << std::endl;
  ijkToras->Delete();
  mrmlScene->Clear(1);
  mrmlScene->Delete();
  emLogic->SetAndObserveMRMLScene(NULL);
  emLogic->Delete();
  std::cerr << "DONE" << std::endl;
  return EXIT_SUCCESS;
}
