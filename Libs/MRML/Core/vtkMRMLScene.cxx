/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLScene.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.18 $

=========================================================================auto=*/


#include "vtkMRMLScene.h"
#include "vtkMRMLParser.h"

#include "vtkCacheManager.h"
#include "vtkDataIOManager.h"
#include "vtkTagTable.h"

#include "vtkMRMLBSplineTransformNode.h"
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLChartNode.h"
#include "vtkMRMLChartViewNode.h"
#include "vtkMRMLClipModelsNode.h"
#include "vtkMRMLColorTableStorageNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLFiducialListStorageNode.h"
#include "vtkMRMLFreeSurferModelOverlayStorageNode.h"
#include "vtkMRMLFreeSurferModelStorageNode.h"
#include "vtkMRMLGridTransformNode.h"
#include "vtkMRMLHierarchyStorageNode.h"
#include "vtkMRMLLabelMapVolumeDisplayNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLROINode.h"
#include "vtkMRMLROIListNode.h"
#include "vtkMRMLScriptedModuleNode.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLSnapshotClipNode.h"
#include "vtkMRMLTransformStorageNode.h"
#include "vtkMRMLUnstructuredGridDisplayNode.h"
#include "vtkMRMLUnstructuredGridNode.h"
#include "vtkMRMLUnstructuredGridStorageNode.h"
#include "vtkMRMLVectorVolumeDisplayNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkURIHandler.h"
#include "vtkMRMLLayoutNode.h"

#include "vtkMRMLDoubleArrayNode.h"
#include "vtkMRMLDoubleArrayStorageNode.h"
#include "vtkMRMLCrosshairNode.h"
#include "vtkMRMLInteractionNode.h"

#ifdef MRML_USE_vtkTeem
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#endif

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkCollection.h>
#include <vtkDebugLeaks.h>
#include <vtkErrorCode.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// STD includes
#include <algorithm>
#include <cassert>
#include <numeric>

//#define MRMLSCENE_VERBOSE 1

vtkCxxSetObjectMacro(vtkMRMLScene, CacheManager, vtkCacheManager)
vtkCxxSetObjectMacro(vtkMRMLScene, DataIOManager, vtkDataIOManager)
vtkCxxSetObjectMacro(vtkMRMLScene, UserTagTable, vtkTagTable)
vtkCxxSetObjectMacro(vtkMRMLScene, URIHandlerCollection, vtkCollection)

//------------------------------------------------------------------------------
vtkMRMLScene::vtkMRMLScene()
{
  this->NodeIDsMTime = 0;
  this->SceneModifiedTime = 0;

  this->ClassNameList = NULL;
  this->RegisteredNodeClasses.clear();
  this->UniqueIDs.clear();
  this->UniqueNames.clear();

  this->Nodes =  vtkCollection::New();
  this->UndoStackSize = 100;
  this->UndoFlag = false;
  this->InUndo = false;

  this->ReferencedIDs.clear() ;
  this->ReferencingNodes.clear();
  this->ReferencedIDChanges.clear();

  this->CacheManager = NULL;
  this->DataIOManager = NULL;
  this->URIHandlerCollection = NULL;
  this->UserTagTable = NULL;

  this->ErrorCode = 0;

  this->LoadFromXMLString = 0;

  this->SaveToXMLString = 0;

  this->ReadDataOnLoad = 1;

  this->LastLoadedVersion = NULL;
  this->Version = NULL;

  this->DeleteEventCallback = vtkCallbackCommand::New();
  this->DeleteEventCallback->SetClientData( reinterpret_cast<void *>(this) );
  this->DeleteEventCallback->SetCallback( vtkMRMLScene::SceneCallback );
  // we want to be first to catch the event, so that SceneAboutToBeClosedEvent,
  // NodeRemovedEvent and SceneCloseddEvent are fired and caught before DeleteEvent
  // is caught by other observers.
  this->AddObserver(vtkCommand::DeleteEvent, this->DeleteEventCallback, 1000.);

  //
  // Register all the 'built-in' nodes for the library
  // - note: the scene will maintain a registered pointer to the nodes,
  //   so we delete them here (same should be done by any module that
  //   creates nodes).
  //
  vtkMRMLScalarVolumeNode *volumenode = vtkMRMLScalarVolumeNode::New();
  this->RegisterNodeClass( volumenode );
  volumenode->Delete();

  vtkMRMLModelNode *modelnode = vtkMRMLModelNode::New();
  this->RegisterNodeClass( modelnode );
  modelnode->Delete();

  vtkMRMLLinearTransformNode *linxnode = vtkMRMLLinearTransformNode::New();
  this->RegisterNodeClass( linxnode );
  linxnode->Delete();

  vtkMRMLModelStorageNode *modelstorenode = vtkMRMLModelStorageNode::New();
  this->RegisterNodeClass( modelstorenode );
  modelstorenode->Delete();

  vtkMRMLFreeSurferModelStorageNode *surfermodelstorenode = vtkMRMLFreeSurferModelStorageNode::New();
  this->RegisterNodeClass( surfermodelstorenode );
  surfermodelstorenode->Delete();

  vtkMRMLFreeSurferModelOverlayStorageNode *surfermodeloverlaystorenode  = vtkMRMLFreeSurferModelOverlayStorageNode::New();
  this->RegisterNodeClass ( surfermodeloverlaystorenode );
  surfermodeloverlaystorenode->Delete();

  vtkMRMLModelDisplayNode *modeldisplaynode = vtkMRMLModelDisplayNode::New();
  this->RegisterNodeClass( modeldisplaynode );
  modeldisplaynode->Delete();

  vtkMRMLClipModelsNode *modelclipnode = vtkMRMLClipModelsNode::New();
  this->RegisterNodeClass( modelclipnode );
  modelclipnode->Delete();

  vtkMRMLFiducialListNode *fidln = vtkMRMLFiducialListNode::New();
  this->RegisterNodeClass( fidln );
  fidln->Delete();

  vtkMRMLFiducialListStorageNode *fidlsn = vtkMRMLFiducialListStorageNode::New();
  this->RegisterNodeClass(fidlsn);
  fidlsn->Delete();

  vtkMRMLROINode *roin = vtkMRMLROINode::New();
  this->RegisterNodeClass( roin );
  roin->Delete();

  vtkMRMLROIListNode *roiln = vtkMRMLROIListNode::New();
  this->RegisterNodeClass( roiln );
  roiln->Delete();

  vtkMRMLSliceCompositeNode *scompn = vtkMRMLSliceCompositeNode::New();
  this->RegisterNodeClass( scompn );
  scompn->Delete();

  vtkMRMLScriptedModuleNode *scriptedNode = vtkMRMLScriptedModuleNode::New();
  this->RegisterNodeClass( scriptedNode );
  scriptedNode->Delete();

  vtkMRMLSelectionNode *sseln = vtkMRMLSelectionNode::New();
  this->RegisterNodeClass( sseln );
  sseln->Delete();

  vtkMRMLSliceNode *snode = vtkMRMLSliceNode::New();
  this->RegisterNodeClass( snode );
  snode->Delete();

  vtkMRMLVolumeArchetypeStorageNode *astoren = vtkMRMLVolumeArchetypeStorageNode::New();
  this->RegisterNodeClass( astoren );
  astoren->Delete();

  vtkMRMLScalarVolumeDisplayNode *vdisn = vtkMRMLScalarVolumeDisplayNode::New();
  this->RegisterNodeClass( vdisn );
  vdisn->Delete();

  vtkMRMLLabelMapVolumeDisplayNode *lmdisn = vtkMRMLLabelMapVolumeDisplayNode::New();
  this->RegisterNodeClass( lmdisn );
  lmdisn->Delete();

  vtkMRMLColorNode *vcn = vtkMRMLColorNode::New();
  this->RegisterNodeClass ( vcn );
  vcn->Delete();

  vtkMRMLDiffusionWeightedVolumeNode *dwvn = vtkMRMLDiffusionWeightedVolumeNode::New();
  this->RegisterNodeClass (dwvn);
  dwvn->Delete();

#ifdef MRML_USE_vtkTeem
  vtkMRMLDiffusionTensorVolumeNode *dtvn = vtkMRMLDiffusionTensorVolumeNode::New();
  this->RegisterNodeClass (dtvn);
  dtvn->Delete();

  vtkMRMLDiffusionTensorVolumeDisplayNode *dtvdn =
                         vtkMRMLDiffusionTensorVolumeDisplayNode::New();
  this->RegisterNodeClass (dtvdn);
  dtvdn->Delete();

  vtkMRMLDiffusionTensorVolumeSliceDisplayNode *dtvsdn =
                         vtkMRMLDiffusionTensorVolumeSliceDisplayNode::New();
  this->RegisterNodeClass (dtvsdn);
  dtvsdn->Delete();

  vtkMRMLNRRDStorageNode *nrrd = vtkMRMLNRRDStorageNode::New();
  this->RegisterNodeClass ( nrrd );
  nrrd->Delete();

  vtkMRMLVectorVolumeNode *vvoln = vtkMRMLVectorVolumeNode::New();
  this->RegisterNodeClass( vvoln );
  vvoln->Delete();

#endif

  vtkMRMLVectorVolumeDisplayNode *vvoldn = vtkMRMLVectorVolumeDisplayNode::New();
  this->RegisterNodeClass( vvoldn );
  vvoldn->Delete();

  vtkMRMLDiffusionWeightedVolumeDisplayNode *dwvdn =
                         vtkMRMLDiffusionWeightedVolumeDisplayNode::New();
  this->RegisterNodeClass (dwvdn);
  dwvdn->Delete();


  vtkMRMLDiffusionTensorDisplayPropertiesNode *dtdpn =
                         vtkMRMLDiffusionTensorDisplayPropertiesNode::New();
  this->RegisterNodeClass (dtdpn);
  dtdpn->Delete();

  vtkMRMLCameraNode *camera = vtkMRMLCameraNode::New();
  this->RegisterNodeClass ( camera );
  camera->Delete();

  vtkMRMLViewNode *view = vtkMRMLViewNode::New();
  this->RegisterNodeClass ( view );
  view->Delete();

  vtkMRMLLayoutNode *layout = vtkMRMLLayoutNode::New();
  this->RegisterNodeClass ( layout );
  layout->Delete();

  vtkMRMLHierarchyNode *hier = vtkMRMLHierarchyNode::New();
  if (hier)
    {
    this->RegisterNodeClass ( hier );
    hier->Delete();
    }
  
  vtkMRMLHierarchyStorageNode *hiersn = vtkMRMLHierarchyStorageNode::New();
  if (hiersn)
    {
    this->RegisterNodeClass ( hiersn );
    hiersn->Delete();
    }
  
  vtkMRMLDisplayableHierarchyNode *dhier = vtkMRMLDisplayableHierarchyNode::New();
  this->RegisterNodeClass ( dhier );
  dhier->Delete();

  vtkMRMLModelHierarchyNode *mhier = vtkMRMLModelHierarchyNode::New();
  this->RegisterNodeClass ( mhier );
  mhier->Delete();

  vtkMRMLSnapshotClipNode *cshot = vtkMRMLSnapshotClipNode::New();
  this->RegisterNodeClass ( cshot );
  cshot->Delete();

  vtkMRMLUnstructuredGridNode *unstg = vtkMRMLUnstructuredGridNode::New();
  this->RegisterNodeClass ( unstg );
  unstg->Delete();

  vtkMRMLUnstructuredGridDisplayNode *unstgd = vtkMRMLUnstructuredGridDisplayNode::New();
  this->RegisterNodeClass ( unstgd );
  unstgd->Delete();

  vtkMRMLUnstructuredGridStorageNode *unstgs = vtkMRMLUnstructuredGridStorageNode::New();
  this->RegisterNodeClass ( unstgs );
  unstgs->Delete();

  vtkMRMLColorTableNode *ctn = vtkMRMLColorTableNode::New();
  this->RegisterNodeClass (ctn);
  ctn->Delete();

  vtkMRMLColorTableStorageNode *ctsn = vtkMRMLColorTableStorageNode::New();
  this->RegisterNodeClass ( ctsn );
  ctsn->Delete();

  vtkMRMLTransformStorageNode *tsn = vtkMRMLTransformStorageNode::New();
  this->RegisterNodeClass( tsn );
  tsn->Delete();

  vtkMRMLNonlinearTransformNode *ntn = vtkMRMLNonlinearTransformNode::New();
  this->RegisterNodeClass( ntn );
  ntn->Delete();

  vtkMRMLGridTransformNode *gtn = vtkMRMLGridTransformNode::New();
  this->RegisterNodeClass( gtn );
  gtn->Delete();

  vtkMRMLBSplineTransformNode *btn = vtkMRMLBSplineTransformNode::New();
  this->RegisterNodeClass( btn );
  btn->Delete();

  vtkMRMLDoubleArrayNode *dan = vtkMRMLDoubleArrayNode::New();
  this->RegisterNodeClass( dan );
  dan->Delete();

  vtkMRMLDoubleArrayStorageNode *dasn = vtkMRMLDoubleArrayStorageNode::New();
  this->RegisterNodeClass(dasn);
  dasn->Delete();

  vtkMRMLCrosshairNode *chn = vtkMRMLCrosshairNode::New();
  this->RegisterNodeClass(chn);
  chn->Delete();

  vtkMRMLInteractionNode *intn = vtkMRMLInteractionNode::New();
  this->RegisterNodeClass(intn);
  intn->Delete();

  vtkMRMLChartNode *chartnode = vtkMRMLChartNode::New(); 
  this->RegisterNodeClass( chartnode );
  chartnode->Delete();

  vtkMRMLChartViewNode *chartviewnode = vtkMRMLChartViewNode::New(); 
  this->RegisterNodeClass( chartviewnode );
  chartviewnode->Delete();

}

