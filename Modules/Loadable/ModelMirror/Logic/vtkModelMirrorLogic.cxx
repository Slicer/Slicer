#include "vtkModelMirrorLogic.h"
// MRML includes
#include "vtkCacheManager.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLFreeSurferModelStorageNode.h"
#include "vtkMRMLTransformStorageNode.h"

// ITKSYS includes
#include <itksys/SystemTools.hxx>

// VTK includes
#include <vtkCleanPolyData.h>
#include <vtkGeneralTransform.h>
#include <vtkObjectFactory.h>
#include <vtkPolyDataNormals.h>

// STD includes

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkModelMirrorLogic);



//----------------------------------------------------------------------------
vtkModelMirrorLogic::vtkModelMirrorLogic()
{

   this->ModelMirrorNode = NULL;
   this->MirrorMatrix = NULL;
   this->MirrorTransformNode = NULL;
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
  if ( this->MirrorTransformNode )
    {
    this->MirrorTransformNode->Delete();
    this->MirrorTransformNode = NULL;
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
  if ( this->GetMRMLScene() == NULL )
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
/*
  if ( this->ModelMirrorNode->GetOutputModel() )
    {
    this->GetMRMLScene()->RemoveNode ( this->ModelMirrorNode->GetOutputModel() );
    this->ModelMirrorNode->GetOutputModel()->Delete();
    }
*/

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

  //---
  //--- Filename and model name wrangling for new model (derived from input model)
  //---
  int useURI = 0;
  if ( this->GetMRMLScene()->GetCacheManager() != NULL )
    {
    useURI = this->GetMRMLScene()->GetCacheManager()->IsRemoteReference (inputStorageNode->GetFileName() );
    }
  std::string localFile;
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
    std::string extension = localFile.substr ( index );
    std::string newURI = localFile.substr( 0, index );
    newURI += "_Mirror";
    newURI += extension;

    localFile.clear();
    localFile = newURI;

    mStorageNode->SetFileName(localFile.c_str());
    fsmStorageNode->SetFileName(localFile.c_str());

    const std::string fname(localFile.c_str());

    // the model name is based on the file name (itksys call should work even if
    // file is not on disk yet)
    const std::string name = itksys::SystemTools::GetFilenameName(fname);

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
    this->GetMRMLScene()->AddNode(storageNode);
    this->GetMRMLScene()->AddNode(displayNode);
    mirrorModelNode->SetAndObserveStorageNodeID(storageNode->GetID());
    mirrorModelNode->SetAndObserveDisplayNodeID(displayNode->GetID());
    this->GetMRMLScene()->AddNode(mirrorModelNode);

    //--- Note: model has no PolyData yet.
    //--- grab a copy of its polydata for further processing.
    vtkPolyData *surface =   this->ModelMirrorNode->GetInputModel()->GetPolyData();
    vtkPolyData *mpolys = vtkPolyData::New();
    mpolys->DeepCopy ( surface );
    if ( mpolys )
      {
      this->ModelMirrorNode->GetOutputModel()->SetAndObservePolyData (mpolys );
      }
    mpolys->Delete();
    mpolys = NULL;
    }
  else
    {
    vtkErrorMacro ( "CreateMirrorModel: Got NULL Storage Node." );
    }

  //---
  //--- inherit the color and other display properties from its source
  //---
//  mirrorModelNode->GetDisplayNode()->Copy( inputModelNode->GetDisplayNode() );
  if ( inputModelNode->GetDisplayNode() )
    {
    if ( inputModelNode->GetDisplayNode()->GetColor() )
      {
      mirrorModelNode->GetDisplayNode()->SetColor (inputModelNode->GetDisplayNode()->GetColor() );
      }
    }
//  mirrorModelNode->GetDisplayNode()->SetDiffuse (inputModelNode->GetDisplayNode()->GetDiffuse() );
//  mirrorModelNode->GetDisplayNode()->SetAmbient (inputModelNode->GetDisplayNode()->GetAmbient() );
//  mirrorModelNode->GetDisplayNode()->SetSpecular (inputModelNode->GetDisplayNode()->GetSpecular() );
//  mirrorModelNode->GetDisplayNode()->SetOpacity (inputModelNode->GetDisplayNode()->GetOpacity() );
//  mirrorModelNode->GetDisplayNode()->SetPower (inputModelNode->GetDisplayNode()->GetPower() );
//  mirrorModelNode->GetDisplayNode()->SetClipping (inputModelNode->GetDisplayNode()->GetClipping() );
//  mirrorModelNode->GetDisplayNode()->SetSliceIntersectionVisibility (inputModelNode->GetDisplayNode()->GetSliceIntersectionVisibility() );

  //---
  //--- disable backface culling on mirrored model
  //---
  mirrorModelNode->GetDisplayNode()->SetBackfaceCulling(0);

  //---
  //--- clean up
  //---
  mirrorModelNode->Delete();
  mStorageNode->Delete();
  fsmStorageNode->Delete();
  displayNode->Delete();
  //---
  mirrorModelNode = NULL;
  mStorageNode = NULL;
  fsmStorageNode = NULL;
  displayNode = NULL;

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
    case vtkMRMLModelMirrorNode::SaggitalMirror:
      this->MirrorMatrix->SetElement (0, 0, -1.0 );
      break;
    case vtkMRMLModelMirrorNode::CoronalMirror:
      this->MirrorMatrix->SetElement ( 1, 1, -1.0 );
      break;
    case vtkMRMLModelMirrorNode::AxialMirror:
      this->MirrorMatrix->SetElement ( 2, 2, -1.0 );
      break;
    default:
      break;
    }

}

