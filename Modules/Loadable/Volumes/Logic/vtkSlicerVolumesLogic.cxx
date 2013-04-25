/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerVolumesLogic.cxx,v $
  Date:      $Date: 2006/01/06 17:56:48 $
  Version:   $Revision: 1.58 $

=========================================================================auto=*/

// STD includes
#include <algorithm>

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

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkImageData.h>
#include <vtkImageThreshold.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtksys/SystemTools.hxx>


//----------------------------------------------------------------------------
namespace
{

//----------------------------------------------------------------------------
class vtkSlicerErrorSink : public vtkCallbackCommand
{
public:

  vtkTypeMacro(vtkSlicerErrorSink,vtkCallbackCommand);
  static vtkSlicerErrorSink *New() {return new vtkSlicerErrorSink; }
  typedef vtkSlicerErrorSink Self;

  void PrintSelf(ostream& os, vtkIndent indent);

  /// Display errors using vtkOutputWindowDisplayErrorText
  /// \sa vtkOutputWindowDisplayErrorText
  void DisplayErrors();

  /// Return True if errors have been recorded
  bool HasErrors() const;

  /// Clear list of errors
  void Clear();

protected:
  vtkSlicerErrorSink();
  virtual ~vtkSlicerErrorSink(){}

private:
  static void CallbackFunction(vtkObject*, long unsigned int,
                               void* clientData, void* callData);

  std::vector<std::string> ErrorList;

private:
  vtkSlicerErrorSink(const vtkSlicerErrorSink&); // Not implemented
  void operator=(const vtkSlicerErrorSink&);     // Not implemented
};

//----------------------------------------------------------------------------
// vtkSlicerErrorSink methods

//----------------------------------------------------------------------------
vtkSlicerErrorSink::vtkSlicerErrorSink()
{
  this->SetCallback(Self::CallbackFunction);
  this->SetClientData(this);
}

//----------------------------------------------------------------------------
void vtkSlicerErrorSink::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  std::vector<std::string>::iterator it = this->ErrorList.begin();
  os << indent << "ErrorList = \n";
  while(it != this->ErrorList.end())
    {
    os << indent.GetNextIndent() << *it << "\n";
    ++it;
    }
}

//----------------------------------------------------------------------------
void vtkSlicerErrorSink::DisplayErrors()
{
  std::vector<std::string>::iterator it = this->ErrorList.begin();
  while(it != this->ErrorList.end())
    {
    vtkOutputWindowDisplayErrorText((*it).c_str());
    ++it;
    }
}

//----------------------------------------------------------------------------
bool vtkSlicerErrorSink::HasErrors() const
{
  return this->ErrorList.size() > 0;
}

//----------------------------------------------------------------------------
void vtkSlicerErrorSink::Clear()
{
  this->ErrorList.clear();
}

//----------------------------------------------------------------------------
void vtkSlicerErrorSink::CallbackFunction(vtkObject* vtkNotUsed(caller),
                                          long unsigned int vtkNotUsed(eventId),
                                          void* clientData, void* callData)
{
  vtkSlicerErrorSink * self = reinterpret_cast<vtkSlicerErrorSink*>(clientData);
  char * message = reinterpret_cast<char*>(callData);
  self->ErrorList.push_back(message);
}

} // end of anonymous namespace

//----------------------------------------------------------------------------
// vtkSlicerVolumesLogic methods

//----------------------------------------------------------------------------
vtkCxxRevisionMacro(vtkSlicerVolumesLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkSlicerVolumesLogic);

