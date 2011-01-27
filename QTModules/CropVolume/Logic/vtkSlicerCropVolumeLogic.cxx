/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerCropVolumeLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

// 
#include "vtkSlicerCropVolumeLogic.h"
#include "vtkSlicerColorLogic.h"

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLVectorVolumeNode.h>
#include <vtkMRMLVolumeArchetypeStorageNode.h>
#include <vtkMRMLVolumeHeaderlessStorageNode.h>
#include <vtkMRMLNRRDStorageNode.h>
#include <vtkMRMLDiffusionTensorVolumeNode.h>
#include <vtkMRMLDiffusionWeightedVolumeNode.h>
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLLabelMapVolumeDisplayNode.h>
#include <vtkMRMLVectorVolumeDisplayNode.h>
#include <vtkMRMLDiffusionTensorVolumeDisplayNode.h>
#include <vtkMRMLDiffusionWeightedVolumeDisplayNode.h>
#include <vtkMRMLDiffusionTensorDisplayPropertiesNode.h>
#include <vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx> 

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkCallbackCommand.h>
#include <vtkSmartPointer.h>
#include <vtkImageThreshold.h>
#include <vtkImageAccumulateDiscrete.h>
#include <vtkImageBimodalAnalysis.h>
#include <vtkImageExtractComponents.h>
#include <vtkDiffusionTensorMathematics.h>
#include <vtkAssignAttribute.h>
#include <vtkStringArray.h>

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerCropVolumeLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerCropVolumeLogic);

//----------------------------------------------------------------------------
vtkSlicerCropVolumeLogic::vtkSlicerCropVolumeLogic()
{
  this->ActiveVolumeNode = 0;
}

