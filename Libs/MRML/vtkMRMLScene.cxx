/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLScene.cxx,v $
Date:      $Date: 2006/03/17 15:10:09 $
Version:   $Revision: 1.18 $

=========================================================================auto=*/
#include <sstream>
#include <map>

//#include <hash_map>
#include "vtkObjectFactory.h"
#include "vtkErrorCode.h"
#include "vtkCommand.h"
#include "vtkGeneralTransform.h"

#include "vtkMRMLConfigure.h" // MRML_USE*

#include "vtkMRMLScene.h"
#include "vtkMRMLParser.h"

#include "vtkMRMLBSplineTransformNode.h"
#include "vtkMRMLCameraNode.h"
#include "vtkMRMLClipModelsNode.h"
#include "vtkMRMLColorNode.h"
#include "vtkMRMLColorTableStorageNode.h"
#include "vtkMRMLDiffusionTensorDisplayPropertiesNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLFreeSurferModelOverlayStorageNode.h"
#include "vtkMRMLFreeSurferModelStorageNode.h"
#include "vtkMRMLGridTransformNode.h"
#include "vtkMRMLLabelMapVolumeDisplayNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLNonlinearTransformNode.h"
#include "vtkMRMLROIListNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLSceneSnapshotNode.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLSliceCompositeNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLSnapshotClipNode.h"
#include "vtkMRMLTransformStorageNode.h"
#include "vtkMRMLUnstructuredGridDisplayNode.h"
#include "vtkMRMLUnstructuredGridNode.h"
#include "vtkMRMLUnstructuredGridStorageNode.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkMRMLVolumeHeaderlessStorageNode.h"
#include "vtkURIHandler.h"
#include "vtkMRMLLayoutNode.h"

#ifdef MRML_USE_vtkTeem
#include "vtkMRMLNRRDStorageNode.h"
#include "vtkMRMLDiffusionTensorVolumeDisplayNode.h"
#include "vtkMRMLDiffusionTensorVolumeNode.h"
#include "vtkMRMLFiberBundleGlyphDisplayNode.h"
#include "vtkMRMLFiberBundleLineDisplayNode.h"
#include "vtkMRMLFiberBundleNode.h"
#include "vtkMRMLFiberBundleStorageNode.h"
#include "vtkMRMLFiberBundleTubeDisplayNode.h"
#include "vtkMRMLVectorVolumeNode.h"
#endif

#include <vtksys/SystemTools.hxx>

