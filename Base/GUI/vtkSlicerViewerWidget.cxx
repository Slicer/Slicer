#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"

#include "vtkSlicerViewerWidget.h"
#include "vtkSlicerColor.h"

#include "vtkActor.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderWindow.h"

#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
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
}


//---------------------------------------------------------------------------
vtkSlicerViewerWidget::~vtkSlicerViewerWidget ( )
{
  this->RemoveMRMLObservers();

  if (this->MainViewer)
    {
    this->MainViewer->RemoveAllViewProps ( );
    this->MainViewer->Delete();
    this->MainViewer = NULL;
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
  if ( vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene 
    && (event == vtkMRMLScene::NodeAddedEvent || event == vtkMRMLScene::NodeRemovedEvent ) )
    {
    vtkMRMLNode *node = (vtkMRMLNode*) (callData);
    if (node->IsA("vtkMRMLModelNode") )
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

  this->MainViewer = vtkKWRenderWidget::New ( );  
  this->MainViewer->SetParent (this->GetParent() );
  this->MainViewer->Create ( );

  //this->MainViewer->SetRendererBackgroundColor (vtkSlicerColor::ViewerBlue ); 
  this->MainViewer->GetRenderer()->GetActiveCamera()->ParallelProjectionOff();
    
    // set up antialiasing
  this->MainViewer->GetRenderWindow()->LineSmoothingOn();
  this->MainViewer->GetRenderWindow()->PolygonSmoothingOn ( );
  this->MainViewer->GetRenderWindow()->PointSmoothingOn();
  // this->MainViewer->SetMultiSamples ( 4 );

  this->Script  ("pack %s -side top -fill both -expand y -padx 0 -pady 0",
                 this->MainViewer->GetWidgetName ( ) );
  this->MainViewer->ResetCamera ( );

  // observe scene for add/remove nodes
  this->AddMRMLObserver(this->MRMLScene, vtkMRMLScene::NodeAddedEvent);
  this->AddMRMLObserver(this->MRMLScene, vtkMRMLScene::NodeRemovedEvent);
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UpdateFromMRML()
{
  vtkMRMLScene *scene = this->GetMRMLScene();
  vtkMRMLNode *node = NULL;
  
  this->RemoveProps ( );
  
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
 
    vtkActor *actor = this->DisplayedModels.find(model->GetID())->second;
    this->SetModelDisplayProperty(model, actor);

    } // end while

    this->RequestRender ( );
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
void vtkSlicerViewerWidget::RemoveProps()
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
void vtkSlicerViewerWidget::RemoveMRMLObservers()
{
  std::map<const char *, vtkActor *>::iterator iter;
  for(iter=this->DisplayedModels.begin(); iter != this->DisplayedModels.end(); iter++) 
    {
    vtkMRMLModelNode *model = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(iter->first));
    if (model == NULL)
      {
      model->RemoveObservers ( vtkMRMLModelNode::PolyDataModifiedEvent, this->MRMLCallbackCommand );
      model->RemoveObservers ( vtkMRMLModelNode::DisplayModifiedEvent, this->MRMLCallbackCommand );
      model->RemoveObservers ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand );
      }
    }
  this->RemoveMRMLObserver(this->MRMLScene, vtkMRMLScene::NodeAddedEvent);
  this->RemoveMRMLObserver(this->MRMLScene, vtkMRMLScene::NodeRemovedEvent);

  
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
  std::map<const char *, vtkActor *>::iterator iter = this->DisplayedModels.find(id);
  if ( iter != this->DisplayedModels.end())
    {
    return iter->second;
    }


  return (NULL);
}

