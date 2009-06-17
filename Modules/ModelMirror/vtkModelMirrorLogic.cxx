#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <itksys/SystemTools.hxx>
#include <itksys/Directory.hxx>

#include "vtkObjectFactory.h"
#include "vtkIntArray.h"
#include "vtkPolyData.h"
#include "vtkPolyDataNormals.h"
#include "vtkCleanPolyData.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLModelStorageNode.h"
#include "vtkMRMLFreeSurferModelStorageNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLTransformableNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLTransformStorageNode.h"
#include "vtkModelMirrorLogic.h"
#include "vtkModelMirror.h"


vtkModelMirrorLogic* vtkModelMirrorLogic::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkModelMirrorLogic");
  if(ret)
    {
      return (vtkModelMirrorLogic*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkModelMirrorLogic;
}



//----------------------------------------------------------------------------
vtkModelMirrorLogic::vtkModelMirrorLogic()
{
   
   this->ModelMirrorNode = NULL;
   this->MirrorMatrix = NULL;
   this->MirrorTransform = NULL;
   this->Visited = false;
   this->Raised = false;
}



//----------------------------------------------------------------------------
vtkModelMirrorLogic::~vtkModelMirrorLogic()
{ 
  this->SetModelMirrorNode(NULL);
  this->SetAndObserveMRMLScene ( NULL );
  if ( this->MirrorMatrix )
    {
    this->MirrorMatrix->Delete();
    this->MirrorMatrix = NULL;
    }
  if ( this->MirrorTransform )
    {
    this->MirrorTransform->Delete();
    this->MirrorTransform = NULL;
    }
  this->Visited = false;  
  this->Raised = false;
}


//----------------------------------------------------------------------------
void vtkModelMirrorLogic::Enter()
{
  this->Visited = true;
  this->Raised = true;

}


//----------------------------------------------------------------------------
void vtkModelMirrorLogic::Exit()
{
  this->Raised = false;
}



//----------------------------------------------------------------------------
vtkIntArray* vtkModelMirrorLogic::NewObservableEvents()
{

  if ( !this->Visited )
    {
    return (NULL);
    }
  return NULL;
}




//----------------------------------------------------------------------------
void vtkModelMirrorLogic::ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData )
{

  if ( !this->Visited )
    {
    return;
    }
  if ( this->ModelMirrorNode == NULL )
    {
    return;
    }
  if ( !this->Raised )
    {
    return;
    }
}


//----------------------------------------------------------------------------
void vtkModelMirrorLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

}




