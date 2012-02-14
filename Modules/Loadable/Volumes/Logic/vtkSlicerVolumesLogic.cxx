/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerVolumesLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

// Volumes includes
#include "vtkSlicerVolumesLogic.h"

// MRML logic includes
#include "vtkMRMLColorLogic.h"

// MRML nodes includes
#include "vtkCacheManager.h"
#include "vtkDataIOManager.h"
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLLabelMapVolumeDisplayNode.h"
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMRMLVectorVolumeDisplayNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLVolumeHeaderlessStorageNode.h"

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkImageData.h>
#include <vtkImageThreshold.h>
#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtksys/SystemTools.hxx>

vtkCxxRevisionMacro(vtkSlicerVolumesLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerVolumesLogic);
vtkCxxSetObjectMacro(vtkSlicerVolumesLogic, ColorLogic, vtkMRMLColorLogic);

//----------------------------------------------------------------------------
vtkSlicerVolumesLogic::vtkSlicerVolumesLogic()
{
  this->ActiveVolumeNode = NULL;
  this->ColorLogic = NULL;
}

//----------------------------------------------------------------------------
vtkSlicerVolumesLogic::~vtkSlicerVolumesLogic()
{
  if (this->ActiveVolumeNode != NULL)
    {
    this->ActiveVolumeNode->Delete();
    this->ActiveVolumeNode = NULL;
    }
  if (this->ColorLogic != NULL)
    {
    this->ColorLogic->Delete();
    this->ColorLogic = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::ProcessMRMLNodesEvents(vtkObject *vtkNotUsed(caller),
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
// bit 3: calculate window/level
// bit 4: set image orientation as IJK, ie. discard image orientation from file
// higher bits are reserved for future use
vtkMRMLVolumeNode* vtkSlicerVolumesLogic::AddHeaderVolume (const char* filename, const char* volname, 
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
  storageNode->AddObserver(vtkCommand::ProgressEvent,  this->GetMRMLNodesCallbackCommand());

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

 storageNode->RemoveObservers(vtkCommand::ProgressEvent,  this->GetMRMLNodesCallbackCommand());

  if (volumeNode != NULL)
    {
    if (volname == NULL)
      {
      const std::string fname(filename);
      std::string baseName = vtksys::SystemTools::GetFilenameName(fname);
      std::string uname = this->GetMRMLScene()->GetUniqueNameByString(baseName.c_str());
      volumeNode->SetName(uname.c_str());
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

    if (this->ColorLogic)
      {
      //displayNode->SetDefaultColorMap();
      if (labelMap) 
        {
        if (this->IsFreeSurferVolume(filename))
          {
          displayNode->SetAndObserveColorNodeID(this->ColorLogic->GetDefaultFreeSurferLabelMapColorNodeID());
          }
        else
          {
          displayNode->SetAndObserveColorNodeID(this->ColorLogic->GetDefaultLabelMapColorNodeID());
          }
        }
      else
        {
        displayNode->SetAndObserveColorNodeID(this->ColorLogic->GetDefaultVolumeColorNodeID());
        }
      }

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
vtkMRMLScalarVolumeNode* vtkSlicerVolumesLogic::AddArchetypeScalarVolume (const char* filename, const char* volname, int loadingOptions)
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
      return NULL;
    }
  else { vtkDebugMacro("LoadArchetypeScalarVolume: filename is a supported type"); }

  storageNode->SetCenterImage(centerImage);
  storageNode->SetSingleFile(singleFile);
  storageNode->SetUseOrientationFromFile(useOrientationFromFile);
  storageNode->AddObserver(vtkCommand::ProgressEvent,  this->GetMRMLNodesCallbackCommand());

  if (volname == NULL)
    {
    const std::string fname(filename);
    std::string basename = vtksys::SystemTools::GetFilenameWithoutExtension(filename);
    std::string uname = this->GetMRMLScene()->GetUniqueNameByString(basename.c_str());
    scalarNode->SetName(uname.c_str());
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

  if (this->ColorLogic)
    {
    if (labelMap) 
      {
      if (this->IsFreeSurferVolume(filename))
        {
        displayNode->SetAndObserveColorNodeID(this->ColorLogic->GetDefaultFreeSurferLabelMapColorNodeID());
        }
      else
        {
        displayNode->SetAndObserveColorNodeID(this->ColorLogic->GetDefaultLabelMapColorNodeID());
        }
      }
    else
      {
      displayNode->SetAndObserveColorNodeID(this->ColorLogic->GetDefaultVolumeColorNodeID());
      }
    }
  
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

  storageNode->RemoveObservers(vtkCommand::ProgressEvent,  this->GetMRMLNodesCallbackCommand());
 
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
// int loadingOptions is bit-coded as following:
// bit 0: label map
// bit 1: centered
// bit 2: loading single file
// bit 3: auto calculate window/level
// bit 4: discard image orientation
// higher bits are reserved for future use
vtkMRMLVolumeNode* vtkSlicerVolumesLogic::AddArchetypeVolume (const char* filename, const char* volname, int loadingOptions, vtkStringArray *fileList)
{
  //bool importingScene = false;
  //if (this->GetMRMLScene() &&
  //    this->GetMRMLScene()->GetIsImporting() == false)
  //  {
  //  importingScene = true;
  this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);
  //  }
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

  vtkSmartPointer<vtkMRMLVolumeNode> volumeNode = NULL;
  vtkSmartPointer<vtkMRMLVolumeDisplayNode> displayNode = NULL;
  vtkSmartPointer<vtkMRMLLabelMapVolumeDisplayNode> lmdisplayNode= NULL;

  vtkSmartPointer<vtkMRMLDiffusionTensorVolumeDisplayNode> dtdisplayNode = NULL;
  vtkSmartPointer<vtkMRMLDiffusionWeightedVolumeDisplayNode> dwdisplayNode = NULL;
  vtkSmartPointer<vtkMRMLVectorVolumeDisplayNode> vdisplayNode = NULL;
  vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode> sdisplayNode = NULL;

  vtkSmartPointer<vtkMRMLScalarVolumeNode> scalarNode = vtkSmartPointer<vtkMRMLScalarVolumeNode>::New();
  vtkSmartPointer<vtkMRMLVectorVolumeNode> vectorNode = vtkSmartPointer<vtkMRMLVectorVolumeNode>::New();
  vtkSmartPointer<vtkMRMLDiffusionTensorVolumeNode> tensorNode = vtkSmartPointer<vtkMRMLDiffusionTensorVolumeNode>::New();
  vtkSmartPointer<vtkMRMLDiffusionWeightedVolumeNode> dwiNode = vtkSmartPointer<vtkMRMLDiffusionWeightedVolumeNode>::New();

  // Instanciation of the two I/O mechanism
  vtkSmartPointer<vtkMRMLNRRDStorageNode> storageNode1 = vtkSmartPointer<vtkMRMLNRRDStorageNode>::New();
  vtkSmartPointer<vtkMRMLVolumeArchetypeStorageNode> storageNode2 = vtkSmartPointer<vtkMRMLVolumeArchetypeStorageNode>::New();
  vtkSmartPointer<vtkMRMLStorageNode> storageNode = NULL;

  // set the volume name
  std::string volumeName;
  if (volname != NULL)
    {
    volumeName = std::string(volname);
    }
  else
    {
    const std::string fname(filename);
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
    if (fileList != NULL)
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
    if (fileList != NULL)
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
  storageNode1->AddObserver(vtkCommand::ProgressEvent,  this->GetMRMLNodesCallbackCommand());

  
  storageNode2->SetCenterImage(centerImage);
  storageNode2->SetSingleFile(singleFile);
  storageNode2->SetUseOrientationFromFile(useOrientationFromFile);
  storageNode2->AddObserver(vtkCommand::ProgressEvent,  this->GetMRMLNodesCallbackCommand());

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
  scalarNode->SetLabelMap(labelMap);
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
      volumeNode = scalarNode;
      storageNode = storageNode2;
      nodeSetUsed = 5;
      }
    }

  storageNode1->RemoveObservers(vtkCommand::ProgressEvent,  this->GetMRMLNodesCallbackCommand());
  storageNode2->RemoveObservers(vtkCommand::ProgressEvent,  this->GetMRMLNodesCallbackCommand());
  
  if (nodeSetUsed != 1)
    {
    dwiNode->SetAndObserveDisplayNodeID(NULL);
    dwiNode->SetAndObserveStorageNodeID(NULL);
    this->GetMRMLScene()->RemoveNode(dwdisplayNode);
    this->GetMRMLScene()->RemoveNode(dwiNode);
    }
  if (nodeSetUsed != 2)
    {
    tensorNode->SetAndObserveDisplayNodeID(NULL);
    tensorNode->SetAndObserveStorageNodeID(NULL);
    this->GetMRMLScene()->RemoveNode(dtdisplayNode);
    this->GetMRMLScene()->RemoveNode(tensorNode);
    }
  if (nodeSetUsed != 3 && nodeSetUsed != 4)
    {
    vectorNode->SetAndObserveDisplayNodeID(NULL);
    vectorNode->SetAndObserveStorageNodeID(NULL);
    this->GetMRMLScene()->RemoveNode(vdisplayNode);
    this->GetMRMLScene()->RemoveNode(vectorNode);
    }
  if (nodeSetUsed != 5)
    {
    scalarNode->SetAndObserveDisplayNodeID(NULL);
    scalarNode->SetAndObserveStorageNodeID(NULL);
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
  
  bool modified = false;
  if (volumeNode != NULL)
    {
    if (this->ColorLogic)
      {
      if (labelMap) 
        {
        if (this->IsFreeSurferVolume(filename))
          {
          displayNode->SetAndObserveColorNodeID(this->ColorLogic->GetDefaultFreeSurferLabelMapColorNodeID());
          }
        else
          {
          displayNode->SetAndObserveColorNodeID(this->ColorLogic->GetDefaultLabelMapColorNodeID());
          }
        }
      else
        {
        displayNode->SetAndObserveColorNodeID(this->ColorLogic->GetDefaultVolumeColorNodeID());
        }
      }
    
    vtkDebugMacro("Name vol node "<<volumeNode->GetClassName());
    vtkDebugMacro("Display node "<<displayNode->GetClassName());
        
    this->SetActiveVolumeNode(volumeNode);

    modified = true;
    // since added the node w/o notification, let the scene know now that it
    // has a new node
    //this->GetMRMLScene()->InvokeEvent(vtkMRMLScene::NodeAddedEvent, volumeNode);
    }
  //if (importingScene)
  //  {
  this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
  //  this->GetMRMLScene()->InvokeEvent(vtkMRMLScene::SceneImportedEvent, NULL);
  //  }
  if (modified)
    {
    this->Modified();
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

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode *vtkSlicerVolumesLogic::CreateLabelVolume (vtkMRMLScene *scene, vtkMRMLVolumeNode *volumeNode, const char *name)
{
  if ( volumeNode == NULL )
    {
    return NULL;
    }

  // create a display node
  vtkMRMLLabelMapVolumeDisplayNode *labelDisplayNode = vtkMRMLLabelMapVolumeDisplayNode::New();

  scene->AddNode(labelDisplayNode);

  // create a volume node as copy of source volume
  vtkMRMLScalarVolumeNode *labelNode = vtkMRMLScalarVolumeNode::New();

  int modifiedSinceRead = volumeNode->GetModifiedSinceRead();
  labelNode->CopyWithScene(volumeNode);
  
  labelNode->SetAndObserveStorageNodeID(NULL);
  labelNode->SetModifiedSinceRead(1);
  labelNode->SetLabelMap(1);

  // associate it with the source volume
  if (volumeNode->GetID())
    {
    labelNode->SetAttribute("AssociatedNodeID", volumeNode->GetID());
    }

  // restore modifiedSinceRead value since copy cause Modify on image data.
  volumeNode->SetModifiedSinceRead(modifiedSinceRead);

  // set the display node to have a label map lookup table
  if (this->ColorLogic)
    {
    labelDisplayNode->SetAndObserveColorNodeID (
      this->ColorLogic->GetDefaultLabelMapColorNodeID());
    }
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
vtkMRMLScalarVolumeNode *vtkSlicerVolumesLogic::FillLabelVolumeFromTemplate (vtkMRMLScene *scene, vtkMRMLScalarVolumeNode *labelNode, vtkMRMLVolumeNode *templateNode)
{
  if ( templateNode == NULL )
    {
    return NULL;
    }

  // create a display node if the label node does not have one
  vtkMRMLLabelMapVolumeDisplayNode *labelDisplayNode = NULL;
  if ( labelNode->GetDisplayNode() == NULL )
    {
    labelDisplayNode = vtkMRMLLabelMapVolumeDisplayNode::New();
    scene->AddNode(labelDisplayNode);
    }

  // we need to copy from the volume node to get required attributes, but
  // the copy copies templateNode's name as well.  So save the original name
  // and re-set the name after the copy.
  std::string origName;
  origName.assign(labelNode->GetName());
  labelNode->Copy(templateNode);
  labelNode->SetName(origName.c_str());

  labelNode->SetModifiedSinceRead(1);
  labelNode->SetLabelMap(1);

  // set the display node to have a label map lookup table
  if ( labelDisplayNode )
    {
    if (this->ColorLogic)
      {
      labelDisplayNode->SetAndObserveColorNodeID(
        this->ColorLogic->GetDefaultLabelMapColorNodeID());
      }
    labelNode->SetAndObserveDisplayNodeID( labelDisplayNode->GetID() );
    }

  // make an image data of the same size and shape as the input volume,
  // but filled with zeros
  vtkImageThreshold *thresh = vtkImageThreshold::New();
  thresh->ReplaceInOn();
  thresh->ReplaceOutOn();
  thresh->SetInValue(0);
  thresh->SetOutValue(0);
  thresh->SetOutputScalarType (VTK_SHORT);
  thresh->SetInput( templateNode->GetImageData() );
  thresh->GetOutput()->Update();
  labelNode->SetAndObserveImageData( thresh->GetOutput() );
  thresh->Delete();

  if ( labelDisplayNode )
    {
    labelDisplayNode->Delete();
    }

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
  clonedVolumeNode->SetAndObserveStorageNodeID(NULL);
  std::string uname = this->GetMRMLScene()->GetUniqueNameByString(name);
  clonedVolumeNode->SetName(uname.c_str());
  clonedVolumeNode->SetAndObserveDisplayNodeID(clonedDisplayNode->GetID());

  // copy over the volume's data
 // Kilian: VTK crashes when volumeNode->GetImageData() = NULL 
  if (volumeNode->GetImageData()) 
    {
      vtkImageData* clonedVolumeData = vtkImageData::New(); 
      clonedVolumeData->DeepCopy(volumeNode->GetImageData());
      clonedVolumeNode->SetAndObserveImageData( clonedVolumeData );
      clonedVolumeData->Delete();
    } else {
      vtkErrorMacro("CloneVolume: The ImageData of VolumeNode with ID " << volumeNode->GetID() << " is null !");
    }

  clonedVolumeNode->SetModifiedSinceRead(1);

  // add the cloned volume to the scene
  scene->AddNode(clonedVolumeNode);

  // remove references
  clonedVolumeNode->Delete();
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

    if (!VNode)
      {
      vtkErrorMacro("ComputeTkRegVox2RASMatrix: input volume node is null");
      return;
      }
    if (!M)
      {
      vtkErrorMacro("ComputeTkRegVox2RASMatrix: input matrix is null");
      return;
      }
    double *spacing = VNode->GetSpacing();
    dC = spacing[0];
    dR = spacing[1];
    dS = spacing[2];

    if (VNode->GetImageData() == NULL)
      {
      vtkErrorMacro("ComputeTkRegVox2RASMatrix: input volume's image data is null");
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
