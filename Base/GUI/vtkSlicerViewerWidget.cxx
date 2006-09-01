#include <string>
#include <sstream>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

#include "vtkSlicerViewerWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerColor.h"

#include "vtkActor.h"
#include "vtkTextActor3D.h"
#include "vtkProperty.h"
#include "vtkTexture.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkPolyDataMapper.h"
#include "vtkGlyphSource2D.h"
#include "vtkRenderWindow.h"

#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLFiducialListNode.h"
#include "vtkMRMLFiducialListDisplayNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScene.h"
#include "vtkKWWidget.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerViewerWidget );
vtkCxxRevisionMacro ( vtkSlicerViewerWidget, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerViewerWidget::vtkSlicerViewerWidget ( )
{
  this->MainViewer = NULL;  
  this->RenderPending = 0;  
  this->ViewerFrame = NULL;
  this->ProcessingMRMLEvent = 0;
}


//---------------------------------------------------------------------------
vtkSlicerViewerWidget::~vtkSlicerViewerWidget ( )
{
  this->RemoveMRMLObservers();
  
  if (this->MainViewer)
    {
      this->SetMRMLScene ( NULL );
      this->MainViewer->RemoveAllViewProps ( );
      this->MainViewer->SetParent ( NULL );
      this->MainViewer->Delete();
      this->MainViewer = NULL;
      this->ViewerFrame->SetParent ( NULL );
      this->ViewerFrame->Delete ( );
      this->ViewerFrame = NULL;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "vtkSlicerViewerWidget: " << this->GetClassName ( ) << "\n";
    // print widgets?
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::ProcessWidgetEvents ( vtkObject *caller,
                                                  unsigned long event, 
                                                  void *callData )
{
} 

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::ProcessMRMLEvents ( vtkObject *caller,
                                                unsigned long event, 
                                                void *callData )
{ 
  if (this->ProcessingMRMLEvent != 0 )
    return;

  this->ProcessingMRMLEvent = event;

  if ( vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene 
    && (event == vtkMRMLScene::NodeAddedEvent || event == vtkMRMLScene::NodeRemovedEvent ) )
    {
    vtkMRMLNode *node = (vtkMRMLNode*) (callData);
    if (node != NULL && node->IsA("vtkMRMLModelNode") )
      {
      this->UpdateFromMRML();
      }
    }
  else 
//  if ((vtkPolyData::SafeDownCast(caller) && event == vtkCommand::ModifiedEvent) ||
//      (vtkMRMLModelDisplayNode::SafeDownCast(caller) && event == vtkCommand::ModifiedEvent))
    {
      this->UpdateFromMRML();
    }
  this->ProcessingMRMLEvent = 0;
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::RemoveWidgetObservers ( ) 
{
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::CreateWidget ( )
{
  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
  
  // Call the superclass to create the whole widget
  
  this->Superclass::CreateWidget();

  this->ViewerFrame = vtkKWFrame::New ( );
  this->ViewerFrame->SetParent ( this->GetParent ( ) );
  this->ViewerFrame->Create ( );
  
  this->MainViewer = vtkKWRenderWidget::New ( );  
  this->MainViewer->SetParent (this->ViewerFrame );
  this->MainViewer->Create ( );

  // Set the viewer's minimum dimension to be the same as that for
  // the three main Slice viewers.
  this->MainViewer->GetRenderer()->GetActiveCamera()->ParallelProjectionOff();
  if ( this->GetApplication() != NULL )
    {
      vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();      
      this->MainViewer->SetWidth ( app->GetMainLayout()->GetSliceViewerMinDim() );
      this->MainViewer->SetHeight ( app->GetMainLayout()->GetSliceViewerMinDim() );
    }
    
    // set up antialiasing
  this->MainViewer->GetRenderWindow()->LineSmoothingOn();
  this->MainViewer->GetRenderWindow()->PolygonSmoothingOn ( );
  this->MainViewer->GetRenderWindow()->PointSmoothingOn();
  // this->MainViewer->SetMultiSamples ( 4 );

  //this->PackWidget ( );
  this->MainViewer->ResetCamera ( );

  // observe scene for add/remove nodes
  this->AddMRMLObserver(this->MRMLScene, vtkMRMLScene::NodeAddedEvent);
  this->AddMRMLObserver(this->MRMLScene, vtkMRMLScene::NodeRemovedEvent);
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UpdateFromMRML()
{
  this->RemoveModelProps ( );
  this->RemoveFiducialProps ( );

  this->UpdateFiducialsFromMRML();
  this->UpdateModelsFromMRML();

  this->RequestRender ( );
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UpdateModelsFromMRML()
{
  vtkMRMLScene *scene = this->GetMRMLScene();
  vtkMRMLNode *node = NULL;
  
  scene->InitTraversal();
  while (node=scene->GetNextNodeByClass("vtkMRMLModelNode"))
    {
    vtkMRMLModelNode *model = vtkMRMLModelNode::SafeDownCast(node);

    // add nodes that are not in the list yet
    if (this->DisplayedModels.find(model->GetID()) == this->DisplayedModels.end() )
      {
      vtkPolyDataMapper *mapper = vtkPolyDataMapper::New ();
      mapper->SetInput ( model->GetPolyData() );

      // observe polydata
      model->AddObserver ( vtkMRMLModelNode::PolyDataModifiedEvent, this->MRMLCallbackCommand );

      // observe display node  
      model->AddObserver ( vtkMRMLModelNode::DisplayModifiedEvent, this->MRMLCallbackCommand );

      model->AddObserver ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand );

      vtkActor *actor = vtkActor::New ( );
      actor->SetMapper ( mapper );
      this->MainViewer->AddViewProp ( actor );

      this->DisplayedModels[model->GetID()] = actor;

      actor->Delete();
      mapper->Delete();
      } // end if
 
    //vtkActor *actor = this->DisplayedModels.find(model->GetID())->second;
    vtkActor *actor = this->DisplayedModels[ model->GetID() ];
    vtkPolyDataMapper *mapper = vtkPolyDataMapper::SafeDownCast (actor->GetMapper());
    mapper->SetInput ( model->GetPolyData() );
    this->SetModelDisplayProperty(model, actor);

    } // end while

}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UpdateFiducialsFromMRML()
{
  vtkMRMLScene *scene = this->GetMRMLScene();
  vtkMRMLNode *node = NULL;

  scene->InitTraversal();
  while (node=scene->GetNextNodeByClass("vtkMRMLFiducialListNode"))
    {
    vtkMRMLFiducialListNode *flist = vtkMRMLFiducialListNode::SafeDownCast(node);
    if (flist->HasObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand ) == 0)
      {
      flist->AddObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
      }
    if (flist->HasObserver ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand ) == 0)
      {
      flist->AddObserver ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand );
      }
      // observe display node  
    if (flist->HasObserver ( vtkMRMLFiducialListNode::DisplayModifiedEvent, this->MRMLCallbackCommand ) == 0)
      {
      flist->AddObserver ( vtkMRMLFiducialListNode::DisplayModifiedEvent, this->MRMLCallbackCommand );
      }

    for (int f=0; f<flist->GetNumberOfFiducials(); f++)
      {
      vtkMRMLFiducial *fnode = flist->GetNthFiducial(f);
      std::string aid = this->GetFiducialActorID(flist->GetID(), f);

      vtkGlyphSource2D *glyph = vtkGlyphSource2D::New();
      glyph->SetGlyphTypeToDiamond();

      vtkPolyDataMapper *mapper = vtkPolyDataMapper::New ();
      mapper->SetInput ( glyph->GetOutput() );

      //observe fiducial node data
      if (fnode->HasObserver(vtkCommand::ModifiedEvent, this->MRMLCallbackCommand ) == 0) 
        {
        fnode->AddObserver ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
        }
      vtkActor *actor = vtkActor::New ( );
      actor->SetMapper ( mapper );
      this->MainViewer->AddViewProp ( actor );

      // handle text
      vtkTextActor3D *textActor = vtkTextActor3D::New();
      textActor->SetInput(fnode->GetLabelText());
      this->MainViewer->AddViewProp ( textActor );

      this->SetFiducialDisplayProperty(flist, fnode, actor, textActor);

      this->DisplayedFiducials[aid.c_str()] = actor;
      this->DisplayedTextFiducials[aid.c_str()] = textActor;
      
      glyph->Delete();
      actor->Delete();
      mapper->Delete();
      textActor->Delete();
    } // and for
  } // end while
}


//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::RequestRender()
{
  if (this->GetRenderPending())
    {
    return;
    }

  this->SetRenderPending(1);
  this->Script("after idle \"%s Render\"", this->GetTclName());
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::Render()
{
  this->MainViewer->Render();
  this->SetRenderPending(0);
}


//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::RemoveModelProps()
{
  std::map<const char *, vtkActor *>::iterator iter;
  std::vector<const char *> removedIDs;
  for(iter=this->DisplayedModels.begin(); iter != this->DisplayedModels.end(); iter++) 
    {
    vtkMRMLModelNode *model = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(iter->first));
    if (model == NULL)
      {
      this->MainViewer->RemoveViewProp(iter->second);
      removedIDs.push_back(iter->first);
      }
    }
  for (unsigned int i=0; i< removedIDs.size(); i++)
    {
    this->DisplayedModels.erase(removedIDs[i]);
    }
  
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::RemoveFiducialProps()
{
  // glyph actors
  std::map<const char *, vtkActor *>::iterator iter;
  for(iter=this->DisplayedFiducials.begin(); iter != this->DisplayedFiducials.end(); iter++) 
    {
    this->MainViewer->RemoveViewProp(iter->second);
    }
  this->DisplayedFiducials.clear();

  // text actors
  std::map<const char *, vtkTextActor3D *>::iterator titer;
  for(titer=this->DisplayedTextFiducials.begin(); titer != this->DisplayedTextFiducials.end(); titer++) 
    {
    this->MainViewer->RemoveViewProp(titer->second);
    }
  this->DisplayedTextFiducials.clear();

}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::RemoveMRMLObservers()
{
  this->RemoveModelObservers();
  this->RemoveFiducialObservers();  
  
  this->RemoveMRMLObserver(this->MRMLScene, vtkMRMLScene::NodeAddedEvent);
  this->RemoveMRMLObserver(this->MRMLScene, vtkMRMLScene::NodeRemovedEvent);

}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::RemoveModelObservers()
{
  std::map<const char *, vtkActor *>::iterator iter;
  for(iter=this->DisplayedModels.begin(); iter != this->DisplayedModels.end(); iter++) 
    {
    vtkMRMLModelNode *model = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(iter->first));
    if (model != NULL)
      {
      model->RemoveObservers ( vtkMRMLModelNode::PolyDataModifiedEvent, this->MRMLCallbackCommand );
      model->RemoveObservers ( vtkMRMLModelNode::DisplayModifiedEvent, this->MRMLCallbackCommand );
      model->RemoveObservers ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand );
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::RemoveFiducialObservers()
{

  std::map<const char *, vtkActor *>::iterator iter;
  for(iter=this->DisplayedFiducials.begin(); iter != this->DisplayedFiducials.end(); iter++) 
    {
    const char *aid = iter->first;
    int index;
    std::string nid = this->GetFiducialNodeID(aid, index);
    vtkMRMLFiducialListNode *flist = vtkMRMLFiducialListNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(nid.c_str()));
    flist->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
    flist->RemoveObservers ( vtkMRMLFiducialListNode::DisplayModifiedEvent, this->MRMLCallbackCommand );
    flist->RemoveObservers ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand );
    if (flist)
      {
      vtkMRMLFiducial *fnode = flist->GetNthFiducial(index);

      if (fnode != NULL)
        {
        fnode->RemoveObservers ( vtkCommand::ModifiedEvent, this->MRMLCallbackCommand );
        }
      }
  }  
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::SetModelDisplayProperty(vtkMRMLModelNode *model,  vtkActor *actor)
{
  vtkMRMLTransformNode* tnode = model->GetParentTransformNode();
  if (tnode != NULL && tnode->IsLinear())
    {
    vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
    vtkMatrix4x4* transformToWorld = vtkMatrix4x4::New();
    transformToWorld->Identity();
    lnode->GetMatrixTransformToWorld(transformToWorld);
    actor->SetUserMatrix(transformToWorld);
    transformToWorld->Delete();
    }
  vtkMRMLModelDisplayNode* dnode = model->GetDisplayNode();
  if (dnode != NULL)
    {
    actor->SetVisibility(dnode->GetVisibility());
    actor->GetProperty()->SetColor(dnode->GetColor());
    actor->GetProperty()->SetOpacity(dnode->GetOpacity());
    actor->GetProperty()->SetAmbient(dnode->GetAmbient());
    actor->GetProperty()->SetDiffuse(dnode->GetDiffuse());
    actor->GetProperty()->SetSpecular(dnode->GetSpecular());
    actor->GetProperty()->SetSpecularPower(dnode->GetPower());
    if (dnode->GetTextureImageData() != NULL)
      {
      if (actor->GetTexture() == NULL)
        {
        vtkTexture *texture = vtkTexture::New();
        texture->SetInterpolate(1);
        actor->SetTexture(texture);
        texture->Delete();
        }
      actor->GetTexture()->SetInput(dnode->GetTextureImageData());
      }
    else
      {
      actor->SetTexture(NULL);
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::SetFiducialDisplayProperty(vtkMRMLFiducialListNode *flist, 
                                                       vtkMRMLFiducial *fnode,
                                                       vtkActor *actor, vtkTextActor3D *textActor)
{
  float *xyz = fnode->GetXYZ();
  actor->SetPosition(xyz[0], xyz[1], xyz[2]);
  actor->SetScale(flist->GetSymbolScale());

  textActor->SetPosition(xyz[0], xyz[1], xyz[2]);
  textActor->SetScale(flist->GetTextScale());

  vtkMRMLTransformNode* tnode = flist->GetParentTransformNode();
  if (tnode != NULL && tnode->IsLinear())
    {
    vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
    vtkMatrix4x4* transformToWorld = vtkMatrix4x4::New();
    transformToWorld->Identity();
    lnode->GetMatrixTransformToWorld(transformToWorld);
    actor->SetUserMatrix(transformToWorld);
    textActor->SetUserMatrix(transformToWorld);
    transformToWorld->Delete();
    }
  vtkMRMLFiducialListDisplayNode* dnode = flist->GetDisplayNode();
  if (dnode != NULL)
    {
    actor->SetVisibility(dnode->GetVisibility());
    actor->GetProperty()->SetColor(dnode->GetColor());
    actor->GetProperty()->SetOpacity(dnode->GetOpacity());
    actor->GetProperty()->SetAmbient(dnode->GetAmbient());
    actor->GetProperty()->SetDiffuse(dnode->GetDiffuse());
    actor->GetProperty()->SetSpecular(dnode->GetSpecular());
    actor->GetProperty()->SetSpecularPower(dnode->GetPower());
    actor->SetTexture(NULL);
    }
}

//---------------------------------------------------------------------------
  // Description:
  // return the current actor corresponding to a give MRML ID
vtkActor *
vtkSlicerViewerWidget::GetActorByID (const char *id)
{
  if ( !id )
    {
    return (NULL);
    }

  std::map<const char *, vtkActor *>::iterator iter;
  // search for matching string (can't use find, since it would look for 
  // matching pointer not matching content)
  for(iter=this->DisplayedModels.begin(); iter != this->DisplayedModels.end(); iter++) 
    {
    if ( iter->first && !strcmp( iter->first, id ) )
      {
      return (iter->second);
      }
    }
  return (NULL);
}

//---------------------------------------------------------------------------
std::string
vtkSlicerViewerWidget::GetFiducialActorID (const char *id, int index)
{
  std::stringstream ss;
  ss << id << " " << index;
  std::string sid;
  ss >> sid;
  return sid;
}

//---------------------------------------------------------------------------
std::string
vtkSlicerViewerWidget::GetFiducialNodeID (const char *actorid, int &index)
{
  std::stringstream ss;
  std::string sid;
  ss << actorid;
  ss >> sid;
  ss >> index;
  return sid;
}

//---------------------------------------------------------------------------
  // Description:
  // return the current actor corresponding to a give MRML ID
vtkActor *
vtkSlicerViewerWidget::GetFiducialActorByID (const char *id, int index)
{
  if ( !id )
    {
    return (NULL);
    }
  
  std::string sid = this->GetFiducialActorID(id, index);

  std::map<const char *, vtkActor *>::iterator iter;
  // search for matching string (can't use find, since it would look for 
  // matching pointer not matching content)
  for(iter=this->DisplayedFiducials.begin(); iter != this->DisplayedFiducials.end(); iter++) 
    {
    if ( iter->first && !strcmp( iter->first, sid.c_str() ) )
      {
      return (iter->second);
      }
    }
  return (NULL);
}


//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::PackWidget ( )
{
    this->Script  ("pack %s -side left -fill both -expand y -padx 0 -pady 0",
                   this->ViewerFrame->GetWidgetName ( ) );
    this->Script  ("pack %s -side top -anchor c  -fill both -expand y -padx 0 -pady 0",
                   this->MainViewer->GetWidgetName ( ) );
}


//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::GridWidget ( int row, int col )
{
    this->Script  ("grid %s -row %d -column %d -sticky news -padx 0 -pady 0",
                   this->ViewerFrame->GetWidgetName ( ), row, col );
    this->Script  ("pack %s -side top -anchor c  -fill both -expand y -padx 0 -pady 0",
                   this->MainViewer->GetWidgetName ( ) );
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UnpackWidget ( )
{
    this->Script ( "pack forget %s ", this->MainViewer->GetWidgetName ( ) );
    this->Script ( "pack forget %s ", this->ViewerFrame->GetWidgetName ( ) );
}

  
//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UngridWidget ( )
{
    this->Script ( "grid forget %s ", this->MainViewer->GetWidgetName ( ) );
    this->Script ( "pack forget %s ", this->ViewerFrame->GetWidgetName ( ) );
}
