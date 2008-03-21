/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerVolumesLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include <vtksys/SystemTools.hxx> 

#include "vtkImageThreshold.h"
#include "vtkImageAccumulateDiscrete.h"
#include "vtkImageBimodalAnalysis.h"
#include "vtkImageExtractComponents.h"
#include "vtkDiffusionTensorMathematics.h"
#include "vtkAssignAttribute.h"

#include "vtkSlicerVolumesLogic.h"
#include "vtkSlicerColorLogic.h"

#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLVolumeHeaderlessStorageNode.h"


#include "vtkMRMLNRRDStorageNode.h"

#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLLabelMapVolumeDisplayNode.h"
#include "vtkMRMLVectorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"
#include "vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h"

vtkCxxRevisionMacro(vtkSlicerVolumesLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerVolumesLogic);

//----------------------------------------------------------------------------
vtkSlicerVolumesLogic::vtkSlicerVolumesLogic()
{
  this->ActiveVolumeNode = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerVolumesLogic::~vtkSlicerVolumesLogic()
{
  if (this->ActiveVolumeNode != NULL)
    {
    this->ActiveVolumeNode->Delete();
    this->ActiveVolumeNode = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::ProcessMRMLEvents(vtkObject *caller, 
                                            unsigned long event, 
                                            void *callData)
{
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::ProcessLogicEvents(vtkObject *caller, 
                                            unsigned long event, 
                                            void *callData)
{
  if (event ==  vtkCommand::ProgressEvent) 
    {
    this->InvokeEvent ( vtkCommand::ProgressEvent,callData );
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::SetActiveVolumeNode(vtkMRMLVolumeNode *activeNode)
{
  vtkSetMRMLNodeMacro(this->ActiveVolumeNode, activeNode );
  this->Modified();
}


//----------------------------------------------------------------------------
// int loadingOptions is bit-coded as following:
// bit 0: label map
// bit 1: centered
// bit 2: loading signal file
// higher bits are reserved for future use
vtkMRMLVolumeNode* vtkSlicerVolumesLogic::AddHeaderVolume (const char* filename, const char* volname, 
                                                           vtkMRMLVolumeHeaderlessStorageNode *headerStorage,
                                                           int loadingOptions)
{

  int centerImage = 0;
  int labelMap = 0;
  int singleFile = 0;
  if ( loadingOptions & 1 )    // labelMap is true
  {
    labelMap = 1;
  }
  if ( loadingOptions & 2 )    // centerImage is true
  {
    centerImage = 1;
  }
  if ( loadingOptions & 4 )    // singleFile is true
  {
    singleFile = 1;
  }

  vtkMRMLVolumeNode *volumeNode = NULL;
  vtkMRMLVolumeDisplayNode *displayNode = NULL;

  vtkMRMLScalarVolumeDisplayNode *sdisplayNode = NULL;
  vtkMRMLVectorVolumeDisplayNode *vdisplayNode = NULL;

  vtkMRMLScalarVolumeNode *scalarNode = vtkMRMLScalarVolumeNode::New();
  vtkMRMLVectorVolumeNode *vectorNode = vtkMRMLVectorVolumeNode::New();

  vtkMRMLVolumeHeaderlessStorageNode *storageNode = vtkMRMLVolumeHeaderlessStorageNode::New();
  storageNode->CopyWithScene(headerStorage);
  
  storageNode->SetFileName(filename);
  storageNode->SetCenterImage(centerImage);
  storageNode->AddObserver(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);

 if (storageNode->ReadData(scalarNode))
    {
    if (labelMap) 
      {
      displayNode = vtkMRMLLabelMapVolumeDisplayNode::New();
      }
    else
      {
      sdisplayNode = vtkMRMLScalarVolumeDisplayNode::New();
      displayNode = sdisplayNode;
     }
    scalarNode->SetLabelMap(labelMap);
    volumeNode = scalarNode;
    }
  else if (storageNode->ReadData(vectorNode))
    {
    vdisplayNode = vtkMRMLVectorVolumeDisplayNode::New();
    displayNode = vdisplayNode;
    volumeNode = vectorNode;
    }

  storageNode->RemoveObservers(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);

  if (volumeNode != NULL)
    {
    if (volname == NULL)
      {
      const vtksys_stl::string fname(filename);
      vtksys_stl::string name = vtksys::SystemTools::GetFilenameName(fname);
      volumeNode->SetName(name.c_str());
      }
    else
      {
      volumeNode->SetName(volname);
      }

    this->GetMRMLScene()->SaveStateForUndo();
    vtkDebugMacro("Setting scene info");
    volumeNode->SetScene(this->GetMRMLScene());
    storageNode->SetScene(this->GetMRMLScene());
    displayNode->SetScene(this->GetMRMLScene());

    vtkMRMLScalarVolumeDisplayNode *sdn = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(displayNode);
    if (sdn)
      {
      double range[2];
      vtkDebugMacro("Set basic display info");
      volumeNode->GetImageData()->GetScalarRange(range);
      sdn->SetLowerThreshold(range[0]);
      sdn->SetUpperThreshold(range[1]);
      sdn->SetWindow(range[1] - range[0]);
      sdn->SetLevel(0.5 * (range[1] + range[0]) );
      }

    vtkDebugMacro("Adding node..");
    this->GetMRMLScene()->AddNode(storageNode);  
    this->GetMRMLScene()->AddNode(displayNode);  

    //displayNode->SetDefaultColorMap();
    vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New();
    if (labelMap) 
      {
      if (this->IsFreeSurferVolume(filename))
        {
        displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultFreeSurferLabelMapColorNodeID());
        }
      else
        {
        displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultLabelMapColorNodeID());
        }
      }
    else
      {
      displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultVolumeColorNodeID());
      }
    colorLogic->Delete();
    
    volumeNode->SetAndObserveStorageNodeID(storageNode->GetID());
    volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());

    vtkDebugMacro("Name vol node "<<volumeNode->GetClassName());
    vtkDebugMacro("Display node "<<displayNode->GetClassName());
    this->GetMRMLScene()->AddNode(volumeNode);
    vtkDebugMacro("Node added to scene");

    this->SetActiveVolumeNode(volumeNode);

    this->Modified();
    }

  scalarNode->Delete();
  vectorNode->Delete();
  storageNode->Delete();
  if (displayNode)
    {
    displayNode->Delete();
    }
  return volumeNode;
}

//----------------------------------------------------------------------------
// int loadingOptions is bit-coded as following:
// bit 0: label map
// bit 1: centered
// bit 2: loading signal file
// higher bits are reserved for future use
vtkMRMLVolumeNode* vtkSlicerVolumesLogic::AddArchetypeVolume (const char* filename, const char* volname, int loadingOptions)
{
  int centerImage = 0;
  int labelMap = 0;
  int singleFile = 0;
  if ( loadingOptions & 1 )    // labelMap is true
  {
    labelMap = 1;
  }
  if ( loadingOptions & 2 )    // centerImage is true
  {
    centerImage = 1;
  }
  if ( loadingOptions & 4 )    // singleFile is true
  {
    singleFile = 1;
  }

  vtkMRMLVolumeNode *volumeNode = NULL;
  vtkMRMLVolumeDisplayNode *displayNode = NULL;
  vtkMRMLDiffusionTensorDisplayPropertiesNode *displayPropertiesNode = NULL;

  vtkMRMLDiffusionTensorVolumeDisplayNode *dtdisplayNode = NULL;
  vtkMRMLDiffusionWeightedVolumeDisplayNode *dwdisplayNode = NULL;
  vtkMRMLVectorVolumeDisplayNode *vdisplayNode = NULL;
  vtkMRMLScalarVolumeDisplayNode *sdisplayNode = NULL;

  vtkMRMLScalarVolumeNode *scalarNode = vtkMRMLScalarVolumeNode::New();
  vtkMRMLVectorVolumeNode *vectorNode = vtkMRMLVectorVolumeNode::New();
  vtkMRMLDiffusionTensorVolumeNode *tensorNode = vtkMRMLDiffusionTensorVolumeNode::New();
  vtkMRMLDiffusionWeightedVolumeNode *dwiNode = vtkMRMLDiffusionWeightedVolumeNode::New();

  // Instanciation of the two I/O mechanism
  vtkMRMLNRRDStorageNode *storageNode1 = vtkMRMLNRRDStorageNode::New();
  vtkMRMLVolumeArchetypeStorageNode *storageNode2 = vtkMRMLVolumeArchetypeStorageNode::New();
  vtkMRMLStorageNode *storageNode = NULL;

  bool useURI = false;
  if (this->GetMRMLScene() &&
      this->GetMRMLScene()->GetCacheManager())
    {
    useURI = this->GetMRMLScene()->GetCacheManager()->IsRemoteReference(filename);
    }
  if (useURI)
    {
    vtkWarningMacro("AddArchetypeVolume: input filename '" << filename << "' is a URI");
    // need to set the scene on the storage node so that it can look for file handlers
    storageNode1->SetURI(filename);
    storageNode1->SetScene(this->GetMRMLScene());
    storageNode2->SetURI(filename);
    storageNode2->SetScene(this->GetMRMLScene());
    }
  else
    {
    storageNode1->SetFileName(filename);
    storageNode2->SetFileName(filename);
    }
  storageNode1->SetCenterImage(centerImage);
  storageNode1->AddObserver(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);

  storageNode2->SetCenterImage(centerImage);
  storageNode2->SetSingleFile(singleFile);
  storageNode2->AddObserver(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);

  // Try to read first with NRRD reader (look if file is a dwi or a tensor)
  vtkDebugMacro("TEST DWI: "<< storageNode1->ReadData(dwiNode));
  
  if (storageNode1->ReadData(dwiNode)) // storageNode1->SupportedFileType(filename)
    {
    vtkDebugMacro("DWI HAS BEEN READ");
    dwdisplayNode = vtkMRMLDiffusionWeightedVolumeDisplayNode::New();
    displayNode = dwdisplayNode;
    // Give a chance to set/update displayNode
    volumeNode =  dwiNode;
    storageNode = storageNode1;
    vtkDebugMacro("Done setting volumeNode to class: "<<volumeNode->GetClassName());
    }
  else if (storageNode1->ReadData(tensorNode))
    {
    vtkDebugMacro("Tensor HAS BEEN READ");
    dtdisplayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::New();
    displayNode = dtdisplayNode;
    displayPropertiesNode = vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
    volumeNode = tensorNode;
    storageNode = storageNode1;
    }
  else if (storageNode1->ReadData(vectorNode))
    {
    vtkDebugMacro("Vector HAS BEEN READ WITH NRRD READER");
    vdisplayNode = vtkMRMLVectorVolumeDisplayNode::New();
    displayNode = vdisplayNode;
    volumeNode = vectorNode;
    storageNode = storageNode1;
    }
  else if (storageNode2->ReadData(vectorNode)) // storageNode2->SupportedFileType(filename))  
    {
    vtkDebugMacro("Vector HAS BEEN READ WITH ARCHTYPE READER");
    vdisplayNode = vtkMRMLVectorVolumeDisplayNode::New();
    displayNode = vdisplayNode;
    volumeNode = vectorNode;
    storageNode = storageNode2;
    }
  else if (storageNode2->ReadData(scalarNode)) //storageNode2->SupportedFileType(filename)) // 
    {
    vtkDebugMacro("Scalar HAS BEEN READ WITH ARCHTYPE READER");
    if (labelMap) 
      {
      displayNode = vtkMRMLLabelMapVolumeDisplayNode::New();
      }
    else
      {
      sdisplayNode = vtkMRMLScalarVolumeDisplayNode::New();
      displayNode = sdisplayNode;
      }
    scalarNode->SetLabelMap(labelMap);
    volumeNode = scalarNode;
    storageNode = storageNode2;
    }

  storageNode1->RemoveObservers(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);
  storageNode2->RemoveObservers(vtkCommand::ProgressEvent,  this->LogicCallbackCommand);

  if (volumeNode != NULL)
    {
    if (volname == NULL)
      {
      const vtksys_stl::string fname(filename);
      vtksys_stl::string name = vtksys::SystemTools::GetFilenameName(fname);
      volumeNode->SetName(name.c_str());
      }
    else
      {
      volumeNode->SetName(volname);
      }

    this->GetMRMLScene()->SaveStateForUndo();
    vtkDebugMacro("Setting scene info");
    volumeNode->SetScene(this->GetMRMLScene());
    storageNode->SetScene(this->GetMRMLScene());
    displayNode->SetScene(this->GetMRMLScene());
    if (displayPropertiesNode)
      {
      displayPropertiesNode->SetScene(this->GetMRMLScene());
      }
  
    //should we give the user the chance to modify this?.

    vtkDebugMacro("Adding node..");
    this->GetMRMLScene()->AddNode(storageNode);
    this->GetMRMLScene()->AddNode(displayNode);
    if (displayPropertiesNode)
      {
      this->GetMRMLScene()->AddNode(displayPropertiesNode);
      }

    //displayNode->SetDefaultColorMap();
    vtkSlicerColorLogic *colorLogic = vtkSlicerColorLogic::New();
    if (labelMap) 
      {
      if (this->IsFreeSurferVolume(filename))
        {
        displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultFreeSurferLabelMapColorNodeID());
        }
      else
        {
        displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultLabelMapColorNodeID());
        }
      }
    else
      {
      displayNode->SetAndObserveColorNodeID(colorLogic->GetDefaultVolumeColorNodeID());
      }
    colorLogic->Delete();
    
    volumeNode->SetAndObserveStorageNodeID(storageNode->GetID());
    volumeNode->SetAndObserveDisplayNodeID(displayNode->GetID());
    if (displayPropertiesNode)
      {
      vtkMRMLDiffusionTensorVolumeDisplayNode *dtiDisplayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(displayNode);
      if (dtiDisplayNode)
        {
        dtiDisplayNode->SetAutoWindowLevel(0);
        dtiDisplayNode->SetWindow(0);
        dtiDisplayNode->SetLevel(0);
        dtiDisplayNode->SetUpperThreshold(0);
        dtiDisplayNode->SetLowerThreshold(0);
        dtiDisplayNode->SetAndObserveDiffusionTensorDisplayPropertiesNodeID(displayPropertiesNode->GetID());
        tensorNode->AddSliceGlyphDisplayNodes();
        }
      }
      
    vtkMRMLScalarVolumeDisplayNode *sdn = vtkMRMLScalarVolumeDisplayNode::SafeDownCast(displayNode);
    if (sdn && volumeNode)
      {
      this->CalculateAutoLevels( volumeNode->GetImageData(), sdn );
      }

    vtkDebugMacro("Name vol node "<<volumeNode->GetClassName());
    vtkDebugMacro("Display node "<<displayNode->GetClassName());
    this->GetMRMLScene()->AddNode(volumeNode);
    vtkDebugMacro("Node added to scene");
    
    // now read it
    //    storageNode->ReadData();
    
    this->SetActiveVolumeNode(volumeNode);

    this->Modified();
    }

  scalarNode->Delete();
  // TODO: figure out why using the itk VectorImage causes a crash when deleting
  vectorNode->Delete();
  dwiNode->Delete();
  tensorNode->Delete();
  storageNode1->Delete();
  storageNode2->Delete();
  if (displayNode)
    {
    displayNode->Delete();
    }
  if (displayPropertiesNode)
    {
    displayPropertiesNode->Delete();
    }
  return volumeNode;
}

