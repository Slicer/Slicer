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
#include "vtkMRMLCrosshairNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLDoubleArrayNode.h"
#include "vtkMRMLDoubleArrayStorageNode.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLFiducialListStorageNode.h"
#include "vtkMRMLFolderDisplayNode.h"
#include "vtkMRMLFreeSurferModelOverlayStorageNode.h"
#include "vtkMRMLFreeSurferModelStorageNode.h"
#include "vtkMRMLGridTransformNode.h"
#include "vtkMRMLHierarchyStorageNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLLabelMapVolumeDisplayNode.h"
#include "vtkMRMLLabelMapVolumeNode.h"
#include "vtkMRMLLayoutNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLPlotChartNode.h"
#include "vtkMRMLPlotSeriesNode.h"
#include "vtkMRMLPlotViewNode.h"
#include "vtkMRMLProceduralColorNode.h"
#include "vtkMRMLProceduralColorStorageNode.h"
#include "vtkMRMLROIListNode.h"
#include "vtkMRMLROINode.h"
#include "vtkMRMLScriptedModuleNode.h"
#include "vtkMRMLSegmentationDisplayNode.h"
#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentationStorageNode.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLSnapshotClipNode.h"
#include "vtkMRMLSubjectHierarchyNode.h"
#include "vtkMRMLTableNode.h"
#include "vtkMRMLTableStorageNode.h"
#include "vtkMRMLTableViewNode.h"
#include "vtkMRMLTextNode.h"
#include "vtkMRMLTextStorageNode.h"
#include "vtkMRMLTransformDisplayNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLTransformStorageNode.h"
#include "vtkMRMLVectorVolumeDisplayNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkURIHandler.h"

#ifdef MRML_USE_vtkTeem
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLDiffusionTensorVolumeSliceDisplayNode.h"
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMRMLStreamingVolumeNode.h"
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
#include <vtksys/RegularExpression.hxx>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <algorithm>
#include <numeric>

//#define MRMLSCENE_VERBOSE

#ifdef MRMLSCENE_VERBOSE
# include <vtkTimerLog.h>
#endif

vtkCxxSetObjectMacro(vtkMRMLScene, CacheManager, vtkCacheManager)
vtkCxxSetObjectMacro(vtkMRMLScene, DataIOManager, vtkDataIOManager)
vtkCxxSetObjectMacro(vtkMRMLScene, UserTagTable, vtkTagTable)
vtkCxxSetObjectMacro(vtkMRMLScene, URIHandlerCollection, vtkCollection)

//------------------------------------------------------------------------------
vtkMRMLScene::vtkMRMLScene()
{
  this->NodeIDsMTime = 0;

  this->RegisteredNodeClasses.clear();
  this->UniqueIDs.clear();
  this->UniqueNames.clear();

  this->Nodes =  vtkCollection::New();
  this->MaximumNumberOfSavedUndoStates = 20;
  this->UndoFlag = false;

  this->NodeReferences.clear();
  this->ReferencedIDChanges.clear();

  this->CacheManager = nullptr;
  this->DataIOManager = nullptr;
  this->URIHandlerCollection = nullptr;
  this->UserTagTable = nullptr;

  this->ErrorCode = 0;

  this->LoadFromXMLString = 0;

  this->SaveToXMLString = 0;

  this->ReadDataOnLoad = 1;

  this->LastLoadedVersion = nullptr;
  this->Version = nullptr;
  this->SetVersion(CURRENT_MRML_VERSION);

  this->DeleteEventCallback = vtkCallbackCommand::New();
  this->DeleteEventCallback->SetClientData( reinterpret_cast<void *>(this) );
  this->DeleteEventCallback->SetCallback( vtkMRMLScene::SceneCallback );
  // we want to be first to catch the event, so that SceneAboutToBeClosedEvent,
  // NodeRemovedEvent and SceneCloseddEvent are fired and caught before DeleteEvent
  // is caught by other observers.
  this->AddObserver(vtkCommand::DeleteEvent, this->DeleteEventCallback, 1000.);

  //
  // Register all the 'built-in' nodes for the library
  // SmartPointer is used to create an instance of the class, and destroy immediately after registration is complete.
  // - note: the scene will maintain a registered pointer to the nodes,
  //   so we delete them here (same should be done by any module that
  //   creates nodes).
  //

  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLScalarVolumeNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLModelNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLLinearTransformNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLModelStorageNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLFreeSurferModelStorageNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLFreeSurferModelOverlayStorageNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLModelDisplayNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLClipModelsNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLFiducialListNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLFiducialListStorageNode >::New() );
  this->RegisterNodeClass(vtkSmartPointer< vtkMRMLFolderDisplayNode >::New());
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLROINode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLROIListNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLSliceCompositeNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLScriptedModuleNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLSegmentationDisplayNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLSegmentationNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLSegmentationStorageNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLSelectionNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLSliceNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLVolumeArchetypeStorageNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLScalarVolumeDisplayNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLLabelMapVolumeDisplayNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLLabelMapVolumeNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLColorNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLDiffusionWeightedVolumeNode >::New() );
#ifdef MRML_USE_vtkTeem
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLDiffusionTensorVolumeNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLDiffusionTensorVolumeDisplayNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLDiffusionTensorVolumeSliceDisplayNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLNRRDStorageNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLVectorVolumeNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLStreamingVolumeNode >::New() );
#endif
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLVectorVolumeDisplayNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLDiffusionWeightedVolumeDisplayNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLDiffusionTensorDisplayPropertiesNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLCameraNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLViewNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLLayoutNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLHierarchyNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLHierarchyStorageNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLDisplayableHierarchyNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLModelHierarchyNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLSnapshotClipNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLColorTableNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLColorTableStorageNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLProceduralColorNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLProceduralColorStorageNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLTransformDisplayNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLTransformStorageNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLTransformNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLGridTransformNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLBSplineTransformNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLDoubleArrayNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLDoubleArrayStorageNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLCrosshairNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLInteractionNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLChartNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLChartViewNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLTableNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLTableStorageNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLTableViewNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLTextNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLTextStorageNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLPlotSeriesNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLPlotChartNode >::New() );
  this->RegisterNodeClass( vtkSmartPointer< vtkMRMLPlotViewNode >::New() );
  this->RegisterNodeClass(vtkSmartPointer<vtkMRMLSubjectHierarchyNode>::New()); // Increments next subject hierarchy item ID
}