//----------------------------------------------------------------------------
void vtkModelMirrorLogic::CreateMirrorModel ( )
{
  //---
  //--- NULL pointer checking
  //---
  if ( this->MRMLScene == NULL )
    {
    vtkErrorMacro ( "CreateMirrorModel: got NULL MRMLScene." );
    return;
    }
  if ( this->ModelMirrorNode == NULL )
    {
    vtkErrorMacro ( "CreateMirrorModel: got NULL ModelMirrorNode." );
    return;
    }
  if ( this->ModelMirrorNode->GetInputModel() == NULL )
    {
    vtkErrorMacro ( "CreateMirrorModel: got NULL input model.");
    return;
    }

  //---
  //--- Start with a clean model.
  //---
  if ( this->ModelMirrorNode->GetOutputModel() )
    {
    this->ModelMirrorNode->GetOutputModel()->Delete();
    this->ModelMirrorNode->SetOutputModel ( NULL );
    }

  //---
  //--- Get the input model stuff
  //---
  vtkMRMLModelNode *inputModelNode = this->ModelMirrorNode->GetInputModel();
  vtkMRMLStorageNode *inputStorageNode = inputModelNode->GetStorageNode();
  if ( inputStorageNode == NULL )
    {
    vtkErrorMacro ( "CreateMirrorModel: Got NULL StorageNode for the input Model." );
    return;
    }

  //---
  //--- Set up the new output model
  //---
  vtkMRMLModelNode *mirrorModelNode = vtkMRMLModelNode::New();
  this->ModelMirrorNode->SetOutputModel (mirrorModelNode); 
  vtkMRMLModelDisplayNode *displayNode = vtkMRMLModelDisplayNode::New();
  vtkMRMLModelStorageNode *mStorageNode = vtkMRMLModelStorageNode::New();
  vtkMRMLFreeSurferModelStorageNode *fsmStorageNode = vtkMRMLFreeSurferModelStorageNode::New();
  fsmStorageNode->SetUseStripper(0);  // turn off stripping by default (breaks some pickers)
  vtkMRMLStorageNode *storageNode = NULL;

  //--- copy poly data from input node.
  /*
  vtkPolyData *p = vtkPolyData::New();
  vtkCleanPolyData *cleaner = vtkCleanPolyData::New();
  cleaner->SetInput (this->ModelMirrorNode->GetInputModel()->GetPolyData() );
  cleaner->Update();
  p = cleaner->GetOutput();
  mirrorModelNode->SetAndObservePolyData ( p );
  displayNode->SetPolyData ( mirrorModelNode->GetPolyData() );
  p->Delete();
  cleaner->Delete();
  */  

  //---
  //--- Filename and model name wrangling for new model (derived from input model)
  //---
  int useURI = 0;
  if ( this->MRMLScene->GetCacheManager() != NULL )
    {
    useURI = this->GetMRMLScene()->GetCacheManager()->IsRemoteReference (inputStorageNode->GetFileName() );
    }
  itksys_stl::string name;
  itksys_stl::string localFile;
  itksys_stl::string newURI;
  itksys_stl::string extension;
  if ( useURI )
    {
    localFile = ((this->GetMRMLScene())->GetCacheManager())->GetFilenameFromURI(inputStorageNode->GetFileName());
    }
  else
    {
    localFile = inputStorageNode->GetFileName();
    }

  //--- use original file but tack a _Mirror onto the end.
    size_t index = localFile.find_last_of (".");
    extension = localFile.substr ( index );
    newURI = localFile.substr( 0, index );
    newURI += "_Mirror";
    newURI += extension;

    localFile.clear();
    localFile = newURI;

    mStorageNode->SetFileName(localFile.c_str());
    fsmStorageNode->SetFileName(localFile.c_str());

    const itksys_stl::string fname(localFile.c_str());

    // the model name is based on the file name (itksys call should work even if
    // file is not on disk yet)
    name = itksys::SystemTools::GetFilenameName(fname);
    
    // check to see which node can read this type of file
    if (mStorageNode->SupportedFileType(name.c_str()))
      {
      storageNode = mStorageNode;
      }
    else if (fsmStorageNode->SupportedFileType(name.c_str()))
      {
      storageNode = fsmStorageNode;
      }



  if (storageNode != NULL)
    {
    //--- set new name: use filename if user has not specified a valid name
    if ( this->ModelMirrorNode->GetNewModelName() == NULL )
      {
      mirrorModelNode->SetName(name.c_str());
      }
    else if (!(strcmp( this->ModelMirrorNode->GetNewModelName(), "" ) ))
      {
      mirrorModelNode->SetName(name.c_str());
      }
      else
      {
      mirrorModelNode->SetName(this->ModelMirrorNode->GetNewModelName() );
      }

    this->GetMRMLScene()->SaveStateForUndo();

    //---
    //--- Set MRML Scene and add nodes to scene
    //---
    mirrorModelNode->SetScene(this->GetMRMLScene());
    storageNode->SetScene(this->GetMRMLScene());
    displayNode->SetScene(this->GetMRMLScene()); 
    this->GetMRMLScene()->AddNodeNoNotify(storageNode);  
    this->GetMRMLScene()->AddNodeNoNotify(displayNode);
    mirrorModelNode->SetAndObserveStorageNodeID(storageNode->GetID());
    mirrorModelNode->SetAndObserveDisplayNodeID(displayNode->GetID());  

    this->GetMRMLScene()->AddNode(mirrorModelNode);
    mirrorModelNode->Delete();

    //---
    //--- Copy data from input model's storage node
    //---
    //storageNode->Copy( inputModelNode->GetStorageNode() );
    }
  else
    {
    vtkErrorMacro ( "CreateMirrorModel: Got NULL Storage Node." );
    mirrorModelNode->Delete();
    mirrorModelNode = NULL;
    }

  //---
  //--- clean up 
  //---
  mStorageNode->Delete();
  fsmStorageNode->Delete();
  displayNode->Delete();
}