//----------------------------------------------------------------------------
namespace
{

//----------------------------------------------------------------------------
ArchetypeVolumeNodeSet DiffusionWeightedVolumeNodeSetFactory(std::string& volumeName, vtkMRMLScene* scene, int options)
{
  ArchetypeVolumeNodeSet nodeSet(scene);

  // set up the dwi node's support nodes
  vtkNew<vtkMRMLDiffusionWeightedVolumeDisplayNode> dwdisplayNode;
  nodeSet.Scene->AddNode(dwdisplayNode.GetPointer());

  vtkNew<vtkMRMLDiffusionWeightedVolumeNode> dwiNode;
  dwiNode->SetName(volumeName.c_str());
  nodeSet.Scene->AddNode(dwiNode.GetPointer());
  dwiNode->SetAndObserveDisplayNodeID(dwdisplayNode->GetID());

  vtkNew<vtkMRMLNRRDStorageNode> storageNode;
  storageNode->SetCenterImage(options & vtkSlicerVolumesLogic::CenterImage);
  nodeSet.Scene->AddNode(storageNode.GetPointer());
  dwiNode->SetAndObserveStorageNodeID(storageNode->GetID());

  nodeSet.StorageNode = storageNode.GetPointer();
  nodeSet.DisplayNode = dwdisplayNode.GetPointer();
  nodeSet.Node = dwiNode.GetPointer();

  return nodeSet;
}

//----------------------------------------------------------------------------
ArchetypeVolumeNodeSet DiffusionTensorVolumeNodeSetFactory(std::string& volumeName, vtkMRMLScene* scene, int options)
{
  ArchetypeVolumeNodeSet nodeSet(scene);

  // set up the tensor node's support nodes
  vtkNew<vtkMRMLDiffusionTensorVolumeDisplayNode> dtdisplayNode;
  // jvm - are these the default settings anyway?
  dtdisplayNode->SetWindow(0);
  dtdisplayNode->SetLevel(0);
  dtdisplayNode->SetUpperThreshold(0);
  dtdisplayNode->SetLowerThreshold(0);
  dtdisplayNode->SetAutoWindowLevel(1);
  nodeSet.Scene->AddNode(dtdisplayNode.GetPointer());

  vtkNew<vtkMRMLDiffusionTensorVolumeNode> tensorNode;
  tensorNode->SetName(volumeName.c_str());
  nodeSet.Scene->AddNode(tensorNode.GetPointer());
  tensorNode->SetAndObserveDisplayNodeID(dtdisplayNode->GetID());

  vtkNew<vtkMRMLVolumeArchetypeStorageNode> storageNode;
  storageNode->SetCenterImage(options & vtkSlicerVolumesLogic::CenterImage);
  storageNode->SetUseOrientationFromFile(!((options & vtkSlicerVolumesLogic::DiscardOrientation) != 0));
  storageNode->SetSingleFile(options & vtkSlicerVolumesLogic::SingleFile);
  nodeSet.Scene->AddNode(storageNode.GetPointer());
  tensorNode->SetAndObserveStorageNodeID(storageNode->GetID());

  nodeSet.StorageNode = storageNode.GetPointer();
  nodeSet.DisplayNode = dtdisplayNode.GetPointer();
  nodeSet.Node = tensorNode.GetPointer();

  return nodeSet;
}

//----------------------------------------------------------------------------
ArchetypeVolumeNodeSet NRRDVectorVolumeNodeSetFactory(std::string& volumeName, vtkMRMLScene* scene, int options)
{
  ArchetypeVolumeNodeSet nodeSet(scene);

  // set up the vector node's support nodes
  vtkNew<vtkMRMLVectorVolumeDisplayNode> vdisplayNode;
  nodeSet.Scene->AddNode(vdisplayNode.GetPointer());

  vtkNew<vtkMRMLVectorVolumeNode> vectorNode;
  vectorNode->SetName(volumeName.c_str());
  nodeSet.Scene->AddNode(vectorNode.GetPointer());
  vectorNode->SetAndObserveDisplayNodeID(vdisplayNode->GetID());

  vtkNew<vtkMRMLNRRDStorageNode> storageNode;
  storageNode->SetCenterImage(options & vtkSlicerVolumesLogic::CenterImage);
  nodeSet.Scene->AddNode(storageNode.GetPointer());
  vectorNode->SetAndObserveStorageNodeID(storageNode->GetID());

  nodeSet.StorageNode = storageNode.GetPointer();
  nodeSet.DisplayNode = vdisplayNode.GetPointer();
  nodeSet.Node = vectorNode.GetPointer();

  return nodeSet;
}

//----------------------------------------------------------------------------
ArchetypeVolumeNodeSet ArchetypeVectorVolumeNodeSetFactory(std::string& volumeName, vtkMRMLScene* scene, int options)
{
  ArchetypeVolumeNodeSet nodeSet(scene);

  // set up the vector node's support nodes
  vtkNew<vtkMRMLVectorVolumeDisplayNode> vdisplayNode;
  nodeSet.Scene->AddNode(vdisplayNode.GetPointer());

  vtkNew<vtkMRMLVectorVolumeNode> vectorNode;
  vectorNode->SetName(volumeName.c_str());
  nodeSet.Scene->AddNode(vectorNode.GetPointer());
  vectorNode->SetAndObserveDisplayNodeID(vdisplayNode->GetID());

  vtkNew<vtkMRMLVolumeArchetypeStorageNode> storageNode;
  storageNode->SetCenterImage(options & vtkSlicerVolumesLogic::CenterImage);
  storageNode->SetUseOrientationFromFile(!((options & vtkSlicerVolumesLogic::DiscardOrientation) != 0));
  storageNode->SetSingleFile(options & vtkSlicerVolumesLogic::SingleFile);
  nodeSet.Scene->AddNode(storageNode.GetPointer());
  vectorNode->SetAndObserveStorageNodeID(storageNode->GetID());

  nodeSet.StorageNode = storageNode.GetPointer();
  nodeSet.DisplayNode = vdisplayNode.GetPointer();
  nodeSet.Node = vectorNode.GetPointer();

  return nodeSet;
}

//----------------------------------------------------------------------------
ArchetypeVolumeNodeSet LabelMapVolumeNodeSetFactory(std::string& volumeName, vtkMRMLScene* scene, int options)
{
  ArchetypeVolumeNodeSet nodeSet(scene);

  // set up the scalar node's support nodes
  vtkNew<vtkMRMLScalarVolumeNode> scalarNode;
  scalarNode->SetName(volumeName.c_str());
  scalarNode->SetLabelMap(1);
  nodeSet.Scene->AddNode(scalarNode.GetPointer());

  vtkNew<vtkMRMLLabelMapVolumeDisplayNode> lmdisplayNode;
  nodeSet.Scene->AddNode(lmdisplayNode.GetPointer());
  scalarNode->SetAndObserveDisplayNodeID(lmdisplayNode->GetID());

  vtkNew<vtkMRMLVolumeArchetypeStorageNode> storageNode;
  storageNode->SetCenterImage(options & vtkSlicerVolumesLogic::CenterImage);
  storageNode->SetUseOrientationFromFile(!((options & vtkSlicerVolumesLogic::DiscardOrientation) != 0));
  storageNode->SetSingleFile(options & vtkSlicerVolumesLogic::SingleFile);
  nodeSet.Scene->AddNode(storageNode.GetPointer());
  scalarNode->SetAndObserveStorageNodeID(storageNode->GetID());

  nodeSet.StorageNode = storageNode.GetPointer();
  nodeSet.DisplayNode = lmdisplayNode.GetPointer();
  nodeSet.Node = scalarNode.GetPointer();

  nodeSet.LabelMap = true;

  return nodeSet;
}

//----------------------------------------------------------------------------
ArchetypeVolumeNodeSet ScalarVolumeNodeSetFactory(std::string& volumeName, vtkMRMLScene* scene, int options)
{
  ArchetypeVolumeNodeSet nodeSet(scene);

  // set up the scalar node's support nodes
  vtkNew<vtkMRMLScalarVolumeNode> scalarNode;
  scalarNode->SetName(volumeName.c_str());
  scalarNode->SetLabelMap(0);
  nodeSet.Scene->AddNode(scalarNode.GetPointer());

  vtkNew<vtkMRMLScalarVolumeDisplayNode> sdisplayNode;
  nodeSet.Scene->AddNode(sdisplayNode.GetPointer());
  scalarNode->SetAndObserveDisplayNodeID(sdisplayNode->GetID());

  vtkNew<vtkMRMLVolumeArchetypeStorageNode> storageNode;
  storageNode->SetCenterImage(options & vtkSlicerVolumesLogic::CenterImage);
  storageNode->SetUseOrientationFromFile(!((options & vtkSlicerVolumesLogic::DiscardOrientation) != 0));
  storageNode->SetSingleFile(options & vtkSlicerVolumesLogic::SingleFile);
  nodeSet.Scene->AddNode(storageNode.GetPointer());
  scalarNode->SetAndObserveStorageNodeID(storageNode->GetID());

  nodeSet.StorageNode = storageNode.GetPointer();
  nodeSet.DisplayNode = sdisplayNode.GetPointer();
  nodeSet.Node = scalarNode.GetPointer();

  return nodeSet;
}

} // end of anonymous namespace

