#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkEMSegmentLogic.h"
#include "vtkEMSegment.h"

#include "vtkMRMLScene.h"

#include "vtkMRMLEMSNode.h"
#include "vtkMRMLEMSSegmenterNode.h"
#include "vtkMRMLEMSTemplateNode.h"
#include "vtkMRMLEMSTreeNode.h"
#include "vtkMRMLEMSTreeParametersLeafNode.h"
#include "vtkMRMLEMSTreeParametersParentNode.h"
#include "vtkMRMLEMSTreeParametersNode.h"
#include "vtkMRMLEMSWorkingDataNode.h"
#include "vtkMRMLEMSIntensityNormalizationParametersNode.h"
#include "vtkImageEMLocalSegmenter.h"
#include "vtkImageEMLocalSuperClass.h"
#include "vtkImageMeanIntensityNormalization.h"

#include "vtkMath.h"
#include "vtkImageReslice.h"

// needed to translate between enums
#include "EMLocalInterface.h"

#include <math.h>
#include <exception>

#define ERROR_NODE_VTKID 0

//----------------------------------------------------------------------------
vtkEMSegmentLogic* vtkEMSegmentLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = 
    vtkObjectFactory::CreateInstance("vtkEMSegmentLogic");
  if(ret)
    {
    return (vtkEMSegmentLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkEMSegmentLogic;
}


//----------------------------------------------------------------------------
vtkEMSegmentLogic::vtkEMSegmentLogic()
{
  this->ModuleName = NULL;

  this->ProgressCurrentAction = NULL;
  this->ProgressGlobalFractionCompleted = 0.0;
  this->ProgressCurrentFractionCompleted = 0.0;

  //this->DebugOn();

  this->MRMLManager = NULL; // NB: must be set before SetMRMLManager is called
  vtkEMSegmentMRMLManager* manager = vtkEMSegmentMRMLManager::New();
  this->SetMRMLManager(manager);
  manager->Delete();
}

//----------------------------------------------------------------------------
vtkEMSegmentLogic::~vtkEMSegmentLogic()
{
  this->SetMRMLManager(NULL);
  this->SetProgressCurrentAction(NULL);
  this->SetModuleName(NULL);
}

//----------------------------------------------------------------------------
void vtkEMSegmentLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  // !!! todo
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
SaveTemplateNow()
{
  //
  // for now just save entire mrml scene, later it might be helpful to
  // decide which nodes are important and which are unimportant
  //
  
  // according to comments this should be "file://path/file.xml"
  // but that does not work

  if (this->MRMLManager->GetNode() == NULL)
    {
    vtkErrorMacro("Attempting to save scene but EMSNode is null");
    return;
    }
  vtksys_stl::string url = ""; 
  url += this->MRMLManager->GetSaveTemplateFilename();
  this->GetMRMLScene()->Commit(url.c_str());
}

//----------------------------------------------------------------------------
bool
vtkEMSegmentLogic::
SaveIntermediateResults()
{
  //
  // get output directory
  std::string outputDirectory(this->MRMLManager->GetSaveWorkingDirectory());

  if (!vtksys::SystemTools::FileExists(outputDirectory.c_str()))
    {
    // directory does not exist, bail out (and let the user create it!)
    return false;
    }  

  //
  // package EMSeg-related parameters together and write them to disk
  bool writeSuccessful = 
    this->MRMLManager->PackageAndWriteData(outputDirectory.c_str());

  return writeSuccessful;
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
StartPreprocessing()
{
  if (!this->MRMLManager->GetWorkingDataNode())
  {
    vtkErrorMacro("Can't preprocess because WorkingDataNode is null.");    
    return;
  }

  this->StartPreprocessingInitializeInputData();
  this->StartPreprocessingTargetIntensityNormalization();
  this->StartPreprocessingTargetToTargetRegistration();
  this->StartPreprocessingAtlasToTargetRegistration();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
StartPreprocessingInitializeInputData()
{
  // set the input to the working data
  this->MRMLManager->GetWorkingDataNode()->SetInputTargetNodeID
    (this->MRMLManager->GetSegmenterNode()->GetTargetNodeID());
  this->MRMLManager->GetWorkingDataNode()->SetInputAtlasNodeID
    (this->MRMLManager->GetSegmenterNode()->GetAtlasNodeID());
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
StartPreprocessingTargetIntensityNormalization()
{
  std::cerr << " EMSEG: Starting intensity normalization..." << std::endl;

  // get a pointer to the mrml manager for easy access
  vtkEMSegmentMRMLManager* m = this->MRMLManager;

  // get input target from working node
  vtkMRMLEMSTargetNode* inputTarget = 
    m->GetWorkingDataNode()->GetInputTargetNode();
  if (inputTarget == NULL)
    {
    vtkErrorMacro("Input target node is null, aborting!");
    }
  
  // check that global parameters exist
  if (!this->MRMLManager->GetGlobalParametersNode())
    {
    vtkErrorMacro("Global parameters node is null, aborting!");
    }
  
  // set up the normalized target node
  vtkMRMLEMSTargetNode* normalizedTarget = 
    m->GetWorkingDataNode()->GetNormalizedTargetNode();
  if (!normalizedTarget)
    {
    // clone intput to new normalized target node
    std::cerr << "  Cloning target node...";
    normalizedTarget = m->CloneTargetNode(inputTarget, "NormalizedTarget");
    std::cerr << "Done." << std::endl;
    std::cerr << "  Node is " << (normalizedTarget ? "Non-null" : "Null")
              << std::endl;
    std::cerr << "  Number of images is: " 
              << normalizedTarget->GetNumberOfVolumes() << std::endl;
    m->GetWorkingDataNode()->
      SetNormalizedTargetNodeID(normalizedTarget->GetID());
    }
  
  //
  // check that the number of target images did not change
  // !!! todo !!!

  //
  // apply normalization
  for (int i = 0; i < normalizedTarget->GetNumberOfVolumes(); ++i)
    {
    if (!m->GetNthTargetVolumeIntensityNormalizationEnabled(i))
      {
      // don't apply normaliation to this image
      std::cerr << "  Skipping image " << i 
                << " (no normalization requested)." << std::endl;
      continue;
      }
    std::cerr << "  Normalizing image " << i << "..." << std::endl;
    
    // get image data
    vtkImageData* inData = 
      inputTarget->GetNthVolumeNode(i)->GetImageData();
    vtkImageData* outData = 
      normalizedTarget->GetNthVolumeNode(i)->GetImageData(); 
    if (inData == NULL)
      {
      vtkErrorMacro("Normalization input is null, skipping: " << i);
      continue;
      }
    if (outData == NULL)
      {
      vtkErrorMacro("Normalization output is null, skipping: " << i);
      continue;
      }

    // setup vtk filter
    vtkImageMeanIntensityNormalization* normFilter =
      vtkImageMeanIntensityNormalization::New();
    normFilter->SetNormValue
      (m->GetNthTargetVolumeIntensityNormalizationNormValue(i));
    normFilter->SetNormType
      (m->GetNthTargetVolumeIntensityNormalizationNormType(i));
    normFilter->SetInitialHistogramSmoothingWidth
      (m->
       GetNthTargetVolumeIntensityNormalizationInitialHistogramSmoothingWidth(i));
    normFilter->SetMaxHistogramSmoothingWidth
      (m->GetNthTargetVolumeIntensityNormalizationMaxHistogramSmoothingWidth(i));
    normFilter->SetRelativeMaxVoxelNum
      (m->GetNthTargetVolumeIntensityNormalizationRelativeMaxVoxelNum(i));
    normFilter->SetPrintInfo
      (m->GetNthTargetVolumeIntensityNormalizationPrintInfo(i));
    normFilter->SetInput(inData);
    normFilter->SetOutput(outData);

    // execute filter
    try
      {
      normFilter->Update();
      }
    catch (...)
      {
      vtkErrorMacro("Error executing normalization filter for target image " 
                    << i << ".  Skipping this image.");
      }
    normFilter->Delete();
    }
    
  std::cerr << " EMSEG: Normalization complete." << std::endl;

  // intensity statistics, if computed from data, must be updated
  m->UpdateIntensityDistributions();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
StartPreprocessingTargetToTargetRegistration()
{
  std::cerr << " EMSEG: Starting target-to-target registration..." 
            << std::endl;

  // get a pointer to the mrml manager for easy access
  vtkEMSegmentMRMLManager* m = this->MRMLManager;

  // get input target from working node
  vtkMRMLEMSTargetNode* normalizedTarget = 
    m->GetWorkingDataNode()->GetNormalizedTargetNode();
  if (normalizedTarget == NULL)
    {
    vtkErrorMacro("Normalized target node is null, aborting!");
    }
  
  // check that global parameters exist
  if (!this->MRMLManager->GetGlobalParametersNode())
    {
    vtkErrorMacro("Global parameters node is null, aborting!");
    }
  
  // set up the aligned target node
  vtkMRMLEMSTargetNode* alignedTarget = 
    m->GetWorkingDataNode()->GetAlignedTargetNode();
  if (!alignedTarget)
    {
    // clone intput to new aligned target node
    std::cerr << "  Cloning target node...";
    alignedTarget = m->CloneTargetNode(normalizedTarget, "AlignedTarget");
    std::cerr << "Done." << std::endl;
    std::cerr << "  Node is " << (alignedTarget ? "Non-null" : "Null")
              << std::endl;
    std::cerr << "  Number of images is: " 
              << alignedTarget->GetNumberOfVolumes() << std::endl;
    m->GetWorkingDataNode()->
      SetAlignedTargetNodeID(alignedTarget->GetID());
    }
  
  //
  // check that the number of target images did not change
  // !!! todo !!!

  //
  // apply registration
  // align image i with image 0
  int fixedTargetImageIndex = 0;
  vtkImageData* fixedImageData =
    alignedTarget->GetNthVolumeNode(fixedTargetImageIndex)->GetImageData();
  for (int i = 0; i < alignedTarget->GetNumberOfVolumes(); ++i)
    {
      if (i == fixedTargetImageIndex)
        {
        std::cerr << "  Skipping fixed target image " << i << std::endl;
        continue;
        }

      //
      // get image data
      vtkImageData* movingImageData = 
        normalizedTarget->GetNthVolumeNode(i)->GetImageData();
      vtkImageData* outImageData = 
        alignedTarget->GetNthVolumeNode(i)->GetImageData(); 

      if (fixedImageData == NULL)
        {
        vtkErrorMacro("Fixed image is null, skipping: " << i);
        continue;
        }
      if (movingImageData == NULL)
        {
        vtkErrorMacro("Moving image is null, skipping: " << i);
        continue;
        }
      if (outImageData == NULL)
        {
        vtkErrorMacro("Registration output is null, skipping: " << i);
        continue;
        }

      //
      // set up registration
//       std::cerr << "Starting registration...";
//       vtkRigidRegistrator* registrator = vtkRigidRegistrator::New();
//       registrator->SetFixedImage(fixedImageData);
//       registrator->SetMovingImage(movingImageData);
//       registrator->SetImageToImageMetricToMutualInformation();
//       registrator->SetMetricComputationSamplingRatioo(0.5);
//       registrator->SetNumberOfIterations(50);
//       registrator->SetIntensityInterpolationTypeToLinear();
//       registrator->SetTransformInitializationTypeToCenterOfMass();
//       try
//         {
//         registrator->RegisterImages();
//         }
//       catch (...)
//         {
//         std::cerr << "Failed to register images!!!" << std::endl;
//         }
//       std::cerr << "Target-to-target transform: " << std::endl;
//       registrator->GetTransform()->PrintSelf(std::cerr, 0);
      
      //
      // resample moving image

      //
      // !!! need to make sure that the output image is in space of
      // 0th target image !!!
      std::cerr << "  Resampling target image " << i << "...";
      vtkImageReslice* resliceFilter = vtkImageReslice::New();
      //resliceFilter->SetResliceTransform(registrator->GetTransform());
      resliceFilter->SetInput(movingImageData);
      resliceFilter->SetOutput(outImageData);
      resliceFilter->SetInformationInput(fixedImageData);
      resliceFilter->Update();
      std::cerr << "DONE" << std::endl;

      //
      // clean up
      //registrator->Delete();
      resliceFilter->Delete();
    }    
  std::cerr << " EMSEG: Target-to-target registration complete." << std::endl;

  // intensity statistics, if computed from data, must be updated
  m->UpdateIntensityDistributions();
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
StartPreprocessingAtlasToTargetRegistration()
{
  std::cerr << " EMSEG: Starting atlas-to-target registration..." << std::endl;

  // get a pointer to the mrml manager for easy access
  vtkEMSegmentMRMLManager* m = this->MRMLManager;

  // get input target from working node
  vtkMRMLEMSTargetNode* alignedTarget = 
    m->GetWorkingDataNode()->GetAlignedTargetNode();
  if (alignedTarget == NULL)
    {
    vtkErrorMacro("Aligned target node is null, aborting!");
    }

  // get input atlas from working node
  vtkMRMLEMSAtlasNode* inputAtlas = 
    m->GetWorkingDataNode()->GetInputAtlasNode();
  if (inputAtlas == NULL)
    {
    vtkErrorMacro("Input atlas node is null, aborting!");
    }
  
  // check that global parameters exist
  if (!this->MRMLManager->GetGlobalParametersNode())
    {
    vtkErrorMacro("Global parameters node is null, aborting!");
    }
  
  // set up the aligned atlas node
  vtkMRMLEMSAtlasNode* alignedAtlas = 
    m->GetWorkingDataNode()->GetAlignedAtlasNode();
  if (!alignedAtlas)
    {
    // clone intput to new aligned atlas node
    std::cerr << "  Cloning atlas node...";
    alignedAtlas = m->CloneAtlasNode(inputAtlas, "AlignedAtlas");
    std::cerr << "Done." << std::endl;
    std::cerr << "  Node is " << (alignedAtlas ? "Non-null" : "Null")
              << std::endl;
    std::cerr << "  Number of images is: " 
              << alignedAtlas->GetNumberOfVolumes() << std::endl;
    m->GetWorkingDataNode()->
      SetAlignedAtlasNodeID(alignedAtlas->GetID());
    }
  
  //
  // check that the number of target images did not change
  // !!! todo !!!

  //
  // apply registration
  // align atlas "registration image" with first target image
  int fixedTargetImageIndex = 0;
  vtkImageData* fixedTargetImageData =
    alignedTarget->GetNthVolumeNode(fixedTargetImageIndex)->GetImageData();
  vtkImageData* movingAtlasImageData = NULL; // !!!
    
  // set up registration between these images !!!

  //
  // resample all the atlas images using the transform
  for (int i = 0; i < alignedAtlas->GetNumberOfVolumes(); ++i)
    {
      //
      // get image data
      vtkImageData* movingImageData = 
        inputAtlas->GetNthVolumeNode(i)->GetImageData();
      vtkImageData* outImageData = 
        alignedAtlas->GetNthVolumeNode(i)->GetImageData(); 

      if (movingImageData == NULL)
        {
        vtkErrorMacro("Moving image is null, skipping: " << i);
        continue;
        }
      if (outImageData == NULL)
        {
        vtkErrorMacro("Registration output is null, skipping: " << i);
        continue;
        }

      std::cerr << "  Resampling atlas image " << i << "...";
      vtkImageReslice* resliceFilter = vtkImageReslice::New();
      //resliceFilter->SetResliceTransform(registrator->GetTransform());
      resliceFilter->SetInput(movingImageData);
      resliceFilter->SetOutput(outImageData);
      resliceFilter->SetInformationInput(fixedTargetImageData);
      resliceFilter->Update();
      std::cerr << "DONE" << std::endl;

      //
      // clean up
      //registrator->Delete();
      resliceFilter->Delete();
    }    
  std::cerr << " EMSEG: Atlas-to-target registration complete." << std::endl;
}

//----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
StartSegmentation()
{
  //
  // make sure preprocessing is up to date
  //
  std::cerr << "EMSEG: Start preprocessing..." << std::endl;
  this->StartPreprocessing();
  std::cerr << "EMSEG: Preprocessing complete." << std::endl;

  //
  // make sure we're ready to start
  //

  // find output volume
  if (!this->MRMLManager->GetSegmenterNode())
    {
    vtkErrorMacro("Segmenter node is null---aborting segmentation.");
    return;
    }
  vtkMRMLScalarVolumeNode *outVolume = 
    this->MRMLManager->GetOutputVolumeNode();
  if (outVolume == NULL)
    {
    vtkErrorMacro("No output volume found---aborting segmentation.");
    return;
    }

  //
  // Copy RASToIJK matrix, and other attributes from input to
  // output. Use first target volume as source for this data.
  //
  
  // get attributes from first target input volume
  const char* inMRLMID = 
    this->MRMLManager->GetTargetNode()->GetNthVolumeNodeID(0);
  vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::
    SafeDownCast(this->GetMRMLScene()->GetNodeByID(inMRLMID));
  if (inVolume == NULL)
    {
    vtkErrorMacro("Can't get first target image.");
    return;
    }

  outVolume->CopyOrientation(inVolume);
  outVolume->SetAndObserveTransformNodeID(inVolume->GetTransformNodeID());

  //
  // create segmenter class
  //
  vtkImageEMLocalSegmenter* segmenter = vtkImageEMLocalSegmenter::New();
  if (segmenter == NULL)
    {
    vtkErrorMacro("Could not create vtkImageEMLocalSegmenter pointer");
    return;
    }

  //
  // copy mrml data to segmenter class
  //
  vtkstd::cerr << "EMSEG: Copying data to algorithm class...";
  this->CopyDataToSegmenter(segmenter);
  vtkstd::cerr << "DONE" << vtkstd::endl;

  if (this->GetDebug())
  {
    vtkstd::cerr << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << vtkstd::endl;
    vtkstd::cerr << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << vtkstd::endl;
    vtkstd::cerr << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << vtkstd::endl;
    segmenter->PrintSelf(vtkstd::cerr, 0);
    vtkstd::cerr << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << vtkstd::endl;
    vtkstd::cerr << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << vtkstd::endl;
    vtkstd::cerr << "BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB" << vtkstd::endl;
  }

  //
  // start segmentation
  //
  try 
    {
    vtkstd::cerr << "[Start] Segmentation algorithm..." 
                 << vtkstd::endl;
    segmenter->Update();
    vtkstd::cerr << "[Done]  Segmentation algorithm." << vtkstd::endl;
    }
  catch (std::exception e)
    {
    vtkErrorMacro("Exception thrown during segmentation: " << e.what());
    }

  if (this->GetDebug())
  {
    vtkstd::cerr << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << vtkstd::endl;
    vtkstd::cerr << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << vtkstd::endl;
    vtkstd::cerr << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << vtkstd::endl;
    segmenter->PrintSelf(vtkstd::cerr, 0);
    vtkstd::cerr << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << vtkstd::endl;
    vtkstd::cerr << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << vtkstd::endl;
    vtkstd::cerr << "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" << vtkstd::endl;
  }

  //
  // copy result to output volume
  //
  
  // set ouput of the filter to VolumeNode's ImageData
  // NB: this comment coppied from Gradient Anisotropic Dif. filter:
  // TODO FIX the bug of the image is deallocated unless we do DeepCopy
  vtkImageData* image = vtkImageData::New(); 
  image->DeepCopy(segmenter->GetOutput());
  outVolume->SetAndObserveImageData(image);
  image->Delete();
  // make sure the output volume is a labelmap
  if (!outVolume->GetLabelMap())
  {
    vtkWarningMacro("Changing output image to labelmap");
    outVolume->LabelMapOn();
  }
  outVolume->SetModifiedSinceRead(1);

  //
  // clean up
  //
  segmenter->Delete();

  //
  // save intermediate results
  if (this->MRMLManager->GetSaveIntermediateResults())
    {
    bool savedResults = this->SaveIntermediateResults();
    if (!savedResults)
      {
      vtkErrorMacro("Error writing intermediate results");
      }
    }
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
PopulateTestingData()
{
  vtkDebugMacro("Begin populating test data");

  //
  // add some nodes to the hierarchy
  //
  vtkDebugMacro("Setting parameters for root node");
  double color[3];
  vtkIdType rootNodeID         = this->MRMLManager->GetTreeRootNodeID();
  this->MRMLManager->SetTreeNodeLabel(rootNodeID, "Root");
  this->MRMLManager->SetTreeNodeName(rootNodeID, "Root");
  color[0] = 1.0; color[1] = 0.0; color[2] = 0.0;
  this->MRMLManager->SetTreeNodeColor(rootNodeID, color);
  this->MRMLManager->SetTreeNodeSpatialPriorWeight(rootNodeID, 0.5);
  this->MRMLManager->SetTreeNodeClassProbability(rootNodeID, 0.5);
  this->MRMLManager->SetTreeNodeAlpha(rootNodeID, 0.5);
  this->MRMLManager->SetTreeNodePrintWeight(rootNodeID, 1);
  this->MRMLManager->SetTreeNodeStoppingConditionEMType(rootNodeID, 1);
  this->MRMLManager->SetTreeNodeStoppingConditionEMIterations(rootNodeID, 15);
  this->MRMLManager->SetTreeNodeStoppingConditionEMValue(rootNodeID, 0.5);
  this->MRMLManager->SetTreeNodeStoppingConditionMFAType(rootNodeID, 2);
  this->MRMLManager->SetTreeNodeStoppingConditionMFAIterations(rootNodeID, 16);
  this->MRMLManager->SetTreeNodeStoppingConditionMFAValue(rootNodeID, 0.6);

  vtkDebugMacro("Setting parameters for background node");
  vtkIdType backgroundNodeID   = this->MRMLManager->AddTreeNode(rootNodeID);
  this->MRMLManager->SetTreeNodeLabel(backgroundNodeID, "Background");
  this->MRMLManager->SetTreeNodeName(backgroundNodeID, "Background");
  color[0] = 0.0; color[1] = 0.0; color[2] = 0.0;
  this->MRMLManager->SetTreeNodeColor(backgroundNodeID, color);
  this->MRMLManager->SetTreeNodeSpatialPriorWeight(backgroundNodeID, 0.4);
  this->MRMLManager->SetTreeNodeClassProbability(backgroundNodeID, 0.4);
  this->MRMLManager->SetTreeNodePrintWeight(backgroundNodeID, 1);

  vtkDebugMacro("Setting parameters for icc node");
  vtkIdType iccNodeID          = this->MRMLManager->AddTreeNode(rootNodeID);
  this->MRMLManager->SetTreeNodeLabel(iccNodeID, "ICC");
  this->MRMLManager->SetTreeNodeName(iccNodeID, "ICC");
  color[0] = 0.0; color[1] = 1.0; color[2] = 0.0;
  this->MRMLManager->SetTreeNodeColor(iccNodeID, color);
  this->MRMLManager->SetTreeNodeSpatialPriorWeight(iccNodeID, 0.3);
  this->MRMLManager->SetTreeNodeClassProbability(iccNodeID, 0.3);
  this->MRMLManager->SetTreeNodeAlpha(iccNodeID, 0.3);
  this->MRMLManager->SetTreeNodePrintWeight(iccNodeID, 1);
  this->MRMLManager->SetTreeNodeStoppingConditionEMType(iccNodeID, 0);
  this->MRMLManager->SetTreeNodeStoppingConditionEMIterations(iccNodeID, 13);
  this->MRMLManager->SetTreeNodeStoppingConditionEMValue(iccNodeID, 0.3);
  this->MRMLManager->SetTreeNodeStoppingConditionMFAType(iccNodeID, 1);
  this->MRMLManager->SetTreeNodeStoppingConditionMFAIterations(iccNodeID, 14);
  this->MRMLManager->SetTreeNodeStoppingConditionMFAValue(iccNodeID, 0.4);

  vtkDebugMacro("Setting parameters for grey matter node");
  vtkIdType greyMatterNodeID   = this->MRMLManager->AddTreeNode(iccNodeID);
  this->MRMLManager->SetTreeNodeLabel(greyMatterNodeID, "Grey Matter");
  this->MRMLManager->SetTreeNodeName(greyMatterNodeID, "Grey Matter");
  color[0] = 0.0; color[1] = 1.0; color[2] = 1.0;
  this->MRMLManager->SetTreeNodeColor(greyMatterNodeID, color);
  this->MRMLManager->SetTreeNodeSpatialPriorWeight(greyMatterNodeID, 0.2);
  this->MRMLManager->SetTreeNodeClassProbability(greyMatterNodeID, 0.2);
  this->MRMLManager->SetTreeNodePrintWeight(greyMatterNodeID, 1);

  vtkDebugMacro("Setting parameters for white matter node");
  vtkIdType whiteMatterNodeID  = this->MRMLManager->AddTreeNode(iccNodeID);
  this->MRMLManager->SetTreeNodeLabel(whiteMatterNodeID, "White Matter");
  this->MRMLManager->SetTreeNodeName(whiteMatterNodeID, "White Matter");
  color[0] = 1.0; color[1] = 1.0; color[2] = 0.0;
  this->MRMLManager->SetTreeNodeColor(whiteMatterNodeID, color);
  this->MRMLManager->SetTreeNodeSpatialPriorWeight(whiteMatterNodeID, 0.1);
  this->MRMLManager->SetTreeNodeClassProbability(whiteMatterNodeID, 0.1);
  this->MRMLManager->SetTreeNodePrintWeight(whiteMatterNodeID, 1);

  vtkDebugMacro("Setting parameters for csf node");
  vtkIdType csfNodeID  = this->MRMLManager->AddTreeNode(iccNodeID);
  this->MRMLManager->SetTreeNodeLabel(csfNodeID, "CSF");
  this->MRMLManager->SetTreeNodeName(csfNodeID, "CSF");

  //
  // set registration parameters
  //
  vtkDebugMacro("Setting registration parameters");
  this->MRMLManager->SetRegistrationAffineType(0);
  this->MRMLManager->SetRegistrationDeformableType(0);
  this->MRMLManager->SetRegistrationInterpolationType(1);

  //
  // set save parameters
  //
  vtkDebugMacro("Setting save parameters");
  this->MRMLManager->SetSaveWorkingDirectory("/tmp");
  this->MRMLManager->SetSaveTemplateFilename("/tmp/EMSTemplate.mrml");
  this->MRMLManager->SetSaveTemplateAfterSegmentation(1);
  this->MRMLManager->SetSaveIntermediateResults(1);
  this->MRMLManager->SetSaveSurfaceModels(1);
  
  this->MRMLManager->SetEnableMultithreading(1);
  this->SetProgressGlobalFractionCompleted(0.9);

  vtkDebugMacro("Done populating test data");
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
SpecialTestingFunction()
{
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
CopyDataToSegmenter(vtkImageEMLocalSegmenter* segmenter)
{
  //
  // copy atlas related parameters to algorithm
  //
  vtkstd::cerr << "atlas data...";
  this->CopyAtlasDataToSegmenter(segmenter);

  //
  // copy target related parameters to algorithm
  //
  vtkstd::cerr << "target data...";
  this->CopyTargetDataToSegmenter(segmenter);

  //
  // copy global parameters to algorithm 
  //
  vtkstd::cerr << "global data...";
  this->CopyGlobalDataToSegmenter(segmenter);

  //
  // copy tree base parameters to algorithm
  //
  vtkstd::cerr << "tree data...";
  vtkImageEMLocalSuperClass* rootNode = vtkImageEMLocalSuperClass::New();
  this->CopyTreeDataToSegmenter(rootNode, 
                                this->MRMLManager->GetTreeRootNodeID());
  segmenter->SetHeadClass(rootNode);
  rootNode->Delete();
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
CopyAtlasDataToSegmenter(vtkImageEMLocalSegmenter* segmenter)
{
  segmenter->
    SetNumberOfTrainingSamples(this->MRMLManager->
                               GetAtlasNumberOfTrainingSamples());
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
CopyTargetDataToSegmenter(vtkImageEMLocalSegmenter* segmenter)
{
  // !!! todo: TESTING HERE!!!
  vtkMRMLEMSTargetNode* workingTarget = 
    this->MRMLManager->GetWorkingDataNode()->GetWorkingTargetNode();
  unsigned int numTargetImages = workingTarget->GetNumberOfVolumes();
  segmenter->SetNumInputImages(numTargetImages);

  for (unsigned int i = 0; i < numTargetImages; ++i)
    {
    std::string mrmlID = workingTarget->GetNthVolumeNodeID(i);
    vtkDebugMacro("Setting target image " << i << " mrmlID=" 
                  << mrmlID.c_str());

    vtkImageData* imageData = 
      workingTarget->GetNthVolumeNode(i)->GetImageData();

    segmenter->SetImageInput(i, imageData);
    }
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
CopyGlobalDataToSegmenter(vtkImageEMLocalSegmenter* segmenter)
{
  segmenter->
    SetDisableMultiThreading(!this->MRMLManager->GetEnableMultithreading());
  segmenter->SetPrintDir(this->MRMLManager->GetSaveWorkingDirectory());
  
  //
  // NB: In the algorithm code alpha is defined globally.  In this
  // logic, it is defined for each parent node.  For now copy alpha
  // from the root tree node. !!!todo!!!
  //
  vtkIdType rootNodeID = this->MRMLManager->GetTreeRootNodeID();
  segmenter->SetAlpha(this->MRMLManager->GetTreeNodeAlpha(rootNodeID));
                      
  //
  // NB: In the algorithm code smoothing widht and sigma parameters
  // are defined globally.  In this logic, they are defined for each
  // parent node.  For now copy parameters from the root tree
  // node. !!!todo!!!
  //
  segmenter->
    SetSmoothingWidth(this->MRMLManager->
                      GetTreeNodeSmoothingKernelWidth(rootNodeID));

  // type mismatch between logic and algorithm !!!todo!!!
  int intSigma = 
    vtkMath::Round(this->MRMLManager->
                   GetTreeNodeSmoothingKernelSigma(rootNodeID));
  segmenter->SetSmoothingSigma(intSigma);

  //
  // registration parameters
  //
  int algType = this->ConvertGUIEnumToAlgorithmEnumInterpolationType
    (this->MRMLManager->GetRegistrationInterpolationType());
  segmenter->SetRegistrationInterpolationType(algType);
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
CopyTreeDataToSegmenter(vtkImageEMLocalSuperClass* node, vtkIdType nodeID)
{
  // need this here because the vtkImageEM* classes don't use
  // virtual functions and so failed initializations lead to
  // memory errors
  node->SetNumInputImages(this->MRMLManager->
                          GetTargetNumberOfSelectedVolumes());

  // copy generic tree node data to segmenter
  this->CopyTreeGenericDataToSegmenter(node, nodeID);
  
  // copy parent specific tree node data to segmenter
  this->CopyTreeParentDataToSegmenter(node, nodeID);

  // add children
  unsigned int numChildren = 
    this->MRMLManager->GetTreeNodeNumberOfChildren(nodeID);
  double totalProbability = 0.0;
  for (unsigned int i = 0; i < numChildren; ++i)
    {
    vtkIdType childID = this->MRMLManager->GetTreeNodeChildNodeID(nodeID, i);
    bool isLeaf = this->MRMLManager->GetTreeNodeIsLeaf(childID);

    if (isLeaf)
      {
      vtkImageEMLocalClass* childNode = vtkImageEMLocalClass::New();
      // need this here because the vtkImageEM* classes don't use
      // virtual functions and so failed initializations lead to
      // memory errors
      childNode->SetNumInputImages(this->MRMLManager->
                                   GetTargetNumberOfSelectedVolumes());
      this->CopyTreeGenericDataToSegmenter(childNode, childID);
      this->CopyTreeLeafDataToSegmenter(childNode, childID);
      node->AddSubClass(childNode, i);
      childNode->Delete();
      }
    else
      {
      vtkImageEMLocalSuperClass* childNode = vtkImageEMLocalSuperClass::New();
      this->CopyTreeDataToSegmenter(childNode, childID);
      node->AddSubClass(childNode, i);
      childNode->Delete();
      }

    totalProbability += 
      this->MRMLManager->GetTreeNodeClassProbability(childID);
    }

  if (totalProbability != 1.0)
    {
    vtkWarningMacro("Warning: child probabilities don't sum to unity for node "
                    << this->MRMLManager->GetTreeNodeName(nodeID)
                    << " they sum to " << totalProbability);
    }

  // update Markov matrices
  const unsigned int numDirections = 6;
  bool nodeHasMatrix = 
    this->MRMLManager->GetTreeClassInteractionNode(nodeID) != NULL;
  if (!nodeHasMatrix)
    {
    vtkWarningMacro("CIM not available, using identity.");
    }
  for (unsigned int d = 0; d < numDirections; ++d)
    {
    for (unsigned int r = 0; r < numChildren; ++r)
      {
      for (unsigned int c = 0; c < numChildren; ++c)
        {
        double val = nodeHasMatrix 
          ? this->MRMLManager->GetTreeNodeClassInteraction(nodeID, d, r, c)
          : (r == c ? 1.0 : 0.0);
        node->SetMarkovMatrix(val, d, c, r);
        }
      }
    }
  node->Update();
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
CopyTreeGenericDataToSegmenter(vtkImageEMLocalGenericClass* node, 
                               vtkIdType nodeID)
{
  unsigned int numTargetImages = 
    this->MRMLManager->GetTargetNumberOfSelectedVolumes();
  
  int boundMin[3];
  this->MRMLManager->GetSegmentationBoundaryMin(boundMin);
  node->SetSegmentationBoundaryMin(boundMin[0], boundMin[1], boundMin[2]);

  int boundMax[3];
  this->MRMLManager->GetSegmentationBoundaryMax(boundMax);
  node->SetSegmentationBoundaryMax(boundMax[0], boundMax[1], boundMax[2]);
  
  node->SetProbDataWeight(this->MRMLManager->
                          GetTreeNodeSpatialPriorWeight(nodeID));

  node->SetTissueProbability(this->MRMLManager->
                             GetTreeNodeClassProbability(nodeID));

  node->SetPrintWeights(this->MRMLManager->GetTreeNodePrintWeight(nodeID));

  // set target input channel weights
  for (unsigned int i = 0; i < numTargetImages; ++i)
    {
    node->SetInputChannelWeights(this->MRMLManager->
                                 GetTreeNodeInputChannelWeight(nodeID, 
                                                               i), i);
    }

  //
  // registration related data
  //
  //!!!bcd!!!

  //
  // set probability data
  //
  vtkIdType probVolumeID = 
    this->MRMLManager->GetTreeNodeSpatialPriorVolumeID(nodeID);
  if (probVolumeID != ERROR_NODE_VTKID)
    {
    vtkDebugMacro("Setting spatial prior: node=" 
                  << this->MRMLManager->GetTreeNodeLabel(nodeID));
    vtkImageData* imageData = this->MRMLManager->GetVolumeNode(probVolumeID)->
      GetImageData();
    node->SetProbDataPtr(imageData);
    }

  int exclude = 
    this->MRMLManager->GetTreeNodeExcludeFromIncompleteEStep(nodeID);
  node->SetExcludeFromIncompleteEStepFlag(exclude);
}


//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
CopyTreeParentDataToSegmenter(vtkImageEMLocalSuperClass* node, 
                              vtkIdType nodeID)
{
  node->SetPrintFrequency (this->MRMLManager->
                           GetTreeNodePrintFrequency(nodeID));
  node->SetPrintBias      (this->MRMLManager->
                           GetTreeNodePrintBias(nodeID));
  node->SetPrintLabelMap  (this->MRMLManager->
                           GetTreeNodePrintLabelMap(nodeID));

  node->SetPrintEMLabelMapConvergence
    (this->MRMLManager->GetTreeNodePrintEMLabelMapConvergence(nodeID));
  node->SetPrintEMWeightsConvergence
    (this->MRMLManager->GetTreeNodePrintEMWeightsConvergence(nodeID));
  node->SetStopEMType(this->ConvertGUIEnumToAlgorithmEnumStoppingConditionType
                      (this->MRMLManager->
                      GetTreeNodeStoppingConditionEMType(nodeID)));
  node->SetStopEMValue(this->MRMLManager->
                       GetTreeNodeStoppingConditionEMValue(nodeID));
  node->SetStopEMMaxIter
    (this->MRMLManager->GetTreeNodeStoppingConditionEMIterations(nodeID));

  node->SetPrintMFALabelMapConvergence
    (this->MRMLManager->GetTreeNodePrintMFALabelMapConvergence(nodeID));
  node->SetPrintMFAWeightsConvergence
    (this->MRMLManager->GetTreeNodePrintMFAWeightsConvergence(nodeID));
  node->SetStopMFAType(this->ConvertGUIEnumToAlgorithmEnumStoppingConditionType
                       (this->MRMLManager->
                       GetTreeNodeStoppingConditionMFAType(nodeID)));
  node->SetStopMFAValue(this->MRMLManager->
                        GetTreeNodeStoppingConditionMFAValue(nodeID));
  node->SetStopMFAMaxIter
    (this->MRMLManager->GetTreeNodeStoppingConditionMFAIterations(nodeID));

  node->SetStopBiasCalculation
    (this->MRMLManager->GetTreeNodeBiasCalculationMaxIterations(nodeID));

  node->SetPrintShapeSimularityMeasure(0);         // !!!bcd!!!

  node->SetPCAShapeModelType(0);                   // !!!bcd!!!

  node->SetRegistrationIndependentSubClassFlag(0); // !!!bcd!!!
  node->SetRegistrationType(0);                    // !!!bcd!!!

  node->SetGenerateBackgroundProbability
    (this->MRMLManager->GetTreeNodeGenerateBackgroundProbability(nodeID));
}

//-----------------------------------------------------------------------------
void
vtkEMSegmentLogic::
CopyTreeLeafDataToSegmenter(vtkImageEMLocalClass* node, 
                            vtkIdType nodeID)
{
  unsigned int numTargetImages = 
    this->MRMLManager->GetTargetNumberOfSelectedVolumes();

  // this label describes the output intensity value for this class in
  // the segmentation result
  node->SetLabel(this->MRMLManager->GetTreeNodeIntensityLabel(nodeID));

  // set log mean and log covariance
  for (unsigned int r = 0; r < numTargetImages; ++r)
    {
    node->SetLogMu(this->MRMLManager->
                   GetTreeNodeDistributionLogMean(nodeID, r), r);

    for (unsigned int c = 0; c < numTargetImages; ++c)
      {
      node->SetLogCovariance(this->MRMLManager->
                             GetTreeNodeDistributionLogCovariance(nodeID,
                                                                  r, c), 
                             r, c);
      }
    }

  node->SetPrintQuality(this->MRMLManager->GetTreeNodePrintQuality(nodeID));
}

//-----------------------------------------------------------------------------
int
vtkEMSegmentLogic::
ConvertGUIEnumToAlgorithmEnumStoppingConditionType(int guiEnumValue)
{
  switch (guiEnumValue)
    {
    case (vtkEMSegmentMRMLManager::StoppingConditionIterations):
      return EMSEGMENT_STOP_FIXED;
    case (vtkEMSegmentMRMLManager::StoppingConditionLabelMapMeasure):
      return EMSEGMENT_STOP_LABELMAP;
    case (vtkEMSegmentMRMLManager::StoppingConditionWeightsMeasure):
      return EMSEGMENT_STOP_WEIGHTS;
    default:
      vtkErrorMacro("Unknown stopping condition type: " << guiEnumValue);
      return -1;
    }
}

//-----------------------------------------------------------------------------
int
vtkEMSegmentLogic::
ConvertGUIEnumToAlgorithmEnumInterpolationType(int guiEnumValue)
{
  switch (guiEnumValue)
    {
    case (vtkEMSegmentMRMLManager::InterpolationLinear):
      return EMSEGMENT_REGISTRATION_INTERPOLATION_LINEAR;
    case (vtkEMSegmentMRMLManager::InterpolationNearestNeighbor):
      return EMSEGMENT_REGISTRATION_INTERPOLATION_NEIGHBOUR;
    case (vtkEMSegmentMRMLManager::InterpolationCubic):
      // !!! not implemented
      vtkErrorMacro("Cubic interpolation not implemented: " << guiEnumValue);
      return -1;
    default:
      vtkErrorMacro("Unknown interpolation type: " << guiEnumValue);
      return -1;
    }
}