//------------------------------------------------------------------------------
vtkMRMLScene::vtkMRMLScene() 
{
  this->NodeIDsMTime = 0;
  this->SceneModifiedTime = 0;
  
  this->ClassNameList = NULL;
  this->RegisteredNodeClasses.clear();
  this->UniqueIDByClass.clear();

  this->CurrentScene =  vtkCollection::New();
  this->UndoStackSize = 100;
  this->UndoFlag = true;
  this->InUndo = false;

  this->ReferencedIDs.clear() ;
  this->ReferencingNodes.clear();
  this->ReferencedIDChanges.clear();


  this->CacheManager = NULL;
  this->DataIOManager = NULL;
  this->URIHandlerCollection = NULL;

  this->ErrorCode = 0;

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
  
  vtkMRMLScalarVolumeNode *svoln = vtkMRMLScalarVolumeNode::New(); 
  this->RegisterNodeClass( svoln );
  svoln->Delete();
  
  vtkMRMLFiducialListNode *fidln = vtkMRMLFiducialListNode::New(); 
  this->RegisterNodeClass( fidln );
  fidln->Delete();

  vtkMRMLROIListNode *roiln = vtkMRMLROIListNode::New(); 
  this->RegisterNodeClass( roiln );
  roiln->Delete();
  
  vtkMRMLSliceCompositeNode *scompn = vtkMRMLSliceCompositeNode::New(); 
  this->RegisterNodeClass( scompn );
  scompn->Delete();
  
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

  vtkMRMLFiberBundleNode *fbn =
                         vtkMRMLFiberBundleNode::New();
  this->RegisterNodeClass (fbn);
  fbn->Delete();

  vtkMRMLFiberBundleLineDisplayNode *fbldn =
                         vtkMRMLFiberBundleLineDisplayNode::New();
  this->RegisterNodeClass (fbldn);
  fbldn->Delete();
  
  vtkMRMLFiberBundleTubeDisplayNode *fbtdn =
                         vtkMRMLFiberBundleTubeDisplayNode::New();
  this->RegisterNodeClass (fbtdn);
  fbtdn->Delete();
  
  vtkMRMLFiberBundleGlyphDisplayNode *fbgdn =
                         vtkMRMLFiberBundleGlyphDisplayNode::New();
  this->RegisterNodeClass (fbgdn);
  fbgdn->Delete();

  vtkMRMLNRRDStorageNode *nrrd = vtkMRMLNRRDStorageNode::New();
  this->RegisterNodeClass ( nrrd );
  nrrd->Delete();

  vtkMRMLFiberBundleStorageNode *fbsn =
                         vtkMRMLFiberBundleStorageNode::New();
  this->RegisterNodeClass (fbsn);
  fbsn->Delete();

  vtkMRMLVectorVolumeNode *vvoln = vtkMRMLVectorVolumeNode::New(); 
  this->RegisterNodeClass( vvoln );
  vvoln->Delete();
  
#endif
  
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
  
  vtkMRMLModelHierarchyNode *mhier = vtkMRMLModelHierarchyNode::New();
  this->RegisterNodeClass ( mhier );
  mhier->Delete();

  vtkMRMLSceneSnapshotNode *sshot = vtkMRMLSceneSnapshotNode::New();
  this->RegisterNodeClass ( sshot );
  sshot->Delete();

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
  
  if ( this->CurrentScene != NULL ) {
      this->CurrentScene->RemoveAllItems ( );
      this->CurrentScene->Delete();
      this->CurrentScene = NULL;
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

}

//------------------------------------------------------------------------------
void vtkMRMLScene::Clear(int removeSingletons) 
{
  this->SetUndoOff();
  this->InvokeEvent(this->SceneClosingEvent, NULL);
  
  if (!removeSingletons)
    {
    this->RemoveAllNodesExceptSingletons();
    this->ClearReferencedNodeID();
    this->InvokeEvent(this->SceneCloseEvent, NULL);
    this->ResetNodes();
    }
  else
    {
    this->CurrentScene->RemoveAllItems();
    this->InvokeEvent(this->SceneCloseEvent, NULL);
    }
  
  this->ClearReferencedNodeID();


  this->ClearUndoStack ( );
  this->ClearRedoStack ( );
  this->UniqueIDByClass.clear();


  this->Modified();
  this->SetUndoOn();
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RemoveAllNodesExceptSingletons()
{
  vtkMRMLNode *node;
  this->InitTraversal();
  std::vector<vtkMRMLNode *> removeNodes;
  std::vector< vtkMRMLNode* > referencingNodes;
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
      this->CurrentScene->vtkCollection::RemoveItem(node);
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

vtkMRMLScene *vtkMRMLScene::ActiveScene = NULL;

//------------------------------------------------------------------------------
void vtkMRMLScene::SetActiveScene(vtkMRMLScene *mrmlScene) {
  vtkMRMLScene::ActiveScene = mrmlScene;
}

//------------------------------------------------------------------------------
vtkMRMLScene *vtkMRMLScene::GetActiveScene() {
  return (vtkMRMLScene::ActiveScene);
}

//------------------------------------------------------------------------------
vtkMRMLScene* vtkMRMLScene::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLScene");
  if(ret) 
    {
    return (vtkMRMLScene*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLScene;
}

//------------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLScene::CreateNodeByClass(const char* className) 
{
  if (className == NULL)
    {
    vtkErrorMacro("CreateNodeByClass: class name is null");
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
      }
    }
  return node;
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RegisterNodeClass(vtkMRMLNode* node) 
{
  node->Register(this);
  this->RegisteredNodeClasses.push_back(node);
  this->RegisteredNodeTags.push_back(std::string(node->GetNodeTagName()));
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
int vtkMRMLScene::Connect()
{
  this->SetErrorCode(0);
  this->SetErrorMessage(std::string(""));

  bool undoFlag = this->GetUndoFlag();
  
  this->SetUndoOff();
  
  this->RemoveAllNodesExceptSingletons();
  this->ClearReferencedNodeID();

  this->InvokeEvent(this->SceneCloseEvent, NULL);

  this->ClearUndoStack ( );
  this->ClearRedoStack ( );
  this->UniqueIDByClass.clear();
  
  int res = this->Import();


  if (!res)
    {
    this->InvokeEvent(vtkMRMLScene::SceneLoadingErrorEvent);
    }
    
  this->SetUndoFlag(undoFlag);
  return res;
}

//------------------------------------------------------------------------------
int vtkMRMLScene::Import()
{
  this->SetErrorCode(0);
  this->SetErrorMessage(std::string(""));

  bool undoFlag = this->GetUndoFlag();
  int n;

  this->SetUndoOff();
  this->ClearReferencedNodeID();

  // read nodes into a temp scene  
  vtkCollection* scene = vtkCollection::New();
  
  int res = this->LoadIntoScene(scene);
  
  std::map<std::string, vtkMRMLNode *> nodesAddedByClass;

  if (res)
    {
    vtkMRMLNode *node;
    int nnodes = scene->GetNumberOfItems();
    for (n=0; n<nnodes; n++) 
      {
      node = (vtkMRMLNode *)scene->GetItemAsObject(n);
      this->AddNodeNoNotify(node);
      }

    // fix node refrences that may be not unique in the imported scene.
    this->UpdateNodeReferences(scene);

    this->InvokeEvent(this->NewSceneEvent, NULL);

    for (n=0; n<nnodes; n++) 
      {
      //double progress = n / (1. * nnodes);
      //this->InvokeEvent(vtkCommand::ProgressEvent,(void *)&progress);

      node = (vtkMRMLNode *)scene->GetItemAsObject(n);
      nodesAddedByClass[std::string(node->GetClassName())] = node;
      if (node->GetAddToScene())
        {
        node->UpdateScene(this);
        }
      }
   
    // send one NodeAddedEvent event per class
    std::map<std::string, vtkMRMLNode *>::iterator iter; 
    for(iter = nodesAddedByClass.begin(); iter != nodesAddedByClass.end(); iter++)
      {
      this->InvokeEvent(this->NodeAddedEvent, iter->second);        
      }
      
    this->Modified();
    }

  scene->RemoveAllItems();
  scene->Delete();

  this->SetUndoFlag(undoFlag);
  //this->ClearReferencedNodeID();

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
  return returnCode;
}

//------------------------------------------------------------------------------
int vtkMRMLScene::LoadIntoScene(vtkCollection* nodeCollection)
{
  if (this->URL == "") 
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
  this->RootDirectory = vtksys::SystemTools::GetParentDirectory(this->GetURL());   
  if ( this->RootDirectory[0] != '\0' )
    {
    this->RootDirectory = this->RootDirectory + vtksys_stl::string("/");
    }
  vtkMRMLParser* parser = vtkMRMLParser::New();
  parser->SetMRMLScene(this);
  if (nodeCollection != this->CurrentScene)
    {
    parser->SetNodeCollection(nodeCollection);
    }
  parser->SetFileName(URL.c_str());
  int result = parser->Parse();
  parser->Delete();

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
    else
      {
      vtkErrorMacro("Commit: URL is not set");
      this->SetErrorCode(vtkErrorCode::GetErrorCodeFromString("CannotOpenFileError"));
      return 1;
      }
    }

  vtkMRMLNode *node;
  ofstream file;
  int indent=0, deltaIndent;
  
  // Open file
#ifdef _WIN32
  file.open(url, std::ios::out | std::ios::binary);
#else
  file.open(url, std::ios::out);
#endif
  if (file.fail()) 
    {
    vtkErrorMacro("Write: Could not open file " << url);
#if (VTK_MAJOR_VERSION <= 5)      
    this->SetErrorCode(2);
#else
    this->SetErrorCode(vtkErrorCode::GetErrorCodeFromString("CannotOpenFileError"));
#endif
    return 1;
    }
  
  //file << "<?xml version=\"1.0\" standalone='no'?>\n";
  //file << "<!DOCTYPE MRML SYSTEM \"mrml20.dtd\">\n";
  file << "<MRML>\n";
   
  // Write each node
  int n;
  for (n=0; n < this->CurrentScene->GetNumberOfItems(); n++) 
    {
    node = (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(n);
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
    file << vindent << "<" << node->GetNodeTagName() << "\n";

    node->WriteXML(file, indent);
    
    file << vindent << ">";
    node->WriteNodeBodyXML(file, indent);
    file << "</" << node->GetNodeTagName() << ">\n";
    
    if ( deltaIndent > 0 ) 
      {
      indent += 2;
      }    
    }
  
  file << "</MRML>\n";
  
  // Close file
  file.close();
#if (VTK_MAJOR_VERSION <= 5)      
  this->SetErrorCode(0);
#else
  this->SetErrorCode(vtkErrorCode::GetErrorCodeFromString("NoError"));
#endif
  return 1;
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RequestNodeID(vtkMRMLNode *node, const char *ID)
{
    if (node == NULL || ID == NULL)
      {
      return;
      }
    // is this ID in use?
    if (this->GetNodeByID(ID) == NULL)
      {
      // not used yet, set it
      vtkDebugMacro("vtkMRMLScene::RequestNodeID: ID " << ID << " not in use, setting node's ID");
      node->SetID(ID);
      }
    else
      {
      // it's used already, get a unique id postfix for this class and use
      // that in conjunction with the ID
      node->ConstructAndSetID(ID, GetUniqueIDIndexByClass(node->GetClassName()));
      vtkDebugMacro("vtkMRMLScene::RequestNodeID: ID " << ID << " was in use, set the node id to " << node->GetID());
      }
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

  //TODO convert URL to Root directory
  //n->SetSceneRootDir("");
  
  // check if node is a singletone 
  if (n->GetSingletonTag() != NULL)
    {
    // check if there is a singletone of this class in the scene 
    // and if found copy this node into it
    int numNodes = GetNumberOfNodesByClass(n->GetClassName());
    for (int i=0; i<numNodes; i++)
      {
      vtkMRMLNode *sn = this->GetNthNodeByClass(i, n->GetClassName());
      if (sn->GetSingletonTag() != NULL && strcmp(sn->GetSingletonTag(),
                                                  n->GetSingletonTag()) == 0)
        {
        sn->CopyWithSceneWithSingleModifiedEvent(n);
        this->RemoveNodeReferences(n);
        return sn;
        }
      }
    }
  if (n->GetID() == NULL || n->GetID()[0] == '\0' || this->GetNodeByID(n->GetID()) != NULL) 
    {
    std::string oldID;
    if (n->GetID())
      {
      oldID = n->GetID();
      }
    //n->SetID(this->GetUniqueIDByClass(n->GetClassName()));
    n->ConstructAndSetID(n->GetClassName(), this->GetUniqueIDIndexByClass(n->GetClassName()));
    vtkDebugMacro("AddNodeNoNotify: got unique id for new " << n->GetClassName() << " node: " << n->GetID() << endl);
    std::string newID(n->GetID());
    if (oldID != newID)
      {
      this->ReferencedIDChanges[oldID] = newID;
      }
    }

  n->SetSceneRootDir(this->RootDirectory.c_str());
  if (n->GetName() == NULL|| n->GetName()[0] == '\0')
    {
    n->SetName(n->GetID());
    }
  n->SetScene( this );
  this->CurrentScene->vtkCollection::AddItem((vtkObject *)n);
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

  vtkMRMLNode* node = this->AddNodeNoNotify(n);
  this->InvokeEvent(this->NodeAddedEvent, n);
  this->Modified();
  return node;
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
    vtkErrorMacro("RemoveNode: unable to remove null node");
    return;
    }
  n->Register(this);
  
  this->RemoveNodeReferences(n);
  this->RemoveReferencesToNode(n);
  
  this->CurrentScene->vtkCollection::RemoveItem((vtkObject *)n);
  this->InvokeEvent(this->NodeRemovedEvent, n);
  n->UnRegister(this);

  vtkMRMLNode *node = NULL;
  for (int i=0; i < this->CurrentScene->GetNumberOfItems(); i++) 
    {
    node = (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(i);
    node->UpdateReferences();
    }
  //this->Modified();
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RemoveNodeNoNotify(vtkMRMLNode *n) 
{
  if (n == NULL)
    {
    vtkErrorMacro("RemoveNodeNoNotify: unable to remove null node");
    return;
    }
  n->Register(this);
  
  this->RemoveNodeReferences(n);
  this->RemoveReferencesToNode(n);

  this->CurrentScene->vtkCollection::RemoveItem((vtkObject *)n);
  
  n->UnRegister(this);

  vtkMRMLNode *node = NULL;
  for (int i=0; i < this->CurrentScene->GetNumberOfItems(); i++) 
    {
    node = (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(i);
    node->UpdateReferences();
    }
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RemoveReferencedNodeID(const char *id, vtkMRMLNode *refrencingNode) 
{
  if (id == NULL || refrencingNode == NULL)
    {
    vtkErrorMacro("RemoveReferencedNodeID: either id is null or the reference node is null.");
    return;
    }
  std::vector< std::string > referencedIDs;
  std::vector< vtkMRMLNode* > referencingNodes;
  //vtkMRMLNode *node = NULL;

  int nnodes = this->ReferencingNodes.size();
  int i=0;
  for( i=0; i<nnodes; i++)
    {
    vtkMRMLNode *node = this->ReferencingNodes[i];
    if (node && node->GetID() && refrencingNode->GetID() && !strcmp(node->GetID(), refrencingNode->GetID())&&
        id && this->ReferencedIDs[i].c_str() && !strcmp(id, this->ReferencedIDs[i].c_str()) )
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
  this->ReferencedIDs = referencedIDs;
  this->ReferencingNodes = referencingNodes;
}

//------------------------------------------------------------------------------
void vtkMRMLScene::RemoveNodeReferences(vtkMRMLNode *n) 
{
  if (n == NULL && n->GetID() == NULL)
    {
    vtkErrorMacro("RemoveNodeReferences: node is null or has null id, can't remove it");
    return;
    }

  std::vector< std::string > referencedIDs;
  std::vector< vtkMRMLNode* > referencingNodes;
  //vtkMRMLNode *node = NULL;

  int nnodes = this->ReferencingNodes.size();
  int i=0;
  for( i=0; i<nnodes; i++)
    {
    vtkMRMLNode *node = this->ReferencingNodes[i];
    if (node && node->GetID() && strcmp(node->GetID(), n->GetID())) 
      {
      referencedIDs.push_back(this->ReferencedIDs[i]);
      referencingNodes.push_back(this->ReferencingNodes[i]);
      }
    }
  this->ReferencedIDs = referencedIDs;
  this->ReferencingNodes = referencingNodes;
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
  std::vector< vtkMRMLNode* > referencingNodes;
  
  int nnodes = this->ReferencingNodes.size();
  int i=0;
  for( i=0; i<nnodes; i++)
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
int vtkMRMLScene::GetNumberOfNodesByClass(const char *className)
{
  if (className == NULL)
    {
    vtkErrorMacro("GetNumberOfNodesByClass: class name is null.");
    return 0;
    }
  int num=0;
  vtkMRMLNode *node;
  int n;
  for (n=0; n < this->CurrentScene->GetNumberOfItems(); n++) 
    {
    node = (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(n);
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
  int n;
  for (n=0; n < this->CurrentScene->GetNumberOfItems(); n++) 
    {
    node = (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(n);
    if (node->IsA(className)) 
      {
      nodes.push_back(node);
      }
    }
  return nodes.size();
}

//------------------------------------------------------------------------------
std::list< std::string > vtkMRMLScene::GetNodeClassesList()
{
  std::list< std::string > classes;

  vtkMRMLNode *node;
  for (int n=0; n < this->CurrentScene->GetNumberOfItems(); n++) 
    {
    node = (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(n);
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
  if ( !this || !this->CurrentScene)
    {
    return NULL;
    }
  if (!className)
    {
    vtkErrorMacro("GetNextNodeByClass: class name is null.");
    return NULL;
    }

  vtkMRMLNode *node = (vtkMRMLNode*)this->CurrentScene->GetNextItemAsObject();

  while (node != NULL && !node->IsA(className)) 
    {
    node = (vtkMRMLNode*)this->CurrentScene->GetNextItemAsObject();
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
vtkMRMLNode* vtkMRMLScene::GetNthNode(int n)
{

  if(n < 0 || n >= this->CurrentScene->GetNumberOfItems()) 
    {
    return NULL;
    }
  else 
    {
    return (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(n);
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
  for (int nn=0; nn < this->CurrentScene->GetNumberOfItems(); nn++) 
    {
    node = (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(nn);
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
  for (int n=0; n < this->CurrentScene->GetNumberOfItems(); n++) 
    {
    node = (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(n);
    if (!strcmp(node->GetName(), name)) 
      {
      nodes->AddItem(node);
      }
    }
  
  return nodes;
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
    return NULL;
    }
  if (id == NULL) 
    {
    vtkDebugMacro("GetNodeByID: id is null");
    return NULL;
    }

  vtkMRMLNode *node = NULL;
  this->UpdateNodeIDs();
  //vtksys::hash_map<const char*, vtkMRMLNode*>::iterator it = this->NodeIDs.find(id);
  std::map<std::string, vtkMRMLNode*>::iterator it = this->NodeIDs.find(std::string(id));
  if (it != this->NodeIDs.end())
    {
    node = it->second;
    }
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
  for (int n=0; n < this->CurrentScene->GetNumberOfItems(); n++) 
    {
    node = (vtkMRMLNode*)this->CurrentScene->GetItemAsObject(n);
    if (!strcmp(node->GetName(), name) && node->IsA(className)) 
      {
      nodes->AddItem(node);
      }
    }

  return nodes;
}

//------------------------------------------------------------------------------
int  vtkMRMLScene::GetTransformBetweenNodes(vtkMRMLNode *node1,
                                            vtkMRMLNode *node2, 
                                            vtkGeneralTransform *xform)
{
  vtkErrorMacro("NOT IMPLEMENTEED YET");
  return 1;
}


//------------------------------------------------------------------------------
void vtkMRMLScene::InsertAfterNode(vtkMRMLNode *item, vtkMRMLNode *n)
{
  vtkErrorMacro("NOT IMPLEMENTEED YET");
}

//------------------------------------------------------------------------------
void vtkMRMLScene::InsertBeforeNode(vtkMRMLNode *item, vtkMRMLNode *n)
{
  vtkErrorMacro("NOT IMPLEMENTEED YET");
}

//------------------------------------------------------------------------------
void vtkMRMLScene::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  this->CurrentScene->vtkCollection::PrintSelf(os,indent);
  std::list<std::string> classes = this->GetNodeClassesList();

  std::list< std::string >::const_iterator iter;
  // Iterate through list and output each element.
  for (iter = classes.begin(); iter != classes.end(); iter++) 
    {
    std::string className = (*iter);
    os << indent << "Number Of Nodes for class " << className.c_str() << " : " << this->GetNumberOfNodesByClass(className.c_str()) << "\n";
    }
}

//------------------------------------------------------------------------------
int vtkMRMLScene::GetUniqueIDIndexByClass(const char* className)
{
  if (!className)
    {
    vtkErrorMacro("GetUniqueIDIndexByClass: class name is null");
    return -1;
    }
  int hint = 1;
  std::map< std::string, int>::iterator it = this->UniqueIDByClass.find(std::string(className));
  if (it != this->UniqueIDByClass.end())
    {
    hint = it->second;
    }
  int index = this->GetUniqueIDIndexByClassFromIndex(className, hint);
  this->UniqueIDByClass[std::string(className)] = index+1;
  return index;
}

//------------------------------------------------------------------------------
int vtkMRMLScene::GetUniqueIDIndexByClassFromIndex(const char* className, int hint)
{
  if (!className)
    {
    vtkErrorMacro("GetUniqueIDIndexByClassFromIndex: class name is null");
    return -1;
    }
    
  // keep looping until you find an id that isn't yet in the scene
  // TODO: this could be speeded up if it becomes a bottleneck
  int index;
  std::string candidateName;
  for (index = hint; ; index++)
    {
    std::stringstream ss;
    ss << className;
    ss << index;
    ss >> candidateName;
    if ( this->GetNodeByID( candidateName.c_str() ) == NULL )
      {
      break;
      }
    }
  return index;
}

//------------------------------------------------------------------------------
const char* vtkMRMLScene::GetUniqueNameByString(const char* className)
{
  if (!className)
    {
    vtkErrorMacro("GetUniqueNameByString: class name is null");
    return "null";
    }
  
  std::string sname(className);
  if (UniqueIDByClass.find(sname) == UniqueIDByClass.end() ) 
    {
    UniqueIDByClass[className] = 1;
    }
  int id = UniqueIDByClass[sname];

  std::string name;

  while (true) 
    {
    std::stringstream ss;
    ss << className;
    ss << id;
    name = ss.str();
    bool nameExists = false;
    unsigned int i;
    for (i=0; i< UniqueIDs.size(); i++) 
      {
      if (UniqueIDs[i] == name) 
        {
        nameExists = true;
        break;
        }
      }
    if (nameExists) 
      {
      id++;
      continue;
      }
    else 
      {
      // double check that no node has this name
      vtkCollection *nodesByName = this->GetNodesByName(name.c_str());
      int numberOfItems = nodesByName->GetNumberOfItems();
      nodesByName->Delete();
      if ( numberOfItems == 0 )
        {
        break;
        }
      else
        {
        vtkDebugMacro("GetUniqueNameByString: Thought we had a valid index " << id << ", but found a name that matches " << name.c_str() << endl);
        nameExists = true;
        id++;
        continue;
        }
      }
    }
  UniqueIDByClass[className] = id + 1;
  UniqueIDs.push_back(name);
  return UniqueIDs[UniqueIDs.size()-1].c_str();
}

//------------------------------------------------------------------------------
// Pushes the current scene onto the undo stack, and makes a backup copy of the 
// passed node so that changes to the node are undoable; several signatures to handle 
// individual nodes or a vtkCollection of nodes, or a vector of nodes
//
void vtkMRMLScene::SaveStateForUndo (vtkMRMLNode *node)
{
  if (this->InUndo) 
    {
    return;
    }

  this->ClearRedoStack();
  this->SetUndoOn();
  this->PushIntoUndoStack();
  if ( node )
    {
    this->CopyNodeInUndoStack(node);
    }
} 

//------------------------------------------------------------------------------
void vtkMRMLScene::SaveStateForUndo (std::vector<vtkMRMLNode *> nodes)
{
  if (this->InUndo) 
    {
    return;
    }
  this->ClearRedoStack();
  this->SetUndoOn();
  this->PushIntoUndoStack();
  unsigned int n;
  for (n=0; n<nodes.size(); n++) 
    {
    vtkMRMLNode *node = nodes[n];
    this->CopyNodeInUndoStack(node);
    }
} 

//------------------------------------------------------------------------------
void vtkMRMLScene::SaveStateForUndo (vtkCollection* nodes)
{
  if (this->InUndo) 
    {
    return;
    }

  if (!nodes)
    {
    return;
    }
  
  this->ClearRedoStack();
  this->SetUndoOn();
  this->PushIntoUndoStack();

  int nnodes = nodes->GetNumberOfItems();
  
  for (int n=0; n<nnodes; n++) 
    {
    vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(nodes->GetItemAsObject(n));
    if (node) 
      {
      this->CopyNodeInUndoStack(node);
      }
    }
} 

//------------------------------------------------------------------------------
void vtkMRMLScene::SaveStateForUndo ()
{
  if (this->CurrentScene)
    {
    this->SaveStateForUndo(this->CurrentScene);
    }
} 

//------------------------------------------------------------------------------
// Make a new collection that has pointers to all the nodes in the current scene
void vtkMRMLScene::PushIntoUndoStack()
{
  if (this->CurrentScene == NULL) 
    {
    return;
    }

  vtkCollection* newScene = vtkCollection::New();

  vtkCollection* currentScene = this->CurrentScene;

  int nnodes = currentScene->GetNumberOfItems();

  for (int n=0; n<nnodes; n++) 
    {
    vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(currentScene->GetItemAsObject(n));
    if (node) 
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
  if (this->CurrentScene == NULL) 
    {
    return;
    }

  vtkCollection* newScene = vtkCollection::New();

  vtkCollection* currentScene = this->CurrentScene;
  
  int nnodes = currentScene->GetNumberOfItems();

  for (int n=0; n<nnodes; n++) 
    {
    vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(currentScene->GetItemAsObject(n));
    if (node) 
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
    snode->CopyWithSceneWithSingleModifiedEvent(copyNode);
    }
  vtkCollection* undoScene = dynamic_cast < vtkCollection *>( this->UndoStack.back() );
  int nnodes = undoScene->GetNumberOfItems();
  for (int n=0; n<nnodes; n++) 
    {
    vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(undoScene->GetItemAsObject(n));
    if (node == copyNode) 
      {
      undoScene->ReplaceItem (n, snode);
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
      }
    }
  snode->Delete();
}

//------------------------------------------------------------------------------
// Replace the current scene by the top of the undo stack
// -- move the current scene on the redo stack
void vtkMRMLScene::Undo()
{
  if (this->UndoStack.size() == 0) 
    {
    return;
    }

  this->InUndo = true;

  int nnodes;
  int n;
  unsigned int nn;

  PushIntoRedoStack();

  vtkCollection* currentScene = this->CurrentScene;
  //std::hash_map<std::string, vtkMRMLNode*> currentMap;
  std::map<std::string, vtkMRMLNode*> currentMap;
  nnodes = currentScene->GetNumberOfItems();
  for (n=0; n<nnodes; n++) 
    {
    vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(currentScene->GetItemAsObject(n));
    if (node) 
      {
      currentMap[node->GetID()] = node;
      }
    }

  std::map<std::string, vtkMRMLNode*> undoMap;
  //std::hash_map<std::string, vtkMRMLNode*> undoMap;

  vtkCollection* undoScene = NULL;

  if (!this->UndoStack.empty())
    {
    undoScene = dynamic_cast < vtkCollection *>( this->UndoStack.back() );
    nnodes = undoScene->GetNumberOfItems();
    for (n=0; n<nnodes; n++) 
      {
      vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(undoScene->GetItemAsObject(n));
      if (node) 
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
      // nodes differ, copy from undo to current scene
      // but before create a copy in redo stack from current
      this->CopyNodeInRedoStack(curIter->second);
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
  if (this->RedoStack.size() == 0) 
    {
    return;
    }

  int nnodes;
  int n;
  unsigned int nn;

  PushIntoUndoStack();

  vtkCollection* currentScene = this->CurrentScene;
  //std::hash_map<std::string, vtkMRMLNode*> currentMap;
  std::map<std::string, vtkMRMLNode*> currentMap;
  nnodes = currentScene->GetNumberOfItems();
  for (n=0; n<nnodes; n++) 
    {
    vtkMRMLNode *node  = dynamic_cast < vtkMRMLNode *>(currentScene->GetItemAsObject(n));
    if (node) 
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
      if (node) 
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
  
  vtksys_stl::vector<vtksys_stl::string> components;
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
void vtkMRMLScene::UpdateNodeReferences()
{
  std::map< std::string, std::string>::const_iterator iterChanged;
  std::map< std::string, vtkMRMLNode*>::const_iterator iterNodes;
  vtkMRMLNode *node;
  int i;

  for (iterChanged = this->ReferencedIDChanges.begin(); iterChanged != this->ReferencedIDChanges.end(); iterChanged++) 
    {
    std::vector< std::string > referencedIDs = this->ReferencedIDs;
    std::vector< vtkMRMLNode* > referencingNodes = this->ReferencingNodes;

    int nupdates = referencedIDs.size();
    for (i=0; i<nupdates; i++)
      {
      if (iterChanged->first == referencedIDs[i])
        {
        node = referencingNodes[i];
        node->UpdateReferenceID(iterChanged->first.c_str(), iterChanged->second.c_str());
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
  std::map< std::string, vtkMRMLNode*>::const_iterator iterNodes;
  vtkMRMLNode *node;
  int i;

  for (iterChanged = this->ReferencedIDChanges.begin(); iterChanged != this->ReferencedIDChanges.end(); iterChanged++) 
    {
    std::vector< std::string > referencedIDs = this->ReferencedIDs;
    std::vector< vtkMRMLNode* > referencingNodes = this->ReferencingNodes;
    int nupdates = referencedIDs.size();

    for (i=0; i<nupdates; i++)
      {
      if (iterChanged->first == referencedIDs[i])
        {
        node = referencingNodes[i];
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
  int nnodes = this->ReferencingNodes.size();
  std::vector< std::string > ids;
  int n=0;
  for( n=0; n<nnodes; n++)
    {
    vtkMRMLNode *rnode = this->ReferencingNodes[n];
    if (rnode && rnode->GetID() && !strcmp(rnode->GetID(), node->GetID())) 
      {
      ids.push_back(ReferencedIDs[n]);
      }
    }
  nnodes = ids.size();
  for (n=0; n<nnodes; n++)
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

//------------------------------------------------------------------------------
void vtkMRMLScene::UpdateNodeIDs()
{
  if (this->CurrentScene->GetNumberOfItems() == 0)
    {
    this->NodeIDs.clear();
    }
  else if (this->CurrentScene->GetMTime() > this->NodeIDsMTime)
    {
    this->NodeIDs.clear();
    vtkMRMLNode *node;
    unsigned int nnodes = this->CurrentScene->GetNumberOfItems();
    for (unsigned int n=0; n<nnodes; n++)
      {
      node = this->GetNthNode(n);
      this->NodeIDs[std::string(node->GetID())] = node;
      }
    }
  this->NodeIDsMTime = this->CurrentScene->GetMTime();
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
  for (int i = 0; i < this->GetURIHandlerCollection()->GetNumberOfItems(); i++)
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
