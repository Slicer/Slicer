#include <string>
#include <sstream>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkMatrix4x4.h"
#include "vtkRenderWindowInteractor.h"

#include "vtkSlicerViewerWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerColor.h"
#include "vtkSlicerGUILayout.h"
#include "vtkSlicerViewerInteractorStyle.h"

#include "vtkActor.h"
#include "vtkFollower.h"
#include "vtkProperty.h"
#include "vtkTexture.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkPolyDataMapper.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkOutlineSource.h"
#include "vtkVectorText.h"
#include "vtkRenderWindow.h"
#include "vtkImplicitBoolean.h"
#include "vtkPlane.h"
#include "vtkClipPolyData.h"

#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLClipModelsNode.h"

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

  this->CameraNode = NULL;

  this->ClipModelsNode = NULL;
  this->RedSliceNode = NULL;
  this->GreenSliceNode = NULL;
  this->YellowSliceNode = NULL;

  this->SlicePlanes = NULL;
  this->RedSlicePlane = NULL;
  this->GreenSlicePlane = NULL;
  this->YellowSlicePlane = NULL;

  this->ViewNode = NULL;
  this->BoxAxisActor = NULL;
}


//---------------------------------------------------------------------------
vtkSlicerViewerWidget::~vtkSlicerViewerWidget ( )
{
  this->RemoveMRMLObservers();

  vtkSetMRMLNodeMacro(this->ClipModelsNode, NULL);
  vtkSetMRMLNodeMacro(this->CameraNode, NULL);
  vtkSetMRMLNodeMacro(this->ViewNode, NULL);

  vtkSetMRMLNodeMacro(this->RedSliceNode, NULL);
  vtkSetMRMLNodeMacro(this->GreenSliceNode, NULL);
  vtkSetMRMLNodeMacro(this->YellowSliceNode, NULL);
  
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

  this->SlicePlanes->Delete();
  this->RedSlicePlane->Delete();
  this->GreenSlicePlane->Delete();
  this->YellowSlicePlane->Delete();

  if (this->BoxAxisActor)
    {
    this->BoxAxisActor->Delete();
    }
  for (unsigned int i=0; i<this->AxisLabelActors.size(); i++)
    {
    this->AxisLabelActors[i]->Delete();
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
void vtkSlicerViewerWidget::CreateClipSlices()
{
  this->SlicePlanes = vtkImplicitBoolean::New();
  this->SlicePlanes->SetOperationTypeToIntersection();
  this->RedSlicePlane = vtkPlane::New();
  this->GreenSlicePlane = vtkPlane::New();
  this->YellowSlicePlane = vtkPlane::New();

  this->ClipType = vtkMRMLClipModelsNode::ClipIntersection;
  this->RedSliceClipState = vtkMRMLClipModelsNode::ClipOff;
  this->YellowSliceClipState = vtkMRMLClipModelsNode::ClipOff;
  this->GreenSliceClipState = vtkMRMLClipModelsNode::ClipOff;
  this->ClippingOn = false;
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::CreateAxis()
{
  vtkOutlineSource *boxSource = vtkOutlineSource::New();
  vtkPolyDataMapper *boxMapper = vtkPolyDataMapper::New();
  boxMapper->SetInput( boxSource->GetOutput() );

  boxMapper->Update();
   
  this->BoxAxisActor = vtkActor::New();
  this->BoxAxisActor->SetMapper( boxMapper );
  this->BoxAxisActor->SetPickable(0);
  this->BoxAxisActor->SetScale(100, 100, 100);
  this->BoxAxisActor->GetProperty()->SetColor( 1.0, 0.0, 1.0 );

  this->AxisLabelActors.clear();
  std::vector<std::string> labels;
  labels.push_back("R");
  labels.push_back("A");
  labels.push_back("S");
  labels.push_back("L");
  labels.push_back("P");
  labels.push_back("I");
  
  for (unsigned int i=0; i<labels.size(); i++)
    {
    vtkVectorText *axisText = vtkVectorText::New();
    axisText->SetText(labels[i].c_str());
    vtkPolyDataMapper *axisMapper = vtkPolyDataMapper::New();
    axisMapper->SetInput(axisText->GetOutput());
    vtkFollower *axisActor = vtkFollower::New();

    axisActor->SetMapper(axisMapper);
    axisActor->SetScale(1,1,1); 
    axisActor->SetPickable (0);

    this->AxisLabelActors.push_back(axisActor);
    
    axisActor->GetProperty()->SetColor(1, 1, 1);
    axisActor->GetProperty()->SetDiffuse (0.0);
    axisActor->GetProperty()->SetAmbient (1.0);
    axisActor->GetProperty()->SetSpecular (0.0);
  }
  double fov = 200;
  double pos = fov * 0.6;

  this->AxisLabelActors[0]->SetPosition(pos,0,0);
  this->AxisLabelActors[1]->SetPosition(0,pos,0);
  this->AxisLabelActors[2]->SetPosition(0,0,pos);
  this->AxisLabelActors[3]->SetPosition(-pos,0,0);
  this->AxisLabelActors[4]->SetPosition(0,-pos,0);
  this->AxisLabelActors[5]->SetPosition(0,0,-pos);

  this->AddAxisActors();

  boxSource->Delete();
  boxMapper->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::AddAxisActors()
{
  if (this->MainViewer)
    {
    if (this->BoxAxisActor)
      {
      this->MainViewer->AddViewProp( this->BoxAxisActor);
      }
    for (unsigned int i=0; i<this->AxisLabelActors.size(); i++)
      {
      this->AxisLabelActors[i]->SetCamera(this->MainViewer->GetRenderer()->GetActiveCamera());
      this->MainViewer->AddViewProp( this->AxisLabelActors[i]);
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UpdateAxis()
{
  this->UpdateViewNode();
  if (this->ViewNode == NULL) 
    {
    return;
    }
  double fov = this->ViewNode->GetFieldOfView();
  this->BoxAxisActor->SetScale(fov/2, fov/2, fov/2);
  this->BoxAxisActor->SetVisibility(this->ViewNode->GetBoxVisible());

  double pos = fov * 0.6;
  double letterSize = this->ViewNode->GetLetterSize();
  double scale = fov * letterSize;

  for (unsigned int i=0; i<AxisLabelActors.size(); i++)
    {
    this->AxisLabelActors[i]->SetScale(scale,scale,scale);
    this->AxisLabelActors[i]->SetVisibility(this->ViewNode->GetAxisLabelsVisible());
    this->AxisLabelActors[i]->SetCamera(this->MainViewer->GetRenderer()->GetActiveCamera());
    }

  this->AxisLabelActors[0]->SetPosition(pos,0,0);
  this->AxisLabelActors[1]->SetPosition(0,pos,0);
  this->AxisLabelActors[2]->SetPosition(0,0,pos);
  this->AxisLabelActors[3]->SetPosition(-pos,0,0);
  this->AxisLabelActors[4]->SetPosition(0,-pos,0);
  this->AxisLabelActors[5]->SetPosition(0,0,-pos);

}

//---------------------------------------------------------------------------
int vtkSlicerViewerWidget::UpdateClipSlicesFormMRML()
{
  if (this->MRMLScene == NULL)
    {
    return 0;
    }

  // update ClipModels node
  vtkMRMLClipModelsNode *clipNode = vtkMRMLClipModelsNode::SafeDownCast(this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLClipModelsNode"));
  if (clipNode != this->ClipModelsNode) 
    {
    vtkSetAndObserveMRMLNodeMacro(this->ClipModelsNode, clipNode);
    }

  if (this->ClipModelsNode == NULL)
    {
    return 0;
    }

  // update Slice nodes
  vtkMRMLSliceNode *node= NULL;
  vtkMRMLSliceNode *nodeRed= NULL;
  vtkMRMLSliceNode *nodeGreen= NULL;
  vtkMRMLSliceNode *nodeYellow= NULL;
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLSliceNode");
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLSliceNode::SafeDownCast (
          this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLSliceNode"));
    // TODO use perhaps SliceLogic to get the name instead of "Red" etc.
    if (!strcmp(node->GetLayoutName(), "Red"))
      {
      nodeRed = node;
      }
    else if (!strcmp(node->GetLayoutName(), "Green"))
      {
      nodeGreen = node;
      }
    else if (!strcmp(node->GetLayoutName(), "Yellow"))
      {
      nodeYellow = node;
      }
    node = NULL;
    }

  if (nodeRed != this->RedSliceNode)
    {
    vtkSetAndObserveMRMLNodeMacro(this->RedSliceNode, nodeRed);
    }
  if (nodeGreen != this->GreenSliceNode)
   {
   vtkSetAndObserveMRMLNodeMacro(this->GreenSliceNode, nodeGreen);
   }
  if (nodeYellow != this->YellowSliceNode)
   {
   vtkSetAndObserveMRMLNodeMacro(this->YellowSliceNode, nodeYellow);
   }

  if (this->RedSliceNode == NULL || this->GreenSliceNode == NULL || this->YellowSliceNode == NULL)
    {
    return 0;
    }

  int modifiedState = 0;

  if ( this->ClipModelsNode->GetClipType() != this->ClipType)
  {
    modifiedState = 1;
    this->ClipType = this->ClipModelsNode->GetClipType();
    if (this->ClipType == vtkMRMLClipModelsNode::ClipIntersection) 
      {
      this->SlicePlanes->SetOperationTypeToIntersection();
      }
    else if (this->ClipType == vtkMRMLClipModelsNode::ClipUnion) 
      {
      this->SlicePlanes->SetOperationTypeToUnion();
      }
    else 
      {
      vtkErrorMacro("vtkMRMLClipModelsNode:: Invalid Clip Type");
      }
  }

  if (this->ClipModelsNode->GetRedSliceClipState() != this->RedSliceClipState)
    {
    if (this->RedSliceClipState == vtkMRMLClipModelsNode::ClipOff)
      {
      this->SlicePlanes->AddFunction(this->RedSlicePlane);
      }
    else if (this->ClipModelsNode->GetRedSliceClipState() == vtkMRMLClipModelsNode::ClipOff)
      {
      this->SlicePlanes->RemoveFunction(this->RedSlicePlane);
      }
    modifiedState = 1;
    this->RedSliceClipState = this->ClipModelsNode->GetRedSliceClipState();
    }

  if (this->ClipModelsNode->GetGreenSliceClipState() != this->GreenSliceClipState)
    {
    if (this->GreenSliceClipState == vtkMRMLClipModelsNode::ClipOff)
      {
      this->SlicePlanes->AddFunction(this->GreenSlicePlane);
      }
    else if (this->ClipModelsNode->GetGreenSliceClipState() == vtkMRMLClipModelsNode::ClipOff)
      {
      this->SlicePlanes->RemoveFunction(this->GreenSlicePlane);
      }
    modifiedState = 1;
    this->GreenSliceClipState = this->ClipModelsNode->GetGreenSliceClipState();
    }

  if (this->ClipModelsNode->GetYellowSliceClipState() != this->YellowSliceClipState)
    {
    if (this->YellowSliceClipState == vtkMRMLClipModelsNode::ClipOff)
      {
      this->SlicePlanes->AddFunction(this->YellowSlicePlane);
      }
    else if (this->ClipModelsNode->GetYellowSliceClipState() == vtkMRMLClipModelsNode::ClipOff)
      {
      this->SlicePlanes->RemoveFunction(this->YellowSlicePlane);
      }
    modifiedState = 1;
    this->YellowSliceClipState = this->ClipModelsNode->GetYellowSliceClipState();
    }

  // compute clipping on/off
  if (this->ClipModelsNode->GetRedSliceClipState() == vtkMRMLClipModelsNode::ClipOff &&
      this->ClipModelsNode->GetGreenSliceClipState() == vtkMRMLClipModelsNode::ClipOff &&
      this->ClipModelsNode->GetYellowSliceClipState() == vtkMRMLClipModelsNode::ClipOff )
    {
    this->ClippingOn = false;
    }
  else
    {
    this->ClippingOn = true;
    }

  // set slice plane normals and origins
  vtkMatrix4x4 *sliceMatrix = NULL;
  double normal[3];
  double origin[3];
  int i;

  sliceMatrix = this->RedSliceNode->GetSliceToRAS();
  for (i=0; i<3; i++) 
    {
    normal[i] = sliceMatrix->GetElement(i,2);
    if (this->RedSliceClipState == vtkMRMLClipModelsNode::ClipNegativeSpace)
      {
      normal[i] = - normal[i];
      }
    origin[i] = sliceMatrix->GetElement(i,3);
    }
  this->RedSlicePlane->SetNormal(normal);
  this->RedSlicePlane->SetOrigin(origin);

  sliceMatrix = this->GreenSliceNode->GetSliceToRAS();
  for (i=0; i<3; i++) 
    {
    normal[i] = sliceMatrix->GetElement(i,2);
    if (this->GreenSliceClipState == vtkMRMLClipModelsNode::ClipNegativeSpace)
      {
      normal[i] = - normal[i];
      }
    origin[i] = sliceMatrix->GetElement(i,3);
    }
  this->GreenSlicePlane->SetNormal(normal);
  this->GreenSlicePlane->SetOrigin(origin);

  sliceMatrix = this->YellowSliceNode->GetSliceToRAS();
  for (i=0; i<3; i++) 
    {
    normal[i] = sliceMatrix->GetElement(i,2);
    if (this->YellowSliceClipState == vtkMRMLClipModelsNode::ClipNegativeSpace)
      {
      normal[i] = - normal[i];
      }
    origin[i] = sliceMatrix->GetElement(i,3);
    }
  this->YellowSlicePlane->SetNormal(normal);
  this->YellowSlicePlane->SetOrigin(origin);

  return modifiedState;
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::ProcessMRMLEvents ( vtkObject *caller,
                                                unsigned long event, 
                                                void *callData )
{ 
  if (this->ProcessingMRMLEvent != 0 )
    {
    return;
    }

  this->ProcessingMRMLEvent = event;

  vtkDebugMacro("processing event " << event);
   
  if (event == vtkMRMLScene::SceneCloseEvent )
    {
    this->SceneClosing = true;
    }
  else 
    {
    this->SceneClosing = false;
    }


  if ( vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene 
    && (event == vtkMRMLScene::NodeAddedEvent || event == vtkMRMLScene::NodeRemovedEvent ) )
    {
    vtkMRMLNode *node = (vtkMRMLNode*) (callData);
    if (node != NULL && node->IsA("vtkMRMLModelNode") )
      {
      this->UpdateFromMRML();
      }
    else if (node != NULL && node->IsA("vtkMRMLClipModelsNode") )
      {
      if (event == vtkMRMLScene::NodeAddedEvent)
        {
          vtkSetAndObserveMRMLNodeMacro(this->ClipModelsNode, node);
        }
      else if (event == vtkMRMLScene::NodeRemovedEvent)
        {
          vtkSetMRMLNodeMacro(this->ClipModelsNode, NULL);
        }
      this->UpdateFromMRML();
      }
    else if (node != NULL && node->IsA("vtkMRMLCameraNode") )
      {
      this->UpdateFromMRML();
      }
    }
  /*
  else if ( vtkMRMLFiducialListNode::SafeDownCast(caller) != NULL
     && event == vtkMRMLFiducialListNode::DisplayModifiedEvent )
    {
    // do a more lightweight update on the fiducial list nodes
    vtkDebugMacro("vtkSlicerViewerWidget::ProcessMRMLEvents got a vtkMRMLFiducialListNode::DisplayModifiedEvent, just calling update fids from mrml\n");
    this->UpdateFiducialsFromMRML();
    }
  else if ( vtkMRMLFiducialListNode::SafeDownCast(caller) != NULL
    && event == vtkMRMLFiducialListNode::FiducialModifiedEvent )
    {
    vtkDebugMacro("vtkSlicerViewerWidget::ProcessMRMLEvents got a FiducialModifiedEvent, removing props and updating from mrml...\n");
    this->RemoveFiducialProps ( );
    this->UpdateFiducialsFromMRML();
    }
  */
  else if (vtkMRMLModelNode::SafeDownCast(caller) != NULL
           && event == vtkMRMLModelNode::PolyDataModifiedEvent)
    {
    this->UpdateModelPolyData(vtkMRMLModelNode::SafeDownCast(caller));
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
void vtkSlicerViewerWidget::UpdateCameraNode()
{
  if (this->SceneClosing)
    {
    return;
    }
  // find an active camera
  // or any camera if none is active
  vtkMRMLCameraNode *node = NULL;
  int nnodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLCameraNode");
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLCameraNode::SafeDownCast (
       this->MRMLScene->GetNthNodeByClass(n, "vtkMRMLCameraNode"));
    if (node->GetActive())
      {
      break;
      }
    }

  if ( this->CameraNode != NULL && node != NULL && this->CameraNode != node)
    {
    // local CameraNode is out of sync with the scene
    this->SetAndObserveCameraNode (NULL);
    }
  if ( this->CameraNode != NULL && this->MRMLScene->GetNodeByID(this->CameraNode->GetID()) == NULL)
    {
    // local node not in the scene
    this->SetAndObserveCameraNode (NULL);
    }
  if ( this->CameraNode == NULL )
    {
    if ( node == NULL )
      {
      // no camera in the scene and local
      // create an active camera
      node = vtkMRMLCameraNode::New();
      node->SetActive(1);
      this->MRMLScene->AddNode(node);
      node->Delete();
      }
    this->SetAndObserveCameraNode (node);
    }
 
  vtkRenderWindowInteractor *rwi = this->MainViewer->GetRenderWindowInteractor();
  if (rwi)
    {
    vtkInteractorObserver *iobs = rwi->GetInteractorStyle();
    vtkSlicerViewerInteractorStyle *istyle = vtkSlicerViewerInteractorStyle::SafeDownCast(iobs);
    if (istyle)
      {
      istyle->SetCameraNode(this->CameraNode);
      }
    }
  this->MainViewer->GetRenderer()->SetActiveCamera(this->CameraNode->GetCamera());
}


//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UpdateViewNode()
{
  if (this->SceneClosing)
    {
    return;
    }

  vtkMRMLViewNode *node =  vtkMRMLViewNode::SafeDownCast (
       this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLViewNode"));

  if ( this->ViewNode != NULL && node != NULL && this->ViewNode != node)
    {
    // local ViewNode is out of sync with the scene
    this->SetAndObserveViewNode (NULL);
    }
  if ( this->ViewNode != NULL && this->MRMLScene->GetNodeByID(this->ViewNode->GetID()) == NULL)
    {
    // local node not in the scene
    this->SetAndObserveViewNode (NULL);
    }
  if ( this->ViewNode == NULL )
    {
    if ( node == NULL )
      {
      // no view in the scene and local
      // create an active camera
      node = vtkMRMLViewNode::New();
      this->MRMLScene->AddNode(node);
      node->Delete();
      }
    this->SetAndObserveViewNode (node);
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

  this->ViewerFrame = vtkKWFrame::New ( );
  this->ViewerFrame->SetParent ( this->GetParent ( ) );
  this->ViewerFrame->Create ( );
  
  this->MainViewer = vtkKWRenderWidget::New ( );  
  this->MainViewer->SetParent (this->ViewerFrame );
  this->MainViewer->Create ( );

  // make a Slicer viewer interactor style to process our events
  // look at the InteractorStyle to get our events
  vtkRenderWindowInteractor *rwi = this->MainViewer->GetRenderWindowInteractor();
  if (rwi)
    {
    vtkSlicerViewerInteractorStyle *iStyle = vtkSlicerViewerInteractorStyle::New();
    rwi->SetInteractorStyle (iStyle);
    iStyle->Delete();
    }


  // Set the viewer's minimum dimension to be the modifiedState as that for
  // the three main Slice viewers.
  this->MainViewer->GetRenderer()->GetActiveCamera()->ParallelProjectionOff();
  if ( this->GetApplication() != NULL )
    {
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();      
    this->MainViewer->SetWidth ( app->GetMainLayout()->GetSliceViewerMinDim() );
    this->MainViewer->SetHeight ( app->GetMainLayout()->GetSliceViewerMinDim() );
    }
    

  // observe scene for add/remove nodes
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
  //events->InsertNextValue(vtkMRMLScene::NewSceneEvent);
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  this->SetAndObserveMRMLSceneEvents(this->MRMLScene, events);
  events->Delete();

  this->CreateClipSlices();

  this->CreateAxis();

  //this->PackWidget ( );
  this->MainViewer->ResetCamera ( );
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UpdateFromMRML()
{
  this->UpdateAxis();

  this->UpdateCameraNode();

  this->UpdateClipSlicesFormMRML();

  this->RemoveModelProps ( );
  
  /*this->RemoveFiducialProps ( );

  this->UpdateFiducialsFromMRML();
  */
  this->UpdateModelsFromMRML();

  this->RequestRender ( );
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UpdateModelsFromMRML()
{
  vtkMRMLScene *scene = this->GetMRMLScene();
  vtkMRMLNode *node = NULL;
  std::vector<vtkMRMLModelNode *> slices;

  // find volume slices
  scene->InitTraversal();
  bool clearDisplayedModels = false;
  while (node=scene->GetNextNodeByClass("vtkMRMLModelNode"))
    {
    vtkMRMLModelNode *model = vtkMRMLModelNode::SafeDownCast(node);
    // render slices last so that transparent objects are rendered in fron of them
    if (!strcmp(model->GetName(), "Red Volume Slice") ||
        !strcmp(model->GetName(), "Green Volume Slice") ||
        !strcmp(model->GetName(), "Yellow Volume Slice"))
      {
      slices.push_back(model);
      if (this->DisplayedModels.find(model->GetID()) == this->DisplayedModels.end() )
        {
        clearDisplayedModels = true;
        }
      }
    }

  if (clearDisplayedModels)
    {
    this->MainViewer->RemoveAllViewProps();
    this->DisplayedModels.clear();
    this->AddAxisActors();
    }

  // render slices first
  for (unsigned int i=0; i<slices.size(); i++)
    {
    vtkMRMLModelNode *model = slices[i];
    // add nodes that are not in the list yet
    if (this->DisplayedModels.find(model->GetID()) == this->DisplayedModels.end() )
      {
      this->UpdateModel(model);
      } 
    //vtkActor *actor = this->DisplayedModels.find(model->GetID())->second;
    vtkActor *actor = this->DisplayedModels[ model->GetID() ];
    this->SetModelDisplayProperty(model, actor);
    }
  
  // render the rest of the models
  scene->InitTraversal();
  while (node=scene->GetNextNodeByClass("vtkMRMLModelNode"))
    {
    vtkMRMLModelNode *model = vtkMRMLModelNode::SafeDownCast(node);
    // render slices last so that transparent objects are rendered in fron of them
    if (!strcmp(model->GetName(), "Red Volume Slice") ||
        !strcmp(model->GetName(), "Green Volume Slice") ||
        !strcmp(model->GetName(), "Yellow Volume Slice"))
      {
      continue;
      }
    // add nodes that are not in the list yet
    if (this->DisplayedModels.find(model->GetID()) == this->DisplayedModels.end() )
      {
      this->UpdateModel(model);
      } 
    //vtkActor *actor = this->DisplayedModels.find(model->GetID())->second;
    vtkActor *actor = this->DisplayedModels[ model->GetID() ];
    this->SetModelDisplayProperty(model, actor);
    } // end while

}


//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UpdateModelPolyData(vtkMRMLModelNode *model)
{
  vtkMRMLModelDisplayNode *modelDisplayNode = model->GetDisplayNode();

  vtkClipPolyData *clipper = NULL;
  if (this->ClippingOn && modelDisplayNode != NULL && modelDisplayNode->GetClipping())
    {
    clipper = vtkClipPolyData::New();
    clipper->SetClipFunction(this->SlicePlanes);
    clipper->SetValue( 0.0);
    }

  vtkPolyDataMapper *mapper = vtkPolyDataMapper::New ();
  if (clipper)
    {
    clipper->SetInput ( model->GetPolyData() );
    clipper->Update();
    mapper->SetInput ( clipper->GetOutput() );
    }
  else
    {
    mapper->SetInput ( model->GetPolyData() );
    }

  vtkActor* actor;
  std::map<const char *, vtkActor *>::iterator ait;

  ait = this->DisplayedModels.find(model->GetID());
  if (ait == this->DisplayedModels.end() )
    {
    actor = vtkActor::New();
    }
  else
    {
    actor = (*ait).second;
    }
  actor->SetMapper( mapper );

  if (ait == this->DisplayedModels.end())
    {
    this->MainViewer->AddViewProp( actor );
    this->DisplayedModels[model->GetID()] = actor;
    actor->Delete();
    }

  if (clipper)
    {
    this->DisplayedModelsClipState[model->GetID()] = 1;
    }
  else
    {
    this->DisplayedModelsClipState[model->GetID()] = 0;
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UpdateModel(vtkMRMLModelNode *model)
{
  this->UpdateModelPolyData(model);

  // observe polydata
  model->AddObserver ( vtkMRMLModelNode::PolyDataModifiedEvent, this->MRMLCallbackCommand );

  // observe display node  
  model->AddObserver ( vtkMRMLModelNode::DisplayModifiedEvent, this->MRMLCallbackCommand );

  model->AddObserver ( vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand );

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
  std::map<const char *, int>::iterator clipIter;
  std::vector<const char *> removedIDs;
  for(iter=this->DisplayedModels.begin(); iter != this->DisplayedModels.end(); iter++) 
    {
    vtkMRMLModelNode *model = vtkMRMLModelNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(iter->first));
    if (model == NULL)
      {
      this->MainViewer->RemoveViewProp(iter->second);
      removedIDs.push_back(iter->first);
      }
    else
      {
      vtkMRMLModelDisplayNode *modelDisplayNode = model->GetDisplayNode();
      int clipModel = 0;
      if (modelDisplayNode != NULL)
        {
        clipModel = modelDisplayNode->GetClipping();
        }
      clipIter = DisplayedModelsClipState.find(iter->first);
      if (clipIter == DisplayedModelsClipState.end())
        {
          std::cerr << "vtkSlicerViewerWidget::RemoveModelProps() Unknown clip state\n";
        }
      else 
        {
        if (clipIter->second && !this->ClippingOn ||
          this->ClippingOn && clipIter->second != clipModel)
          {
          this->MainViewer->RemoveViewProp(iter->second);
          removedIDs.push_back(iter->first);
          }     
        }
      }
    }
  for (unsigned int i=0; i< removedIDs.size(); i++)
    {
    this->DisplayedModels.erase(removedIDs[i]);
    this->DisplayedModelsClipState.erase(removedIDs[i]);
    }
  
}


//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::RemoveMRMLObservers()
{
  this->RemoveModelObservers();
  
  //this->RemoveFiducialObservers();

  this->SetAndObserveMRMLScene(NULL);
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
    actor->GetMapper()->SetScalarVisibility(dnode->GetScalarVisibility());
    // if the scalars are visible, try to get the lookup table
    if (actor->GetMapper()->GetScalarVisibility())
      {
      if (dnode->GetColorNode() != NULL &&
          dnode->GetColorNode()->GetLookupTable() != NULL)
        {
        actor->GetMapper()->SetLookupTable(dnode->GetColorNode()->GetLookupTable());
        }
      }
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
  // Description:
  // return the ID for the given actor 
const char *
vtkSlicerViewerWidget::GetIDByActor (vtkActor *actor)
{
  if ( !actor )
    {
    return (NULL);
    }

  std::map<const char *, vtkActor *>::iterator iter;
  for(iter=this->DisplayedModels.begin(); iter != this->DisplayedModels.end(); iter++) 
    {
    if ( iter->second && ( iter->second == actor ) )
      {
      return (iter->first);
      }
    }
  return (NULL);
}



//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::PackWidget ( vtkKWFrame *f )
{
  this->Script  ("pack %s -side left -fill both -expand y -padx 0 -pady 0 -in %s",
                 this->ViewerFrame->GetWidgetName ( ), f->GetWidgetName() );
  this->Script  ("pack %s -side top -anchor c  -fill both -expand y -padx 0 -pady 0",
                 this->MainViewer->GetWidgetName ( ) );
}


//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::GridWidget ( vtkKWFrame *f, int row, int col )
{
  this->Script ( "grid configure %s -in %s -row %d -column %d -stick news -padx 0 -pady 0",
                 this->ViewerFrame->GetWidgetName(), f->GetWidgetName(), row, col );
//  this->Script  ("grid %s -row %d -column %d -sticky news -padx 0 -pady 0 -in %s",
//                 this->ViewerFrame->GetWidgetName ( ), row, col, f->GetWidgetName()  );
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