//----------------------------------------------------------------------------
void vtkModelMirrorLogic::DeleteMirrorTransform()
{
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ( "DeleteMirrorTransform: Got NULL MRMLScene.");
    return;
    }
  this->GetMRMLScene()->RemoveNode ( this->MirrorTransformNode );
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
  if ( this->ModelMirrorNode->GetOutputModel() == NULL )
    {
    vtkErrorMacro ( "CreateMirrorTransform: Got NULL OutputModel.");
    return;
    }
  if ( this->GetMRMLScene() == NULL )
    {
    vtkErrorMacro ( "CreateMirrorTransform: Got NULL MRMLScene.");
    return;
    }

  //---
  //---   Start with a fresh transform
  //---
/*
  if ( this->GetMirrorTransformNode() )
    {
    this->GetMRMLScene()->RemoveNode ( this->GetMirrorTransformNode() );
    this->GetMirrorTransformNode()->Delete();
    }
*/

  //---
  //---  Create the transform node and accompaniments
  //---
  vtkMRMLLinearTransformNode *mirrorTransformNode = vtkMRMLLinearTransformNode::New();
  vtkMRMLTransformStorageNode *storageNode = vtkMRMLTransformStorageNode::New();
  mirrorTransformNode->SetScene ( this->GetMRMLScene());
  storageNode->SetScene ( this->GetMRMLScene());

  //---
  //--- Configure the node and storage node.
  //---
  mirrorTransformNode->SetName ( "mirror");
  this->GetMRMLScene()->AddNode ( storageNode );
  this->GetMRMLScene()->AddNode(mirrorTransformNode );
  this->SetMirrorTransformNode ( mirrorTransformNode );
  mirrorTransformNode->SetAndObserveStorageNodeID ( storageNode->GetID() );

  //---
  //--- create and install  the mirroring matrix
  //---
  this->CreateMirrorMatrix ( );
  if ( this->MirrorMatrix != NULL )
    {
    vtkMatrix4x4 *m = vtkMatrix4x4::New();
    for ( int row = 0; row < 4; row++ )
      {
      for ( int col = 0; col < 4; col++ )
        {
        m->SetElement ( row, col, this->GetMirrorMatrix()->GetElement ( row, col ));
        }
      }
    mirrorTransformNode->SetMatrixTransformToParent(m);
    m->Delete();
    }

  //---
  //--- Set the parent transform of the output node to be this transform node.
  //---
  this->ModelMirrorNode->GetOutputModel()->SetAndObserveTransformNodeID (mirrorTransformNode->GetID() );

  //---
  // clean up
  //---
  if ( storageNode )
    {
    storageNode->Delete();
    storageNode = NULL;
    }
  if ( mirrorTransformNode )
    {
    mirrorTransformNode->Delete();
    mirrorTransformNode = NULL;
    }

}