//----------------------------------------------------------------------------
int vtkSlicerVolumesLogic::SaveArchetypeVolume (const char* filename, vtkMRMLVolumeNode *volumeNode)
{
  if (volumeNode == NULL || filename == NULL)
    {
    return 0;
    }
  
  vtkMRMLNRRDStorageNode *storageNode1 = NULL;
  vtkMRMLVolumeArchetypeStorageNode *storageNode2 = NULL;
  vtkMRMLStorageNode *storageNode = NULL;
  vtkMRMLStorageNode *snode = volumeNode->GetStorageNode();

  if (snode != NULL)
    {
    storageNode2 = vtkMRMLVolumeArchetypeStorageNode::SafeDownCast(snode);
    storageNode1 = vtkMRMLNRRDStorageNode::SafeDownCast(snode);
    }

  bool useURI = false;
  if (this->GetMRMLScene() &&
      this->GetMRMLScene()->GetCacheManager())
    {
    useURI = this->GetMRMLScene()->GetCacheManager()->IsRemoteReference(filename);
    }
  
  // Use NRRD writer if we are dealing with DWI, DTI or vector volumes

  if (volumeNode->IsA("vtkMRMLDiffusionWeightedVolumeNode") ||
      volumeNode->IsA("vtkMRMLDiffusionTensorVolumeNode") ||
      volumeNode->IsA("vtkMRMLVectorVolumeNode"))
    {

    if (storageNode1 == NULL)
      {
      storageNode1 = vtkMRMLNRRDStorageNode::New();
      storageNode1->SetScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(storageNode1);
      volumeNode->SetAndObserveStorageNodeID(storageNode1->GetID());
      storageNode1->Delete();
      }
    if (useURI)
      {
      storageNode1->SetURI(filename);
      }
    else
      {
      storageNode1->SetFileName(filename);
      }
    storageNode = storageNode1;
    }
  else
    {
    if (storageNode2 == NULL)
      {
      storageNode2 = vtkMRMLVolumeArchetypeStorageNode::New();
      storageNode2->SetScene(this->GetMRMLScene());
      this->GetMRMLScene()->AddNode(storageNode2);
      volumeNode->SetAndObserveStorageNodeID(storageNode2->GetID());
      storageNode2->Delete();
      }

    if (useURI)
      {
      storageNode2->SetURI(filename);
      }
    else
      {
      storageNode2->SetFileName(filename);
      }
    storageNode = storageNode2;
    }

  int res = storageNode->WriteData(volumeNode);
  return res;
}