//------------------------------------------------------------------------------
vtkMRMLScene::~vtkMRMLScene()
{
  if (this->ClassNameList)
    {
    delete this->ClassNameList;
    }

  this->ClearUndoStack ( );
  this->ClearRedoStack ( );

  if ( this->Nodes != NULL )
    {
    if (this->Nodes->GetNumberOfItems() > 0)
      {
      vtkDebugMacro("CurrentScene should have already been cleared in DeleteEvent callback: ");
      this->Nodes->RemoveAllItems ( );
      }
    this->Nodes->Delete();
    this->Nodes = NULL;
    }

  for (unsigned int n=0; n<this->RegisteredNodeClasses.size(); n++)
    {
    this->RegisteredNodeClasses[n]->Delete();
    }


  if ( this->CacheManager != NULL )
    {
    this->CacheManager->Delete();
    this->CacheManager = NULL;
    }
  if ( this->DataIOManager != NULL )
    {
    this->DataIOManager->Delete();
    this->DataIOManager = NULL;
    }
  if ( this->URIHandlerCollection != NULL )
    {
    this->URIHandlerCollection->RemoveAllItems();
    this->URIHandlerCollection->Delete();
    this->URIHandlerCollection = NULL;
    }
  if ( this->UserTagTable != NULL )
    {
    this->UserTagTable->Delete();
    this->UserTagTable = NULL;
    }
  if ( this->DeleteEventCallback != NULL )
    {
    this->DeleteEventCallback->Delete();
    this->DeleteEventCallback = NULL;
    }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::SceneCallback( vtkObject *vtkNotUsed(caller),
                                  unsigned long vtkNotUsed(eid),
                                  void *clientData, void *vtkNotUsed(callData) )
{
  vtkMRMLScene *self = reinterpret_cast<vtkMRMLScene *>(clientData);
  if (self == NULL)
    {
    return;
    }
  // here we know that SceneCallback has only be called by DeleteEvent,
  // we directly process the event here. If in the future, more events
  // are processed, then add a ProcessMRMLEvents method (instead of
  // doing everything in the static function).
  self->Clear(1);
}

//------------------------------------------------------------------------------
void vtkMRMLScene::Clear(int removeSingletons)
{
#ifdef MRMLSCENE_VERBOSE
  vtkTimerLog* timer = vtkTimerLog::New();
  timer->StartTimer();
#endif
  bool undoFlag = this->GetUndoFlag();
  this->SetUndoOff();
  this->StartState(vtkMRMLScene::CloseState);

  if (!removeSingletons)
    {
    this->RemoveAllNodesExceptSingletons();
    this->ClearReferencedNodeID();
    this->ResetNodes();
    // See comment below
    //    this->InvokeEvent(this->SceneClosedEvent, NULL);
    }
  else
    {
    //this->CurrentScene->RemoveAllItems();
    while (this->GetNumberOfNodes() > 0)
      {
      this->RemoveNode(this->GetNthNode(0));
      }
    // See comment below
    //  this->InvokeEvent(this->SceneClosedEvent, NULL);
    }

  this->ClearReferencedNodeID();


  this->ClearUndoStack ( );
  this->ClearRedoStack ( );
  this->UniqueIDs.clear();
  this->UniqueNames.clear();

  if ( this->GetUserTagTable() != NULL )
    {
    this->GetUserTagTable()->ClearTagTable();
    }


  this->Modified();
  this->SetUndoFlag(undoFlag);

  // Unless I'm wrong, this should be at the very end. SceneAboutToBeClosedEvent
  // is the event that gives a "chance" to objects to release resources.
  // SceneClosedEvent however means, we are done. At this point it seems
  // logical objects are free to create new objects/nodes, for example
  // to create a few new scene once the current one has been close.
  // Therefore, it should be put at the end, certainly after UniqueIDByClass
  // has been cleared
  this->EndState(vtkMRMLScene::CloseState);
#ifdef MRMLSCENE_VERBOSE
  timer->StopTimer();
  std::cerr << "vtkMRMLScene::Clear():" << timer->GetElapsedTime() << "\n";
  timer->Delete();
#endif
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RemoveAllNodesExceptSingletons()
{
  vtkMRMLNode *node;
  this->InitTraversal();
  std::vector<vtkMRMLNode *> removeNodes;
  std::vector< vtkSmartPointer<vtkMRMLNode> > referencingNodes;
  node = this->GetNextNode();
  while(node)
    {
    if (node->GetSingletonTag() == NULL)
      {
      removeNodes.push_back(node);
      }
    else
      {
      referencingNodes.push_back(node);
      }
    node = this->GetNextNode();
    }
  for(unsigned int i=0; i<removeNodes.size(); i++)
    {
    //this->RemoveNode(removeNodes[i]);
    node = removeNodes[i];
    //node->Register(this);
    //this->CurrentScene->vtkCollection::RemoveItem(node);
    this->RemoveNode(node);
    //this->InvokeEvent(this->NodeRemovedEvent,node);
    //node->UnRegister(this);
    }

  this->ReferencingNodes = referencingNodes;
}

//------------------------------------------------------------------------------
void vtkMRMLScene::ResetNodes()
{
  vtkMRMLNode *node;
  std::vector <vtkMRMLNode *> nodes;
  this->InitTraversal();
  node = this->GetNextNode();
  while(node)
    {
    nodes.push_back(node);
    node = this->GetNextNode();
    }
  for(unsigned int i=0; i<nodes.size(); i++)
    {
    nodes[i]->Reset();
    }

}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLScene);

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::CreateNodeByClass(const char* className)
{
  if (className == NULL)
    {
    vtkErrorMacro("CreateNodeByClass: className is NULL");
    return NULL;
    }
  vtkMRMLNode* node = NULL;
  for (unsigned int i=0; i<RegisteredNodeClasses.size(); i++)
    {
    if (!strcmp(RegisteredNodeClasses[i]->GetClassName(), className))
      {
      node = RegisteredNodeClasses[i]->CreateNodeInstance();
      break;
      }
    }
  // non-registered nodes can have a registered factory
  if (node == NULL)
    {
    vtkObject* ret = vtkObjectFactory::CreateInstance(className);
    if(ret)
      {
      node = static_cast<vtkMRMLNode *>(ret);
      // Clean memory
      if (!node)
        {
        ret->Delete();
        }
      }
    else
      {
#ifdef VTK_DEBUG_LEAKS
      vtkDebugLeaks::DestructClass(className);
#endif
      }
    }
  return node;
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RegisterNodeClass(vtkMRMLNode* node)
{
  this->RegisterNodeClass(node, node->GetNodeTagName());
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RegisterNodeClass(vtkMRMLNode* node, const char* tagName)
{
  if (!node)
    {
    vtkErrorMacro("RegisterNodeClass: can't register a null node");
    return;
    }
  if (!tagName)
    {
    tagName = node->GetNodeTagName();
    }
  if (!tagName)
    {
    vtkErrorMacro( << __FUNCTION__
      << ": can't register a null tag name for node class "
      << (node->GetClassName() ? node->GetClassName() : "null"));
    return;
    }
  std::string xmlTag(tagName);
  // Replace the previously registered node if any.
  // By doing so we make sure there is no more than 1 node matching a given
  // XML tag. It allows plugins to MRML to overide default behavior when
  // instantiating nodes via XML tags.
  for (unsigned int i = 0; i < this->RegisteredNodeTags.size(); ++i)
    {
    if (this->RegisteredNodeTags[i] == xmlTag)
      {
      vtkWarningMacro("Tag " << tagName
                      << " has already been registered, unregistering previous node class "
                      << (this->RegisteredNodeClasses[i]->GetClassName() ? this->RegisteredNodeClasses[i]->GetClassName() : "(no class name)")
                      << " to register "
                      << (node->GetClassName() ? node->GetClassName() : "(no class name)"));
      // As the node was previously Registered to the scene, we need to
      // unregister it here. It should destruct the pointer as well (only 1
      // reference on the node).
      this->RegisteredNodeClasses[i]->Delete();
      // Remove the outdated reference to the tag, it will then be added later
      // (after the for loop).
      // we could have replace the entry with the new node also.
      this->RegisteredNodeClasses.erase(this->RegisteredNodeClasses.begin() + i);
      this->RegisteredNodeTags.erase(this->RegisteredNodeTags.begin() + i);
      // we found a matching tag, there is maximum one in the list, no need to
      // search any further
      break;
      }
    }

  node->Register(this);
  this->RegisteredNodeClasses.push_back(node);
  this->RegisteredNodeTags.push_back(xmlTag);
}

//------------------------------------------------------------------------------
void vtkMRMLScene::CopyRegisteredNodesToScene(vtkMRMLScene *scene)
{
  if (scene)
    {
    vtkMRMLNode* node = NULL;
    for (unsigned int i=0; i<this->RegisteredNodeClasses.size(); i++)
      {
      node = this->RegisteredNodeClasses[i]->CreateNodeInstance();
      scene->RegisterNodeClass(node);
      node->Delete();
      }
    }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::CopySingletonNodesToScene(vtkMRMLScene *scene)
{
  if (!scene)
    {
    return;
    }
  vtkMRMLNode* node = NULL;
  int n;
  for (n=0; n < this->Nodes->GetNumberOfItems(); n++)
    {
    node = (vtkMRMLNode*)this->Nodes->GetItemAsObject(n);
    if (node->GetSingletonTag() != 0)
      {
      vtkMRMLNode* newNode = node->CreateNodeInstance();
      newNode->Copy(node);

      scene->AddNode(newNode);
      newNode->Delete();
      }
    }
}

//------------------------------------------------------------------------------
const char* vtkMRMLScene::GetClassNameByTag(const char *tagName)
{
  if (tagName == NULL)
    {
    vtkErrorMacro("GetClassNameByTag: tagname is null");
    return NULL;
    }
  for (unsigned int i=0; i<RegisteredNodeTags.size(); i++)
    {
    if (!strcmp(RegisteredNodeTags[i].c_str(), tagName))
      {
      return (RegisteredNodeClasses[i])->GetClassName();
      }
    }
  return NULL;
}

//------------------------------------------------------------------------------
const char* vtkMRMLScene::GetTagByClassName(const char *className)
{
  if ( !this )
    {
    return NULL;
    }
  if ( !className )
    {
    vtkErrorMacro("GetTagByClassName: className is null");
    return NULL;
    }
  for (unsigned int i=0; i<this->RegisteredNodeClasses.size(); i++)
    {
    if (!strcmp(this->RegisteredNodeClasses[i]->GetClassName(), className))
      {
      return (this->RegisteredNodeClasses[i])->GetNodeTagName();
      }
    }
  return NULL;
}

//------------------------------------------------------------------------------
namespace
{
int bitwiseOr(int firstValue, int secondValue)
{
  return firstValue | secondValue;
}
}

//------------------------------------------------------------------------------
int vtkMRMLScene::GetStates()const
{
  
  return std::accumulate(this->States.begin(), this->States.end(),
                         0x0000, bitwiseOr);
}

//------------------------------------------------------------------------------
void vtkMRMLScene::StartState(unsigned long state, int anticipatedMaxProgress)
{
  bool wasBatchProcessing = this->IsBatchProcessing();
  bool wasInState = ((this->GetStates() & state ) == state);
  this->States.push_back(state);
  if (this->IsBatchProcessing() && !wasBatchProcessing)
    {
    this->InvokeEvent( StateEvent | StartEvent | BatchProcessState);
    }
  if (state != vtkMRMLScene::BatchProcessState &&
      !wasInState)
    {
    this->InvokeEvent( StateEvent | StartEvent | state,
                       reinterpret_cast<void*>(anticipatedMaxProgress));
    }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::EndState(unsigned long state)
{
  assert(this->States.back() == state);
  this->States.pop_back();

  bool isInState = ((this->GetStates() & state) == state);
  // vtkMRMLScene::BatchProcessState is handled after
  if (state != vtkMRMLScene::BatchProcessState &&
      !isInState)
    {
    this->InvokeEvent( StateEvent | EndEvent | state );
    }

  if ((state & vtkMRMLScene::BatchProcessState) &&
      !this->IsBatchProcessing())
    {
    this->InvokeEvent( StateEvent | EndEvent |
                       vtkMRMLScene::BatchProcessState );
    }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::ProgressState(unsigned long state, int progress)
{
  if (state & vtkMRMLScene::BatchProcessState)
    {
    this->InvokeEvent( StateEvent | ProgressEvent | vtkMRMLScene::BatchProcessState,
                       reinterpret_cast<void*>(progress));
    }
  if (state != vtkMRMLScene::BatchProcessState)
    {
    this->InvokeEvent( StateEvent | ProgressEvent | state ,
                       reinterpret_cast<void*>(progress));
    }
}

//------------------------------------------------------------------------------
int vtkMRMLScene::Connect()
{
  assert(!this->IsClosing());
  assert(!this->IsImporting());

#ifdef MRMLSCENE_VERBOSE
  vtkTimerLog* timer = vtkTimerLog::New();
  timer->StartTimer();
#endif
  this->StartState(vtkMRMLScene::BatchProcessState);
  this->Clear(0);
  bool undoFlag = this->GetUndoFlag();
  int res = this->Import();

  this->EndState(vtkMRMLScene::BatchProcessState);
  this->SetUndoFlag(undoFlag);
#ifdef MRMLSCENE_VERBOSE
  timer->StopTimer();
  std::cerr << "vtkMRMLScene::Connect():" << timer->GetElapsedTime() << "\n";
  timer->Delete();
#endif
  return res;
}

//------------------------------------------------------------------------------
int vtkMRMLScene::Import()
{
#ifdef MRMLSCENE_VERBOSE
  vtkTimerLog* addNodesTimer = vtkTimerLog::New();
  vtkTimerLog* updateSceneTimer = vtkTimerLog::New();
  vtkTimerLog* timer = vtkTimerLog::New();
  timer->StartTimer();
#endif
  this->SetErrorCode(0);
  this->SetErrorMessage(std::string(""));

  bool undoFlag = this->GetUndoFlag();

  this->SetUndoOff();
  this->StartState(vtkMRMLScene::ImportState);
  this->ClearReferencedNodeID();

  // read nodes into a temp scene
  vtkCollection* scene = vtkCollection::New();

  int res = this->LoadIntoScene(scene);

  std::map<std::string, vtkMRMLNode *> nodesAddedByClass;

  if (res)
    {
    /// In case the scene needs to change the ID of some nodes to add, the new
    /// ID should not be one already existing in the scene nor one of the
    /// imported scene.
    /// Mark all the node IDs of the scene as reserved so the node ID
    /// generator doesn't choose them.
    vtkMRMLNode *node;
    vtkCollectionSimpleIterator it;
    for (scene->InitTraversal(it);
         (node = (vtkMRMLNode*)scene->GetNextItemAsObject(it)) ;)
      {
      this->AddReservedID(node->GetID());
      }
#ifdef MRMLSCENE_VERBOSE
    addNodesTimer->StartTimer();
#endif
    for (scene->InitTraversal(it);
         (node = (vtkMRMLNode*)scene->GetNextItemAsObject(it)) ;)
      {
      //this->AddNodeNoNotify(node);
      this->AddNode(node);
      }
#ifdef MRMLSCENE_VERBOSE
    addNodesTimer->StopTimer();
    updateSceneTimer->StartTimer();
#endif
    // fix node references that may be not unique in the imported scene.
    this->UpdateNodeReferences(scene);

    this->RemoveReservedIDs();

    this->InvokeEvent(vtkMRMLScene::NewSceneEvent, NULL);
    for (scene->InitTraversal(it);
         (node = (vtkMRMLNode*)scene->GetNextItemAsObject(it)) ;)
      {

      //double progress = n / (1. * nnodes);
      //this->InvokeEvent(vtkCommand::ProgressEvent,(void *)&progress);

      nodesAddedByClass[std::string(node->GetClassName())] = node;
      vtkDebugMacro("Adding Node: " << node->GetName());
      if (node->GetAddToScene())
        {
        node->UpdateScene(this);
        }
      }
    if (this->GetErrorCode() == 1)
      {
      //vtkErrorMacro("Import: error updating node " << node->GetID());
      // TODO: figure out the best way to deal with an error (encountering
      // it when fail to read a file), removing a node isn't quite right
      // (nodes are still in the scene when save it later)
      // this->RemoveNode(node);
      // this->SetErrorCode(0);
      }

    // send one NodeAddedEvent event per class
    // std::map<std::string, vtkMRMLNode *>::iterator iter;
    // for(iter = nodesAddedByClass.begin(); iter != nodesAddedByClass.end(); iter++)
    //   {
    //   vtkDebugMacro("Invoking NodeAddedEvent for: " << (iter->second)->GetName());
    //   this->InvokeEvent(this->NodeAddedEvent, iter->second);
    //   }

    this->Modified();
    this->RemoveUnusedNodeReferences();
#ifdef MRMLSCENE_VERBOSE
    updateSceneTimer->StopTimer();
#endif
    } //if (res)
  else
    {
    this->ClearReferencedNodeID();
    }

  scene->RemoveAllItems();
  scene->Delete();

  this->SetUndoFlag(undoFlag);

#ifdef MRMLSCENE_VERBOSE
  vtkTimerLog* importingTimer = vtkTimerLog::New();
  importingTimer->StartTimer();
#endif
  this->EndState(vtkMRMLScene::ImportState);
#ifdef MRMLSCENE_VERBOSE
  importingTimer->StopTimer();
#endif

  int returnCode = 1;
  if (this->GetErrorCode() == 0)
    {
    // report parser error
    if (res == 0)
      {
      this->SetErrorMessage (std::string("Error parsing scene file"));
      }
    returnCode = res;
    }
  else
    {
    returnCode = 0;
    }
#ifdef MRMLSCENE_VERBOSE
  timer->StopTimer();
  std::cerr<<"vtkMRMLScene::Import()::AddNodes:" << addNodesTimer->GetElapsedTime() << "\n";
  std::cerr<< "vtkMRMLScene::Import()::UpdateScene" << updateSceneTimer->GetElapsedTime() << "\n";
  std::cerr<<"vtkMRMLScene::Import()::SceneImported:" << importingTimer->GetElapsedTime() << "\n";
  std::cerr<<"vtkMRMLScene::Import():" << timer->GetElapsedTime() << "\n";
  addNodesTimer->Delete();
  updateSceneTimer->Delete();
  importingTimer->Delete();
  timer->Delete();
#endif
  this->StoredTime.Modified();
  return returnCode;
}

//------------------------------------------------------------------------------
int vtkMRMLScene::LoadIntoScene(vtkCollection* nodeCollection)
{
  if ((this->URL == "" && this->GetLoadFromXMLString()==0) ||
      (this->GetLoadFromXMLString() == 1 && this->GetSceneXMLString().empty()))
    {
    vtkErrorMacro("No URL specified");
    return 0;
    }
  // check to see if the mrml file lives on a remote disk
  if (this->GetCacheManager())
    {
    int remote = this->GetCacheManager()->IsRemoteReference(this->URL.c_str());
    if (remote)
      {
      vtkDebugMacro("LoadIntoScene: mrml file lives on a remote disk: " << this->URL.c_str());
      // do a synchronous download for now
      vtkURIHandler *handler = this->FindURIHandler(this->URL.c_str());
      if (handler != NULL)
        {
        // put it on disk somewhere
        const char *localURL = this->GetCacheManager()->GetFilenameFromURI(this->URL.c_str());
        handler->StageFileRead(this->URL.c_str(), localURL);
        // now over ride the URL setting
        vtkDebugMacro("LoadIntoScene: downloaded the remote MRML file " << this->URL.c_str() << ", resetting URL to local file " << localURL);
        this->SetURL(localURL);
        }
      else
        {
        vtkErrorMacro("LoadIntoScene: unable to find a file handler for uri " << this->URL.c_str());
        }
      }
    }

  if (this->GetLoadFromXMLString()==0)
    {
    this->RootDirectory = vtksys::SystemTools::GetParentDirectory(this->GetURL());
    }

  if ( this->RootDirectory[0] != '\0' )
    {
    this->RootDirectory = this->RootDirectory + std::string("/");
    }
  else
    {
    this->RootDirectory = std::string("./");
    }
  vtkMRMLParser* parser = vtkMRMLParser::New();
  parser->SetMRMLScene(this);
  if (nodeCollection != this->Nodes)
    {
    parser->SetNodeCollection(nodeCollection);
    }

  int result = 0;
  if (this->GetLoadFromXMLString())
    {
    result = parser->Parse(this->GetSceneXMLString().c_str());
    }
  else
    {
    vtkDebugMacro("Parsing: " << this->URL.c_str());
    parser->SetFileName(URL.c_str());
    result = parser->Parse();
   }

  parser->Delete();

  vtkDebugMacro("Done Parsing: " << this->URL.c_str());
  return result;
}

//------------------------------------------------------------------------------
int vtkMRMLScene::Commit(const char* url)
{
  if (url == NULL)
    {
    if (this->URL != "")
      {
      url = this->URL.c_str();
      }
    else if (this->GetSaveToXMLString() == 0)
      {
      vtkErrorMacro("Commit: URL is not set");
      this->SetErrorCode(vtkErrorCode::GetErrorCodeFromString("CannotOpenFileError"));
      return 1;
      }
    }

  vtkMRMLNode *node;

  std::stringstream oss;
  std::ofstream ofs;

  std::ostream *os = NULL;

  if (this->GetSaveToXMLString())
    {
    os = &oss;
    }
  else
    {
    os = &ofs;
    // Open file
#ifdef _WIN32
    ofs.open(url, std::ios::out | std::ios::binary);
#else
    ofs.open(url, std::ios::out);
#endif
    if (ofs.fail())
      {
      vtkErrorMacro("Write: Could not open file " << url);
#if (VTK_MAJOR_VERSION <= 5)
      this->SetErrorCode(2);
#else
     this->SetErrorCode(vtkErrorCode::GetErrorCodeFromString("CannotOpenFileError"));
#endif
      return 1;
      }
    }

  int indent=0, deltaIndent;


    // this event is being detected by GUI to provide feedback during load
    // of data. But,
    // commented out for now because CLI modules are using MRML to write
    // data in another thread, causing GUI to crash.
//  this->InvokeEvent (vtkMRMLScene::SaveProgressFeedbackEvent );

  //file << "<?xml version=\"1.0\" standalone='no'?>\n";
  //file << "<!DOCTYPE MRML SYSTEM \"mrml20.dtd\">\n";

  //--- BEGIN test of user tags
  //file << "<MRML>\n";
  *os << "<MRML ";

  // write version
  if (this->GetVersion())
    {
    *os << " version=\"" << this->GetVersion() << "\" ";
    }

  //---write any user tags.
  std::stringstream ss;
  if ( this->GetUserTagTable() != NULL )
    {
    ss.clear();
    ss.str ( "" );
    int numc = this->GetUserTagTable()->GetNumberOfTags();
    const char *kwd, *val;
    for (int i=0; i < numc; i++ )
      {
      kwd = this->GetUserTagTable()->GetTagAttribute(i);
      val = this->GetUserTagTable()->GetTagValue (i);
      if (kwd != NULL && val != NULL)
        {
        ss << kwd << "=" << val;
        if ( i < (numc-1) )
          {
          ss << " ";
          }
        }
      }
    if ( ss.str().c_str()!= NULL )
      {
      *os << "userTags=\"" << ss.str().c_str() << "\"";
      }
    }

  *os << ">\n";
  //--- END test of user tags

  // Write each node
  int n;
  for (n=0; n < this->Nodes->GetNumberOfItems(); n++)
    {
    node = (vtkMRMLNode*)this->Nodes->GetItemAsObject(n);
    if (!node->GetSaveWithScene())
      {
      continue;
      }

    deltaIndent = node->GetIndent();
    if ( deltaIndent < 0 )
      {
      indent -=2;
      }

    vtkIndent vindent(indent);
    *os << vindent << "<" << node->GetNodeTagName() << "\n";

    if(indent<=0)
      indent = 1;

    node->WriteXML(*os, indent);

    *os << vindent << ">";
    node->WriteNodeBodyXML(*os, indent);
    *os << "</" << node->GetNodeTagName() << ">\n";

    if ( deltaIndent > 0 )
      {
      indent += 2;
      }
    }

  *os << "</MRML>\n";

  // Close file
  if (this->GetSaveToXMLString())
    {
    SceneXMLString = oss.str();
    }
  else
    {
    ofs.close();
    }
#if (VTK_MAJOR_VERSION <= 5)
  this->SetErrorCode(0);
#else
  this->SetErrorCode(vtkErrorCode::GetErrorCodeFromString("NoError"));
#endif
  this->StoredTime.Modified();
  return 1;
}

//------------------------------------------------------------------------------
vtkMRMLNode*  vtkMRMLScene::AddNodeNoNotify(vtkMRMLNode *n)
{
  if (!n)
    {
    vtkErrorMacro("AddNodeNoNotify: unable to add a null node to the scene");
    return NULL;
    }

  if (!n->GetAddToScene())
    {
    return NULL;
    }

  int wasModifying = n->StartModify();

  //TODO convert URL to Root directory
  //n->SetSceneRootDir("");

  // check if node is a singletone
  if (n->GetSingletonTag() != NULL)
    {
    // check if there is a singletone of this class in the scene
    // and if found copy this node into it
    vtkMRMLNode *sn = this->GetSingletonNode(n->GetSingletonTag(), n->GetClassName());
    if (sn != NULL)
      {
      // A node can't be added twice into the scene
      assert(sn != n);
      std::string oldId(sn->GetID());
      std::string newId(n->GetID() ? n->GetID() : sn->GetID());
      sn->CopyWithSceneWithSingleModifiedEvent(n);

      this->RemoveNodeReferences(n);
      // cache the node so the whole scene cache stays up-to-date
      // Remove the old node ID only if the ID was associated with sn.
      std::map< std::string, vtkSmartPointer<vtkMRMLNode> >::iterator nodeID =
        this->NodeIDs.find(oldId);
      if (nodeID != this->NodeIDs.end() &&
          nodeID->second == sn)
        {
        this->RemoveNodeID((char*)nodeID->first.c_str());
        }
      // If NodeIDs[newId] already contains a value, that node can't be found
      // anymore in the NodeIDs cache.
      this->NodeIDs[newId] = sn;
      this->NodeIDsMTime = this->Nodes->GetMTime();

      n->EndModify(wasModifying);
      return sn;
      }
    }
  // Usually nodes to add to the scene don't have an ID set. However, when
  // importing a scene (vtkMRMLScene::Import()), nodes to add already have an
  // ID. This can create a node ID conflict with the nodes already in the
  // scene. A new unique ID  must be set to the node to add and references to
  // the node must be updated with the new ID.
  if (n->GetID() == NULL || n->GetID()[0] == '\0' || this->GetNodeByID(n->GetID()) != NULL)
    {
    std::string oldID;
    if (n->GetID())
      {
      oldID = n->GetID();
      }
    n->SetID(this->GenerateUniqueID(n).c_str());

    vtkDebugMacro("AddNodeNoNotify: got unique id for new " << n->GetClassName() << " node: " << n->GetID() << endl);
    std::string newID(n->GetID());
    if (oldID != newID)
      {
      this->ReferencedIDChanges[oldID] = newID;
      }
    }

  // Set a default name if none is given automatically
  if (n->GetName() == NULL|| n->GetName()[0] == '\0')
    {
    n->SetName(this->GenerateUniqueName(n).c_str());
    }
  n->SetScene( this );
  this->Nodes->vtkCollection::AddItem((vtkObject *)n);

  // cache the node so the whole scene cache stays up-todate
  this->AddNodeID(n);

  //n->OnNodeAddedToScene();

  n->EndModify(wasModifying);
  return n;
}

//------------------------------------------------------------------------------
vtkMRMLNode*  vtkMRMLScene::AddNode(vtkMRMLNode *n)
{
  if (!n)
    {
    vtkErrorMacro("AddNode: unable to add a null node to the scene");
    return NULL;
    }
  if (!n->GetAddToScene())
    {
    return NULL;
    }
#ifndef NDEBUG
  // Since calling IsNodePresent is costly, a "developper hint" is printed only
  // if build as debug. We can't exit here as the release would then be
  // different from debug.
  // The caller should make sure the node has not been added yet.
  if (this->IsNodePresent(n) != 0)
    {
    vtkErrorMacro("AddNode: Node " << n->GetClassName()<< "/"
                    << n->GetName() << "/" << n->GetID()
                    << "[" << n << "]" << " already added");
    }
#endif
  // We need to know if the node will be actually added to the scene before
  // it is effectively added to know if NodeAboutToBeAddedEvent needs to be
  // fired.
  bool add = true;
  if (n->GetSingletonTag() != NULL &&
      this->GetSingletonNode(n->GetSingletonTag(), n->GetClassName()) != NULL)
    {
    // if the node is a singleton, then it won't be added, just replaced
    add = false;
    }
#ifdef MRMLSCENE_VERBOSE
  vtkTimerLog* timer = vtkTimerLog::New();
  timer->StartTimer();
#endif
  if (add)
    {
    this->InvokeEvent(this->NodeAboutToBeAddedEvent, n);
    }
  vtkMRMLNode* node = this->AddNodeNoNotify(n);
  // If the node is a singleton, the returned node is the existing singleton
  assert( add || node != n);
  if (add)
    {
    this->InvokeEvent(this->NodeAddedEvent, n);
    }
  this->Modified();
#ifdef MRMLSCENE_VERBOSE
  timer->StopTimer();
  std::cerr << "AddNode: " << n->GetID() << " :" << timer->GetElapsedTime() << "\n";
  timer->Delete();
#endif
  return node;
}


//------------------------------------------------------------------------------
void vtkMRMLScene::NodeAdded(vtkMRMLNode *n)
{
  this->InvokeEvent(this->NodeAddedEvent, n);
}

//------------------------------------------------------------------------------
vtkMRMLNode*  vtkMRMLScene::CopyNode(vtkMRMLNode *n)
{
   if (!n)
    {
    vtkErrorMacro("CopyNode: unable to copy a null node");
    return NULL;
    }

  if (!n->GetAddToScene())
    {
    return NULL;
    }
  vtkMRMLNode* node = n->CreateNodeInstance();
  node->Copy(n);

  vtkMRMLNode* nnode = this->AddNode(node);
  node->Delete();
  return nnode;
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RemoveNode(vtkMRMLNode *n)
{
  if (n == NULL)
    {
    vtkDebugMacro("RemoveNode: unable to remove null node");
    return;
    }

#ifndef NDEBUG
  // Since calling IsNodePresent cost, let's display a "developper hint" only if build as Debug
  // The caller should make sure the node isn't already removed
  if (this->IsNodePresent(n) == 0)
    {
    vtkErrorMacro("RemoveNode: Node " << n->GetClassName() << "/"
                  << n->GetName() << "[" << n << "]" << " already removed");
    }
  // As callbacks may want to look for the removed node, the nodeID list should
  // be up to date.
  if (this->GetNodeByID(n->GetID()) == NULL)
    {
    vtkErrorMacro("RemoveNode: class: " << n->GetClassName() << " name:"
                  << n->GetName() << " id: " << n->GetID()
                  << "["  << n << "]" << " can't be found by ID");
    }
#endif

  n->Register(this);
  this->InvokeEvent(vtkMRMLScene::NodeAboutToBeRemovedEvent, n);

  this->RemoveNodeReferences(n);
  this->RemoveReferencesToNode(n);
  if (n->GetScene() == this) // extra precaution that might not be useful
    {
    n->SetScene(0);
    }
  this->Nodes->vtkCollection::RemoveItem((vtkObject *)n);

  this->RemoveNodeID(n->GetID());

  this->InvokeEvent(vtkMRMLScene::NodeRemovedEvent, n);
  n->UnRegister(this);

  if (!this->IsBatchProcessing())
    {
    vtkMRMLNode *node = NULL;
    vtkCollectionSimpleIterator it;
    for (this->Nodes->InitTraversal(it);
         (node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject(it)) ;)
      {
      node->UpdateReferences();
      }
    }

  this->RemoveUnusedNodeReferences();

  this->Modified();
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RemoveReferencedNodeID(const char *id, vtkMRMLNode *referencingNode)
{
  if (id == NULL || referencingNode == NULL)
    {
    vtkErrorMacro("RemoveReferencedNodeID: either id is null or the reference node is null.");
    return;
    }

  std::vector< std::string > referencedIDs;
  std::vector< vtkSmartPointer<vtkMRMLNode> > referencingNodes;

  const size_t nnodes = this->ReferencingNodes.size();
  referencedIDs.reserve(nnodes);
  referencingNodes.reserve(nnodes);
  for(size_t i=0; i<nnodes; ++i)
    {
    vtkMRMLNode *node = this->ReferencingNodes[i];
    if ( node  && referencingNode )
      {
      if (node->GetID() && referencingNode->GetID()
          && !strcmp(node->GetID(), referencingNode->GetID())
          && id && this->ReferencedIDs[i].c_str()
          && !strcmp(id, this->ReferencedIDs[i].c_str()) )
        {
        // need to remove do nothing
        continue;
        }
      else
        {
        referencedIDs.push_back(this->ReferencedIDs[i]);
        referencingNodes.push_back(this->ReferencingNodes[i]);
        }
      }
    }
  this->ReferencedIDs = referencedIDs;
  this->ReferencingNodes = referencingNodes;
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RemoveNodeReferences(vtkMRMLNode *n)
{
  if (n == NULL)
    {
    vtkErrorMacro("RemoveNodeReferences: node is null can't remove it");
    return;
    }
  if (n->GetID() == NULL)
    {
    // can happen when adding singleton nodes that are not really added but copied
    return;
    }

  std::vector< std::string > referencedIDs;
  std::vector< vtkSmartPointer<vtkMRMLNode> > referencingNodes;

  const size_t nnodes = this->ReferencingNodes.size();
  referencedIDs.reserve(nnodes);
  referencingNodes.reserve(nnodes);
  for( size_t i=0; i<nnodes; ++i)
    {
    vtkMRMLNode *node = this->ReferencingNodes[i];
    if ( node && n )
      {
      if (node->GetID() && strcmp(node->GetID(), n->GetID()))
        {
        referencedIDs.push_back(this->ReferencedIDs[i]);
        referencingNodes.push_back(this->ReferencingNodes[i]);
        }
      }
    }
  this->ReferencedIDs = referencedIDs;
  this->ReferencingNodes = referencingNodes;
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RemoveUnusedNodeReferences()
{
  std::vector< std::string > referencedIDs;
  std::vector< vtkSmartPointer<vtkMRMLNode> > referencingNodes;

  const size_t nnodes = this->ReferencedIDs.size();
  referencedIDs.reserve(nnodes);
  referencingNodes.reserve(nnodes);
  for( size_t i=0; i<nnodes; ++i)
    {
    if (this->ReferencedIDs[i].size() == 0 ||
        this->ReferencingNodes[i]->GetID() == NULL)
      {
      continue;
      }
    vtkMRMLNode *node = this->GetNodeByID(this->ReferencedIDs[i]);
    if ( node && node->GetID())
      {
      vtkMRMLNode *referencingNode = this->GetNodeByID(
        this->ReferencingNodes[i]->GetID());
      if (referencingNode && referencingNode == this->ReferencingNodes[i])
        {
        referencedIDs.push_back(this->ReferencedIDs[i]);
        referencingNodes.push_back(this->ReferencingNodes[i]);
        }
      }
    }
  this->ReferencedIDs = referencedIDs;
  this->ReferencingNodes = referencingNodes;
/*
  std::vector< std::string > referencedIDs1;
  std::vector< vtkSmartPointer<vtkMRMLNode> > referencingNodes1;

  nnodes = referencingNodes.size();
  referencedIDs1.reserve(nnodes);
  referencingNodes1.reserve(nnodes);
  for( size_t i=0; i<nnodes; i++)
    {
    if (referencingNodes[i]->GetID() == NULL)
      {
      continue;
      }
    vtkMRMLNode *node = this->GetNodeByID(referencingNodes[i]->GetID());
    if ( node && node == referencingNodes[i])
      {
      referencedIDs1.push_back(referencedIDs[i]);
      referencingNodes1.push_back(referencingNodes[i]);
      }
    }
  this->ReferencedIDs = referencedIDs1;
  this->ReferencingNodes = referencingNodes1;
*/
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RemoveReferencesToNode(vtkMRMLNode *n)
{
  if (n == NULL && n->GetID() == NULL)
    {
    vtkErrorMacro("RemoveReferencesToNode: node is null or has null id, can't remove refs");
    return;
    }

  std::vector< std::string > referencedIDs;
  std::vector< vtkSmartPointer<vtkMRMLNode> > referencingNodes;

  const size_t nnodes = this->ReferencingNodes.size();
  referencedIDs.reserve(nnodes);
  referencingNodes.reserve(nnodes);
  for( size_t i=0; i<nnodes; ++i)
    {
    if ( this->ReferencedIDs[i].c_str() && strcmp(this->ReferencedIDs[i].c_str(), n->GetID()))
      {
      referencedIDs.push_back(this->ReferencedIDs[i]);
      referencingNodes.push_back(this->ReferencingNodes[i]);
      }
    }
  this->ReferencedIDs = referencedIDs;
  this->ReferencingNodes = referencingNodes;

}

//------------------------------------------------------------------------------
int vtkMRMLScene::IsNodePresent(vtkMRMLNode *n)
{
  return this->Nodes->vtkCollection::IsItemPresent((vtkObject *)n);
}

//------------------------------------------------------------------------------
void vtkMRMLScene::InitTraversal()
{
  assert(this);
  this->Nodes->InitTraversal();
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetNextNode()
{
  return vtkMRMLNode::SafeDownCast(this->Nodes->GetNextItemAsObject());
}

//------------------------------------------------------------------------------
int vtkMRMLScene::GetNumberOfNodes ()
{
  return this->Nodes->GetNumberOfItems();
}

//------------------------------------------------------------------------------
int vtkMRMLScene::GetNumberOfNodesByClass(const char *className)
{
  if (className == NULL)
    {
    vtkErrorMacro("GetNumberOfNodesByClass: class name is null.");
    return 0;
    }
  int num=0;
  vtkMRMLNode *node;
  vtkCollectionSimpleIterator it;
  for (this->Nodes->InitTraversal(it);
       (node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject(it)) ;)
    {
    if (node->IsA(className))
      {
      num++;
      }
    }
  return num;
}

//------------------------------------------------------------------------------
int vtkMRMLScene::GetNodesByClass(const char *className, std::vector<vtkMRMLNode *> &nodes)
{
  if (className == NULL)
    {
    vtkErrorMacro("GetNodesByClass: class name is null.");
    return 0;
    }
  vtkMRMLNode *node;
  vtkCollectionSimpleIterator it;
  for (this->Nodes->InitTraversal(it);
       (node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject(it)) ;)
    {
    if (node->IsA(className))
      {
      nodes.push_back(node);
      }
    }
  return static_cast<int>(nodes.size());
}

//------------------------------------------------------------------------------
vtkCollection* vtkMRMLScene::GetNodesByClass(const char *className)
{
  if (className == NULL)
    {
    vtkErrorMacro("GetNodesByClass: class name is null.");
    return 0;
    }
  vtkCollection* nodes = vtkCollection::New();
  vtkMRMLNode *node;
  vtkCollectionSimpleIterator it;
  for (this->Nodes->InitTraversal(it);
       (node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject(it)) ;)
    {
    if (node->IsA(className))
      {
      nodes->AddItem(node);
      }
    }
  return nodes;
}

//------------------------------------------------------------------------------
std::list< std::string > vtkMRMLScene::GetNodeClassesList()
{
  std::list< std::string > classes;

  vtkMRMLNode *node;
  vtkCollectionSimpleIterator it;
  for (this->Nodes->InitTraversal(it);
       (node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject(it)) ;)
    {
    classes.push_back(node->GetClassName());
    }
  classes.sort();
  classes.unique();
  return classes;

}

//------------------------------------------------------------------------------
const char* vtkMRMLScene::GetNodeClasses()
{
  std::list< std::string > classes = this->GetNodeClassesList();
  std::string classList;

  std::list< std::string >::const_iterator iter;
  // Iterate through list and output each element.
  for (iter = classes.begin(); iter != classes.end(); iter++)
    {
    if (!(iter == classes.begin()))
      {
      classList += std::string(" ");
      }
    classList.append(*iter);
    }
  SetClassNameList(classList.c_str());
  return this->ClassNameList;
}



//------------------------------------------------------------------------------
vtkMRMLNode *vtkMRMLScene::GetNextNodeByClass(const char *className)
{
  if ( !this || !this->Nodes)
    {
    return NULL;
    }
  if (!className)
    {
    vtkErrorMacro("GetNextNodeByClass: class name is null.");
    return NULL;
    }

  vtkMRMLNode *node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject();

  while (node != NULL && !node->IsA(className))
    {
    node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject();
    }
  if (node != NULL && node->IsA(className))
    {
    return node;
    }
  else
    {
    return NULL;
    }
}
//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetSingletonNode(const char* singletonTag, const char* className)
{
  assert(singletonTag);
  assert(className);

  vtkCollectionSimpleIterator it;
  vtkMRMLNode* node = NULL;
  for (this->Nodes->InitTraversal(it);
       (node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject(it)) ;)
    {
    if (node->IsA(className) &&
        node->GetSingletonTag() != NULL &&
        strcmp(node->GetSingletonTag(), singletonTag) == 0)
      {
      return node;
      }
    }
  return NULL;
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetNthNode(int n)
{

  if(n < 0 || n >= this->Nodes->GetNumberOfItems())
    {
    return NULL;
    }
  else
    {
    return (vtkMRMLNode*)this->Nodes->GetItemAsObject(n);
    }
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetNthNodeByClass(int n, const char *className)
{
  if (className == NULL || n < 0)
    {
    vtkErrorMacro("GetNthNodeByClass: class name is null or n is less than zero: " << n);
    return NULL;
    }

  int num=0;
  vtkMRMLNode *node;
  vtkCollectionSimpleIterator it;
  for (this->Nodes->InitTraversal(it);
       (node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject(it)) ;)
    {
    if (node->IsA(className))
      {
      if (num == n)
        {
        return node;
        }
      num++;
      }
    }
  return NULL;
}

//------------------------------------------------------------------------------
vtkCollection* vtkMRMLScene::GetNodesByName(const char* name)
{

  vtkCollection* nodes = vtkCollection::New();

  if (!name)
    {
    vtkErrorMacro("GetNodesByName: name is null");
    return nodes;
    }

  vtkMRMLNode *node;
  vtkCollectionSimpleIterator it;
  for (this->Nodes->InitTraversal(it);
       (node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject(it)) ;)
    {
    if (!strcmp(node->GetName(), name))
      {
      nodes->AddItem(node);
      }
    }
  return nodes;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetFirstNode(const char* byName,
                                        const char* byClass,
                                        const int* byHideFromEditors)
{
  vtkCollectionSimpleIterator it;
  vtkMRMLNode* node;
  for (this->Nodes->InitTraversal(it);
       (node= vtkMRMLNode::SafeDownCast(
          this->Nodes->GetNextItemAsObject(it))) ;)
    {
    if (byName && node->GetName() != 0 && strcmp(node->GetName(), byName) != 0)
      {
      continue;
      }
    if (byClass && !node->IsA(byClass))
      {
      continue;
      }
    if (byHideFromEditors && node->GetHideFromEditors() != *byHideFromEditors)
      {
      continue;
      }
    return node;
    }
  return 0;
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetFirstNodeByName(const char* name)
{
  vtkMRMLNode *node = 0;
  if (name == 0)
    {
    vtkErrorMacro("GetNodesByName: name is null");
    return node;
    }

  vtkCollectionSimpleIterator it;
  for (this->Nodes->InitTraversal(it);
       (node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject(it)) ;)
    {
    if (node->GetName() != 0 && !strcmp(node->GetName(), name))
      {
      return node;
      }
    }
  return 0;
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetNodeByID(std::string id)
{
  return this->GetNodeByID(id.c_str());
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetNodeByID(const char* id)
{
  if (this == NULL)
    {
    vtkErrorMacro("GetNodeByID: That's scary, you accessed a null scene !");
    return NULL;
    }
  if (id == NULL)
    {
    // don't use a macro here, can cause a crash on exit
    return NULL;
    }

  vtkMRMLNode *node = NULL;
  this->UpdateNodeIDs();
  std::map< std::string, vtkSmartPointer<vtkMRMLNode> >::iterator it = this->NodeIDs.find(std::string(id));
  if (it != this->NodeIDs.end())
    {
    node = it->second;
    }
#ifndef NDEBUG
  else
    {
    // Ensure the node can't be found, and there is no error with the cache
    // mechanism.
    vtkMRMLNode* foundNode = 0;
    vtkCollectionSimpleIterator it;
    for (this->Nodes->InitTraversal(it);
         (node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject(it)) ;)
      {
      if (!strcmp(node->GetID(), id))
        {
        foundNode = node;
        }
      }
    if ( foundNode )
      {
      vtkErrorMacro("GetNodeByID: No node found for ID: " << id);
      }
    }
#endif
  return node;
}

//------------------------------------------------------------------------------
vtkCollection* vtkMRMLScene::GetNodesByClassByName(const char* className, const char* name)
{
  vtkCollection* nodes = vtkCollection::New();

  if (!className || !name)
    {
    vtkErrorMacro("GetNodesByClassByName: classname or name are null");
    return nodes;
    }

  vtkMRMLNode *node;
  for (int n=0; n < this->Nodes->GetNumberOfItems(); n++)
    {
    node = (vtkMRMLNode*)this->Nodes->GetItemAsObject(n);
    if (!strcmp(node->GetName(), name) && node->IsA(className))
      {
      nodes->AddItem(node);
      }
    }

  return nodes;
}

//------------------------------------------------------------------------------
int  vtkMRMLScene::GetTransformBetweenNodes(vtkMRMLNode * vtkNotUsed(node1),
                                            vtkMRMLNode * vtkNotUsed(node2),
                                            vtkGeneralTransform * vtkNotUsed(xform))
{
  vtkErrorMacro("NOT IMPLEMENTEED YET");
  return 1;
}


//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::InsertAfterNode(vtkMRMLNode *item, vtkMRMLNode *n)
{
  if (!n)
    {
    vtkErrorMacro("InsertAfterNode: unable to add a null node to the scene");
    return NULL;
    }
  if (!n->GetAddToScene())
    {
    return NULL;
    }
  if (n->GetSingletonTag() != NULL)
    {
    vtkDebugMacro("InsertAfterNode: node is a singleton, not inserting after item, just calling AddNode");
    return this->AddNode(n);
    }

  this->InvokeEvent(this->NodeAboutToBeAddedEvent, n);

  // code from add node no notify
  if (n->GetID() == NULL || n->GetID()[0] == '\0' || this->GetNodeByID(n->GetID()) != NULL)
    {
    std::string oldID;
    if (n->GetID())
      {
      oldID = n->GetID();
      }
    int modifyStatus = n->GetDisableModifiedEvent();
    n->SetDisableModifiedEvent(1);
    n->SetID(this->GenerateUniqueID(n).c_str());
    n->SetDisableModifiedEvent(modifyStatus);
    std::string newID(n->GetID());
    if (oldID != newID)
      {
      this->ReferencedIDChanges[oldID] = newID;
      }
    }

  int modifyStatus = n->GetDisableModifiedEvent();
  n->SetDisableModifiedEvent(1);

  n->SetSceneRootDir(this->RootDirectory.c_str());
  if (n->GetName() == NULL|| n->GetName()[0] == '\0')
    {
    n->SetName(n->GetID());
    }
  n->SetScene( this );

  // this is the major difference from AddNodeNoNotify, instead of AddItem,
  // use InsertItem (it inserts the passed object after the index passed)
  int index = 0;
  int itemIndex = 0;
  // find the index of the item to insert after
  itemIndex = this->IsNodePresent(item);
  if (itemIndex == 0)
    {
    // it wasn't found, just add
    this->Nodes->vtkCollection::AddItem((vtkObject *)n);
    }
  else
    {
    // the object was found, the location is the return value-1.
    index = itemIndex - 1;
    vtkDebugMacro("InsertAfterNode: item index = " << itemIndex-1 << ", inserting after index = " << index);
    this->Nodes->vtkCollection::InsertItem(index, (vtkObject *)n);
    }
  // cache the node so the whole scene cache stays up-todate
  this->AddNodeID(n);

  n->SetDisableModifiedEvent(modifyStatus);

  this->InvokeEvent(this->NodeAddedEvent, n);

  this->Modified();
  //return node;
  return n;
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::InsertBeforeNode(vtkMRMLNode *item, vtkMRMLNode *n)
{
  if (!n)
    {
    vtkErrorMacro("InsertBeforeNode: unable to add a null node to the scene");
    return NULL;
    }
  if (!n->GetAddToScene())
    {
    return NULL;
    }
  if (n->GetSingletonTag() != NULL)
    {
    vtkDebugMacro("InsertBeforeNode: node is a singleton, not inserting before item, just calling AddNode");
    return this->AddNode(n);
    }

  this->InvokeEvent(this->NodeAboutToBeAddedEvent, n);

  // code from add node no notify
  if (n->GetID() == NULL || n->GetID()[0] == '\0' || this->GetNodeByID(n->GetID()) != NULL)
    {
    std::string oldID;
    if (n->GetID())
      {
      oldID = n->GetID();
      }
    int modifyStatus = n->GetDisableModifiedEvent();
    n->SetDisableModifiedEvent(1);
    n->SetID(this->GenerateUniqueID(n).c_str());
    n->SetDisableModifiedEvent(modifyStatus);
    std::string newID(n->GetID());
    if (oldID != newID)
      {
      this->ReferencedIDChanges[oldID] = newID;
      }
    }

  int modifyStatus = n->GetDisableModifiedEvent();
  n->SetDisableModifiedEvent(1);

  n->SetSceneRootDir(this->RootDirectory.c_str());
  if (n->GetName() == NULL|| n->GetName()[0] == '\0')
    {
    n->SetName(n->GetID());
    }
  n->SetScene( this );

  // this is the major difference from AddNodeNoNotify, instead of AddItem,
  // use InsertItem (it inserts the passed object after the index passed, so
  // use the index of the item before the item to insert before
  int index = 0;
  int itemIndex = 0;
  // find the index of the item to insert before
  itemIndex = this->IsNodePresent(item);
  if (itemIndex == 0)
    {
    // it wasn't found, just add
    this->Nodes->vtkCollection::AddItem((vtkObject *)n);
    }
  else
    {
    // the object was found, the location is the return value-1.
    // then go one more up to get before it
    index = itemIndex - 2;
    vtkDebugMacro("InsertBeforeNode: item index = " << itemIndex-1 << ", inserting after index = " << index);
    this->Nodes->vtkCollection::InsertItem(index, (vtkObject *)n);
    }
  // cache the node so the whole scene cache stays up-todate
  this->AddNodeID(n);

  n->SetDisableModifiedEvent(modifyStatus);

  this->InvokeEvent(this->NodeAddedEvent, n);

  this->Modified();
  //return node;
  return n;
}

//------------------------------------------------------------------------------
void vtkMRMLScene::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Version = " << (this->GetVersion() ? this->GetVersion() : "NULL") << "\n";
  os << indent << "LastLoadedVersion = " << (this->GetLastLoadedVersion() ? this->GetLastLoadedVersion() : "NULL") << "\n";
  os << indent << "ErrorCode = " << this->ErrorCode << "\n";
  os << indent << "URL = " << this->GetURL() << "\n";
  os << indent << "Root Directory = " << this->GetRootDirectory() << "\n";

  this->Nodes->vtkCollection::PrintSelf(os,indent);
  std::list<std::string> classes = this->GetNodeClassesList();

  std::list< std::string >::const_iterator iter;
  // Iterate through list and output each element.
  for (iter = classes.begin(); iter != classes.end(); iter++)
    {
    std::string className = (*iter);
    os << indent << "Number Of Nodes for class " << className.c_str() << " : " << this->GetNumberOfNodesByClass(className.c_str()) << "\n";
    }
  if ( this->GetUserTagTable() != NULL )
    {
    this->UserTagTable->PrintSelf(os, indent);
    }

  os << indent << "Registered node classes:\n";
  for (unsigned int n = 0; n < this->RegisteredNodeClasses.size(); n++)
    {
    os << indent.GetNextIndent() << "Class name = " << this->RegisteredNodeClasses[n]->GetClassName() << endl;
    if (this->RegisteredNodeClasses[n]->IsA("vtkMRMLStorageNode"))
      {
      vtkMRMLStorageNode *snode = vtkMRMLStorageNode::SafeDownCast(this->RegisteredNodeClasses[n]);
      const char *exts = snode->GetDefaultWriteFileExtension();
      os << indent.GetNextIndent().GetNextIndent() << "Default write extension = " << (exts != NULL ? exts : "NULL") << endl;
      }
   }
}

//------------------------------------------------------------------------------
int vtkMRMLScene::GetNumberOfRegisteredNodeClasses()
{
  return static_cast<int>(this->RegisteredNodeClasses.size());
}

//------------------------------------------------------------------------------
vtkMRMLNode *vtkMRMLScene::GetNthRegisteredNodeClass(int n)
{
  if (n >= 0 && n < this->GetNumberOfRegisteredNodeClasses())
    {
    return this->RegisteredNodeClasses[n];
    }
  else
    {
    vtkErrorMacro("GetNthRegisteredNodeClass: index " << n << " out of bounds 0 - " << this->GetNumberOfRegisteredNodeClasses());
    return NULL;
    }
}

//------------------------------------------------------------------------------
std::string vtkMRMLScene::GenerateUniqueID(vtkMRMLNode* node)
{
  assert(node != 0);
  std::string baseID = node->GetClassName();
  if (node->GetSingletonTag())
    {
    return baseID + node->GetSingletonTag();
    }
  return this->GenerateUniqueID(baseID);
}

//------------------------------------------------------------------------------
std::string vtkMRMLScene::GenerateUniqueID(const std::string& baseID)
{
  int uniqueIDIndex = this->GetUniqueIDIndex(baseID);
  // Register ID so it won't be available next time GetUniqueIDIndex is called
  this->UniqueIDs[baseID] = uniqueIDIndex;
  // Create the unique ID
  return this->BuildID(baseID, uniqueIDIndex);
}

//------------------------------------------------------------------------------
int vtkMRMLScene::GetUniqueIDIndex(const std::string& baseID)
{
  assert(baseID.size() != 0);
  int lastIDIndex = 0;
  std::map< std::string, int>::const_iterator uidIt =
    this->UniqueIDs.find(baseID);
  if (uidIt != this->UniqueIDs.end())
    {
    lastIDIndex = uidIt->second;
    }
  bool isUnique = false;
  int index = lastIDIndex;
  // keep looping until you find an id that isn't yet in the scene
  // TODO: this could be speeded up if it becomes a bottleneck
  for (; !isUnique; )
    {
    ++index;
    std::string candidateID = this->BuildID(baseID, index);
    isUnique =
      (this->GetNodeByID(candidateID) == 0) &&
      (std::find(this->ReservedIDs.begin(), this->ReservedIDs.end(), candidateID)
         == this->ReservedIDs.end());
    }
  return index;
}

//------------------------------------------------------------------------------
std::string vtkMRMLScene::BuildID(const std::string& baseID, int idIndex)const
{
  std::stringstream ss;
  ss << baseID;
  // idIndex == 0 is for singleton nodes.
  if (idIndex != 0)
    {
    ss << idIndex;
    }
  return ss.str();
}

//------------------------------------------------------------------------------
std::string vtkMRMLScene::GenerateUniqueName(vtkMRMLNode* node)
{
  assert(node);
  return this->GenerateUniqueName(node->GetNodeTagName());
}

//------------------------------------------------------------------------------
std::string vtkMRMLScene::GenerateUniqueName(const std::string& baseName)
{
  int uniqueNameIndex = this->GetUniqueNameIndex(baseName);
  // Register name so it won't be available next time GetUniqueNameIndex is called
  this->UniqueNames[baseName] = uniqueNameIndex;
  // Create the unique name
  return this->BuildName(baseName, uniqueNameIndex);
}

//------------------------------------------------------------------------------
const char* vtkMRMLScene::GetUniqueNameByString(const char* baseName)
{
  static std::string unsafeName;
  unsafeName = this->GenerateUniqueName(baseName);
  return unsafeName.c_str();
}

//------------------------------------------------------------------------------
int vtkMRMLScene::GetUniqueNameIndex(const std::string& baseName)
{
  assert(baseName.size() > 0);
  int lastNameIndex = -1;
  std::map< std::string, int>::const_iterator uNameIt =
    this->UniqueNames.find(baseName);
  if (uNameIt != this->UniqueNames.end())
    {
    lastNameIndex = uNameIt->second;
    }
  bool isUnique = false;
  int index = lastNameIndex;
  // keep looping until you find a name that isn't yet in the scene
  // TODO: this could be speeded up if it becomes a bottleneck
  for (; !isUnique; )
    {
    ++index;
    std::string candidateName = this->BuildName(baseName, index);
    isUnique = (this->GetFirstNodeByName(candidateName.c_str()) == 0);
    }
  return index;
}

//------------------------------------------------------------------------------
std::string vtkMRMLScene::BuildName(const std::string& baseName, int nameIndex)const
{
  std::stringstream name;
  name << baseName;
  if (nameIndex > 0)
    {
    // first name is "foo", then "foo_1", then "foo_2"
    name << "_" << nameIndex;
    }
  return name.str();
}

//------------------------------------------------------------------------------
void vtkMRMLScene::AddReservedID(const char *id)
{
  if (id == NULL)
    {
    return;
    }
  this->ReservedIDs.push_back(std::string(id));
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RemoveReservedIDs()
{
  this->ReservedIDs.clear();
}

//------------------------------------------------------------------------------
// Pushes the current scene onto the undo stack, and makes a backup copy of the
// passed node so that changes to the node are undoable; several signatures to handle
// individual nodes or a vtkCollection of nodes, or a vector of nodes
//
void vtkMRMLScene::SaveStateForUndo (vtkMRMLNode *node)
{
  if (!this->UndoFlag)
    {
    return;
    }

  if (this->InUndo)
    {
    return;
    }

  if (this->IsBatchProcessing())
    {
    return;
    }

  this->ClearRedoStack();
  //this->SetUndoOn();
  this->PushIntoUndoStack();
  if ( node && !node->IsA("vtkMRMLSceneViewNode"))
    {
    this->CopyNodeInUndoStack(node);
    }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::SaveStateForUndo (std::vector<vtkMRMLNode *> nodes)
{
  if (!this->UndoFlag)
    {
    return;
    }

  if (this->InUndo )
    {
    return;
    }
  if (this->IsBatchProcessing())
    {
    return;
    }

  this->ClearRedoStack();
  //this->SetUndoOn();
  this->PushIntoUndoStack();
  unsigned int n;
  for (n=0; n<nodes.size(); n++)
    {
    vtkMRMLNode *node = nodes[n];
    if (node && !node->IsA("vtkMRMLSceneViewNode"))
      {
      this->CopyNodeInUndoStack(node);
      }
    }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::SaveStateForUndo (vtkCollection* nodes)
{
  if (!this->UndoFlag)
    {
    return;
    }

  if (this->InUndo)
    {
    return;
    }

  if (this->IsBatchProcessing())
    {
    return;
    }

  if (!nodes)
    {
    return;
    }

  this->ClearRedoStack();
  //this->SetUndoOn();
  this->PushIntoUndoStack();

  int nnodes = nodes->GetNumberOfItems();

  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(nodes->GetItemAsObject(n));
    if (node && !node->IsA("vtkMRMLSceneViewNode"))
      {
      this->CopyNodeInUndoStack(node);
      }
    }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::SaveStateForUndo ()
{
  if (!this->UndoFlag)
    {
    return;
    }

  if (this->IsBatchProcessing())
    {
    return;
    }
  if (this->Nodes)
    {
    this->SaveStateForUndo(this->Nodes);
    }
}

//------------------------------------------------------------------------------
// Make a new collection that has pointers to all the nodes in the current scene
void vtkMRMLScene::PushIntoUndoStack()
{
  if (this->Nodes == NULL)
    {
    return;
    }


  vtkCollection* newScene = vtkCollection::New();

  vtkCollection* currentScene = this->Nodes;

  int nnodes = currentScene->GetNumberOfItems();

  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(currentScene->GetItemAsObject(n));
    if (node && !node->IsA("vtkMRMLSceneViewNode"))
      {
      newScene->AddItem(node);
      }
    }

  //TODO check max stack size
  this->UndoStack.push_back(newScene);
}

//------------------------------------------------------------------------------
// Make a new collection that has pointers to the current scene nodes
void vtkMRMLScene::PushIntoRedoStack()
{
  if (this->Nodes == NULL)
    {
    return;
    }

  vtkCollection* newScene = vtkCollection::New();

  vtkCollection* currentScene = this->Nodes;

  int nnodes = currentScene->GetNumberOfItems();

  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(currentScene->GetItemAsObject(n));
    if (node && !node->IsA("vtkMRMLSceneViewNode"))
      {
      newScene->AddItem(node);
      }
    }

  //TODO check max stack size
  this->RedoStack.push_back(newScene);
}

//------------------------------------------------------------------------------
// Put a replacement node into the undoable copy of the scene so that the node
// can be edited
void vtkMRMLScene::CopyNodeInUndoStack(vtkMRMLNode *copyNode)
{
  if (!copyNode)
    {
    vtkErrorMacro("CopyNodeInUndoStack: node is null");
    return;
    }

  vtkMRMLNode *snode = copyNode->CreateNodeInstance();
  if (snode != NULL)
    {
    snode->CopyWithSceneWithoutModifiedEvent(copyNode);
    }
  vtkCollection* undoScene = dynamic_cast < vtkCollection *>( this->UndoStack.back() );
  int nnodes = undoScene->GetNumberOfItems();
  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(undoScene->GetItemAsObject(n));
    if (node == copyNode)
      {
      undoScene->ReplaceItem (n, snode);
      break;
      }
    }
  snode->Delete();
}

//------------------------------------------------------------------------------
// Put a replacement node into the redoable copy of the scene so that the node
// can be replaced by the Undo version
void vtkMRMLScene::CopyNodeInRedoStack(vtkMRMLNode *copyNode)
{
  if (!copyNode)
    {
    vtkErrorMacro("CopyNodeInRedoStack: node is null");
    return;
    }
  vtkMRMLNode *snode = copyNode->CreateNodeInstance();
  if (snode != NULL)
    {
    snode->CopyWithSceneWithSingleModifiedEvent(copyNode);
    }
  vtkCollection* undoScene = dynamic_cast < vtkCollection *>( this->RedoStack.back() );
  int nnodes = undoScene->GetNumberOfItems();
  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(undoScene->GetItemAsObject(n));
    if (node == copyNode)
      {
      undoScene->ReplaceItem (n, snode);
      break;
      }
    }
  snode->Delete();
}

//------------------------------------------------------------------------------
// Replace the current scene by the top of the undo stack
// -- move the current scene on the redo stack
void vtkMRMLScene::Undo()
{
  if (!this->UndoFlag)
    {
    return;
    }

  if (this->UndoStack.size() == 0)
    {
    return;
    }

  this->RemoveUnusedNodeReferences();

  this->InUndo = true;

  int nnodes;
  int n;
  unsigned int nn;

  PushIntoRedoStack();

  vtkCollection* currentScene = this->Nodes;
  // We use 2 vectors instead of a map in order to keep the ordering of the
  // nodes.
  std::vector<std::string> currentIDs;
  std::vector<vtkMRMLNode*> currentNodes;
  nnodes = currentScene->GetNumberOfItems();
  for (n=0; n<nnodes; n++)
    {
    vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(currentScene->GetItemAsObject(n));
    if (node && !node->IsA("vtkMRMLSceneViewNode"))
      {
      currentIDs.push_back(node->GetID());
      currentNodes.push_back(node);
      }
    }

  vtkCollection* undoScene = NULL;
  std::vector<std::string> undoIDs;
  std::vector<vtkMRMLNode*> undoNodes;

  if (!this->UndoStack.empty())
    {
    undoScene = dynamic_cast < vtkCollection *>( this->UndoStack.back() );
    nnodes = undoScene->GetNumberOfItems();
    for (n=0; n<nnodes; n++)
      {
      vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(undoScene->GetItemAsObject(n));
      if (node && !node->IsA("vtkMRMLSceneViewNode"))
        {
        undoIDs.push_back(node->GetID());
        undoNodes.push_back(node);
        }
      }
    }

  std::vector<std::string>::iterator iterID;
  std::vector<vtkMRMLNode*>::iterator iterNode;
  std::vector<std::string>::iterator curIterID;
  std::vector<vtkMRMLNode*>::iterator curIterNode;

  // copy back changes and add deleted nodes to the current scene
  std::vector<vtkMRMLNode*> addNodes;

  for(iterID=undoIDs.begin(), iterNode = undoNodes.begin(); iterID != undoIDs.end(); iterID++, iterNode++)
    {
    curIterID = std::find(currentIDs.begin(), currentIDs.end(), *iterID);
    curIterNode = currentNodes.begin() + std::distance(currentIDs.begin(), curIterID);
    if ( curIterID == currentIDs.end() )
      {
      // the node was deleted, add Node back to the curreent scene
      addNodes.push_back(*iterNode);
      }
    else if (*iterNode != *curIterNode)
      {
      // nodes differ, copy from undo to current scene
      // but before create a copy in redo stack from current
      this->CopyNodeInRedoStack(*curIterNode);
      (*curIterNode)->CopyWithSceneWithSingleModifiedEvent(*iterNode);
      }
    }

  // remove new nodes created before Undo
  std::vector<vtkMRMLNode*> removeNodes;
  for(curIterID=currentIDs.begin(), curIterNode = currentNodes.begin(); curIterID != currentIDs.end(); curIterID++, curIterNode++)
    {
    iterID = std::find(undoIDs.begin(),undoIDs.end(), *curIterID);
    // Remove only if the node is not present in the previous state.
    if ( iterID == undoIDs.end() )
      {
      removeNodes.push_back(*curIterNode);
      }
    }

  for (nn=0; nn<addNodes.size(); nn++)
    {
    this->AddNode(addNodes[nn]);
    }
  for (nn=0; nn<removeNodes.size(); nn++)
    {
    vtkMRMLNode* nodeToRemove = removeNodes[nn];
    // Maybe the node has been removed already by a side effect of a previous
    // node removal.
    if (this->IsNodePresent(nodeToRemove))
      {
      this->RemoveNode(nodeToRemove);
      }
    }

  if (undoScene)
    {
    undoScene->RemoveAllItems();
    undoScene->Delete();
    }

  this->RemoveUnusedNodeReferences();

  if (!this->UndoStack.empty())
   {
   UndoStack.pop_back();
   }
  this->Modified();

  this->InUndo = false;
}

//------------------------------------------------------------------------------
void vtkMRMLScene::Redo()
{
  if (!this->UndoFlag)
    {
    return;
    }

  if (this->RedoStack.size() == 0)
    {
    return;
    }

  int nnodes;
  int n;
  unsigned int nn;

  this->RemoveUnusedNodeReferences();

  PushIntoUndoStack();

  vtkCollection* currentScene = this->Nodes;
  //std::hash_map<std::string, vtkMRMLNode*> currentMap;
  std::map<std::string, vtkMRMLNode*> currentMap;
  nnodes = currentScene->GetNumberOfItems();
  for (n=0; n<nnodes; n++)
    {
    vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(currentScene->GetItemAsObject(n));
    if (node && !node->IsA("vtkMRMLSceneViewNode"))
      {
      currentMap[node->GetID()] = node;
      }
    }

  //std::hash_map<std::string, vtkMRMLNode*> undoMap;
  std::map<std::string, vtkMRMLNode*> undoMap;

  vtkCollection* undoScene = NULL;

  if (!this->RedoStack.empty())
    {
    undoScene = dynamic_cast < vtkCollection *>( this->RedoStack.back() );;
    nnodes = undoScene->GetNumberOfItems();
    for (n=0; n<nnodes; n++)
      {
      vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(undoScene->GetItemAsObject(n));
      if (node && !node->IsA("vtkMRMLSceneViewNode"))
        {
        undoMap[node->GetID()] = node;
        }
      }
    }

  //std::hash_map<std::string, vtkMRMLNode*>::iterator iter;
  //std::hash_map<std::string, vtkMRMLNode*>::iterator curIter;
  std::map<std::string, vtkMRMLNode*>::iterator iter;
  std::map<std::string, vtkMRMLNode*>::iterator curIter;

  // copy back changes and add deleted nodes to the current scene
  std::vector<vtkMRMLNode*> addNodes;

  for(iter=undoMap.begin(); iter != undoMap.end(); iter++)
    {
    curIter = currentMap.find(iter->first);
    if ( curIter == currentMap.end() )
      {
      // the node was deleted, add Node back to the curreent scene
      addNodes.push_back(iter->second);
      }
    else if (iter->second != curIter->second)
      {
      // nodes differ, copy from redo to current scene
      // but before create a copy in undo stack from current
      this->CopyNodeInUndoStack(curIter->second);
      curIter->second->CopyWithSceneWithSingleModifiedEvent(iter->second);
      }
    }

  // remove new nodes created before Undo
  std::vector<vtkMRMLNode*> removeNodes;
  for(curIter=currentMap.begin(); curIter != currentMap.end(); curIter++)
    {
    iter = undoMap.find(curIter->first);
    if ( iter == undoMap.end() )
      {
      removeNodes.push_back(curIter->second);
      }
    }

  for (nn=0; nn<addNodes.size(); nn++)
    {
    this->AddNode(addNodes[nn]);
    }
  for (nn=0; nn<removeNodes.size(); nn++)
    {
    this->RemoveNode(removeNodes[nn]);
    }

  if (undoScene)
    {
    undoScene->RemoveAllItems();
    undoScene->Delete();
    }

  RedoStack.pop_back();

  this->Modified();
}

//------------------------------------------------------------------------------
void vtkMRMLScene::ClearUndoStack()
{
  std::list< vtkCollection* >::iterator iter;
  for(iter=this->UndoStack.begin(); iter != this->UndoStack.end(); iter++)
    {
    (*iter)->RemoveAllItems();
    (*iter)->Delete();
    }
  this->UndoStack.clear();
}

//------------------------------------------------------------------------------
void vtkMRMLScene::ClearRedoStack()
{
  std::list< vtkCollection* >::iterator iter;
  for(iter=this->RedoStack.begin(); iter != this->RedoStack.end(); iter++)
    {
    (*iter)->RemoveAllItems();
    (*iter)->Delete();
    }
  this->RedoStack.clear();
}

//------------------------------------------------------------------------------
void vtkMRMLScene::AddReferencedNodeID(const char *id, vtkMRMLNode *referencingNode)
{
  if (id && referencingNode &&
      referencingNode->GetScene() && referencingNode->GetID() &&
      !this->IsNodeReferencingNodeID(referencingNode, id))
    {
    this->ReferencedIDs.push_back(id);
    this->ReferencingNodes.push_back(referencingNode);
    }
}

//------------------------------------------------------------------------------
bool vtkMRMLScene::IsNodeReferencingNodeID(vtkMRMLNode* referencingNode, const char* id)
{
  std::vector<std::string>::iterator idIt = this->ReferencedIDs.begin();
  std::vector< vtkSmartPointer<vtkMRMLNode> >::iterator nodeIt = this->ReferencingNodes.begin();
  for ( ; idIt != this->ReferencedIDs.end(); ++idIt, ++nodeIt)
    {
    if (*idIt == std::string(id) &&
        *nodeIt == referencingNode)
      {
      return true;
      }
    }
  return false;
}

//------------------------------------------------------------------------------
int vtkMRMLScene::IsFilePathRelative(const char * filepath)
{
  if (filepath == NULL)
    {
    vtkErrorMacro("IsFilePathRelative: file path is null");
    return 0;
    }

  // check for shared memory objects
  if (strncmp("slicer:", filepath, 7) == 0)
    {
    return 0;
    }

  // check for remote files, assume they're absolute paths
  if (this->GetCacheManager() != NULL)
    {
    if (this->GetCacheManager()->IsRemoteReference(filepath))
      {
      return 0;
      }
    }

  std::vector<std::string> components;
  vtksys::SystemTools::SplitPath((const char*)filepath, components);
  if (components[0] == "")
    {
    return 1;
    }
  else
    {
    return 0;
    }

}

//------------------------------------------------------------------------------
const char* vtkMRMLScene::GetChangedID(const char* id)
{
  std::map< std::string, std::string>::const_iterator iter = this->ReferencedIDChanges.find(std::string(id));
  if (iter == this->ReferencedIDChanges.end())
    {
    return NULL;
    }
  else
    {
    return iter->second.c_str();
    }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::UpdateNodeReferences()
{
  std::map< std::string, std::string>::const_iterator iterChanged;
  //std::map< std::string, vtkMRMLNode*>::const_iterator iterNodes;

  for (iterChanged = this->ReferencedIDChanges.begin(); iterChanged != this->ReferencedIDChanges.end(); iterChanged++)
    {
    std::vector< std::string > referencedIDs = this->ReferencedIDs;
    std::vector< vtkSmartPointer<vtkMRMLNode> > referencingNodes = this->ReferencingNodes;

    size_t nupdates = referencedIDs.size();
    for ( size_t i=0; i<nupdates; ++i)
      {
      if (iterChanged->first == referencedIDs[i])
        {
        vtkMRMLNode *node = this->GetNodeByID(referencingNodes[i]->GetID());
        if (node)
          {
          node->UpdateReferenceID(iterChanged->first.c_str(), iterChanged->second.c_str());
          }
        }
    }
    this->ReferencedIDs = referencedIDs;
    this->ReferencingNodes = referencingNodes;
  }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::UpdateNodeReferences(vtkCollection* checkNodes)
{
  if (!checkNodes)
    {
    vtkErrorMacro("UpdateNodeReferences: no nodes to check");
    return;
    }
  std::map< std::string, std::string>::const_iterator iterChanged;
  //std::map< std::string, vtkMRMLNode*>::const_iterator iterNodes;

  for (iterChanged = this->ReferencedIDChanges.begin(); iterChanged != this->ReferencedIDChanges.end(); iterChanged++)
    {
    std::vector< std::string > referencedIDs = this->ReferencedIDs;
    std::vector< vtkSmartPointer<vtkMRMLNode> > referencingNodes = this->ReferencingNodes;
    size_t nupdates = referencedIDs.size();

    for (size_t i=0; i<nupdates; ++i)
      {
      if (iterChanged->first == referencedIDs[i])
        {
          vtkMRMLNode *node = referencingNodes[i];
        if (checkNodes->IsItemPresent(node))
          {
          node->UpdateReferenceID(iterChanged->first.c_str(), iterChanged->second.c_str());
          }
        }
      }
    this->ReferencedIDs = referencedIDs;
    this->ReferencingNodes = referencingNodes;
  }

}

//------------------------------------------------------------------------------
void vtkMRMLScene::AddReferencedNodes(vtkMRMLNode *node, vtkCollection *refNodes)
{
  if (!node || !refNodes)
    {
    vtkErrorMacro("AddReferencedNodes: null node or reference nodes");
    return;
    }
  vtkMRMLNode *rnode = NULL;
  size_t nnodes = this->ReferencingNodes.size();
  std::vector< std::string > ids;
  ids.reserve(nnodes);
  for(size_t n=0; n<nnodes; ++n)
    {
    vtkMRMLNode *rrnode = this->ReferencingNodes[n];
    if (rrnode && rrnode->GetID() && !strcmp(rrnode->GetID(), node->GetID()))
      {
      ids.push_back(ReferencedIDs[n]);
      }
    }
  nnodes = ids.size();
  for (size_t n=0; n<nnodes; ++n)
    {
    rnode = this->GetNodeByID(ids[n]);
    if (rnode != NULL && !refNodes->IsItemPresent(rnode))
      {
      refNodes->AddItem(rnode);
      this->AddReferencedNodes(rnode, refNodes);
      }
    }
}

//------------------------------------------------------------------------------
vtkCollection* vtkMRMLScene::GetReferencedNodes(vtkMRMLNode *node)
{
  vtkCollection* nodes = vtkCollection::New();
  if (node != NULL)
    {
    nodes->AddItem(node);
    this->AddReferencedNodes(node, nodes);
    }
  return nodes;
}

//-----------------------------------------------------------------------------
void vtkMRMLScene::GetReferencingNodes(vtkMRMLNode* referencedNode, std::vector<vtkMRMLNode *> &referencingNodes)
{
  referencingNodes.clear();

  if (!referencedNode || referencedNode->GetID() == 0)
    {
    vtkErrorMacro("GetReferencingNodes: null node or referenced node");
    return;
    }

  size_t nnodes = this->ReferencedIDs.size();
  for(size_t n=0; n<nnodes; ++n)
    {
    if ( !strcmp(referencedNode->GetID(), this->ReferencedIDs[n].c_str()))
      {
      referencingNodes.push_back(this->ReferencingNodes[n]);
      }
    }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::CopyNodeReferences(vtkMRMLScene *scene)
{
  if (!scene)
    {
    return;
    }

  this->ReferencedIDChanges = scene->ReferencedIDChanges;

  //assuming the nodes exist in this scene
  for (unsigned int i=0; i < scene->ReferencedIDs.size(); i++)
    {
    vtkMRMLNode *referencingNode = scene->ReferencingNodes[i];
    const char *id = referencingNode->GetID();
    vtkMRMLNode *node = this->GetNodeByID(id);
    if (node)
      {
      this->AddReferencedNodeID(scene->ReferencedIDs[i].c_str(), node);
      }
    }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::UpdateNodeChangedIDs()
{
  std::map< std::string, std::string>::const_iterator iterChanged;
  vtkMRMLNode *node;

  for (iterChanged = this->ReferencedIDChanges.begin(); iterChanged != this->ReferencedIDChanges.end(); iterChanged++)
    {
    if (iterChanged->first.c_str() && iterChanged->first != "")
      {
      node = this->GetNodeByID(iterChanged->first.c_str());
      if (node)
        {
        node->SetID(iterChanged->second.c_str());
        }
      }
    }
  this->NodeIDsMTime = 0;
  this->UpdateNodeIDs();
}

//------------------------------------------------------------------------------
void vtkMRMLScene::UpdateNodeIDs()
{
  if (this->Nodes->GetNumberOfItems() == 0)
    {
    this->ClearNodeIDs();
    }
  else if (this->Nodes->GetMTime() > this->NodeIDsMTime)
    {
    if (this->NodeIDsMTime > 0)
      {
      // TODO: we should get rid of NodeIDsMTime: the map should always be up
      // to date.
      vtkWarningMacro("There is a danger here. What if AddNode or RemoveNode"
                      " were called prior, the NodeIDsMTime would be in sync"
                      " without having the map in sync.");
      }
    this->ClearNodeIDs();
#ifdef MRMLSCENE_VERBOSE
    std::cerr << "Recompute node id cache..." << std::endl;
#endif
    vtkMRMLNode *node;
    vtkCollectionSimpleIterator it;
    for (this->Nodes->InitTraversal(it);
         (node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject(it)) ;)
      {
      if (node->GetID())
        {
        this->AddNodeID(node);
        }
      }
    }
}

void vtkMRMLScene::AddNodeID(vtkMRMLNode *node)
{
  if (this->Nodes && node && node->GetID())
    {
    this->NodeIDs[std::string(node->GetID())] = node;
    this->NodeIDsMTime = this->Nodes->GetMTime();
    }
}

void vtkMRMLScene::RemoveNodeID(char *nodeID)
{
  if (this->Nodes && nodeID)
    {
    this->NodeIDs.erase(std::string(nodeID));
    this->NodeIDsMTime = this->Nodes->GetMTime();
    }
}


void vtkMRMLScene::ClearNodeIDs()
{
  if (this->Nodes)
    {
    this->NodeIDs.clear();
    this->NodeIDsMTime = this->Nodes->GetMTime();    
  }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::AddURIHandler(vtkURIHandler *handler)
{
  if (this->GetURIHandlerCollection() == NULL)
    {
    return;
    }
  if (handler == NULL)
    {
    return;
    }
  this->GetURIHandlerCollection()->AddItem(handler);
}

//------------------------------------------------------------------------------
vtkURIHandler * vtkMRMLScene::FindURIHandlerByName(const char *name)
{
  vtkURIHandler *u;
  if ( name == NULL )
    {
    vtkErrorMacro("FindURIHandlerByName: name is null.");
    return NULL;
    }
  if (this->GetURIHandlerCollection() == NULL)
    {
    vtkWarningMacro("FindURIHandlerByName: No URI handlers registered on the scene.");
    return NULL;
    }
  for (int i = 0; i < this->GetURIHandlerCollection()->GetNumberOfItems(); i++)
    {
    vtkObject *object = this->GetURIHandlerCollection()->GetItemAsObject(i);
    if (object == NULL)
      {
      vtkErrorMacro("FindURIHandlerByName: got a null handler at index " << i);
      return NULL;
      }
    u = vtkURIHandler::SafeDownCast(object);
    if ( u == NULL )
      {
      vtkErrorMacro("FindURIHandlerByName: Got NULL URIHandler from URIHandlerCollection." );
      return NULL;
      }
    if (  !strcmp (u->GetName(), name ) )
      {
      vtkDebugMacro("FindURIHandlerByName: found a handler with name " << name << " at index " << i << " in the handler collection");
      return u;
      }
    }
  vtkWarningMacro("FindURIHandlerByName: unable to find a URI handler in the collection of " << this->GetURIHandlerCollection()->GetNumberOfItems() << " handlers to match the name " << name);
  return NULL;

}


//------------------------------------------------------------------------------
vtkURIHandler * vtkMRMLScene::FindURIHandler(const char *URI)
{
  if (URI == NULL)
    {
    vtkErrorMacro("FindURIHandler: URI is null.");
    return NULL;
    }
  if (this->GetURIHandlerCollection() == NULL)
    {
    vtkWarningMacro("No URI handlers registered on the scene.");
    return NULL;
    }
  const int numberOfItems = this->GetURIHandlerCollection()->GetNumberOfItems();
  for (int i = 0; i < numberOfItems; i++)
    {
    if (vtkURIHandler::SafeDownCast(this->GetURIHandlerCollection()->GetItemAsObject(i)) &&
        vtkURIHandler::SafeDownCast(this->GetURIHandlerCollection()->GetItemAsObject(i))->CanHandleURI(URI))
      {
      vtkDebugMacro("FindURIHandler: found a handler for URI " << URI << " at index " << i << " in the handler collection");
      return vtkURIHandler::SafeDownCast(this->GetURIHandlerCollection()->GetItemAsObject(i));
      }
    }
  vtkWarningMacro("FindURIHandler: unable to find a URI handler in the collection of " << this->GetURIHandlerCollection()->GetNumberOfItems() << " handlers to handle " << URI);
  return NULL;
}


//-----------------------------------------------------------------------------
void vtkMRMLScene
::GetReferencedSubScene(vtkMRMLNode *rnode, vtkMRMLScene* newScene)
{
  //
  // clear scene
  //
  newScene->Clear(1);


  if (rnode == NULL)
    {
    return;
    }

  // to get all references up-to-date
  // save the scene into a string
  // and restore it into a new scene
  this->SetSaveToXMLString(1);
  this->Commit();
  this->CopyRegisteredNodesToScene(newScene);
  newScene->SetSceneXMLString(this->GetSceneXMLString());
  newScene->SetLoadFromXMLString(1);

  std::string url = std::string(newScene->GetURL());
  std::string root = std::string(newScene->GetRootDirectory());
  int loadDataOld = newScene->GetReadDataOnLoad();

  newScene->SetURL(this->GetURL());
  newScene->SetRootDirectory(this->GetRootDirectory());
  newScene->SetReadDataOnLoad(0);

  // copy singleton nodes from existing scene
  this->CopySingletonNodesToScene(newScene);

  // load new scene
  newScene->Connect();

  this->SetSaveToXMLString(0);

  newScene->SetLoadFromXMLString(0);
  newScene->SetURL(url.c_str());
  newScene->SetRootDirectory(root.c_str());
  newScene->SetReadDataOnLoad(loadDataOld);

  //
  // get all nodes associated with this node
  vtkCollection* nodes = newScene->GetReferencedNodes(rnode);

  //
  // clear scene
  //
  newScene->Clear(1);


  //
  // copy over nodes from the current scene to the new scene
  //
  nodes->InitTraversal();
  vtkObject* currentObject = NULL;
  while ((currentObject = nodes->GetNextItemAsObject()) &&
         (currentObject != NULL))
    {
    vtkMRMLNode* n = vtkMRMLNode::SafeDownCast(currentObject);
    if (n == NULL)
      {
      continue;
      }

    vtkMRMLNode* node = n->CreateNodeInstance();
    vtkMRMLNode* originalNode = this->GetNodeByID(n->GetID());

    if (originalNode)
      {
      node->Copy(originalNode);
      node->SetID(originalNode->GetID());
      }

    // add the nodes to the scene
    newScene->AddNodeNoNotify(node);
    node->Delete();
    }

  // clean up
  nodes->Delete();
}

//-----------------------------------------------------------------------------
const std::vector< vtkSmartPointer<vtkMRMLNode> >& vtkMRMLScene::GetReferencingNodes()
{
  return this->ReferencingNodes;
}

//-----------------------------------------------------------------------------
const std::vector< std::string >& vtkMRMLScene::GetReferencedIDs()
{
  return this->ReferencedIDs;
}

//-----------------------------------------------------------------------------
unsigned long vtkMRMLScene::GetSceneModifiedTime()
{
  if (this->Nodes->GetMTime() > this->SceneModifiedTime)
    {
    this->SceneModifiedTime = this->Nodes->GetMTime();
    }
  return this->SceneModifiedTime;
}

//-----------------------------------------------------------------------------
bool vtkMRMLScene::GetModifiedSinceRead()
{
  int hideFromEditors = 0;
  bool hasAtLeast1DisplayableNode =
    (this->GetFirstNode(0, "vtkMRMLDisplayableNode", &hideFromEditors) != 0);
  return this->GetMTime() > this->StoredTime &&
    // There is no need to save the scene if it just has view nodes
    hasAtLeast1DisplayableNode;
}

//-----------------------------------------------------------------------------
bool vtkMRMLScene
::GetStorableNodesModifiedSinceRead(vtkCollection* modifiedStorableNodes)
{
  bool found = false;
  vtkSmartPointer<vtkCollection> storableNodes;
  storableNodes.TakeReference(
    this->GetNodesByClass("vtkMRMLStorableNode"));
  vtkCollectionSimpleIterator it;
  vtkMRMLStorableNode* storableNode;
  for (storableNodes->InitTraversal(it);
       (storableNode= vtkMRMLStorableNode::SafeDownCast(
          storableNodes->GetNextItemAsObject(it))) ;)
    {
    if (!storableNode->GetHideFromEditors() &&
         storableNode->GetModifiedSinceRead())
      {
      found = true;
      if (modifiedStorableNodes)
        {
        modifiedStorableNodes->AddItem(storableNode);
        }
      }
    }
  return found;
}