//----------------------------------------------------------------------------
vtkSlicerCropVolumeLogic::~vtkSlicerCropVolumeLogic()
{
  if (this->ActiveVolumeNode != 0)
    {
    this->ActiveVolumeNode->Delete();
    this->ActiveVolumeNode = 0;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerCropVolumeLogic::ProcessMRMLEvents(vtkObject *vtkNotUsed(caller),
                                              unsigned long vtkNotUsed(event),
                                              void *vtkNotUsed(callData))
{
}

//----------------------------------------------------------------------------
void vtkSlicerCropVolumeLogic::ProcessLogicEvents(vtkObject *vtkNotUsed(caller), 
                                            unsigned long event, 
                                            void *callData)
{
  if (event ==  vtkCommand::ProgressEvent) 
    {
    this->InvokeEvent(vtkCommand::ProgressEvent,callData);
    }
}

//#include <vtkMRMLScalarVolumeDisplayNode.h"
//----------------------------------------------------------------------------
void vtkSlicerCropVolumeLogic::SetActiveVolumeNode(vtkMRMLVolumeNode *activeNode)
{
  vtkSetMRMLNodeMacro(this->ActiveVolumeNode, activeNode );
  this->Modified();
}


//----------------------------------------------------------------------------
// int loadingOptions is bit-coded as following:
// bit 0: label map
// bit 1: centered
// bit 2: loading signal file
// bit 3: calculate window/level
// bit 4: set image orientation as IJK, ie. discard image orientation from file
// higher bits are reserved for future use
vtkMRMLVolumeNode* vtkSlicerCropVolumeLogic::AddHeaderVolume (const char* filename, const char* volname, 
                                                           vtkMRMLVolumeHeaderlessStorageNode *headerStorage,
                                                           int loadingOptions)
{

  int centerImage = 0;
  int labelMap = 0;
  int singleFile = 0;
  int useOrientationFromFile = 1;

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
  if ( loadingOptions & 16 )    // discard image orientation from file
    {
    useOrientationFromFile = 0;
    }

  vtkMRMLVolumeNode *volumeNode = 0;
  vtkMRMLVolumeDisplayNode *displayNode = 0;

  vtkMRMLScalarVolumeDisplayNode *sdisplayNode = 0;
  vtkMRMLVectorVolumeDisplayNode *vdisplayNode = 0;

  vtkMRMLScalarVolumeNode *scalarNode = vtkMRMLScalarVolumeNode::New();
  vtkMRMLVectorVolumeNode *vectorNode = vtkMRMLVectorVolumeNode::New();

  vtkMRMLVolumeHeaderlessStorageNode *storageNode = vtkMRMLVolumeHeaderlessStorageNode::New();
  storageNode->CopyWithScene(headerStorage);
  
  storageNode->SetFileName(filename);
  storageNode->SetCenterImage(centerImage);
  storageNode->AddObserver(vtkCommand::ProgressEvent,  this->GetLogicCallbackCommand());

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

  storageNode->RemoveObservers(vtkCommand::ProgressEvent,  this->GetLogicCallbackCommand());

  if (volumeNode != 0)
    {
    if (volname == 0)
      {
      const vtksys_stl::string fname(filename);
      vtksys_stl::string name = vtksys::SystemTools::GetFilenameName(fname);
      name = this->GetMRMLScene()->GetUniqueNameByString(name.c_str());
      volumeNode->SetName(name.c_str());
      }
    else
      {
      std::string name = this->GetMRMLScene()->GetUniqueNameByString(volname);
      volumeNode->SetName(name.c_str());
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
vtkMRMLScalarVolumeNode* vtkSlicerCropVolumeLogic::AddArchetypeScalarVolume (const char* filename, const char* volname, int loadingOptions)
{ 
  int centerImage = 0;
  int labelMap = 0;
  int singleFile = 0;
  int useOrientationFromFile = 1;
  int autoLevel = 0;
  int interpolate = 1;
  
  if ( loadingOptions & 1 )    // labelMap is true
    {
    labelMap = 1;
    interpolate = 0;
    }
  if ( loadingOptions & 2 )    // centerImage is true
    {
    centerImage = 1;
    }
  if ( loadingOptions & 4 )    // singleFile is true
    {
    singleFile = 1;
    }
  if ( loadingOptions & 8 ) // calculate window level automaticaly
    {
    if (!labelMap)
      {
      autoLevel = 1;
      }
    }
  if ( loadingOptions & 16 )    // discard image orientation from file
    {
    useOrientationFromFile = 0;
    }

  vtkMRMLScalarVolumeDisplayNode *displayNode = vtkMRMLScalarVolumeDisplayNode::New();
  vtkMRMLScalarVolumeNode *scalarNode = vtkMRMLScalarVolumeNode::New();
  vtkMRMLVolumeArchetypeStorageNode *storageNode = vtkMRMLVolumeArchetypeStorageNode::New();

  displayNode->SetAutoWindowLevel(autoLevel);
  displayNode->SetInterpolate(interpolate);
  
  bool useURI = false;
  const char *localFile;

  if (this->GetMRMLScene() &&
      this->GetMRMLScene()->GetCacheManager())
    {
    useURI = this->GetMRMLScene()->GetCacheManager()->IsRemoteReference(filename);
    }
  if (useURI)
    {
    vtkDebugMacro("AddArchetypeScalarVolume: input filename '" << filename << "' is a URI");
    storageNode->SetURI(filename);
    storageNode->SetScene(this->GetMRMLScene());
    localFile = ((this->GetMRMLScene())->GetCacheManager())->GetFilenameFromURI(filename);
    vtkDebugMacro("AddArchetypeScalarVolume: local file name = " << localFile);
    }
  else
    {
      storageNode->SetFileName(filename);
      localFile = filename;
    }

  // check to see if can read this type of file
  if (storageNode->SupportedFileType(filename) == 0)
    {
      vtkErrorMacro("LoadArchetypeScalarVolume: volume archetype storage node can't read this kind of file: " << filename);
      return 0;
    }
  else { vtkDebugMacro("LoadArchetypeScalarVolume: filename is a supported type"); }

  storageNode->SetCenterImage(centerImage);
  storageNode->SetSingleFile(singleFile);
  storageNode->SetUseOrientationFromFile(useOrientationFromFile);
  storageNode->AddObserver(vtkCommand::ProgressEvent,  this->GetLogicCallbackCommand());

  if (volname == 0)
    {
    const vtksys_stl::string fname(filename);
    vtksys_stl::string name = vtksys::SystemTools::GetFilenameName(fname);
    name = this->GetMRMLScene()->GetUniqueNameByString(name.c_str());
    scalarNode->SetName(name.c_str());
    }
  else
    {
    std::string name = this->GetMRMLScene()->GetUniqueNameByString(volname);
    scalarNode->SetName(name.c_str());
    }
  vtkDebugMacro("LoadArchetypeScalarVolume: set scalar node name: " << scalarNode->GetName());
  scalarNode->SetLabelMap(labelMap);
  
  this->GetMRMLScene()->SaveStateForUndo();
  
  scalarNode->SetScene(this->GetMRMLScene());
  displayNode->SetScene(this->GetMRMLScene());

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
  
  vtkDebugMacro("LoadArchetypeScalarVolume: adding storage node to the scene");
  this->GetMRMLScene()->AddNode(storageNode);
  vtkDebugMacro("LoadArchetypeScalarVolume: adding display node to the scene");
  this->GetMRMLScene()->AddNode(displayNode);
  
  scalarNode->SetAndObserveStorageNodeID(storageNode->GetID());
  scalarNode->SetAndObserveDisplayNodeID(displayNode->GetID());

  vtkDebugMacro("LoadArchetypeScalarVolume: adding scalar node to the scene");
  this->GetMRMLScene()->AddNode(scalarNode);


  // now read
  vtkDebugMacro("AddArchetypeScalarVolume: about to read data into scalar node " << scalarNode->GetName() << ", asynch = " << this->GetMRMLScene()->GetDataIOManager()->GetEnableAsynchronousIO() << ", read state = " << storageNode->GetReadState());
  if (this->GetDebug())
    {
    storageNode->DebugOn();
    }
  storageNode->ReadData(scalarNode);
  vtkDebugMacro("AddArchetypeScalarVolume: finished reading data into scalarNode");

  storageNode->RemoveObservers(vtkCommand::ProgressEvent,  this->GetLogicCallbackCommand());
 
  if (storageNode->GetReadState() == vtkMRMLStorageNode::TransferDone ||
      storageNode->GetReadState() == vtkMRMLStorageNode::Idle)
    {
    vtkDebugMacro("AddArchetypeScalarVolume: setting active volume node " << scalarNode->GetName());
    this->SetActiveVolumeNode(scalarNode);
    }
  else 
    {
    vtkDebugMacro("AddArchetypeScalarVollume: not setting this volume as the active node, dl not flagged as finished yet on node " << scalarNode->GetName() << ", read flag = " << storageNode->GetReadStateAsString()); 
    }
  this->Modified();

  scalarNode->Delete();
  storageNode->Delete();
  displayNode->Delete();
  
  return scalarNode;
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkSlicerCropVolumeLogic::AddArchetypeScalarVolume(
  const char *filename, const char* volname) 
{
  return this->AddArchetypeScalarVolume(filename, volname, 0);
}

//----------------------------------------------------------------------------
// int loadingOptions is bit-coded as following:
// bit 0: label map
// bit 1: centered
// bit 2: loading single file
// bit 3: auto calculate window/level
// bit 4: discard image orientation
// higher bits are reserved for future use
vtkMRMLVolumeNode* vtkSlicerCropVolumeLogic::AddArchetypeVolume (const char* filename, const char* volname, int loadingOptions, vtkStringArray *fileList)
{
  bool importingScene = false;
  if (this->GetMRMLScene() &&
      this->GetMRMLScene()->GetIsImporting() == false)
    {
    importingScene = true;
    this->GetMRMLScene()->SetIsImporting(true);
    this->GetMRMLScene()->InvokeEvent(vtkMRMLScene::SceneAboutToBeImportedEvent, 0);
    }
  int centerImage = 0;
  int labelMap = 0;
  int singleFile = 0;
  int useOrientationFromFile = 1;
  int autoLevel = 0;
  int interpolate = 1;
  if ( loadingOptions & 1 )    // labelMap is true
    {
    labelMap = 1;
    interpolate = 0;
    }
  if ( loadingOptions & 2 )    // centerImage is true
    {
    centerImage = 1;
    }
  if ( loadingOptions & 4 )    // singleFile is true
    {
    singleFile = 1;
    }
  if ( loadingOptions & 8 ) // calculate window level automaticaly
    {
    if (!labelMap)
      {
      autoLevel = 1;
      }
    }
  if ( loadingOptions & 16 )    // discard image orientation from file
    {
    useOrientationFromFile = 0;
    }

  vtkSmartPointer<vtkMRMLVolumeNode> volumeNode = 0;
  vtkSmartPointer<vtkMRMLVolumeDisplayNode> displayNode = 0;
  vtkSmartPointer<vtkMRMLLabelMapVolumeDisplayNode> lmdisplayNode= 0;

  vtkSmartPointer<vtkMRMLDiffusionTensorVolumeDisplayNode> dtdisplayNode = 0;
  vtkSmartPointer<vtkMRMLDiffusionWeightedVolumeDisplayNode> dwdisplayNode = 0;
  vtkSmartPointer<vtkMRMLVectorVolumeDisplayNode> vdisplayNode = 0;
  vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode> sdisplayNode = 0;

  vtkSmartPointer<vtkMRMLScalarVolumeNode> scalarNode = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
  vtkSmartPointer<vtkMRMLVectorVolumeNode> vectorNode = vtkSmartPointer<vtkMRMLVectorVolumeNode>::New();
  vtkSmartPointer<vtkMRMLDiffusionTensorVolumeNode> tensorNode = vtkSmartPointer<vtkMRMLDiffusionTensorVolumeNode>::New();
  vtkSmartPointer<vtkMRMLDiffusionWeightedVolumeNode> dwiNode = vtkSmartPointer<vtkMRMLDiffusionWeightedVolumeNode>::New();

  // Instanciation of the two I/O mechanism
  vtkSmartPointer<vtkMRMLNRRDStorageNode> storageNode1 = vtkSmartPointer<vtkMRMLNRRDStorageNode>::New();
  vtkSmartPointer<vtkMRMLVolumeArchetypeStorageNode> storageNode2 = vtkSmartPointer<vtkMRMLVolumeArchetypeStorageNode>::New();
  vtkSmartPointer<vtkMRMLStorageNode> storageNode = 0;

  // set the volume name
  std::string volumeName;
  if (volname != 0)
    {
    volumeName = std::string(volname);
    }
  else
    {
    const vtksys_stl::string fname(filename);
    volumeName = vtksys::SystemTools::GetFilenameName(fname);
    }
  // now set all the volume names, before add the volumes to the scene
  volumeName = this->GetMRMLScene()->GetUniqueNameByString(volumeName.c_str());

  scalarNode->SetName(volumeName.c_str());
  vectorNode->SetName(volumeName.c_str());
  tensorNode->SetName(volumeName.c_str());
  dwiNode->SetName(volumeName.c_str());

  bool useURI = false;
  if (this->GetMRMLScene() &&
      this->GetMRMLScene()->GetCacheManager())
    {
    useURI = this->GetMRMLScene()->GetCacheManager()->IsRemoteReference(filename);
    }
  if (useURI)
    {
    vtkDebugMacro("AddArchetypeVolume: input filename '" << filename << "' is a URI");
    // need to set the scene on the storage node so that it can look for file handlers
    storageNode1->SetURI(filename);
    storageNode1->SetScene(this->GetMRMLScene());
    storageNode2->SetURI(filename);
    storageNode2->SetScene(this->GetMRMLScene());
    if (fileList != 0)
      {
      // it's a list of uris
      int numURIs = fileList->GetNumberOfValues();
      vtkDebugMacro("Have a list of " << numURIs << " uris that go along with the archetype");
      vtkStdString thisURI;
      storageNode1->ResetURIList();
      storageNode2->ResetURIList();
      for (int n = 0; n < numURIs; n++)
        {
        thisURI = fileList->GetValue(n);
        storageNode1->AddURI(thisURI);
        storageNode2->AddURI(thisURI);
        }
      }
      
    }
  else
    {
    storageNode1->SetFileName(filename);
    storageNode2->SetFileName(filename);
    if (fileList != 0)
      {
      int numFiles = fileList->GetNumberOfValues();
      vtkDebugMacro("Have a list of " << numFiles << " files that go along with the archetype");
      vtkStdString thisFileName;
      storageNode1->ResetFileNameList();
      storageNode2->ResetFileNameList();
      for (int n = 0; n < numFiles; n++)
        {
        thisFileName = fileList->GetValue(n);
        //vtkDebugMacro("\tfile " << n << " =  " << thisFileName);
        storageNode1->AddFileName(thisFileName);
        storageNode2->AddFileName(thisFileName);
        }
      }
    }
  storageNode1->SetCenterImage(centerImage);
  storageNode1->AddObserver(vtkCommand::ProgressEvent,  this->GetLogicCallbackCommand());

  
  storageNode2->SetCenterImage(centerImage);
  storageNode2->SetSingleFile(singleFile);
  storageNode2->SetUseOrientationFromFile(useOrientationFromFile);
  storageNode2->AddObserver(vtkCommand::ProgressEvent,  this->GetLogicCallbackCommand());

  this->GetMRMLScene()->SaveStateForUndo();
   
  // add storage nodes to the scene so can observe them
  this->GetMRMLScene()->AddNode(storageNode1);
  this->GetMRMLScene()->AddNode(storageNode2);

  // Try to read first with NRRD reader (look if file is a dwi or a tensor)

  // set up the dwi node's support nodes
  dwdisplayNode = vtkSmartPointer<vtkMRMLDiffusionWeightedVolumeDisplayNode>::New();
  this->GetMRMLScene()->AddNode(dwdisplayNode);
  this->GetMRMLScene()->AddNode(dwiNode);
  dwiNode->SetAndObserveStorageNodeID(storageNode1->GetID());
  dwiNode->SetAndObserveDisplayNodeID(dwdisplayNode->GetID());

  // set up the tensor node's support nodes
  dtdisplayNode = vtkSmartPointer<vtkMRMLDiffusionTensorVolumeDisplayNode>::New();
  dtdisplayNode->SetWindow(0);
  dtdisplayNode->SetLevel(0);
  dtdisplayNode->SetUpperThreshold(0);
  dtdisplayNode->SetLowerThreshold(0);
  dtdisplayNode->SetAutoWindowLevel(1);
  this->GetMRMLScene()->AddNode(dtdisplayNode);
  this->GetMRMLScene()->AddNode(tensorNode);
  tensorNode->SetAndObserveStorageNodeID(storageNode1->GetID());
  tensorNode->SetAndObserveDisplayNodeID(dtdisplayNode->GetID());
    
  // set up the vector node's support nodes
  vdisplayNode = vtkSmartPointer<vtkMRMLVectorVolumeDisplayNode>::New();
  this->GetMRMLScene()->AddNode(vdisplayNode);
  this->GetMRMLScene()->AddNode(vectorNode);
  vectorNode->SetAndObserveStorageNodeID(storageNode1->GetID());
  vectorNode->SetAndObserveDisplayNodeID(vdisplayNode->GetID());

  // set up the scalar node's support nodes
  this->GetMRMLScene()->AddNode(scalarNode);
  scalarNode->SetAndObserveStorageNodeID(storageNode2->GetID());
  if (labelMap)
    {
    lmdisplayNode = vtkSmartPointer<vtkMRMLLabelMapVolumeDisplayNode>::New();
    this->GetMRMLScene()->AddNode(lmdisplayNode);
    scalarNode->SetAndObserveDisplayNodeID(lmdisplayNode->GetID());
    }
  else
    {
    sdisplayNode = vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode>::New();
    this->GetMRMLScene()->AddNode(sdisplayNode);
    scalarNode->SetAndObserveDisplayNodeID(sdisplayNode->GetID());
    }

  // try reading, keeping track of which one suceeded
  int nodeSetUsed = 0;
  if (storageNode1->ReadData(dwiNode))
    {
    vtkDebugMacro("DWI HAS BEEN READ");
   
    displayNode = dwdisplayNode;
    // Give a chance to set/update displayNode
    volumeNode =  dwiNode;
    storageNode = storageNode1;
    vtkDebugMacro("Done setting volumeNode to class: "<<volumeNode->GetClassName());
    nodeSetUsed = 1;
    }
  else if (storageNode1->ReadData(tensorNode))
    {
    vtkDebugMacro("Tensor HAS BEEN READ");    
    displayNode = dtdisplayNode;
    dtdisplayNode->AddSliceGlyphDisplayNodes(tensorNode);
    volumeNode = tensorNode;
    storageNode = storageNode1;
    nodeSetUsed = 2;
    }
  else if (storageNode1->ReadData(vectorNode))
    {
    vtkDebugMacro("Vector HAS BEEN READ WITH NRRD READER");
    
    displayNode = vdisplayNode;
    volumeNode = vectorNode;
    storageNode = storageNode1;
    nodeSetUsed = 3;
    }
  else
    {
    vectorNode->SetAndObserveStorageNodeID(storageNode2->GetID());
    if (storageNode2->ReadData(vectorNode)) // storageNode2->SupportedFileType(filename))  
      {
      vtkDebugMacro("Vector HAS BEEN READ WITH ARCHTYPE READER");
      displayNode = vdisplayNode;
      volumeNode = vectorNode;
      storageNode = storageNode2;
      nodeSetUsed = 4;
      }
    else if (storageNode2->ReadData(scalarNode)) //storageNode2->SupportedFileType(filename)) // 
      {
      vtkDebugMacro("Scalar HAS BEEN READ WITH ARCHTYPE READER");
      if (labelMap) 
        {
        displayNode = lmdisplayNode;
        }
      else
        {
        displayNode = sdisplayNode;
        }
      scalarNode->SetLabelMap(labelMap);
      volumeNode = scalarNode;
      storageNode = storageNode2;
      nodeSetUsed = 5;
      }
    }

  storageNode1->RemoveObservers(vtkCommand::ProgressEvent,  this->GetLogicCallbackCommand());
  storageNode2->RemoveObservers(vtkCommand::ProgressEvent,  this->GetLogicCallbackCommand());
  
  if (nodeSetUsed != 1)
    {
    dwiNode->SetAndObserveDisplayNodeID(0);
    dwiNode->SetAndObserveStorageNodeID(0);
    this->GetMRMLScene()->RemoveNode(dwdisplayNode);
    this->GetMRMLScene()->RemoveNode(dwiNode);
    }
  if (nodeSetUsed != 2)
    {
    tensorNode->SetAndObserveDisplayNodeID(0);
    tensorNode->SetAndObserveStorageNodeID(0);
    this->GetMRMLScene()->RemoveNode(dtdisplayNode);
    this->GetMRMLScene()->RemoveNode(tensorNode);
    }
  if (nodeSetUsed != 3 && nodeSetUsed != 4)
    {
    vectorNode->SetAndObserveDisplayNodeID(0);
    vectorNode->SetAndObserveStorageNodeID(0);
    this->GetMRMLScene()->RemoveNode(vdisplayNode);
    this->GetMRMLScene()->RemoveNode(vectorNode);
    }
  if (nodeSetUsed != 5)
    {
    scalarNode->SetAndObserveDisplayNodeID(0);
    scalarNode->SetAndObserveStorageNodeID(0);
    if (labelMap)
      {
      this->GetMRMLScene()->RemoveNode(lmdisplayNode);
      }
    else
      {
      this->GetMRMLScene()->RemoveNode(sdisplayNode);
      }

    this->GetMRMLScene()->RemoveNode(scalarNode);
    }

  // clean up the storage node
  if (storageNode != storageNode1)
    {
    this->GetMRMLScene()->RemoveNode(storageNode1);
    }
  else if (storageNode != storageNode2)
    {
    this->GetMRMLScene()->RemoveNode(storageNode2);
    }
  
  if (volumeNode != 0)
    {
    vtkSmartPointer<vtkSlicerColorLogic> colorLogic = vtkSmartPointer<vtkSlicerColorLogic>::New();
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
    
    vtkDebugMacro("Name vol node "<<volumeNode->GetClassName());
    vtkDebugMacro("Display node "<<displayNode->GetClassName());
        
    this->SetActiveVolumeNode(volumeNode);

    this->Modified();
    // since added the node w/o notification, let the scene know now that it
    // has a new node
    //this->GetMRMLScene()->InvokeEvent(vtkMRMLScene::NodeAddedEvent, volumeNode);
    }
  if (importingScene)
    {
    this->GetMRMLScene()->SetIsImporting(false);
    this->GetMRMLScene()->InvokeEvent(vtkMRMLScene::SceneImportedEvent, 0);
    }
  return volumeNode;
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerCropVolumeLogic::AddArchetypeVolume(const char* filename, 
                                                                    const char* volname, 
                                                                    int loadingOptions) 
{
  return (this->AddArchetypeVolume(filename, volname, loadingOptions, 0));
}
  
//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerCropVolumeLogic::AddArchetypeVolume(const char *filename, 
                                                                    const char* volname) 
{
  return this->AddArchetypeVolume(filename, volname, 0, 0);
};

//----------------------------------------------------------------------------
int vtkSlicerCropVolumeLogic::SaveArchetypeVolume(const char* filename, 
                                                      vtkMRMLVolumeNode *volumeNode)
{
  if (volumeNode == 0 || filename == 0)
    {
    return 0;
    }
  
  vtkMRMLNRRDStorageNode *storageNode1 = 0;
  vtkMRMLVolumeArchetypeStorageNode *storageNode2 = 0;
  vtkMRMLStorageNode *storageNode = 0;
  vtkMRMLStorageNode *snode = volumeNode->GetStorageNode();

  if (snode != 0)
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

    if (storageNode1 == 0)
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
    if (storageNode2 == 0)
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

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode *vtkSlicerCropVolumeLogic::CreateLabelVolume (vtkMRMLScene *scene, vtkMRMLVolumeNode *volumeNode, const char *name)
{
  if ( volumeNode == 0 ) 
    {
    return 0;
    }

  // create a display node
  vtkMRMLLabelMapVolumeDisplayNode *labelDisplayNode  = vtkMRMLLabelMapVolumeDisplayNode::New();

  scene->AddNode(labelDisplayNode);

  // create a volume node as copy of source volume
  vtkMRMLScalarVolumeNode *labelNode = vtkMRMLScalarVolumeNode::New();
  
  int modifiedSinceRead = volumeNode->GetModifiedSinceRead();
  labelNode->CopyWithScene(volumeNode);
  
  labelNode->SetAndObserveStorageNodeID(0);
  labelNode->SetModifiedSinceRead(1);
  labelNode->SetLabelMap(1);
  
  // restore modifiedSinceRead value since copy cause Modify on image data.
  volumeNode->SetModifiedSinceRead(modifiedSinceRead);

  // set the display node to have a label map lookup table
  vtkSmartPointer<vtkSlicerColorLogic> colorLogic = vtkSmartPointer<vtkSlicerColorLogic>::New();
  labelDisplayNode->SetAndObserveColorNodeID (colorLogic->GetDefaultLabelMapColorNodeID());
  std::string uname = this->GetMRMLScene()->GetUniqueNameByString(name);

  labelNode->SetName(uname.c_str());
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
vtkSlicerCropVolumeLogic::
CloneVolume (vtkMRMLScene *scene, 
             vtkMRMLVolumeNode *volumeNode, 
             const char *name)
{
  if ( scene == 0 || volumeNode == 0 ) 
    {
    return 0;
    }

  // clone the display node
  vtkMRMLDisplayNode *clonedDisplayNode;
  vtkMRMLLabelMapVolumeDisplayNode *labelDisplayNode = vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(volumeNode->GetDisplayNode());
  if ( labelDisplayNode )
    {
    clonedDisplayNode = vtkMRMLLabelMapVolumeDisplayNode::New();
    }
  else
    {
    clonedDisplayNode = vtkMRMLScalarVolumeDisplayNode::New();
    }
  clonedDisplayNode->CopyWithScene(volumeNode->GetDisplayNode());
  scene->AddNode(clonedDisplayNode);

  // clone the volume node
  vtkMRMLScalarVolumeNode *clonedVolumeNode = vtkMRMLScalarVolumeNode::New();
  clonedVolumeNode->CopyWithScene(volumeNode);
  clonedVolumeNode->SetAndObserveStorageNodeID(0);
  std::string uname = this->GetMRMLScene()->GetUniqueNameByString(name);
  clonedVolumeNode->SetName(uname.c_str());
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
void vtkSlicerCropVolumeLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);

  os << indent << "vtkSlicerCropVolumeLogic:             " << this->GetClassName() << "\n";

  os << indent << "ActiveVolumeNode: " <<
    (this->ActiveVolumeNode ? this->ActiveVolumeNode->GetName() : "(none)") << "\n";
}

//----------------------------------------------------------------------------
int vtkSlicerCropVolumeLogic::IsFreeSurferVolume (const char* filename)
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
void vtkSlicerCropVolumeLogic::ComputeTkRegVox2RASMatrix ( vtkMRMLVolumeNode *VNode,
                                                                       vtkMatrix4x4 *M )
{
    double dC, dS, dR;
    double Nc, Ns, Nr;
    int dim[3];

    if (!VNode)
      {
      vtkErrorMacro("ComputeTkRegVox2RASMatrix: input volume node is 0");
      return;
      }
    if (!M)
      {
      vtkErrorMacro("ComputeTkRegVox2RASMatrix: input matrix is 0");
      return;
      }
    double *spacing = VNode->GetSpacing();
    dC = spacing[0];
    dR = spacing[1];
    dS = spacing[2];

    if (VNode->GetImageData() == 0)
      {
      vtkErrorMacro("ComputeTkRegVox2RASMatrix: input volume's image data is 0");
      return;
      }
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
void vtkSlicerCropVolumeLogic::TranslateFreeSurferRegistrationMatrixIntoSlicerRASToRASMatrix( vtkMRMLVolumeNode *V1Node,
                                                                       vtkMRMLVolumeNode *V2Node,
                                                                       vtkMatrix4x4 *FSRegistrationMatrix,
                                                                       vtkMatrix4x4 *RAS2RASMatrix)
{
  if  ( V1Node  && V2Node && FSRegistrationMatrix  && RAS2RASMatrix )
    {

    if ( RAS2RASMatrix == 0 )
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