void vtkSlicerVolumesLogic::CalculateAutoLevels(vtkImageData *imageData, vtkMRMLScalarVolumeDisplayNode *displayNode)
{
  if ( !imageData || !displayNode) 
    {
    return;
    }
  vtkImageData *imageDataScalar = imageData;
  
  vtkImageExtractComponents *extractComp = NULL;
  vtkDiffusionTensorMathematics *DTIMathematics = NULL;  
  vtkAssignAttribute *AssignAttributeTensorsFromScalars = NULL;

  vtkMRMLDiffusionTensorVolumeDisplayNode *dtDisplayNode = vtkMRMLDiffusionTensorVolumeDisplayNode::SafeDownCast(displayNode);
  vtkMRMLDiffusionWeightedVolumeDisplayNode *dwDisplayNode = vtkMRMLDiffusionWeightedVolumeDisplayNode::SafeDownCast(displayNode);
 
 if (dtDisplayNode != NULL ) 
    {
    if (dtDisplayNode->GetDiffusionTensorDisplayPropertiesNode())
      {
      DTIMathematics = vtkDiffusionTensorMathematics::New();    
      AssignAttributeTensorsFromScalars= vtkAssignAttribute::New();
      AssignAttributeTensorsFromScalars->Assign(vtkDataSetAttributes::TENSORS, vtkDataSetAttributes::SCALARS, vtkAssignAttribute::POINT_DATA);  
      
      DTIMathematics->SetInput(imageData);
      DTIMathematics->SetOperation(dtDisplayNode->GetDiffusionTensorDisplayPropertiesNode()->
                                     GetScalarInvariant());
      DTIMathematics->Update();
      imageDataScalar = DTIMathematics->GetOutput();
      }
    else
      {
      imageDataScalar = NULL;
      }
    }
  else if (dwDisplayNode != NULL) 
    {
    extractComp = vtkImageExtractComponents::New();
    extractComp->SetInput(imageData);
    extractComp->SetComponents(dwDisplayNode->GetDiffusionComponent());
    imageDataScalar = extractComp->GetOutput();
    }
  
  vtkSlicerVolumesLogic::CalculateScalarAutoLevels(imageDataScalar, displayNode);
    
  if (extractComp)
    {
    extractComp->Delete();
    }
  if (DTIMathematics)
    {
    DTIMathematics->Delete();
    }
  if (AssignAttributeTensorsFromScalars)
    {
    AssignAttributeTensorsFromScalars->Delete();
    }
}