//----------------------------------------------------------------------------
void vtkModelMirrorLogic::CreateMirrorMatrix( )
{
  //---
  //--- NULL pointer checking
  //---
  if ( this->ModelMirrorNode == NULL )
    {
    vtkErrorMacro ( "CreateMirrorMatrix: Got NULL ModelMirrorNode." );
    return;
    }
  if ( this->MirrorMatrix == NULL )
    {
    this->MirrorMatrix = vtkMatrix4x4::New();
    }
  else
    {
    this->MirrorMatrix->Identity();
    }
  
  //---
  //--- Figure out which is which
  //---
  switch ( this->ModelMirrorNode->GetMirrorPlane() )
    {
    case vtkMRMLModelMirrorNode::AxialMirror:
      this->MirrorMatrix->SetElement ( 1, 1, -1.0 );
      break;
    case vtkMRMLModelMirrorNode::SaggitalMirror:
      this->MirrorMatrix->SetElement (0, 0, -1.0 );
      break;
    case vtkMRMLModelMirrorNode::CoronalMirror:
      this->MirrorMatrix->SetElement ( 2, 2, -1.0 );
      break;
    default:
      break;
    }

}

//----------------------------------------------------------------------------
void vtkModelMirrorLogic::CreateMirrorTransform ( )
{
  //---
  //--- NULL pointer checking
  //---
  if ( this->ModelMirrorNode == NULL )
    {
    vtkErrorMacro ( "CreateMirrorTransform: Got NULL ModelMirrorNode." );
    return;
    }
  if ( this->ModelMirrorNode->GetInputModel() == NULL )
    {
    vtkErrorMacro ( "CreateMirrorTransform: Got NULL InputModel.");
    return;
    }

  //---
  //---   Start with a fresh transform
  //---
  if ( this->GetMirrorTransform() )
    {
    this->GetMirrorTransform()->Delete();
    this->SetMirrorTransform ( NULL );
    }

  //---
  //---  Create the transform node and accompaniments
  //---
  vtkMRMLLinearTransformNode *mirrorTransformNode = vtkMRMLLinearTransformNode::New();
  vtkMRMLTransformStorageNode *storageNode = vtkMRMLTransformStorageNode::New();
  mirrorTransformNode->SetScene ( this->MRMLScene);
  storageNode->SetScene ( this->MRMLScene);

  //---
  //--- Configure the node and storage node.
  //---
  mirrorTransformNode->SetName ( "mirror");
  this->MRMLScene->AddNodeNoNotify ( storageNode );
  this->MRMLScene->AddNode(mirrorTransformNode );
  this->SetMirrorTransform ( mirrorTransformNode );
  mirrorTransformNode->SetAndObserveStorageNodeID ( storageNode->GetID() );

  //---
  //--- 1. set the parent transform of this transform node to be the same as input node
  //--- 2. set the parent transform of the input node to be this transform node.
  //---
  vtkMRMLTransformNode *tnode = this->ModelMirrorNode->GetInputModel()->GetParentTransformNode();
  if ( tnode == NULL )
    {
    this->ModelMirrorNode->GetOutputModel()->SetAndObserveTransformNodeID ( NULL );
    }
  else
    {
    mirrorTransformNode->SetAndObserveTransformNodeID ( tnode->GetID() );
    this->ModelMirrorNode->GetOutputModel()->SetAndObserveTransformNodeID (mirrorTransformNode->GetID() );
    }

  //---
  //--- create and install  the mirroring matrix
  //---
  this->CreateMirrorMatrix ( );
  if ( this->MirrorMatrix != NULL )
    {
    vtkMatrix4x4 *m = mirrorTransformNode->GetMatrixTransformToParent();
    for ( int row = 0; row < 4; row++ )
      {
      for ( int col = 0; col < 4; col++ )
        {
        m->SetElement ( row, col, this->GetMirrorMatrix()->GetElement ( row, col ));
        }
      }

    // invoke event? probably don't need since this happens under the hood...
    vtkMRMLTransformableNode *tbnode = this->ModelMirrorNode->GetOutputModel();
    tbnode->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
    }

  //---
  // clean up
  //---
  if ( mirrorTransformNode )
    {
    mirrorTransformNode->Delete();
    }
  if ( storageNode )
    {
    storageNode->Delete();
    }
  
}