//----------------------------------------------------------------------------
int vtkModelMirrorLogic::HardenTransform()
{


  //---
  //--- NULL pointer checking
  //---
  if ( this->GetMirrorTransformNode() == NULL )
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

  //---
  //--- Harden the mirror transform
  //---
  const char *id = this->ModelMirrorNode->GetOutputModel()->GetID();
  vtkMRMLTransformableNode *tbnode = vtkMRMLTransformableNode::SafeDownCast ( this->GetMRMLScene()->GetNodeByID(id));
  if ( tbnode == NULL )
    {
    vtkErrorMacro ( << "Output model doesn't appear to be correctly constructed. Mirroring output may be incorrect." );
    return ( 0 );
    }

  if (this->GetMirrorTransformNode()->IsTransformToWorldLinear())
    {
    vtkMatrix4x4* hardeningMatrix = vtkMatrix4x4::New();
    this->GetMirrorTransformNode()->GetMatrixTransformToWorld( hardeningMatrix);
    tbnode->ApplyTransformMatrix(hardeningMatrix);
    hardeningMatrix->Delete();
    }
  else
    {
    vtkGeneralTransform* hardeningTransform = vtkGeneralTransform::New();
    this->GetMirrorTransformNode()->GetTransformToWorld(hardeningTransform);
    this->GetMirrorTransformNode()->ApplyTransform(hardeningTransform);
    hardeningTransform->Delete();
    }

  tbnode->SetAndObserveTransformNodeID(NULL);

  return ( 1 );
}



//----------------------------------------------------------------------------
int vtkModelMirrorLogic::PositionInHierarchy ()
{

  if ( this->ModelMirrorNode == NULL )
    {
    vtkErrorMacro ( "" );
    return (0);
    }
  if ( this->ModelMirrorNode->GetInputModel() == NULL )
    {
    vtkErrorMacro ( "" );
    return (0);
    }
  if ( this->ModelMirrorNode->GetOutputModel() == NULL )
    {
    vtkErrorMacro ( "" );
    return (0);
    }

  //--- set the parent transform of this model node to be the same as input node
/*
  vtkMRMLTransformNode *tnode = this->ModelMirrorNode->GetInputModel()->GetParentTransformNode();
  if ( tnode != NULL )
    {
    this->ModelMirrorNode->GetOutputModel()->SetAndObserveTransformNodeID ( tnode->GetID() );
    }
*/
  return (1);
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

  //--- NOTE: This filter recomputes normals for polygons and
  //--- triangle strips only. Normals are not computed for lines or vertices.
  //--- Triangle strips are broken up into triangle polygons.
  //--- Polygons are not automatically re-stripped.
  vtkPolyDataNormals *normals = vtkPolyDataNormals::New();
  vtkAlgorithmOutput *polyDataConnection =
    this->ModelMirrorNode->GetOutputModel()->GetPolyDataConnection();
  normals->SetInputConnection(polyDataConnection);
  //--- NOTE: This assumes a completely closed surface
  //---(i.e. no boundary edges) and no non-manifold edges.
  //--- If these constraints do not hold, the AutoOrientNormals
  //--- is not guaranteed to work.
  normals->AutoOrientNormalsOn();
  //--- Turn on the global flipping of normal orientation.
  //--- This reverves the meaning of "front" and "back" for
  //--- Frontface and Backface culling.
  //--- Flipping modifies both the normal direction
  //--- and the order of a cell's points.
  normals->FlipNormalsOn();
  normals->SplittingOff();
  //--- enforce consistent polygon ordering.
  normals->ConsistencyOn();
  //normals->Update();

  //--- now get the output of the flip & cleaner and put into new model node.
  vtkCleanPolyData *cleaner = vtkCleanPolyData::New();
  cleaner->SetInputConnection( normals->GetOutputPort() );
  //cleaner->Update();

  //--- refresh polydata
  this->ModelMirrorNode->GetOutputModel()->SetPolyDataConnection ( cleaner->GetOutputPort() );
  cleaner->Delete();
  cleaner= NULL;

  normals->Delete();
  normals = NULL;
  return 1;
}