void vtkSlicerVolumesLogic::CalculateScalarAutoLevels(vtkImageData *imageData, vtkMRMLScalarVolumeDisplayNode *displayNode)
{
  if ( !imageData || !displayNode) 
    {
    return;
    }
  vtkImageData *imageDataScalar = imageData;

  //if (imageDataScalar) 
  if (imageDataScalar && imageDataScalar->GetNumberOfScalarComponents() == 1) 
    {
    vtkImageAccumulateDiscrete *accumulate = vtkImageAccumulateDiscrete::New();
    vtkImageBimodalAnalysis *bimodal = vtkImageBimodalAnalysis::New();

    accumulate->SetInput(imageDataScalar);
    bimodal->SetInput(accumulate->GetOutput());
    bimodal->Update();

    // Workaround for image data where all accumulate samples fall
    // within the same histogram bin
    if (bimodal->GetWindow() == 0.0 && bimodal->GetLevel() == 0.0) 
      {
      double range[2];
      imageDataScalar->GetScalarRange(range);
      double min = range[0];
      double max = range[1];
      displayNode->SetWindow (max-min);
      displayNode->SetLevel (0.5*(max+min));
      displayNode->SetLowerThreshold (displayNode->GetLevel());
      displayNode->SetUpperThreshold (range[1]);
      }
    else
      {
      displayNode->SetWindow (bimodal->GetWindow());
      displayNode->SetLevel (bimodal->GetLevel());
      displayNode->SetLowerThreshold (bimodal->GetThreshold());
      displayNode->SetUpperThreshold (bimodal->GetMax());
      }

    accumulate->Delete();
    bimodal->Delete();
    }
    
}