//------------------------------------------------------------------------------
vtkMRMLScene::~vtkMRMLScene()
{
  this->ClearUndoStack ( );
  this->ClearRedoStack ( );

  if ( this->Nodes != nullptr )
    {
    if (this->Nodes->GetNumberOfItems() > 0)
      {
      vtkDebugMacro("CurrentScene should have already been cleared in DeleteEvent callback: ");
      this->Nodes->RemoveAllItems ( );
      }
    this->Nodes->Delete();
    this->Nodes = nullptr;
    }

  for (unsigned int n=0; n<this->RegisteredNodeClasses.size(); n++)
    {
    this->RegisteredNodeClasses[n]->Delete();
    }

  if ( this->CacheManager != nullptr )
    {
    this->CacheManager->Delete();
    this->CacheManager = nullptr;
    }
  if ( this->DataIOManager != nullptr )
    {
    this->DataIOManager->Delete();
    this->DataIOManager = nullptr;
    }
  if ( this->URIHandlerCollection != nullptr )
    {
    this->URIHandlerCollection->RemoveAllItems();
    this->URIHandlerCollection->Delete();
    this->URIHandlerCollection = nullptr;
    }
  if ( this->UserTagTable != nullptr )
    {
    this->UserTagTable->Delete();
    this->UserTagTable = nullptr;
    }
  this->SetLastLoadedVersion(nullptr);
  this->SetVersion(nullptr);
  if ( this->DeleteEventCallback != nullptr )
    {
    this->DeleteEventCallback->Delete();
    this->DeleteEventCallback = nullptr;
    }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::SceneCallback( vtkObject *vtkNotUsed(caller),
                                  unsigned long vtkNotUsed(eid),
                                  void *clientData, void *vtkNotUsed(callData) )
{
  vtkMRMLScene *self = reinterpret_cast<vtkMRMLScene *>(clientData);
  if (self == nullptr)
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

  this->RemoveAllNodes(removeSingletons);
  this->NodeReferences.clear();
  this->ReferencedIDChanges.clear();
  this->ResetNodes();

  this->ClearUndoStack ( );
  this->ClearRedoStack ( );
  this->UniqueIDs.clear();
  this->UniqueNames.clear();

  if ( this->GetUserTagTable() != nullptr )
    {
    this->GetUserTagTable()->ClearTagTable();
    }

  this->Modified();
  this->SetUndoFlag(undoFlag);

  // SceneAboutToBeClosedEvent
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
void vtkMRMLScene::RemoveAllNodes(bool removeSingletons)
{

  // Store the node ids because a module may decide to delete some helper nodes
  // when a node is deleted
  std::deque< std::string > removeNodeIds;
  vtkMRMLNode *node = nullptr;
  vtkCollectionSimpleIterator it;
  for (this->Nodes->InitTraversal(it);
    (node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject(it));)
    {
    if (removeSingletons || node->GetSingletonTag() == nullptr)
      {
      removeNodeIds.push_back(node->GetID());
      }
    }
  for(std::deque< std::string >::iterator nodeIt=removeNodeIds.begin(); nodeIt!=removeNodeIds.end(); ++nodeIt)
    {
    vtkMRMLNode* node=this->GetNodeByID(*nodeIt);
    if (node)
      {
      // node is still in the scene
      this->RemoveNode(node);
      }
    }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::ResetNodes()
{
  std::vector <vtkMRMLNode *> nodes;
  vtkMRMLNode *node = nullptr;
  vtkCollectionSimpleIterator it;
  for (this->Nodes->InitTraversal(it);
    (node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject(it));)
    {
    nodes.push_back(node);
    }
  for(unsigned int i=0; i<nodes.size(); i++)
    {
    nodes[i]->Reset(GetDefaultNodeByClass(nodes[i]->GetClassName()));
    }
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetDefaultNodeByClass(const char* className)
{
  if (className==nullptr)
    {
    return nullptr;
    }
  std::map< std::string, vtkSmartPointer<vtkMRMLNode> >::iterator it = this->DefaultNodes.find(std::string(className));
  if (it == this->DefaultNodes.end())
    {
    return nullptr;
    }
  return it->second;
}

//----------------------------------------------------------------------------
void vtkMRMLScene::AddDefaultNode(vtkMRMLNode* node)
{
  if (node==nullptr)
    {
    vtkErrorMacro("vtkMRMLScene::AddDefaultNode failed: invalid node");
    return;
    }
  const char* className = node->GetClassName();
  if (className==nullptr)
    {
    vtkErrorMacro("vtkMRMLScene::AddDefaultNode failed: invalid className");
    return;
    }
  this->DefaultNodes[std::string(className)] = node;
}

//----------------------------------------------------------------------------
void vtkMRMLScene::RemoveAllDefaultNodes()
{
  this->DefaultNodes.clear();
}

//------------------------------------------------------------------------------
void vtkMRMLScene::CopyDefaultNodesToScene(vtkMRMLScene *scene)
{
  if (!scene)
    {
    return;
    }
  for (std::map< std::string, vtkSmartPointer<vtkMRMLNode> >::iterator it = this->DefaultNodes.begin();
    it != this->DefaultNodes.end(); ++it)
    {
    scene->AddDefaultNode(it->second);
    }
}

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLScene);

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::CreateNodeByClass(const char* className)
{
  if (className == nullptr)
    {
    vtkErrorMacro("CreateNodeByClass: className is NULL");
    return nullptr;
    }
  vtkMRMLNode* node = nullptr;
  for (unsigned int i=0; i<RegisteredNodeClasses.size(); i++)
    {
    if (!strcmp(RegisteredNodeClasses[i]->GetClassName(), className))
      {
      node = RegisteredNodeClasses[i]->CreateNodeInstance();
      break;
      }
    }
  // non-registered nodes can have a registered factory
  if (node == nullptr)
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
#ifndef VTK_HAS_INITIALIZE_OBJECT_BASE
#ifdef VTK_DEBUG_LEAKS
      vtkDebugLeaks::DestructClass(className);
#endif
#endif
      }
    }
  // If a default node is specified for this class then initialize the
  // node contents with that
  vtkMRMLNode* defaultNode = this->GetDefaultNodeByClass(className);
  if (defaultNode)
    {
    node->Reset(defaultNode);
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
  // XML tag. It allows plugins to MRML to override default behavior when
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
    vtkMRMLNode* node = nullptr;
    for (unsigned int i=0; i<this->RegisteredNodeClasses.size(); i++)
      {
      node = this->RegisteredNodeClasses[i]->CreateNodeInstance();
      if (!scene->GetClassNameByTag(node->GetNodeTagName()))
        {
        scene->RegisterNodeClass(node);
        }
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
  vtkMRMLNode* node = nullptr;
  int n;
  for (n=0; n < this->Nodes->GetNumberOfItems(); n++)
    {
    node = (vtkMRMLNode*)this->Nodes->GetItemAsObject(n);
    if (node->GetSingletonTag() != nullptr)
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
  if (tagName == nullptr)
    {
    vtkErrorMacro("GetClassNameByTag: tagname is null");
    return nullptr;
    }
  for (unsigned int i=0; i<RegisteredNodeTags.size(); i++)
    {
    if (!strcmp(RegisteredNodeTags[i].c_str(), tagName))
      {
      return (RegisteredNodeClasses[i])->GetClassName();
      }
    }
  return nullptr;
}

//------------------------------------------------------------------------------
const char* vtkMRMLScene::GetTagByClassName(const char *className)
{
  if ( !className )
    {
    vtkErrorMacro("GetTagByClassName: className is null");
    return nullptr;
    }
  for (unsigned int i=0; i<this->RegisteredNodeClasses.size(); i++)
    {
    if (!strcmp(this->RegisteredNodeClasses[i]->GetClassName(), className))
      {
      return (this->RegisteredNodeClasses[i])->GetNodeTagName();
      }
    }
  return nullptr;
}

//------------------------------------------------------------------------------
vtkCollection* vtkMRMLScene::GetNodes()
{
  return this->Nodes;
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
  if (this->States.empty())
  {
    vtkErrorMacro("vtkMRMLScene::EndState failed: there was no previous state");
    return;
  }
  if (this->States.back() != state)
  {
    vtkWarningMacro("vtkMRMLScene::EndState found inconsistent state");
  }
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
  if (this->IsClosing())
    {
    vtkWarningMacro("vtkMRMLScene::Connect(): scene is in closing state");
    }
  if (this->IsImporting())
    {
    vtkWarningMacro("vtkMRMLScene::Connect(): scene is in importing state");
    }

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
  this->ReferencedIDChanges.clear();

  // read nodes into a temp scene
  vtkSmartPointer<vtkCollection> loadedNodes = vtkSmartPointer<vtkCollection>::New();

  int parsingSuccess = this->LoadIntoScene(loadedNodes);

  if (parsingSuccess)
    {
    /// In case the scene needs to change the ID of some nodes to add, the new
    /// ID should not be one already existing in the scene nor one of the
    /// imported scene.
    /// Mark all the node IDs of the scene as reserved so the node ID
    /// generator doesn't choose them.
    vtkMRMLNode *node=nullptr;
    vtkCollectionSimpleIterator it;
    for (loadedNodes->InitTraversal(it);
         (node = (vtkMRMLNode*)loadedNodes->GetNextItemAsObject(it)) ;)
      {
      this->AddReservedID(node->GetID());
      }
#ifdef MRMLSCENE_VERBOSE
    addNodesTimer->StartTimer();
#endif
    // Loaded node is not always the same the one that is actually added:
    // in case of singleton nodes the existing singleton node is kept
    // and only the contents is overwritten.
    vtkSmartPointer<vtkCollection> addedNodes = vtkSmartPointer<vtkCollection>::New();
    for (loadedNodes->InitTraversal(it);
         (node = (vtkMRMLNode*)loadedNodes->GetNextItemAsObject(it)) ;)
      {
      addedNodes->AddItem(this->AddNode(node));
      }
#ifdef MRMLSCENE_VERBOSE
    addNodesTimer->StopTimer();
    updateSceneTimer->StartTimer();
#endif
    // Update the node references to the changed node IDs
    // (that conflicted in the current scene and the imported scene)
    this->UpdateNodeReferences(addedNodes);
    this->RemoveReservedIDs();

    this->InvokeEvent(vtkMRMLScene::NewSceneEvent, nullptr);

    // Notify the imported nodes about that all nodes are created
    // (so the observers can be attached to referenced nodes, etc.)
    // by calling UpdateScene on each node
    for (addedNodes->InitTraversal(it);
         (node = (vtkMRMLNode*)addedNodes->GetNextItemAsObject(it)) ;)
      {
      //double progress = n / (1. * nnodes);
      //this->InvokeEvent(vtkCommand::ProgressEvent,(void *)&progress);
      vtkDebugMacro("Adding Node: " << (node->GetName() ? node->GetName() : "(undefined)"));
      if (node->GetAddToScene())
        {
        node->UpdateScene(this);
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
      }

    this->Modified();
    this->RemoveUnusedNodeReferences();
#ifdef MRMLSCENE_VERBOSE
    updateSceneTimer->StopTimer();
#endif
    }
  else
    {
    // parsing was not successful
    this->SetErrorMessage (std::string("Error parsing scene file"));
    this->ReferencedIDChanges.clear();
    }

  this->SetUndoFlag(undoFlag);

#ifdef MRMLSCENE_VERBOSE
  vtkTimerLog* importingTimer = vtkTimerLog::New();
  importingTimer->StartTimer();
#endif
  this->EndState(vtkMRMLScene::ImportState);
#ifdef MRMLSCENE_VERBOSE
  importingTimer->StopTimer();
#endif

  int returnCode = parsingSuccess; // nonzero = success
  if (this->GetErrorCode() != 0)
    {
    // error was reported, return with 0 (failure)
    returnCode = 0;
    }
#ifdef MRMLSCENE_VERBOSE
  timer->StopTimer();
  std::cerr << "vtkMRMLScene::Import()::AddNodes:" << addNodesTimer->GetElapsedTime() << std::endl;
  std::cerr << "vtkMRMLScene::Import()::UpdateScene" << updateSceneTimer->GetElapsedTime() << std::endl;
  std::cerr << "vtkMRMLScene::Import()::SceneImported:" << importingTimer->GetElapsedTime() << std::endl;
  std::cerr << "vtkMRMLScene::Import():" << timer->GetElapsedTime() << std::endl;
  addNodesTimer->Delete();
  updateSceneTimer->Delete();
  importingTimer->Delete();
  timer->Delete();
#endif
  this->StoredTime.Modified();

  // Once the import is finished, give the SH a chance to ensure consistency
  this->SetSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode::ResolveSubjectHierarchy(this));

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
      if (handler != nullptr)
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

  int result = 0; // 0 means failure
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
  if (url == nullptr)
    {
    if (this->URL != "")
      {
      url = this->URL.c_str();
      }
    else if (this->GetSaveToXMLString() == 0)
      {
      vtkErrorMacro("Commit: URL is not set");
      this->SetErrorCode(vtkErrorCode::GetErrorCodeFromString("CannotOpenFileError"));
      return 0;
      }
    }

  vtkMRMLNode *node;

  std::stringstream oss;
  std::ofstream ofs;

  std::ostream *os = nullptr;

  if (this->GetSaveToXMLString())
    {
    os = &oss;
    }
  else
    {
    os = &ofs;
    // set the root directory from the URL
    this->RootDirectory = vtksys::SystemTools::GetParentDirectory(url);

    // Open file
#ifdef _WIN32
    ofs.open(url, std::ios::out | std::ios::binary);
#else
    ofs.open(url, std::ios::out);
#endif
    if (ofs.fail())
      {
      vtkErrorMacro("Write: Could not open file " << url);
     this->SetErrorCode(vtkErrorCode::GetErrorCodeFromString("CannotOpenFileError"));
      return 0;
      }
    }

  int indent=0;

  // this event is being detected by GUI to provide feedback during load
  // of data. But, commented out for now because CLI modules are using MRML
  // to write data in another thread, causing GUI to crash.
  //this->InvokeEvent (vtkMRMLScene::SaveProgressFeedbackEvent );

  //file << "<?xml version=\"1.0\" standalone='no'?>\n";
  //file << "<!DOCTYPE MRML SYSTEM \"mrml20.dtd\">\n";

  // Add XML encoding specification. Slicer uses the UTF-8 character set.
  *os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

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
  if ( this->GetUserTagTable() != nullptr )
    {
    ss.clear();
    ss.str ( "" );
    int numc = this->GetUserTagTable()->GetNumberOfTags();
    const char *kwd, *val;
    for (int i=0; i < numc; i++ )
      {
      kwd = this->GetUserTagTable()->GetTagAttribute(i);
      val = this->GetUserTagTable()->GetTagValue (i);
      if (kwd != nullptr && val != nullptr)
        {
        ss << kwd << "=" << val;
        if ( i < (numc-1) )
          {
          ss << " ";
          }
        }
      }
    if ( ss.str().c_str()!= nullptr )
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

    vtkIndent vindent(indent);
    *os << vindent << "<" << node->GetNodeTagName() << "\n ";

    if(indent<=0)
      indent = 1;

    node->WriteXML(*os, indent);

    *os << vindent << ">";
    node->WriteNodeBodyXML(*os, indent);
    *os << "</" << node->GetNodeTagName() << ">\n";
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
  this->SetErrorCode(vtkErrorCode::GetErrorCodeFromString("NoError"));
  this->StoredTime.Modified();
  return 1;
}

namespace
{

//------------------------------------------------------------------------------
inline bool IsNodeWithoutID(vtkMRMLNode* node)
{
  return node->GetID() == nullptr || node->GetID()[0] == '\0';
}

//------------------------------------------------------------------------------
inline bool IsNodeWithoutName(vtkMRMLNode* node)
{
  return node->GetName() == nullptr || node->GetName()[0] == '\0';
}

}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::AddNodeNoNotify(vtkMRMLNode *n)
{
  if (!n)
    {
    vtkErrorMacro("AddNodeNoNotify: unable to add a null node to the scene");
    return nullptr;
    }

  if (!n->GetAddToScene())
    {
    return nullptr;
    }

  int wasModifying = n->StartModify();

  //TODO convert URL to Root directory

  // check if node is a singleton
  if (n->GetSingletonTag() != nullptr)
    {
    // check if there is a singleton of this class in the scene
    // and if found copy this node into it
    vtkMRMLNode *sn = this->GetSingletonNode(n);
    if (sn != nullptr)
      {
      // A node can't be added twice into the scene
      if (sn == n)
        {
        vtkWarningMacro("Same node is already added to the scene");
        }

      // Keep the existing singleton node and its node ID,
      // just copy the contents of the new node into the existing singleton node

      // If the node ID is changed (not very common) then save that
      // info so that the nodes that are being imported can update
      // their references.
      std::string newId(sn->GetID());
      std::string oldId(n->GetID() ? n->GetID() : sn->GetID());
      if ( (this->IsImporting() || this->IsRestoring())
        && (oldId != newId && n->GetID()) )
        {
        this->ReferencedIDChanges[oldId] = newId;
        }

      // Update contents of existing singleton node
      sn->Copy(n);

      // Stores the node references in this->NodeReferences.
      // This is required for UpdateNodeReferences() to work.
      sn->SetSceneReferences();
      return sn;
      }
    }
  // Usually nodes to add to the scene don't have an ID set. However, when
  // importing a scene (vtkMRMLScene::Import()), nodes to add already have an
  // ID. This can create a node ID conflict with the nodes already in the
  // scene. A new unique ID  must be set to the node to add and references to
  // the node must be updated with the new ID.
  if (IsNodeWithoutID(n) || this->GetNodeByID(n->GetID()) != nullptr)
    {
    std::string oldID(n->GetID() ? n->GetID() : "");
    n->SetID(this->GenerateUniqueID(n).c_str());
    if (n->GetScene())
      {
      // The scene is set already so update the ID references for this new ID
      n->SetSceneReferences();
      }

    vtkDebugMacro("AddNodeNoNotify: got unique id for new " << n->GetClassName() << " node: " << n->GetID() << endl);
    std::string newID(n->GetID() ? n->GetID() : "");
    if ( (this->IsImporting() || this->IsRestoring())
      && (oldID != newID && !oldID.empty()) )
      {
      this->ReferencedIDChanges[oldID] = newID;
      }
    }

  // Set a default name if none is given automatically
  if (IsNodeWithoutName(n))
    {
    n->SetName(this->GenerateUniqueName(n).c_str());
    }
  n->SetScene( this );
  this->Nodes->vtkCollection::AddItem((vtkObject *)n);

  // cache the node so the whole scene cache stays up-to date
  this->AddNodeID(n);

  // Keep the SH up-to-date
  if (vtkMRMLSubjectHierarchyNode::SafeDownCast(n) != nullptr &&
    !(this->IsImporting() || this->IsRestoring()) )
  {
    // This should rarely ever be called, but just in case someone added a SH node manually, let's make sure it works
    this->SetSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode::ResolveSubjectHierarchy(this));
  }

  n->EndModify(wasModifying);
  return n;
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::AddNode(vtkMRMLNode *n)
{
  if (!n)
    {
    vtkErrorMacro("AddNode: unable to add a null node to the scene");
    return nullptr;
    }
  if (!n->GetAddToScene())
    {
    return nullptr;
    }
#ifndef NDEBUG
  // Since calling IsNodePresent is costly, a "developer hint" is printed only
  // if build as debug. We can't exit here as the release would then be
  // different from debug.
  // The caller should make sure the node has not been added yet.
  if (this->IsNodePresent(n) != 0)
    {
    vtkErrorMacro("AddNode: Node " << n->GetClassName() << "/"
      << (n->GetName() ? n->GetName() : "(undefined)") << "/"
      << (n->GetID() ? n->GetID() : "(undefined)")
      << "[" << n << "]" << " already added");
    }
#endif
  // We need to know if the node will be actually added to the scene before
  // it is effectively added to know if NodeAboutToBeAddedEvent needs to be
  // fired.
  bool add = true;
  if (n->GetSingletonTag() != nullptr &&
      this->GetSingletonNode(n) != nullptr)
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
  if (add)
    {
    this->InvokeEvent(this->NodeAddedEvent, n);
    }
  else if (node==n)
    {
    vtkWarningMacro("vtkMRMLScene::AddNode: Adding of a new node is not notified");
    }
  // Convert all node reference IDs to pointers and add observers
  // (only do that if not importing, because during import node IDs are not final yet).
  if (!this->IsImporting() && !this->IsRestoring())
    {
    node->UpdateNodeReferences();
    }
  this->Modified();
#ifdef MRMLSCENE_VERBOSE
  timer->StopTimer();
  std::cerr << "AddNode: " << n->GetID() << " :" << timer->GetElapsedTime() << "\n";
  timer->Delete();
#endif
  // If the node is a singleton, the returned node is the existing singleton
  return node;
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::AddNewNodeByClass(
    std::string className, std::string nodeBaseName /* = "" */)
{
  if (className.empty())
    {
    vtkErrorMacro("AddNewNodeByClass: className is an empty string");
    return nullptr;
    }
  vtkSmartPointer<vtkMRMLNode> nodeToAdd =
      vtkSmartPointer<vtkMRMLNode>::Take(this->CreateNodeByClass(className.c_str()));
  if (nodeToAdd == nullptr)
    {
    vtkErrorMacro("AddNewNodeByClass: failed to create node by class " << className);
    return nullptr;
    }
  if (!nodeBaseName.empty())
    {
    nodeToAdd->SetName(nodeBaseName.c_str());
    }
  return this->AddNode(nodeToAdd);
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::AddNewNodeByClassWithID(std::string className, std::string nodeBaseName, std::string nodeID)
{
  if (className.empty())
    {
    vtkErrorMacro("AddNewNodeByClassWithID: className is an emptry string");
    return NULL;
    }

  if (nodeID.empty())
    {
    vtkErrorMacro("AddNewNodeByClassWithID: nodeID is an emptry string");
    return nullptr;
    }

  bool isUnique = ((this->GetNodeByID(nodeID) == nullptr) &&
                   (!this->IsReservedID(nodeID)) &&
                   (!this->IsNodeIDReservedByUndo(nodeID)));
  if (!isUnique)
    {
    vtkErrorMacro("AddNewNodeByClassWithID: nodeID is already in use");
    return nullptr;
    }

  if (this->NodeIDs.find(nodeID) != this->NodeIDs.end())
    {
    vtkErrorMacro("AddNewNodeByClassWithID: node already exists with ID - " << nodeID);
    return NULL;
    }

  vtkSmartPointer<vtkMRMLNode> nodeToAdd =
    vtkSmartPointer<vtkMRMLNode>::Take(this->CreateNodeByClass(className.c_str()));
  if (nodeToAdd == nullptr)
    {
    vtkErrorMacro("AddNewNodeByClassWithID: failed to create node by class " << className);
    return nullptr;
    }

  nodeToAdd->SetID(nodeID.c_str());
  if (!nodeBaseName.empty())
    {
    nodeToAdd->SetName(nodeBaseName.c_str());
    }

  return this->AddNode(nodeToAdd);
}

//------------------------------------------------------------------------------
void vtkMRMLScene::NodeAdded(vtkMRMLNode *n)
{
  this->InvokeEvent(this->NodeAddedEvent, n);
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::CopyNode(vtkMRMLNode *n)
{
   if (!n)
    {
    vtkErrorMacro("CopyNode: unable to copy a null node");
    return nullptr;
    }

  if (!n->GetAddToScene())
    {
    return nullptr;
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
  if (n == nullptr)
    {
    vtkDebugMacro("RemoveNode: unable to remove null node");
    return;
    }

#ifndef NDEBUG
  // Since calling IsNodePresent cost, let's display a "developer hint" only if build as Debug
  // The caller should make sure the node isn't already removed
  if (this->IsNodePresent(n) == 0)
    {
    vtkErrorMacro("RemoveNode: Node " << n->GetClassName() << "/"
                  << (n->GetName() ? n->GetName() : "(undefined)") << "[" << n << "]" << " already removed");
    }
  // As callbacks may want to look for the removed node, the nodeID list should
  // be up to date.
  if (this->GetNodeByID(n->GetID()) == nullptr)
    {
    vtkErrorMacro("RemoveNode: class: " << n->GetClassName() << " name:"
                  << (n->GetName() ? n->GetName() : "(undefined)")
                  << " id: " << (n->GetID() ? n->GetID() : "(undefined)")
                  << "["  << n << "]" << " can't be found by ID");
    }
#endif

  n->Register(this);
  this->InvokeEvent(vtkMRMLScene::NodeAboutToBeRemovedEvent, n);

  if (n->GetScene() == this) // extra precaution that might not be useful
    {
    n->SetScene(nullptr);
    }
  this->Nodes->vtkCollection::RemoveItem((vtkObject *)n);

  std::string nid = (n->GetID() ? n->GetID() : "");
  this->RemoveNodeID(n->GetID());

  this->InvokeEvent(vtkMRMLScene::NodeRemovedEvent, n);

  if (!this->IsBatchProcessing() && !this->IsClosing())
    {
    // We are not doing batch processing, so update the node references now.
    // Node references will be all removed for the removed node and for
    // all the nodes that the deleted node referred to.
    this->RemoveNodeReferences(n);
    // Notify nodes that referred to the deleted node to update their references
    NodeReferencesType::iterator referencedNodeIdIt=this->NodeReferences.find(nid);
    if (referencedNodeIdIt!=this->NodeReferences.end())
      {
      // make a copy of the referring node list, as the list may change as a result of UpdateReferences calls
      std::set<std::string> referringNodes=referencedNodeIdIt->second;
      for (NodeReferencesType::value_type::second_type::iterator referringNodesIt = referringNodes.begin();
        referringNodesIt != referringNodes.end();
        ++referringNodesIt)
        {
        vtkMRMLNode* node=this->GetNodeByID(*referringNodesIt);
        if (node)
          {
          node->UpdateReferences();
          }
        }
      }
    this->RemoveReferencesToNode(n);
    }

  n->UnRegister(this);
  n=nullptr;

  this->Modified();
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RemoveReferencedNodeID(const char *id, vtkMRMLNode *referencingNode)
{
  if (id == nullptr || referencingNode == nullptr)
    {
    vtkErrorMacro("RemoveReferencedNodeID: either id is null or the reference node is null.");
    return;
    }
  NodeReferencesType::iterator referenceIt=this->NodeReferences.find(id);
  if (referenceIt==this->NodeReferences.end())
    {
    // no referrers to id
    return;
    }
  if (referencingNode->GetID()==nullptr)
    {
    // invalid referencing node id
    return;
    }
  referenceIt->second.erase(referencingNode->GetID());
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RemoveNodeReferences(vtkMRMLNode *n)
{
  if (n == nullptr)
    {
    vtkErrorMacro("RemoveNodeReferences: node is null can't remove it");
    return;
    }
  if (n->GetID() == nullptr)
    {
    // can happen when adding singleton nodes that are not really added but copied
    return;
    }
  std::string nid=n->GetID();

  NodeReferencesType::value_type::second_type::iterator referringNodesIt;
  for (NodeReferencesType::iterator referenceIt = this->NodeReferences.begin();
    referenceIt != this->NodeReferences.end();
    ++referenceIt)
    {
    referringNodesIt=referenceIt->second.find(nid);
    if (referringNodesIt!=referenceIt->second.end())
      {
      // observation has been deleted, so remove it from the index
      referenceIt->second.erase(referringNodesIt);
      }
    }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RemoveUnusedNodeReferences()
{
  // Remove Referring node IDs that are no longer in the scene
  for (NodeReferencesType::iterator referenceIt = this->NodeReferences.begin();
    referenceIt != this->NodeReferences.end();
    ++referenceIt)
    {
    for (NodeReferencesType::value_type::second_type::iterator referringNodesIt = referenceIt->second.begin();
      referringNodesIt != referenceIt->second.end();
      /*upon deletion the increment is done already, so don't increment here*/)
      {
      vtkMRMLNode *currentReferencingNodePtr=this->GetNodeByID(*referringNodesIt);
      if (currentReferencingNodePtr==nullptr)
        {
        // ### Slicer 4.4: Simplify this logic when adding support for C++11 across all supported platform/compilers
        // the node is not in the scene (or in the scene but with a different pointer), remove it
        NodeReferencesType::value_type::second_type::iterator referringNodesItToRemove = referringNodesIt;
        ++referringNodesIt;
        referenceIt->second.erase(referringNodesItToRemove);
        continue;
        }
      ++referringNodesIt;
      }
    }

  // Remove Referenced node IDs that are no longer in the scene
  for (NodeReferencesType::iterator referenceIt = this->NodeReferences.begin();
    referenceIt != this->NodeReferences.end();
    /*upon deletion the increment is done already, so don't increment here*/)
    {
    vtkMRMLNode *node = this->GetNodeByID(referenceIt->first);
    if (node==nullptr || referenceIt->second.empty())
      {
      // ### Slicer 4.4: Simplify this logic when adding support for C++11 across all supported platform/compilers
      // the referenced ID is no longer in the scene (or no more references), so remove all related references
      NodeReferencesType::iterator referenceItToBeRemoved = referenceIt;
      ++referenceIt;
      this->NodeReferences.erase(referenceItToBeRemoved);
      continue;
      }
    // go to next referenced ID
    ++referenceIt;
    }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RemoveReferencesToNode(vtkMRMLNode *n)
{
  if (n == nullptr || n->GetID() == nullptr)
    {
    vtkErrorMacro("RemoveReferencesToNode: node is null or has null id, can't remove refs");
    return;
    }
  this->NodeReferences.erase(n->GetID());
}

//------------------------------------------------------------------------------
int vtkMRMLScene::IsNodePresent(vtkMRMLNode *n)
{
  return this->Nodes->vtkCollection::IsItemPresent((vtkObject *)n);
}

//------------------------------------------------------------------------------
void vtkMRMLScene::InitTraversal()
{
  vtkWarningMacro("Usage of vtkMRMLScene::InitTraversal() is unsafe.")
  this->Nodes->InitTraversal();
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetNextNode()
{
  vtkWarningMacro("Usage of vtkMRMLScene::GetNextNode() is unsafe.")
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
  if (className == nullptr)
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
  nodes.clear();
  if (className == nullptr)
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
  if (className == nullptr)
    {
    vtkErrorMacro("GetNodesByClass: class name is null.");
    return nullptr;
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
vtkMRMLNode *vtkMRMLScene::GetNextNodeByClass(const char *className)
{
  vtkWarningMacro("Usage of vtkMRMLScene::GetNextNodeByClass(const char *) is unsafe.")
  if (!className)
    {
    vtkErrorMacro("GetNextNodeByClass: class name is null.");
    return nullptr;
    }

  vtkMRMLNode *node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject();

  while (node != nullptr && !node->IsA(className))
    {
    node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject();
    }
  if (node != nullptr && node->IsA(className))
    {
    return node;
    }
  else
    {
    return nullptr;
    }
}
//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetSingletonNode(const char* singletonTag, const char* className)
{
  if (singletonTag==nullptr || strlen(singletonTag)==0)
    {
    vtkErrorMacro("vtkMRMLScene::GetSingletonNode: received invalid singletonTag");
    return nullptr;
    }
  if (className==nullptr || strlen(className)==0)
    {
    vtkErrorMacro("vtkMRMLScene::GetSingletonNode: received invalid className");
    return nullptr;
    }

  vtkCollectionSimpleIterator it;
  vtkMRMLNode* node = nullptr;
  for (this->Nodes->InitTraversal(it);
       (node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject(it)) ;)
    {
    if (node->IsA(className) &&
        node->GetSingletonTag() != nullptr &&
        strcmp(node->GetSingletonTag(), singletonTag) == 0)
      {
      return node;
      }
    }
  return nullptr;
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetSingletonNode(vtkMRMLNode* n)
{
  if (n == nullptr || n->GetSingletonTag() == nullptr)
    {
    vtkErrorMacro("vtkMRMLScene::GetSingletonNode failed: singleton node is expected");
    return nullptr;
    }
  vtkMRMLNode *sn = this->GetSingletonNode(n->GetSingletonTag(), n->GetClassName());
  if (sn != nullptr)
    {
    // singleton node found
    return sn;
    }

  // No singleton node found, but it may be possible that a non-singleton node exists with
  // the same ID, which is probably a singleton node where the tag was not set by mistake.
  // The vtkMRMLNode was updated to serialize the singleton tag but legacy scene files from
  // before October 2015 may have saved nodes without the singleton tag having been set.
  std::string singletonId = this->GenerateUniqueID(n);
  sn = this->GetNodeByID(singletonId.c_str());
  if (sn != nullptr)
    {
    if (sn->GetSingletonTag() == nullptr)
      {
      vtkWarningMacro("The " << singletonId << " ID belongs to a singleton, therefore it will be treated as a singleton");
      }
    else
      {
      vtkWarningMacro("The node ID " << singletonId << " is inconsistent with its singleton tag " << sn->GetSingletonTag()
        << ", the singleton tag is updated");
      }
    sn->SetSingletonTag(n->GetSingletonTag());
    return sn;
    }

  return nullptr;
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetNthNode(int n)
{
  if(n < 0 || n >= this->Nodes->GetNumberOfItems())
    {
    return nullptr;
    }
  else
    {
    return (vtkMRMLNode*)this->Nodes->GetItemAsObject(n);
    }
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetNthNodeByClass(int n, const char *className)
{
  if (className == nullptr || n < 0)
    {
    vtkErrorMacro("GetNthNodeByClass: class name is null or n is less than zero: " << n);
    return nullptr;
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
  return nullptr;
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetFirstNodeByClass(const char *className)
{
  return this->GetNthNodeByClass(0, className);
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
    if (node->GetName() != nullptr && !strcmp(node->GetName(), name))
      {
      nodes->AddItem(node);
      }
    }
  return nodes;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetFirstNode(const char* byName,
                                        const char* byClass,
                                        const int* byHideFromEditors,
                                        bool exactNameMatch)
{
  vtkCollectionSimpleIterator it;
  vtkMRMLNode* node;
  for (this->Nodes->InitTraversal(it);
       (node= vtkMRMLNode::SafeDownCast(
          this->Nodes->GetNextItemAsObject(it))) ;)
    {
    if (exactNameMatch && byName &&
        node->GetName() != nullptr && strcmp(node->GetName(), byName) != 0)
      {
      continue;
      }
    if (!exactNameMatch && byName &&
        node->GetName() != nullptr && !vtksys::RegularExpression(byName).find(node->GetName()))
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
  return nullptr;
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetFirstNodeByName(const char* name)
{
  vtkMRMLNode *node = nullptr;
  if (name == nullptr)
    {
    vtkErrorMacro("GetNodesByName: name is null");
    return node;
    }

  vtkCollectionSimpleIterator it;
  for (this->Nodes->InitTraversal(it);
       (node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject(it)) ;)
    {
    if (node->GetName() != nullptr && !strcmp(node->GetName(), name))
      {
      return node;
      }
    }
  return nullptr;
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetNodeByID(std::string id)
{
  return this->GetNodeByID(id.c_str());
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetNodeByID(const char* id)
{
  if (id == nullptr)
    {
    // don't use a macro here, can cause a crash on exit
    return nullptr;
    }

  vtkMRMLNode *node = nullptr;
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
      vtkErrorMacro("GetNodeByID: Node is in the scene, but its ID is missing from the NodeIDs cache: " << id);
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
    if (node->GetName() != nullptr && !strcmp(node->GetName(), name) && node->IsA(className))
      {
      nodes->AddItem(node);
      }
    }

  return nodes;
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::InsertAfterNode(vtkMRMLNode *item, vtkMRMLNode *n)
{
  if (!n)
    {
    vtkErrorMacro("InsertAfterNode: unable to add a null node to the scene");
    return nullptr;
    }
  if (!n->GetAddToScene())
    {
    return nullptr;
    }
#ifndef NDEBUG
  // Since calling IsNodePresent is costly, a "developer hint" is printed only
  // if build as debug. We can't exit here as the release would then be
  // different from debug.
  // The caller should make sure the node has not been added yet.
  if (this->IsNodePresent(n) != 0)
    {
    vtkErrorMacro("InsertAfterNode: Node " << n->GetClassName() << "/"
      << (n->GetName() ? n->GetName() : "(undefined)") << "/"
      << (n->GetID() ? n->GetID() : "(undefined)")
      << "[" << n << "]" << " already added");
    }
#endif
  if (n->GetSingletonTag() != nullptr)
    {
    vtkDebugMacro("InsertAfterNode: node is a singleton, not inserting after item, just calling AddNode");
    return this->AddNode(n);
    }

  this->InvokeEvent(this->NodeAboutToBeAddedEvent, n);

  // code from add node no notify
  if (IsNodeWithoutID(n) || this->GetNodeByID(n->GetID()) != nullptr)
    {
    std::string oldID;
    if (n->GetID())
      {
      oldID = n->GetID();
      }
    int modifyStatus = n->GetDisableModifiedEvent();
    n->SetDisableModifiedEvent(1);
    n->SetID(this->GenerateUniqueID(n).c_str());
    if (n->GetScene())
      {
      // The scene is set already so update the ID references for this new ID
      n->SetSceneReferences();
      }
    n->SetDisableModifiedEvent(modifyStatus);
    std::string newID(n->GetID());
    if (oldID != newID)
      {
      this->ReferencedIDChanges[oldID] = newID;
      }
    }

  int modifyStatus = n->GetDisableModifiedEvent();
  n->SetDisableModifiedEvent(1);

  if (IsNodeWithoutName(n))
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
  // cache the node so the whole scene cache stays up-to-date
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
    return nullptr;
    }
  if (!n->GetAddToScene())
    {
    return nullptr;
    }
#ifndef NDEBUG
  // Since calling IsNodePresent is costly, a "developer hint" is printed only
  // if build as debug. We can't exit here as the release would then be
  // different from debug.
  // The caller should make sure the node has not been added yet.
  if (this->IsNodePresent(n) != 0)
    {
    vtkErrorMacro("InsertBeforeNode: Node " << n->GetClassName() << "/"
      << (n->GetName() ? n->GetName() : "(undefined)") << "/"
      << (n->GetID() ? n->GetID() : "(undefined)")
      << "[" << n << "]" << " already added");
    }
#endif
  if (n->GetSingletonTag() != nullptr)
    {
    vtkDebugMacro("InsertBeforeNode: node is a singleton, not inserting before item, just calling AddNode");
    return this->AddNode(n);
    }

  this->InvokeEvent(this->NodeAboutToBeAddedEvent, n);

  // code from add node no notify
  if (IsNodeWithoutID(n) || this->GetNodeByID(n->GetID()) != nullptr)
    {
    std::string oldID;
    if (n->GetID())
      {
      oldID = n->GetID();
      }
    int modifyStatus = n->GetDisableModifiedEvent();
    n->SetDisableModifiedEvent(1);
    n->SetID(this->GenerateUniqueID(n).c_str());
    if (n->GetScene())
      {
      // The scene is set already so update the ID references for this new ID
      n->SetSceneReferences();
      }
    n->SetDisableModifiedEvent(modifyStatus);
    std::string newID(n->GetID());
    if (oldID != newID)
      {
      this->ReferencedIDChanges[oldID] = newID;
      }
    }

  int modifyStatus = n->GetDisableModifiedEvent();
  n->SetDisableModifiedEvent(1);

  if (IsNodeWithoutName(n))
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
  if ( this->GetUserTagTable() != nullptr )
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
      os << indent.GetNextIndent().GetNextIndent() << "Default write extension = " << (exts != nullptr ? exts : "NULL") << endl;
      }
   }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::SetURL(const char *url)
{
  this->URL = std::string(url?url:"");
}

//------------------------------------------------------------------------------
const char * vtkMRMLScene::GetURL()
{
  return this->URL.c_str();
}

//------------------------------------------------------------------------------
void vtkMRMLScene::SetRootDirectory(const char *dir)
{
  this->RootDirectory = std::string(dir);
}

//------------------------------------------------------------------------------
const char * vtkMRMLScene::GetRootDirectory()
{
  return this->RootDirectory.c_str();
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
    return nullptr;
    }
}

//------------------------------------------------------------------------------
bool vtkMRMLScene::IsNodeClassRegistered(const std::string& className)
{
  for (int index=0; index < this->GetNumberOfRegisteredNodeClasses(); ++index)
    {
    vtkMRMLNode* registeredNodeClass = this->GetNthRegisteredNodeClass(index);
    if (!registeredNodeClass)
      {
      continue;
      }
    if (className == registeredNodeClass->GetClassName())
      {
      return true;
      }
    }
  return false;
}

//------------------------------------------------------------------------------
std::string vtkMRMLScene::GenerateUniqueID(vtkMRMLNode* node)
{
  if (!node)
    {
    vtkWarningMacro("vtkMRMLScene::GenerateUniqueID: invalid node");
    return this->GenerateUniqueID("Node");
    }
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
  if (baseID.empty())
    {
    vtkWarningMacro("vtkMRMLScene::GetUniqueIDIndex: baseID is empty");
    }
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
      (this->GetNodeByID(candidateID) == nullptr) &&
      (!this->IsReservedID(candidateID)) &&
      (!this->IsNodeIDReservedByUndo(candidateID));
    }
  return index;
}

//------------------------------------------------------------------------------
bool vtkMRMLScene::IsNodeIDReservedByUndo(const std::string id) const
{
  NodeReferencesType::const_iterator referenceIt = this->NodeReferences.find(id);
  if (referenceIt != this->NodeReferences.end())
    {
    // ID is referenced by a node in the scene.
    // It cannot be reserved by undo.
    return false;
    }

  std::set<std::string> undoReferenceIDs;
  this->GetNodeReferenceIDsFromUndoStack(undoReferenceIDs);
  if (undoReferenceIDs.find(id) != undoReferenceIDs.end())
    {
    return true;
    }

  return false;
}

//------------------------------------------------------------------------------
void vtkMRMLScene::GetNodeReferenceIDsFromUndoStack(std::set<std::string>& referenceIDs) const
{
  referenceIDs.clear();

  std::list<vtkCollection*>::const_iterator undoStackIt;
  for (undoStackIt = this->UndoStack.begin(); undoStackIt != this->UndoStack.end(); ++undoStackIt)
    {
    vtkCollection* nodes = *undoStackIt;
    for (int i = 0; i < nodes->GetNumberOfItems(); ++i)
      {
      vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(nodes->GetItemAsObject(i));
      if (!node)
        {
        continue;
        }

      std::vector<std::string> roles;
      node->GetNodeReferenceRoles(roles);
      std::vector<std::string>::iterator roleIt;
      for (roleIt = roles.begin(); roleIt != roles.end(); ++roleIt)
        {
        std::string role = *roleIt;
        std::vector<const char*> currentReferenceIDs;
        node->GetNodeReferenceIDs(role.c_str(), currentReferenceIDs);
        std::vector<const char*>::iterator referenceIDIt;
        for (referenceIDIt = currentReferenceIDs.begin(); referenceIDIt != currentReferenceIDs.end(); ++referenceIDIt)
          {
          if (!(*referenceIDIt))
            {
            continue;
            }
          referenceIDs.insert(*referenceIDIt);
          }
        }
      }
    }
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
  if (!node)
    {
    vtkErrorMacro("vtkMRMLScene::GenerateUniqueName: input node is invalid");
    return "";
    }
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
  unsafeName = this->GenerateUniqueName(baseName ? baseName : "Node");
  return unsafeName.c_str();
}

//------------------------------------------------------------------------------
int vtkMRMLScene::GetUniqueNameIndex(const std::string& baseName)
{
  if (baseName.empty())
    {
    vtkWarningMacro("vtkMRMLScene::GetUniqueNameIndex: baseName is invalid");
    }
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
    isUnique = (this->GetFirstNodeByName(candidateName.c_str()) == nullptr);
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
bool vtkMRMLScene::IsReservedID(const std::string& id)
{
  return !(this->ReservedIDs.find(id) == this->ReservedIDs.end());
}

//------------------------------------------------------------------------------
void vtkMRMLScene::AddReservedID(const char *id)
{
  if (id == nullptr)
    {
    return;
    }
  this->ReservedIDs.insert(std::string(id));
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

  if (this->IsUndoing())
    {
    return;
    }

  if (this->IsBatchProcessing())
    {
    return;
    }

  if (node && !node->GetUndoEnabled())
    {
    return;
    }

  this->ClearRedoStack();
  //this->SetUndoOn();
  this->PushIntoUndoStack();
  if (node)
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

  if (this->IsUndoing())
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
    if (node && node->GetUndoEnabled())
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

  if (this->IsUndoing())
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
    vtkMRMLNode *node  = vtkMRMLNode::SafeDownCast(nodes->GetItemAsObject(n));
    if (node && node->GetUndoEnabled())
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
  if (this->Nodes == nullptr)
    {
    return;
    }

  vtkCollection* newScene = vtkCollection::New();

  vtkCollection* currentScene = this->Nodes;

  int nnodes = currentScene->GetNumberOfItems();

  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLNode *node  = vtkMRMLNode::SafeDownCast(currentScene->GetItemAsObject(n));
    if (node && node->GetUndoEnabled())
      {
      newScene->AddItem(node);
      }
    }

  this->UndoStack.push_back(newScene);
  this->TrimUndoStack();
}

//------------------------------------------------------------------------------
// Make a new collection that has pointers to the current scene nodes
void vtkMRMLScene::PushIntoRedoStack()
{
  if (this->Nodes == nullptr)
    {
    return;
    }

  vtkCollection* newScene = vtkCollection::New();

  vtkCollection* currentScene = this->Nodes;

  int nnodes = currentScene->GetNumberOfItems();

  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLNode *node  = vtkMRMLNode::SafeDownCast(currentScene->GetItemAsObject(n));
    if (node && node->GetUndoEnabled())
      {
      newScene->AddItem(node);
      }
    }

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
  if (snode != nullptr)
    {
    snode->CopyWithScene(copyNode);
    }

  vtkCollection* undoScene = this->UndoStack.back();
  int nnodes = undoScene->GetNumberOfItems();
  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLNode *node  = vtkMRMLNode::SafeDownCast(undoScene->GetItemAsObject(n));
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
  if (snode != nullptr)
    {
    snode->CopyWithSceneWithSingleModifiedEvent(copyNode);
    }
  vtkCollection* undoScene = this->RedoStack.back();
  int nnodes = undoScene->GetNumberOfItems();
  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLNode *node  = vtkMRMLNode::SafeDownCast(undoScene->GetItemAsObject(n));
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

  this->StartState(vtkMRMLScene::UndoState);
  this->RemoveUnusedNodeReferences();

  int nnodes;
  int n;
  unsigned int nn;

  this->PushIntoRedoStack();

  vtkCollection* currentScene = this->Nodes;
  // We use 2 vectors instead of a map in order to keep the ordering of the
  // nodes.
  std::vector<std::string> currentIDs;
  std::vector<vtkMRMLNode*> currentNodes;
  nnodes = currentScene->GetNumberOfItems();
  for (n=0; n<nnodes; n++)
    {
    vtkMRMLNode *node  = vtkMRMLNode::SafeDownCast(currentScene->GetItemAsObject(n));
    if (node && node->GetUndoEnabled())
      {
      currentIDs.push_back(node->GetID());
      currentNodes.push_back(node);
      }
    }

  vtkCollection* undoScene = nullptr;
  std::vector<std::string> undoIDs;
  std::vector<vtkMRMLNode*> undoNodes;

  if (!this->UndoStack.empty())
    {
    undoScene = this->UndoStack.back();
    nnodes = undoScene->GetNumberOfItems();
    for (n=0; n<nnodes; n++)
      {
      vtkMRMLNode *node  = vtkMRMLNode::SafeDownCast(undoScene->GetItemAsObject(n));
      if (node && node->GetUndoEnabled())
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
      // the node was deleted, add Node back to the current scene
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
    addNodes[nn]->SetSceneReferences();
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

  if (!this->UndoStack.empty())
   {
   this->UndoStack.pop_back();
   }
  this->Modified();

  this->EndState(vtkMRMLScene::UndoState);

  // Some untracked nodes may need to be restored to the scene after undo has completed
  // Do not remove unused node references until these nodes have been added
  this->RemoveUnusedNodeReferences();
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

  this->StartState(vtkMRMLScene::RedoState);

  this->RemoveUnusedNodeReferences();

  this->PushIntoUndoStack();


  vtkCollection* currentScene = this->Nodes;
  //std::hash_map<std::string, vtkMRMLNode*> currentMap;
  std::map<std::string, vtkWeakPointer<vtkMRMLNode> > currentMap;
  nnodes = currentScene->GetNumberOfItems();
  for (n=0; n<nnodes; n++)
    {
    vtkMRMLNode *node  = vtkMRMLNode::SafeDownCast(currentScene->GetItemAsObject(n));
    if (node && node->GetUndoEnabled())
      {
      currentMap[node->GetID()] = node;
      }
    }

  //std::hash_map<std::string, vtkMRMLNode*> undoMap;
  std::map<std::string, vtkWeakPointer<vtkMRMLNode> > undoMap;

  vtkCollection* undoScene = nullptr;

  if (!this->RedoStack.empty())
    {
    undoScene = this->RedoStack.back();
    if (undoScene)
      {
      nnodes = undoScene->GetNumberOfItems();
      for (n=0; n<nnodes; n++)
        {
        vtkMRMLNode *node = vtkMRMLNode::SafeDownCast(undoScene->GetItemAsObject(n));
        if (node && node->GetUndoEnabled())
          {
          undoMap[node->GetID()] = node;
          }
        }
      }
    }

  std::map<std::string, vtkWeakPointer<vtkMRMLNode> >::iterator iter;
  std::map<std::string, vtkWeakPointer<vtkMRMLNode> >::iterator curIter;

  // copy back changes and add deleted nodes to the current scene
  std::vector<vtkWeakPointer<vtkMRMLNode> > addNodes;
  for(iter=undoMap.begin(); iter != undoMap.end(); iter++)
    {
    curIter = currentMap.find(iter->first);
    if ( curIter == currentMap.end() )
      {
      // the node was deleted, add Node back to the current scene
      addNodes.push_back(iter->second);
      }
    else if (!curIter->second || !iter->second)
      {
      continue;
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
  std::vector<vtkWeakPointer<vtkMRMLNode> > removeNodes;
  for(curIter=currentMap.begin(); curIter != currentMap.end(); curIter++)
    {
    if (!curIter->second)
      {
      continue;
      }

    iter = undoMap.find(curIter->first);
    if ( iter == undoMap.end() )
      {
      this->CopyNodeInUndoStack(curIter->second);
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
  this->RedoStack.pop_back();
  this->Modified();

  this->EndState(vtkMRMLScene::RedoState);
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
  if (id==nullptr)
    {
    // vtkErrorMacro("Failed to add node reference: invalid referenced node ID");
    // Do not log an error for now, because many places there are calls like
    //   this->Scene->AddReferencedNodeID(this->ActiveVolumeID, this);
    // TODO: replace manual reference handling by proper node references
    //   then this error checking can be activated.
    return;
    }
  if (referencingNode==nullptr)
    {
    vtkErrorMacro("Failed to add node reference: the referencing node is invalid");
    return;
    }
  if (referencingNode->GetScene()==nullptr || referencingNode->GetID()==nullptr)
    {
    // Scene is not yet set for the referencing node, so we don't need to add the reference to the scene yet.
    // When the scene will be set then all the references will be added.
    return;
    }
  if (this->IsNodeReferencingNodeID(referencingNode, id))
    {
    // this reference already exists, there is nothing to do
    return;
    }
  this->NodeReferences[id].insert(referencingNode->GetID());
}

//------------------------------------------------------------------------------
bool vtkMRMLScene::IsNodeReferencingNodeID(vtkMRMLNode* referencingNode, const char* id)
{
  if (id == nullptr || referencingNode == nullptr)
    {
    vtkErrorMacro("RemoveReferencedNodeID: either id is null or the reference node is null.");
    return false;
    }
  NodeReferencesType::iterator referenceIt=this->NodeReferences.find(id);
  if (referenceIt==this->NodeReferences.end())
    {
    // no referrers to id
    return false;
    }
  if (referencingNode->GetID()==nullptr)
    {
    // invalid referencing node id
    return false;
    }
  if (referenceIt->second.find(referencingNode->GetID())==referenceIt->second.end())
    {
    return false;
    }
  return true;
}

//------------------------------------------------------------------------------
int vtkMRMLScene::IsFilePathRelative(const char * filepath)
{
  if (filepath == nullptr)
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
  if (this->GetCacheManager() != nullptr)
    {
    if (this->GetCacheManager()->IsRemoteReference(filepath))
      {
      return 0;
      }
    }

  const bool absoluteFilePath = vtksys::SystemTools::FileIsFullPath(filepath);
  return absoluteFilePath ? 0 : 1;
}

//------------------------------------------------------------------------------
const char* vtkMRMLScene::GetChangedID(const char* id)
{
  std::map< std::string, std::string>::const_iterator iter = this->ReferencedIDChanges.find(std::string(id));
  if (iter == this->ReferencedIDChanges.end())
    {
    return nullptr;
    }
  else
    {
    return iter->second.c_str();
    }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::UpdateNodeReferences(vtkCollection* checkNodes/*=nullptr*/)
{
  for (std::map< std::string, std::string>::const_iterator iterChanged = this->ReferencedIDChanges.begin();
    iterChanged != this->ReferencedIDChanges.end(); iterChanged++)
    {
    const std::string& oldID = iterChanged->first;
    const std::string& newID = iterChanged->second;
    NodeReferencesType::iterator referencedIdIt=this->NodeReferences.find(oldID);
    if (referencedIdIt==this->NodeReferences.end())
      {
      // this updated ID is not observed by any node
      continue;
      }
    // make a copy of the node list, as the list may change as a result of UpdateReferenceID calls
    std::set<std::string> nodesToNotify=referencedIdIt->second;
    for (NodeReferencesType::value_type::second_type::iterator referringNodesIt = nodesToNotify.begin();
      referringNodesIt!=nodesToNotify.end();
      ++referringNodesIt)
      {
      vtkMRMLNode *node = this->GetNodeByID(*referringNodesIt);
      if (node==nullptr)
        {
        continue;
        }
      if (checkNodes!=nullptr && !checkNodes->IsItemPresent(node))
        {
        continue;
        }
      node->UpdateReferenceID(oldID.c_str(), newID.c_str());
      }
    }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::AddReferencedNodes(vtkMRMLNode *node, vtkCollection *refNodes, bool recursive/*=true*/)
{
  if (!node || !node->GetID() || !refNodes)
    {
    vtkErrorMacro("AddReferencedNodes: null node or reference nodes");
    return;
    }

  std::deque<vtkMRMLNode*> newFoundReferencedNodes;

  for (NodeReferencesType::iterator referenceIt = this->NodeReferences.begin();
    referenceIt != this->NodeReferences.end();
    ++referenceIt)
    {
    NodeReferencesType::value_type::second_type::iterator referringNodesIt = referenceIt->second.find(node->GetID());
    if (referringNodesIt!=referenceIt->second.end())
      {
      // this ID is referenced by this node
      vtkMRMLNode *referencedNode = this->GetNodeByID(referenceIt->first);
      if (referencedNode!=nullptr && !refNodes->IsItemPresent(referencedNode))
        {
        // this ID is not yet in the list of reference nodes, so add it
        refNodes->AddItem(referencedNode);
        newFoundReferencedNodes.push_back(referencedNode);
        }
      }
    }

  if (recursive)
    {
    // recursively add all the referenced nodes' referenced nodes
    for (std::deque<vtkMRMLNode*>::iterator newReferencedNodeIt=newFoundReferencedNodes.begin();
      newReferencedNodeIt!=newFoundReferencedNodes.end();
      ++newReferencedNodeIt)
      {
      this->AddReferencedNodes(*newReferencedNodeIt, refNodes);
      }
    }
}

//------------------------------------------------------------------------------
vtkCollection* vtkMRMLScene::GetReferencedNodes(vtkMRMLNode *node, bool recursive/*=true*/)
{
  vtkCollection* nodes = vtkCollection::New();
  if (node != nullptr)
    {
    nodes->AddItem(node);
    this->AddReferencedNodes(node, nodes, recursive);
    }
  return nodes;
}

//-----------------------------------------------------------------------------
void vtkMRMLScene::GetReferencingNodes(vtkMRMLNode* referencedNode, std::vector<vtkMRMLNode *> &referencingNodes)
{
  referencingNodes.clear();

  if (!referencedNode || referencedNode->GetID() == nullptr)
    {
    vtkErrorMacro("GetReferencingNodes: null node or referenced node");
    return;
    }
  const char* referencedId=referencedNode->GetID();

  NodeReferencesType::iterator referencedNodeIdIt=this->NodeReferences.find(referencedId);

  if (referencedNodeIdIt==this->NodeReferences.end())
    {
    // no references to this node
    return;
    }
  for (NodeReferencesType::value_type::second_type::iterator referringNodesIt = referencedNodeIdIt->second.begin();
    referringNodesIt != referencedNodeIdIt->second.end();
    ++referringNodesIt)
    {
    vtkMRMLNode* node=this->GetNodeByID(*referringNodesIt);
    if (node)
      {
      referencingNodes.push_back(node);
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

  //assuming the nodes exist in this scene
  this->NodeReferences=scene->NodeReferences;
}

//------------------------------------------------------------------------------
void vtkMRMLScene::CopyNodeChangedIDs(vtkMRMLScene *scene)
{
  if (!scene)
    {
    return;
    }

  this->ReferencedIDChanges = scene->ReferencedIDChanges;
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

//-----------------------------------------------------------------------------
void vtkMRMLScene::AddNodeID(vtkMRMLNode *node)
{
  if (this->Nodes && node && node->GetID())
    {
    this->NodeIDs[std::string(node->GetID())] = node;
    this->NodeIDsMTime = this->Nodes->GetMTime();
    }
}

//-----------------------------------------------------------------------------
void vtkMRMLScene::RemoveNodeID(char *nodeID)
{
  if (this->Nodes && nodeID)
    {
    this->NodeIDs.erase(std::string(nodeID));
    this->NodeIDsMTime = this->Nodes->GetMTime();
    }
}

//-----------------------------------------------------------------------------
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
  if (this->GetURIHandlerCollection() == nullptr)
    {
    return;
    }
  if (handler == nullptr)
    {
    return;
    }
  this->GetURIHandlerCollection()->AddItem(handler);
}

//------------------------------------------------------------------------------
vtkURIHandler * vtkMRMLScene::FindURIHandlerByName(const char *name)
{
  vtkURIHandler *u;
  if ( name == nullptr )
    {
    vtkErrorMacro("FindURIHandlerByName: name is null.");
    return nullptr;
    }
  if (this->GetURIHandlerCollection() == nullptr)
    {
    vtkWarningMacro("FindURIHandlerByName: No URI handlers registered on the scene.");
    return nullptr;
    }
  for (int i = 0; i < this->GetURIHandlerCollection()->GetNumberOfItems(); i++)
    {
    vtkObject *object = this->GetURIHandlerCollection()->GetItemAsObject(i);
    if (object == nullptr)
      {
      vtkErrorMacro("FindURIHandlerByName: got a null handler at index " << i);
      return nullptr;
      }
    u = vtkURIHandler::SafeDownCast(object);
    if ( u == nullptr )
      {
      vtkErrorMacro("FindURIHandlerByName: Got nullptr URIHandler from URIHandlerCollection." );
      return nullptr;
      }
    if (u->GetName() != nullptr && !strcmp(u->GetName(), name))
      {
      vtkDebugMacro("FindURIHandlerByName: found a handler with name " << name << " at index " << i << " in the handler collection");
      return u;
      }
    }
  vtkWarningMacro("FindURIHandlerByName: unable to find a URI handler in the collection of " << this->GetURIHandlerCollection()->GetNumberOfItems() << " handlers to match the name " << name);
  return nullptr;
}

//------------------------------------------------------------------------------
vtkURIHandler * vtkMRMLScene::FindURIHandler(const char *URI)
{
  if (URI == nullptr)
    {
    vtkErrorMacro("FindURIHandler: URI is null.");
    return nullptr;
    }
  if (this->GetURIHandlerCollection() == nullptr)
    {
    vtkWarningMacro("No URI handlers registered on the scene.");
    return nullptr;
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
  return nullptr;
}

//-----------------------------------------------------------------------------
void vtkMRMLScene
::GetReferencedSubScene(vtkMRMLNode *rnode, vtkMRMLScene* newScene)
{
  //
  // clear scene
  //
  newScene->Clear(1);

  if (rnode == nullptr)
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
  vtkMRMLNode *currentObject = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it);
    (currentObject = (vtkMRMLNode*)nodes->GetNextItemAsObject(it));)
    {
    vtkMRMLNode* n = vtkMRMLNode::SafeDownCast(currentObject);
    if (n == nullptr)
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
void vtkMRMLScene::SetErrorMessage(const std::string &error)
{
  this->ErrorMessage = error;
}

//-----------------------------------------------------------------------------
std::string vtkMRMLScene::GetErrorMessage()
{
  return this->ErrorMessage;
}

//-----------------------------------------------------------------------------
void vtkMRMLScene::SetSceneXMLString(const std::string &xmlString)
{
  this->SceneXMLString = xmlString;
}

//-----------------------------------------------------------------------------
const std::string& vtkMRMLScene::GetSceneXMLString()
{
  return this->SceneXMLString;
}

//-----------------------------------------------------------------------------
void vtkMRMLScene::SetErrorMessage(const char * message)
  {
  this->SetErrorMessage(std::string(message));
  }

//-----------------------------------------------------------------------------
const char * vtkMRMLScene::GetErrorMessagePointer()
  {
  return (this->GetErrorMessage().c_str());
  }

//----------------------------------------------------------------------------
vtkMRMLSubjectHierarchyNode* vtkMRMLScene::GetSubjectHierarchyNode()
{
  if (this->SubjectHierarchyNode == nullptr)
  {
    this->SetSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode::ResolveSubjectHierarchy(this));
    if (this->SubjectHierarchyNode == nullptr)
    {
      vtkErrorMacro("Unable to resolve subject hierarchy. No node available.");
    }
  }
  return this->SubjectHierarchyNode;
}

//-----------------------------------------------------------------------------
void vtkMRMLScene::SetSubjectHierarchyNode(vtkMRMLSubjectHierarchyNode* node)
{
  this->SubjectHierarchyNode = node;
}

//-----------------------------------------------------------------------------
bool vtkMRMLScene::GetModifiedSinceRead()
{
  int hideFromEditors = 0;

  // There is no need to save the scene if it does not have any displayable node.
  bool hasAtLeast1DisplayableNode =
    (this->GetFirstNode(nullptr, "vtkMRMLDisplayableNode", &hideFromEditors) != nullptr);
  if (!hasAtLeast1DisplayableNode)
    {
    return false;
    }

  vtkMTimeType latestNodeMTime = this->GetMTime();
  vtkMRMLNode *node;
  vtkCollectionSimpleIterator it;
  for (this->Nodes->InitTraversal(it);
    (node = (vtkMRMLNode*)this->Nodes->GetNextItemAsObject(it));)
    {
    if (node->IsA("vtkMRMLAbstractViewNode"))
      {
      // We do not consider view node changes as scene change,
      // because view nodes may change because application window is resized, etc.
      continue;
      }
    if (node->GetMTime() > latestNodeMTime)
      {
      latestNodeMTime = node->GetMTime();
      }
    }

  return  latestNodeMTime > this->StoredTime;
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

//-----------------------------------------------------------------------------
void vtkMRMLScene::SetStorableNodesModifiedSinceRead()
{
  vtkSmartPointer<vtkCollection> storableNodes;
  storableNodes.TakeReference(this->GetNodesByClass("vtkMRMLStorableNode"));
  vtkMRMLScene::SetStorableNodesModifiedSinceRead(storableNodes);
}

//-----------------------------------------------------------------------------
void vtkMRMLScene::
SetStorableNodesModifiedSinceRead(vtkCollection* storableNodes)
{
  vtkCollectionSimpleIterator it;
  vtkMRMLStorableNode* storableNode;
  for (storableNodes->InitTraversal(it);
       (storableNode = vtkMRMLStorableNode::SafeDownCast(
          storableNodes->GetNextItemAsObject(it))) ;)
    {
    if (!storableNode->GetHideFromEditors() &&
        !storableNode->GetModifiedSinceRead())
      {
      storableNode->StorableModified();
      }
    }
}

//-----------------------------------------------------------------------------
int vtkMRMLScene::GetNumberOfNodeReferences()
{
  int totalNumberOfReferences=0;
  for (NodeReferencesType::iterator referenceIt = this->NodeReferences.begin();
    referenceIt != this->NodeReferences.end();
    ++referenceIt)
    {
    totalNumberOfReferences+=referenceIt->second.size();
    }
  return totalNumberOfReferences;
}

//-----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::GetNthReferencingNode(int n)
{
  for (NodeReferencesType::iterator referenceIt = this->NodeReferences.begin();
    referenceIt != this->NodeReferences.end();
    ++referenceIt)
    {
    if (n<static_cast<int>(referenceIt->second.size()))
      {
      NodeReferencesType::value_type::second_type::iterator referringNodesIt=referenceIt->second.begin();
      std::advance( referringNodesIt, n );
      return this->GetNodeByID(*referringNodesIt);
      }
    n-=referenceIt->second.size();
    }
  return nullptr;
}

//-----------------------------------------------------------------------------
const char* vtkMRMLScene::GetNthReferencedID(int n)
{
  for (NodeReferencesType::iterator referenceIt = this->NodeReferences.begin();
    referenceIt != this->NodeReferences.end();
    ++referenceIt)
    {
    if (n<static_cast<int>(referenceIt->second.size()))
      {
      //NodeReferencesType::value_type::second_type::iterator referringNodesIt=referenceIt->second.begin();
      return referenceIt->first.c_str();
      }
    n-=referenceIt->second.size();
    }
  return nullptr;
}

//-----------------------------------------------------------------------------
void vtkMRMLScene::SetMaximumNumberOfSavedUndoStates(int stackSize)
{
  if (stackSize == this->MaximumNumberOfSavedUndoStates)
    {
    return;
    }

  if (stackSize < 0)
    {
    vtkErrorMacro("Cannot set maximum stack size to be a value less than 0");
    }

  this->MaximumNumberOfSavedUndoStates = stackSize;
  this->TrimUndoStack();
  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkMRMLScene::TrimUndoStack()
{
  std::list<vtkSmartPointer<vtkCollection> > removedStacks;
  while(static_cast<int>(this->UndoStack.size()) > this->MaximumNumberOfSavedUndoStates)
    {
    removedStacks.push_back(this->UndoStack.front());
    this->UndoStack.pop_front();
    }
}