//----------------------------------------------------------------------------
vtkSlicerVolumesLogic::vtkSlicerVolumesLogic()
{
  // register the default factories for nodesets. this is done in a specific order
  this->RegisterArchetypeVolumeNodeSetFactory( DiffusionWeightedVolumeNodeSetFactory );
  this->RegisterArchetypeVolumeNodeSetFactory( DiffusionTensorVolumeNodeSetFactory );
  this->RegisterArchetypeVolumeNodeSetFactory( NRRDVectorVolumeNodeSetFactory );
  this->RegisterArchetypeVolumeNodeSetFactory( ArchetypeVectorVolumeNodeSetFactory );
  this->RegisterArchetypeVolumeNodeSetFactory( LabelMapVolumeNodeSetFactory );
  this->RegisterArchetypeVolumeNodeSetFactory( ScalarVolumeNodeSetFactory );
}

//----------------------------------------------------------------------------
vtkSlicerVolumesLogic::~vtkSlicerVolumesLogic()
{
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
void vtkSlicerVolumesLogic::SetColorLogic(vtkMRMLColorLogic *colorLogic)
{
  if (this->ColorLogic == colorLogic)
    {
    return;
    }
  this->ColorLogic = colorLogic;
  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLColorLogic* vtkSlicerVolumesLogic::GetColorLogic()const
{
  return this->ColorLogic;
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::SetActiveVolumeNode(vtkMRMLVolumeNode *activeNode)
{
  vtkSetMRMLNodeMacro(this->ActiveVolumeNode, activeNode);
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerVolumesLogic::GetActiveVolumeNode()const
{
  return this->ActiveVolumeNode;
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic
::SetAndObserveColorToDisplayNode(vtkMRMLDisplayNode * displayNode,
                                  int labelMap, const char* filename)
{
  vtkMRMLColorLogic * colorLogic = this->GetColorLogic();
  if (colorLogic == NULL)
    {
    return;
    }
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
}

//----------------------------------------------------------------------------
void vtkSlicerVolumesLogic::InitializeStorageNode(
    vtkMRMLStorageNode * storageNode, const char * filename, vtkStringArray *fileList)
{
  bool useURI = false;
  if (this->GetMRMLScene() && this->GetMRMLScene()->GetCacheManager())
    {
    useURI = this->GetMRMLScene()->GetCacheManager()->IsRemoteReference(filename);
    }
  if (useURI)
    {
    vtkDebugMacro("AddArchetypeVolume: input filename '" << filename << "' is a URI");
    // need to set the scene on the storage node so that it can look for file handlers
    storageNode->SetURI(filename);
    storageNode->SetScene(this->GetMRMLScene());
    if (fileList != NULL)
      {
      // it's a list of uris
      int numURIs = fileList->GetNumberOfValues();
      vtkDebugMacro("Have a list of " << numURIs << " uris that go along with the archetype");
      vtkStdString thisURI;
      storageNode->ResetURIList();
      for (int n = 0; n < numURIs; n++)
        {
        thisURI = fileList->GetValue(n);
        storageNode->AddURI(thisURI);
        }
      }
    }
  else
    {
    storageNode->SetFileName(filename);
    if (fileList != NULL)
      {
      int numFiles = fileList->GetNumberOfValues();
      vtkDebugMacro("Have a list of " << numFiles << " files that go along with the archetype");
      vtkStdString thisFileName;
      storageNode->ResetFileNameList();
      for (int n = 0; n < numFiles; n++)
        {
        thisFileName = fileList->GetValue(n);
        //vtkDebugMacro("\tfile " << n << " =  " << thisFileName);
        storageNode->AddFileName(thisFileName);
        }
      }
    }
  storageNode->AddObserver(vtkCommand::ProgressEvent,  this->GetMRMLNodesCallbackCommand());
}

//----------------------------------------------------------------------------
vtkMRMLVolumeNode* vtkSlicerVolumesLogic::AddArchetypeVolume(
    const char* filename, const char* volname,
    int loadingOptions, vtkStringArray *fileList)
{
  return this->AddArchetypeVolume(this->VolumeRegistry, filename, volname, loadingOptions, fileList);
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkSlicerVolumesLogic::AddArchetypeScalarVolume(
    const char* filename, const char* volname, int loadingOptions, vtkStringArray *fileList)
{
  NodeSetFactoryRegistry nodeSetFactoryRegistry;
  nodeSetFactoryRegistry.push_back(&ScalarVolumeNodeSetFactory);
  return vtkMRMLScalarVolumeNode::SafeDownCast(this->AddArchetypeVolume(nodeSetFactoryRegistry, filename, volname, loadingOptions, fileList));
}

//----------------------------------------------------------------------------
// int loadingOptions is bit-coded as following:
// bit 0: label map
// bit 1: centered
// bit 2: loading single file
// bit 3: auto calculate window/level
// bit 4: discard image orientation
// higher bits are reserved for future use
vtkMRMLVolumeNode* vtkSlicerVolumesLogic::AddArchetypeVolume (
    const NodeSetFactoryRegistry& volumeRegistry,
    const char* filename, const char* volname, int loadingOptions,
    vtkStringArray *fileList)
{
  this->GetMRMLScene()->StartState(vtkMRMLScene::BatchProcessState);

  bool labelMap = false;
  if ( loadingOptions & 1 )    // labelMap is true
    {
    labelMap = true;
    }

  this->GetMRMLScene()->SaveStateForUndo();

  vtkSmartPointer<vtkMRMLVolumeNode> volumeNode;
  vtkSmartPointer<vtkMRMLVolumeDisplayNode> displayNode;
  vtkSmartPointer<vtkMRMLStorageNode> storageNode;

  // Compute volume name
  std::string volumeName = volname != NULL ? volname : vtksys::SystemTools::GetFilenameName(filename);
  volumeName = this->GetMRMLScene()->GetUniqueNameByString(volumeName.c_str());

  vtkNew<vtkSlicerErrorSink> errorSink;

  // Run through the factory list and test each factory until success
  for (NodeSetFactoryRegistry::const_iterator fit = volumeRegistry.begin();
       fit != volumeRegistry.end(); ++fit)
    {
    ArchetypeVolumeNodeSet nodeSet( (*fit)(volumeName, this->GetMRMLScene(), loadingOptions) );

    nodeSet.StorageNode->AddObserver(vtkCommand::ErrorEvent, errorSink.GetPointer());
    nodeSet.StorageNode->AddObserver(vtkCommand::ProgressEvent,  this->GetMRMLNodesCallbackCommand());

    // if the labelMap flags for reader and factory are consistent
    // (both true or both false)
    if (labelMap == nodeSet.LabelMap)
      {
      this->InitializeStorageNode(nodeSet.StorageNode, filename, fileList);

      vtkDebugMacro("Attempt to read file as a volume of type "
                    << nodeSet.Node->GetNodeTagName() << " using "
                    << nodeSet.Node->GetClassName() << " [filename = " << filename << "]");
      if (nodeSet.StorageNode->ReadData(nodeSet.Node))
        {
        displayNode = nodeSet.DisplayNode;
        volumeNode =  nodeSet.Node;
        storageNode = nodeSet.StorageNode;
        vtkDebugMacro(<< "File successfully read as " << nodeSet.Node->GetNodeTagName()
                      << " [filename = " << filename << "]");
        break;
        }
      }

    // 
    // Wasn't the right factory, so we need to clean up
    //
    
    // disconnect the observers
    nodeSet.StorageNode->RemoveObservers(vtkCommand::ErrorEvent, errorSink.GetPointer());
    nodeSet.StorageNode->RemoveObservers(vtkCommand::ProgressEvent,  this->GetMRMLNodesCallbackCommand());

    // clean up the scene
    nodeSet.Node->SetAndObserveDisplayNodeID(NULL);
    nodeSet.Node->SetAndObserveStorageNodeID(NULL);
    this->GetMRMLScene()->RemoveNode(nodeSet.DisplayNode);
    this->GetMRMLScene()->RemoveNode(nodeSet.StorageNode);
    this->GetMRMLScene()->RemoveNode(nodeSet.Node);
    }

  // display any errors
  if (volumeNode == 0)
    {
    errorSink->DisplayErrors();
    }
    
  
  bool modified = false;
  if (volumeNode != NULL)
    {
    this->SetAndObserveColorToDisplayNode(displayNode, labelMap, filename);
    
    vtkDebugMacro("Name vol node "<<volumeNode->GetClassName());
    vtkDebugMacro("Display node "<<displayNode->GetClassName());
        
    this->SetActiveVolumeNode(volumeNode);

    modified = true;
    // since added the node w/o notification, let the scene know now that it
    // has a new node
    //this->GetMRMLScene()->InvokeEvent(vtkMRMLScene::NodeAddedEvent, volumeNode);
    }

  this->GetMRMLScene()->EndState(vtkMRMLScene::BatchProcessState);
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
//      volumeNode->IsA("vtkMRMLDiffusionTensorVolumeNode") ||
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
void vtkSlicerVolumesLogic
::SetVolumeAsLabelMap(vtkMRMLVolumeNode *volumeNode, bool labelMap)
{
  vtkMRMLScalarVolumeNode *scalarNode =
    vtkMRMLScalarVolumeNode::SafeDownCast(volumeNode);
  if (scalarNode == 0 ||
      scalarNode->IsA("vtkMRMLTensorVolumeNode") ||
      static_cast<bool>(scalarNode->GetLabelMap()) == labelMap)
    {
    return;
    }
  vtkMRMLDisplayNode *oldDisplayNode = scalarNode->GetDisplayNode();
  if (oldDisplayNode)
    {
    scalarNode->GetScene()->RemoveNode(oldDisplayNode);
    }
  vtkMRMLVolumeDisplayNode* displayNode = 0;
  if (labelMap)
    {
    displayNode = vtkMRMLLabelMapVolumeDisplayNode::New();
    }
  else
    {
    displayNode = vtkMRMLScalarVolumeDisplayNode::New();
    }
  displayNode->SetAndObserveColorNodeID (
    labelMap ? "vtkMRMLColorTableNodeLabels" : "vtkMRMLColorTableNodeGrey");
  scalarNode->GetScene()->AddNode(displayNode);
  scalarNode->SetAndObserveDisplayNodeID( displayNode->GetID() );
  scalarNode->SetLabelMap( labelMap );
  displayNode->Delete();
}


//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkSlicerVolumesLogic
::CreateAndAddLabelVolume(vtkMRMLVolumeNode *volumeNode, const char *name)
{
  return this->CreateAndAddLabelVolume(this->GetMRMLScene(), volumeNode, name);
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode *
vtkSlicerVolumesLogic::CreateAndAddLabelVolume(vtkMRMLScene *scene,
                                               vtkMRMLVolumeNode *volumeNode,
                                               const char *name)
{
  if ( scene == NULL || volumeNode == NULL || name == NULL)
    {
    return NULL;
    }

  // create a display node
  vtkNew<vtkMRMLLabelMapVolumeDisplayNode> labelDisplayNode;
  scene->AddNode(labelDisplayNode.GetPointer());

  // create a volume node as copy of source volume
  vtkNew<vtkMRMLScalarVolumeNode> labelNode;
  labelNode->CopyWithScene(volumeNode);
  labelNode->RemoveAllDisplayNodeIDs();
  labelNode->SetAndObserveStorageNodeID(NULL);
  labelNode->SetLabelMap(1);

  // associate it with the source volume
  if (volumeNode->GetID())
    {
    labelNode->SetAttribute("AssociatedNodeID", volumeNode->GetID());
    }

  // set the display node to have a label map lookup table
  this->SetAndObserveColorToDisplayNode(labelDisplayNode.GetPointer(),
                                        /* labelMap= */ 1,
                                        /* filename= */ 0);

  std::string uname = this->GetMRMLScene()->GetUniqueNameByString(name);

  labelNode->SetName(uname.c_str());
  labelNode->SetAndObserveDisplayNodeID( labelDisplayNode->GetID() );

  // make an image data of the same size and shape as the input volume,
  // but filled with zeros
  vtkNew<vtkImageThreshold> thresh;
  thresh->ReplaceInOn();
  thresh->ReplaceOutOn();
  thresh->SetInValue(0);
  thresh->SetOutValue(0);
  thresh->SetOutputScalarType (VTK_SHORT);
  thresh->SetInput( volumeNode->GetImageData() );
  thresh->GetOutput()->Update();

  vtkNew<vtkImageData> imageData;
  imageData->DeepCopy( thresh->GetOutput() );
  labelNode->SetAndObserveImageData( imageData.GetPointer() );

  // add the label volume to the scene
  scene->AddNode(labelNode.GetPointer());

  return labelNode.GetPointer();
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkSlicerVolumesLogic
::CreateLabelVolume(vtkMRMLVolumeNode *volumeNode,
                    const char *name)
{
  vtkWarningMacro("Deprecated, please use CreateAndAddLabelVolume instead");
  return this->CreateAndAddLabelVolume(volumeNode, name);
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode* vtkSlicerVolumesLogic
::CreateLabelVolume(vtkMRMLScene* scene,
                    vtkMRMLVolumeNode *volumeNode,
                    const char *name)
{
  vtkWarningMacro("Deprecated, please use CreateAndAddLabelVolume instead");
  return this->CreateAndAddLabelVolume(scene, volumeNode, name);
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode *
vtkSlicerVolumesLogic::FillLabelVolumeFromTemplate(vtkMRMLScalarVolumeNode *labelNode,
                                                   vtkMRMLVolumeNode *templateNode)
{
  return Self::FillLabelVolumeFromTemplate(this->GetMRMLScene(), labelNode, templateNode);
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode *
vtkSlicerVolumesLogic::FillLabelVolumeFromTemplate(vtkMRMLScene *scene,
                                                   vtkMRMLScalarVolumeNode *labelNode,
                                                   vtkMRMLVolumeNode *templateNode)
{
  if (scene == NULL || labelNode == NULL || templateNode == NULL)
    {
    return NULL;
    }

  // Create a display node if the label node does not have one
  vtkSmartPointer<vtkMRMLLabelMapVolumeDisplayNode> labelDisplayNode =
      vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(labelNode->GetDisplayNode());
  if ( labelDisplayNode.GetPointer() == NULL )
    {
    labelDisplayNode = vtkSmartPointer<vtkMRMLLabelMapVolumeDisplayNode>::New();
    scene->AddNode(labelDisplayNode);
    }

  // We need to copy from the volume node to get required attributes, but
  // the copy copies templateNode's name as well.  So save the original name
  // and re-set the name after the copy.
  std::string origName(labelNode->GetName());
  labelNode->Copy(templateNode);
  labelNode->SetName(origName.c_str());
  labelNode->SetLabelMap(1);

  // Set the display node to have a label map lookup table
  this->SetAndObserveColorToDisplayNode(labelDisplayNode,
                                        /* labelMap = */ 1, /* filename= */ 0);

  // Make an image data of the same size and shape as the input volume, but filled with zeros
  vtkNew<vtkImageThreshold> thresh;
  thresh->ReplaceInOn();
  thresh->ReplaceOutOn();
  thresh->SetInValue(0);
  thresh->SetOutValue(0);
  thresh->SetOutputScalarType (VTK_SHORT);
  thresh->SetInput( templateNode->GetImageData() );
  thresh->GetOutput()->Update();
  labelNode->SetAndObserveImageData( thresh->GetOutput() );

  return labelNode;
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode*
vtkSlicerVolumesLogic::CloneVolume(vtkMRMLVolumeNode *volumeNode, const char *name)
{
  return Self::CloneVolume(this->GetMRMLScene(), volumeNode, name);
}

//----------------------------------------------------------------------------
vtkMRMLScalarVolumeNode*
vtkSlicerVolumesLogic::
CloneVolume (vtkMRMLScene *scene, vtkMRMLVolumeNode *volumeNode, const char *name)
{
  if ( scene == NULL || volumeNode == NULL ) 
    {
    return NULL;
    }

  // clone the display node
  vtkSmartPointer<vtkMRMLDisplayNode> clonedDisplayNode;
  vtkMRMLLabelMapVolumeDisplayNode *labelDisplayNode = vtkMRMLLabelMapVolumeDisplayNode::SafeDownCast(volumeNode->GetDisplayNode());
  if ( labelDisplayNode )
    {
    clonedDisplayNode = vtkSmartPointer<vtkMRMLLabelMapVolumeDisplayNode>::New();
    }
  else
    {
    clonedDisplayNode = vtkSmartPointer<vtkMRMLScalarVolumeDisplayNode>::New();
    }
  clonedDisplayNode->CopyWithScene(volumeNode->GetDisplayNode());
  scene->AddNode(clonedDisplayNode);

  // clone the volume node
  vtkNew<vtkMRMLScalarVolumeNode> clonedVolumeNode;
  clonedVolumeNode->CopyWithScene(volumeNode);
  clonedVolumeNode->SetAndObserveStorageNodeID(NULL);
  std::string uname = scene->GetUniqueNameByString(name);
  clonedVolumeNode->SetName(uname.c_str());
  clonedVolumeNode->SetAndObserveDisplayNodeID(clonedDisplayNode->GetID());

  // copy over the volume's data
 // Kilian: VTK crashes when volumeNode->GetImageData() = NULL 
  if (volumeNode->GetImageData()) 
    {
    vtkNew<vtkImageData> clonedVolumeData;
    clonedVolumeData->DeepCopy(volumeNode->GetImageData());
    clonedVolumeNode->SetAndObserveImageData( clonedVolumeData.GetPointer() );
    }
  else
    {
    vtkErrorWithObjectMacro(scene, "CloneVolume: The ImageData of VolumeNode with ID "
                            << volumeNode->GetID() << " is null !");
    }

  // add the cloned volume to the scene
  scene->AddNode(clonedVolumeNode.GetPointer());

  return clonedVolumeNode.GetPointer();
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
  if (filename == NULL)
    {
    return 0;
    }
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
  
    vtkSmartPointer<vtkMatrix4x4> T = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkSmartPointer<vtkMatrix4x4> S = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkSmartPointer<vtkMatrix4x4> Sinv = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkSmartPointer<vtkMatrix4x4> M = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkSmartPointer<vtkMatrix4x4> Minv = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkSmartPointer<vtkMatrix4x4> N = vtkSmartPointer<vtkMatrix4x4>::New();
    vtkSmartPointer<vtkMatrix4x4> Ninv = vtkSmartPointer<vtkMatrix4x4>::New();

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
    }
}


// Add a class to the list of registry of volume types.
// The default storage nodes for these volume types will be tested in
// order of front to back.
void
vtkSlicerVolumesLogic
::RegisterArchetypeVolumeNodeSetFactory(ArchetypeVolumeNodeSetFactory factory)
{
  NodeSetFactoryRegistry::iterator 
    rit = std::find(this->VolumeRegistry.begin(), this->VolumeRegistry.end(), factory);

  if (rit == this->VolumeRegistry.end())
    {
    this->VolumeRegistry.push_back(factory);
    }
}


void
vtkSlicerVolumesLogic
::PreRegisterArchetypeVolumeNodeSetFactory(ArchetypeVolumeNodeSetFactory factory)
{
  NodeSetFactoryRegistry::iterator 
    rit = std::find(this->VolumeRegistry.begin(), this->VolumeRegistry.end(), factory);

  if (rit == this->VolumeRegistry.end())
    {
    this->VolumeRegistry.push_front(factory);
    }
  else
    {
    this->VolumeRegistry.erase(rit);
    this->VolumeRegistry.push_front(factory);
    }
}