//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode *vtkSlicerVolumesLogic::CreateLabelVolume (vtkMRMLScene *scene, vtkMRMLVolumeNode *volumeNode, char *name)
{
  if ( volumeNode == NULL ) 
    {
    return NULL;
    }

  // create a display node
  vtkMRMLLabelMapVolumeDisplayNode *labelDisplayNode  = vtkMRMLLabelMapVolumeDisplayNode::New();

  scene->AddNode(labelDisplayNode);

  // create a volume node as copy of source volume
  vtkMRMLScalarVolumeNode *labelNode = vtkMRMLScalarVolumeNode::New();
  labelNode->CopyWithScene(volumeNode);
  labelNode->SetAndObserveStorageNodeID(NULL);
  labelNode->SetModifiedSinceRead(1);
  labelNode->SetLabelMap(1);

  // set the display node to have a label map lookup table
  labelDisplayNode->SetAndObserveColorNodeID ("vtkMRMLColorTableNodeLabels");
  labelNode->SetName(name);
  labelNode->SetAndObserveDisplayNodeID( labelDisplayNode->GetID() );

  // make an image data of the same size and shape as the input volume,
  // but filled with zeros
  vtkImageThreshold *thresh = vtkImageThreshold::New();
  thresh->ReplaceInOn();
  thresh->ReplaceOutOn();
  thresh->SetInValue(0);
  thresh->SetOutValue(0);
  thresh->SetOutputScalarType (VTK_SHORT);
  thresh->SetInput( volumeNode->GetImageData() );
  thresh->GetOutput()->Update();
  labelNode->SetAndObserveImageData( thresh->GetOutput() );
  thresh->Delete();

  // add the label volume to the scene
  scene->AddNode(labelNode);

  labelNode->Delete();
  labelDisplayNode->Delete();

  return (labelNode);
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode*
vtkSlicerVolumesLogic::
CloneVolume (vtkMRMLScene *scene, 
             vtkMRMLVolumeNode *volumeNode, 
             const char *name)
{
  if ( scene == NULL || volumeNode == NULL ) 
    {
    return NULL;
    }

  // clone the display node
  vtkMRMLScalarVolumeDisplayNode *clonedDisplayNode = 
    vtkMRMLScalarVolumeDisplayNode::New();
  clonedDisplayNode->CopyWithScene(volumeNode->GetDisplayNode());
  scene->AddNode(clonedDisplayNode);

  // clone the volume node
  vtkMRMLScalarVolumeNode *clonedVolumeNode = vtkMRMLScalarVolumeNode::New();
  clonedVolumeNode->CopyWithScene(volumeNode);
  clonedVolumeNode->SetAndObserveStorageNodeID(NULL);
  clonedVolumeNode->SetName(name);
  clonedVolumeNode->SetAndObserveDisplayNodeID(clonedDisplayNode->GetID());

  // copy over the volume's data
  vtkImageData* clonedVolumeData = vtkImageData::New(); 
  clonedVolumeData->DeepCopy(volumeNode->GetImageData());
  clonedVolumeNode->SetAndObserveImageData( clonedVolumeData );
  clonedVolumeNode->SetModifiedSinceRead(1);

  // add the cloned volume to the scene
  scene->AddNode(clonedVolumeNode);

  // remove references
  clonedVolumeNode->Delete();
  clonedVolumeData->Delete();
  clonedDisplayNode->Delete();

  return (clonedVolumeNode);
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerVolumesLogic:             " << this->GetClassName() << "\n";

  os << indent << "ActiveVolumeNode: " <<
    (this->ActiveVolumeNode ? this->ActiveVolumeNode->GetName() : "(none)") << "\n";
}

//----------------------------------------------------------------------------
int vtkSlicerVolumesLogic::IsFreeSurferVolume (const char* filename)
{
  std::string fname(filename);
  std::string::size_type loc = fname.find(".");
  if (loc != std::string::npos)
    {
    std::string extension = fname.substr(loc);
    if (extension == std::string(".mgz") ||
        extension == std::string(".mgh") ||
        extension == std::string(".mgh.gz"))
      {
      return 1;
      }
    else
      {
      return 0;
      }
    }
  else
    {
    return 0;
    }
}


//-------------------------------------------------------------------------
void vtkSlicerVolumesLogic::ComputeTkRegVox2RASMatrix ( vtkMRMLVolumeNode *VNode,
                                                                       vtkMatrix4x4 *M )
{
    double dC, dS, dR;
    double Nc, Ns, Nr;
    int dim[3];


    double *spacing = VNode->GetSpacing();
    dC = spacing[0];
    dR = spacing[1];
    dS = spacing[2];

    VNode->GetImageData()->GetDimensions(dim);
    Nc = dim[0] * dC;
    Nr = dim[1] * dR;
    Ns = dim[2] * dS;

    M->Zero();
    M->SetElement ( 0, 0, -dC );
    M->SetElement ( 0, 3, Nc/2.0 );
    M->SetElement ( 1, 2, dS );
    M->SetElement ( 1, 3, -Ns/2.0 );
    M->SetElement ( 2, 1, -dR );
    M->SetElement ( 2, 3, Nr/2.0 );
    M->SetElement ( 3, 3, 1.0 );
}




//-------------------------------------------------------------------------
void vtkSlicerVolumesLogic::TranslateFreeSurferRegistrationMatrixIntoSlicerRASToRASMatrix( vtkMRMLVolumeNode *V1Node,
                                                                       vtkMRMLVolumeNode *V2Node,
                                                                       vtkMatrix4x4 *FSRegistrationMatrix,
                                                                       vtkMatrix4x4 *RAS2RASMatrix)
{
  if  ( V1Node  && V2Node && FSRegistrationMatrix  && RAS2RASMatrix )
    {

    if ( RAS2RASMatrix == NULL )
      {
      RAS2RASMatrix = vtkMatrix4x4::New();
      }
    RAS2RASMatrix->Zero();
    
    //
    // Looking for RASv1_To_RASv2:
    //
    //---
    //
    // In Slicer:
    // [ IJKv1_To_IJKv2] = [ RAS_To_IJKv2 ]  [ RASv1_To_RASv2 ] [ IJK_To_RASv1 ] [i,j,k]transpose
    //
    // In FreeSurfer:
    // [ IJKv1_To_IJKv2] = [FStkRegVox_To_RASv2 ]inverse [ FSRegistrationMatrix] [FStkRegVox_To_RASv1 ] [ i,j,k] transpose
    //
    //----
    //
    // So:
    // [FStkRegVox_To_RASv2 ] inverse [ FSRegistrationMatrix] [FStkRegVox_To_RASv1 ] =
    // [ RAS_To_IJKv2 ]  [ RASv1_To_RASv2 ] [ IJKv1_2_RAS ]
    //
    //---
    //
    // Below use this shorthand:
    //
    // S = FStkRegVox_To_RASv2
    // T = FStkRegVox_To_RASv1
    // N = RAS_To_IJKv2
    // M = IJK_To_RASv1
    // R = FSRegistrationMatrix
    // [Sinv]  [R]  [T] = [N]  [RASv1_To_RASv2]  [M];
    //
    // So this is what we'll compute and use in Slicer instead
    // of the FreeSurfer register.dat matrix:
    //
    // [Ninv]  [Sinv]  [R]  [T]  [Minv]  = RASv1_To_RASv2
    //
    // I think we need orientation in FreeSurfer: nothing in the tkRegVox2RAS
    // handles scanOrder. The tkRegVox2RAS = IJKToRAS matrix for a coronal
    // volume. But for an Axial volume, these two matrices are different.
    // How do we compute the correct orientation for FreeSurfer Data?
  
    vtkMatrix4x4 *T = vtkMatrix4x4::New();
    vtkMatrix4x4 *S = vtkMatrix4x4::New();
    vtkMatrix4x4 *Sinv = vtkMatrix4x4::New();
    vtkMatrix4x4 *M = vtkMatrix4x4::New();
    vtkMatrix4x4 *Minv = vtkMatrix4x4::New();
    vtkMatrix4x4 *N = vtkMatrix4x4::New();
    vtkMatrix4x4 *Ninv = vtkMatrix4x4::New();

    //--
    // compute FreeSurfer tkRegVox2RAS for V1 volume
    //--
    ComputeTkRegVox2RASMatrix ( V1Node, T );

    //--
    // compute FreeSurfer tkRegVox2RAS for V2 volume
    //--
    ComputeTkRegVox2RASMatrix ( V2Node, S );

    // Probably a faster way to do these things?
    vtkMatrix4x4::Invert (S, Sinv );
    V1Node->GetIJKToRASMatrix( M );
    V2Node->GetRASToIJKMatrix( N );
    vtkMatrix4x4::Invert (M, Minv );
    vtkMatrix4x4::Invert (N, Ninv );

   //    [Ninv]  [Sinv]  [R]  [T]  [Minv]
    vtkMatrix4x4::Multiply4x4 ( T, Minv, RAS2RASMatrix );
    vtkMatrix4x4::Multiply4x4 ( FSRegistrationMatrix, RAS2RASMatrix, RAS2RASMatrix );
    vtkMatrix4x4::Multiply4x4 ( Sinv, RAS2RASMatrix, RAS2RASMatrix );
    vtkMatrix4x4::Multiply4x4 ( Ninv, RAS2RASMatrix, RAS2RASMatrix );    
  
    // clean up
    M->Delete();
    N->Delete();
    Minv->Delete();
    Ninv->Delete();
    S->Delete();
    T->Delete();
    Sinv->Delete();
    }
}