//----------------------------------------------------------------------------
int vtkModelMirrorLogic::HardenTransform()
{


  //---
  //--- NULL pointer checking
  //---
  if ( this->GetMirrorTransform() == NULL )
    {
    vtkErrorMacro(<<"HardenTransform: got a NULL Transform node.");
    return ( 0 );
    }
  if ( this->ModelMirrorNode == NULL )
    {
    vtkErrorMacro(<<"HardenTransform: got a NULL ModelMirrorNode.");
    return ( 0 );
    }
  if ( this->ModelMirrorNode->GetOutputModel() == NULL )
    {
    vtkErrorMacro(<<"HardenTransform: got a NULL Output model." );
    return ( 0 );
    }

  const char *id = this->ModelMirrorNode->GetInputModel()->GetID();
  vtkMRMLTransformableNode *tbnode = vtkMRMLTransformableNode::SafeDownCast ( this->MRMLScene->GetNodeByID(id));

  if (this->GetMirrorTransform()->IsTransformToWorldLinear())
    {
    vtkMatrix4x4* hardeningMatrix = vtkMatrix4x4::New();
    this->GetMirrorTransform()->GetMatrixTransformToWorld( hardeningMatrix);
    tbnode->ApplyTransform(hardeningMatrix);
    hardeningMatrix->Delete();
    }
  else
    {
    vtkGeneralTransform* hardeningTransform = vtkGeneralTransform::New();
    this->GetMirrorTransform()->GetTransformToWorld(hardeningTransform);
    this->GetMirrorTransform()->ApplyTransform(hardeningTransform);
    hardeningTransform->Delete();
    }

  tbnode->SetAndObserveTransformNodeID(NULL);
  tbnode->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);

  return ( 1 );
}





//----------------------------------------------------------------------------
int vtkModelMirrorLogic::FlipNormals()
{
  //---
  //--- NULL pointer checking
  //---
  if ( this->ModelMirrorNode == NULL )
    {
    vtkErrorMacro ( "FlipNormals: got NULL ModelMirrorNode." );
    return ( 0 );
    }
  if ( this->ModelMirrorNode->GetOutputModel() == NULL )
    {
    vtkErrorMacro ( "FlipNormals: got NULL OutputModel." );
    return ( 0 );
    }
  if ( this->ModelMirrorNode->GetOutputModel()->GetDisplayNode() == NULL )
    {
    vtkErrorMacro ( "FlipNormals: got NULL DisplayNode for OutputModel." );
    return ( 0 );
    }
  if ( this->ModelMirrorNode->GetInputModel()->GetDisplayNode()->GetPolyData() == NULL )
    {
    vtkErrorMacro ( "FlipNormals: got NULL PolyData." );
    return ( 0 );
    }

  
  vtkPolyData *surface =   this->ModelMirrorNode->GetInputModel()->GetPolyData();
  vtkPolyDataNormals *normals = vtkPolyDataNormals::New();
  normals->SetInput ( surface );
  normals->AutoOrientNormalsOn();
  normals->FlipNormalsOn();
  normals->ConsistencyOn();
  normals->Update();
  surface = normals->GetOutput();

  //--- now get the output of the flip & cleaner and put into new model node.
  if ( surface )
    {
    vtkCleanPolyData *cleaner = vtkCleanPolyData::New();
    cleaner->SetInput ( surface );
    cleaner->Update();
    vtkPolyData *p = vtkPolyData::New();
    p = cleaner->GetOutput();

    //--- This is causing trouble...
    if ( p )
      {
      this->ModelMirrorNode->GetOutputModel()->SetAndObservePolyData ( p );
      vtkMRMLModelDisplayNode *dnode = vtkMRMLModelDisplayNode::SafeDownCast (this->ModelMirrorNode->GetOutputModel()->GetDisplayNode() );
      if ( dnode )
        {
        dnode->SetPolyData ( this->ModelMirrorNode->GetOutputModel()->GetPolyData() );
        }
      }

    p->Delete();
    cleaner->Delete();
    }

  normals->Delete();
  if ( surface )
    {
    return ( 1 );
    }
  else
    {
    return ( 0 );
    }
}
