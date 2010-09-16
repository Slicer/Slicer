/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/
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

#include "vtkMath.h"
#include "vtkProp3D.h"
#include "vtkActor.h"
#include "vtkImageActor.h"
#include "vtkFollower.h"
#include "vtkProperty.h"
#include "vtkTexture.h"
#include "vtkRenderer.h"
#include "vtkCamera.h"
#include "vtkPolyDataMapper.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkOutlineSource.h"
#include "vtkVectorText.h"
#include "vtkRenderWindow.h"
#include "vtkImplicitBoolean.h"
#include "vtkPlane.h"
#include "vtkClipPolyData.h"
#include "vtkBoundingBox.h"

#include "vtkMRMLDisplayableNode.h"
#include "vtkMRMLDisplayNode.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLClipModelsNode.h"
#include "vtkMRMLModelHierarchyNode.h"
#include "vtkMRMLColorNode.h"
#include "vtkMRMLProceduralColorNode.h"

#include "vtkSlicerModelHierarchyLogic.h"
#include "vtkSlicerTheme.h"

#include "vtkKWWidget.h"

// for picking
#include "vtkWorldPointPicker.h"
#include "vtkPropPicker.h"
#include "vtkCellPicker.h"
#include "vtkPointPicker.h"


#include "vtkSlicerBoxWidget2.h"
#include "vtkSlicerBoxRepresentation.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerViewerWidget );
vtkCxxRevisionMacro ( vtkSlicerViewerWidget, "$Revision$");

//---------------------------------------------------------------------------
vtkSlicerViewerWidget::vtkSlicerViewerWidget ( )
{
  this->MainViewer = NULL;  
  this->ViewerFrame = NULL;
  this->ProcessingMRMLEvent = 0;
  this->UpdateFromMRMLRequested = 0;

  this->CameraNode = NULL;
  this->CameraNodeWasCreated = 0;

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
  this->BoxAxisBoundingBox = new vtkBoundingBox;

  this->SceneClosing = false;

  this->ModelHierarchiesPresent = false;
  this->ModelHierarchyLogic = NULL;

  this->ApplicationLogic = NULL;
  this->WorldPointPicker = vtkWorldPointPicker::New();
  this->PropPicker = vtkPropPicker::New();
  this->CellPicker = vtkCellPicker::New();
  this->CellPicker->SetTolerance(0.00001);
  this->PointPicker = vtkPointPicker::New();
  this->ResetPick();

  this->PickedCellID = 0;
  this->PickedPointID = 0;

    
  this->BoxWidget = vtkSlicerBoxWidget2::New();
  this->BoxWidgetRepresentation = vtkSlicerBoxRepresentation::New();
  this->BoxWidget->SetRepresentation(this->BoxWidgetRepresentation);
  
  this->EnableRender = 1;
  this->UpdatingAxis = 0;
  this->IsRendering = 0;
  this->CameraNodeUpdatePending = 0;

}

//---------------------------------------------------------------------------
vtkSlicerViewerWidget::~vtkSlicerViewerWidget ( )
{
  delete this->BoxAxisBoundingBox;

  this->SetModelHierarchyLogic(NULL);

  this->RemoveMRMLObservers();

  vtkSetMRMLNodeMacro(this->ClipModelsNode, NULL);
  vtkSetMRMLNodeMacro(this->CameraNode, NULL);
  vtkSetMRMLNodeMacro(this->ViewNode, NULL);

  vtkSetMRMLNodeMacro(this->RedSliceNode, NULL);
  vtkSetMRMLNodeMacro(this->GreenSliceNode, NULL);
  vtkSetMRMLNodeMacro(this->YellowSliceNode, NULL);

  if (this->MainViewer)
    {
    vtkSlicerViewerInteractorStyle *iStyle;
    iStyle = vtkSlicerViewerInteractorStyle::SafeDownCast (this->MainViewer->GetRenderWindowInteractor()->GetInteractorStyle());
    iStyle->SetApplicationLogic ( NULL);
    this->SetMRMLScene ( NULL );
    this->MainViewer->RemoveAllViewProps ( );
    }

  this->SlicePlanes->Delete();
  this->SlicePlanes = NULL;
  this->RedSlicePlane->Delete();
  this->RedSlicePlane = NULL;
  this->GreenSlicePlane->Delete();
  this->GreenSlicePlane = NULL;
  this->YellowSlicePlane->Delete();
  this->YellowSlicePlane = NULL;

  if (this->BoxAxisActor)
    {
    this->BoxAxisActor->Delete();
    this->BoxAxisActor = NULL;
    }
  for (unsigned int i=0; i<this->AxisLabelActors.size(); i++)
    {
    this->AxisLabelActors[i]->SetCamera ( NULL );
    this->AxisLabelActors[i]->Delete();
    }
  this->AxisLabelActors.clear();

  if (this->MainViewer)
    {

    this->MainViewer->SetParent ( NULL );
    this->MainViewer->Delete();
    this->MainViewer = NULL;
    }

  // release the DisplayedModelActors
  this->DisplayedActors.clear();
  
  this->ViewerFrame->SetParent ( NULL );
  this->ViewerFrame->Delete ( );
  this->ViewerFrame = NULL;

  if (this->WorldPointPicker)
    {
    this->WorldPointPicker->Delete();
    this->WorldPointPicker = NULL;
    }
  if (this->PropPicker)
    {
    this->PropPicker->Delete();
    this->PropPicker = NULL;
    }
  if (this->CellPicker)
    {
    this->CellPicker->Delete();
    this->CellPicker = NULL;
    }
  if (this->PointPicker)
    {
    this->PointPicker->Delete();
    this->PointPicker = NULL;
    }
  this->ApplicationLogic = NULL; 

  if (this->ModelHierarchyLogic)
    {
    this->ModelHierarchyLogic->Delete();
    }
  
  this->BoxWidget->SetRepresentation(NULL);
  this->BoxWidgetRepresentation->Delete();
  this->BoxWidget->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "vtkSlicerViewerWidget: " << this->GetClassName ( ) << "\n";

  os << indent << "ProcessingMRMLEvent = " << this->ProcessingMRMLEvent << "\n";
    
  os << indent << "ClipType = " << this->ClipType << "\n";
  os << indent << "RedSliceClipState = " << this->RedSliceClipState << "\n";
  os << indent << "YellowSliceClipState = " << this->YellowSliceClipState << "\n";
  os << indent << "GreenSliceClipState = " << this->GreenSliceClipState << "\n";
  os << indent << "ClippingOn = " << (this->ClippingOn ? "true" : "false") << "\n";

  os << indent << "ModelHierarchiesPresent = " << this->ModelHierarchiesPresent << "\n";
  os << indent << "SceneClosing = " << this->SceneClosing << "\n";
    
  os << indent << "PickedNodeName = " << this->PickedNodeName.c_str() << "\n";
  os << indent << "PickedRAS = (" << this->PickedRAS[0] << ", " << this->PickedRAS[1] << ", "<< this->PickedRAS[2] << ")\n";
  os << indent << "PickedCellID = " << this->PickedCellID << "\n";
  os << indent << "PickedPointID = " << this->PickedPointID << "\n";
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::ProcessWidgetEvents(vtkObject *vtkNotUsed(caller),
                                                unsigned long vtkNotUsed(event),
                                                void *vtkNotUsed(callData))
{
  // Not currently used... 
  this->RequestRender();
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
  // Create the default bounding box
  vtkSmartPointer<vtkOutlineSource> boxSource =
    vtkSmartPointer<vtkOutlineSource>::New();

  vtkSmartPointer<vtkPolyDataMapper> boxMapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
  boxMapper->SetInput( boxSource->GetOutput() );
   
  this->BoxAxisActor = vtkActor::New();
  this->BoxAxisActor->SetMapper( boxMapper );
  this->BoxAxisActor->SetScale(1.0, 1.0, 1.0);
  this->BoxAxisActor->GetProperty()->SetColor( 1.0, 0.0, 1.0 );
  this->BoxAxisActor->SetPickable(0);

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
    vtkSmartPointer<vtkVectorText> axisText =
      vtkSmartPointer<vtkVectorText>::New();
    axisText->SetText(labels[i].c_str());

    vtkSmartPointer<vtkPolyDataMapper> axisMapper =
      vtkSmartPointer<vtkPolyDataMapper>::New();
    axisMapper->SetInput(axisText->GetOutput());
    vtkFollower *axisActor = vtkFollower::New();

    axisActor->SetMapper(axisMapper);
    axisActor->SetPickable (0);

    this->AxisLabelActors.push_back(axisActor);
    
    axisActor->GetProperty()->SetColor(1, 1, 1);
    axisActor->GetProperty()->SetDiffuse (0.0);
    axisActor->GetProperty()->SetAmbient (1.0);
    axisActor->GetProperty()->SetSpecular (0.0);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::ColorAxisLabelActors ( double r, double g, double b)
{
  for (unsigned int i=0; i<this->AxisLabelActors.size(); i++)
    {
    this->AxisLabelActors[i]->GetProperty()->SetColor ( r, g, b );
    }

}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::AddAxisActors()
{
  if (this->MainViewer)
    {
    if (this->BoxAxisActor)
      {
      this->MainViewer->AddViewProp( this->BoxAxisActor);
      //--- handle the case where box visibility is set to 0
      if ( this->ViewNode != NULL )
        {
        this->BoxAxisActor->SetVisibility ( this->ViewNode->GetBoxVisible() );
        }
      }
    for (unsigned int i=0; i<this->AxisLabelActors.size(); i++)
      {
      vtkCamera *camera = this->MainViewer->GetRenderer()->IsActiveCameraCreated() ? 
        this->MainViewer->GetRenderer()->GetActiveCamera() : NULL;
      this->AxisLabelActors[i]->SetCamera(camera);
      this->MainViewer->AddViewProp( this->AxisLabelActors[i]);
      //--- handle the case where axis label visibility is set to 0
      if ( this->ViewNode != NULL )
        {
        this->AxisLabelActors[i]->SetVisibility(this->ViewNode->GetAxisLabelsVisible());
        }
      }
    }
}



//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UpdateAxisLabelActors()
{
  for (unsigned int i=0; i<AxisLabelActors.size(); i++)
    {
    if (this->MainViewer != NULL && this->MainViewer->GetRenderer() != NULL )
      {
      this->AxisLabelActors[i]->SetCamera(this->MainViewer->GetRenderer()->GetActiveCamera());
      }
    else
      {
      this->AxisLabelActors[i]->SetCamera(NULL );
      }
    this->AxisLabelActors[i]->SetVisibility(this->ViewNode->GetAxisLabelsVisible());
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
  if (this->UpdatingAxis || this->IsRendering)
    {
    return;
    }
  this->UpdatingAxis = 1;

  // Turn off box and axis labels to compute bounds
  this->BoxAxisActor->VisibilityOff();
  for (unsigned int i=0; i < AxisLabelActors.size(); i++)
    {
    this->AxisLabelActors[i]->VisibilityOff();
    }
  double bounds[6];
  this->MainViewer->GetRenderer()->ComputeVisiblePropBounds(bounds);

  // If there are no visible props, create a default set of bounds
  vtkBoundingBox newBBox;
  if (!vtkMath::AreBoundsInitialized(bounds))
    {
    newBBox.SetBounds(-100.0, 100.0,
                      -100.0, 100.0,
                      -100.0, 100.0);
    }
  else
    {
    newBBox.SetBounds(bounds);

    // Check for degenerate bounds
    double maxLength = newBBox.GetMaxLength();
    double minPoint[3], maxPoint[3];
    newBBox.GetMinPoint(minPoint[0], minPoint[1], minPoint[2]);
    newBBox.GetMaxPoint(maxPoint[0], maxPoint[1], maxPoint[2]);

    for (unsigned int i = 0; i < 3; i++)
      {
      if (newBBox.GetLength(i) == 0.0)
        {
        minPoint[i] = minPoint[i] - maxLength * .05;
        maxPoint[i] = maxPoint[i] + maxLength * .05;
        }
      }
    newBBox.SetMinPoint(minPoint);
    newBBox.SetMaxPoint(maxPoint);
    }

  // See if bounding box has changed. If not, no need to change the
  // axis actors.
  bool bBoxChanged = false;
  if (newBBox != *(this->BoxAxisBoundingBox))
    {
    bBoxChanged = true;
    *(this->BoxAxisBoundingBox) = newBBox;

    double bounds[6];
    this->BoxAxisBoundingBox->GetBounds(bounds);

    vtkSmartPointer<vtkOutlineSource> boxSource =
      vtkSmartPointer<vtkOutlineSource>::New();
    boxSource->SetBounds(bounds);

    vtkSmartPointer<vtkPolyDataMapper> boxMapper =
      vtkSmartPointer<vtkPolyDataMapper>::New();
    boxMapper->SetInput(boxSource->GetOutput());

    this->BoxAxisActor->SetMapper(boxMapper);
    this->BoxAxisActor->SetScale(1.0, 1.0, 1.0);

    double letterSize = this->ViewNode->GetLetterSize();
    
    for (unsigned int i=0; i<AxisLabelActors.size(); i++)
      {
      this->AxisLabelActors[i]->SetScale(
        this->BoxAxisBoundingBox->GetMaxLength() * letterSize,
        this->BoxAxisBoundingBox->GetMaxLength() * letterSize,
        this->BoxAxisBoundingBox->GetMaxLength() * letterSize);
      this->AxisLabelActors[i]->SetOrigin(.5, .5,.5);
// Removing this from inside bbox-changed logic.
// if camera is changed, the axis label actors have to
// pay attention to new camera regardless of bbox changes...
//      vtkCamera *camera =
//        this->MainViewer->GetRenderer()->IsActiveCameraCreated() ? 
//        this->MainViewer->GetRenderer()->GetActiveCamera() : NULL;
//      this->AxisLabelActors[i]->SetCamera(camera);
      }

    // Position the axis labels
    double center[3];
    this->BoxAxisBoundingBox->GetCenter(center);

    double offset =
      this->BoxAxisBoundingBox->GetMaxLength() * letterSize * 1.5;
    this->AxisLabelActors[0]->SetPosition(               // R
      bounds[1] + offset,
      center[1],
      center[2]);
    this->AxisLabelActors[1]->SetPosition(               // A
      center[0],
      bounds[3] + offset,
      center[2]);
    this->AxisLabelActors[2]->SetPosition(               // S
      center[0],
      center[1],
      bounds[5] + offset);

    this->AxisLabelActors[3]->SetPosition(               // L
      bounds[0] - offset,
      center[1],
      center[2]);
    this->AxisLabelActors[4]->SetPosition(               // P
      center[0],
      bounds[2] - offset,
      center[2]);
    this->AxisLabelActors[5]->SetPosition(               // I
      center[0],
      center[1],
      bounds[4] - offset);
    }

  // Make the axis visible again
  this->BoxAxisActor->SetVisibility(this->ViewNode->GetBoxVisible());
  this->UpdateAxisLabelActors();

  // Until we come up with a solution for all use cases, the resetting
  // of the camera is disabled
#if 0
  if (bBoxChanged)
    {
    this->MainViewer->ResetCamera ( );
    this->MainViewer->GetRenderer()->GetActiveCamera()->Dolly(1.5);
    this->MainViewer->GetRenderer()->ResetCameraClippingRange();
    }
#endif

  this->UpdatingAxis = 0;

}

//---------------------------------------------------------------------------
int vtkSlicerViewerWidget::UpdateClipSlicesFromMRML()
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
  
  std::vector<vtkMRMLNode *> snodes;
  int nnodes = this->MRMLScene->GetNodesByClass("vtkMRMLSliceNode", snodes);
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLSliceNode::SafeDownCast (snodes[n]);
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
  int planeDirection = 1;

  sliceMatrix = this->RedSliceNode->GetSliceToRAS();
  planeDirection = (this->RedSliceClipState == vtkMRMLClipModelsNode::ClipNegativeSpace) ? -1 : 1;
  this->SetClipPlaneFromMatrix(sliceMatrix, planeDirection, this->RedSlicePlane);

  sliceMatrix = this->GreenSliceNode->GetSliceToRAS();
  planeDirection = (this->GreenSliceClipState == vtkMRMLClipModelsNode::ClipNegativeSpace) ? -1 : 1;
  this->SetClipPlaneFromMatrix(sliceMatrix, planeDirection, this->GreenSlicePlane);

  sliceMatrix = this->YellowSliceNode->GetSliceToRAS();
  planeDirection = (this->YellowSliceClipState == vtkMRMLClipModelsNode::ClipNegativeSpace) ? -1 : 1;
  this->SetClipPlaneFromMatrix(sliceMatrix, planeDirection, this->YellowSlicePlane);

  return modifiedState;
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::SetClipPlaneFromMatrix(vtkMatrix4x4 *sliceMatrix, 
                                                   int planeDirection,
                                                   vtkPlane *plane)
{
  double normal[3];
  double origin[3];
  int i;

  for (i=0; i<3; i++) 
    {
    normal[i] = planeDirection * sliceMatrix->GetElement(i,2);
    origin[i] = sliceMatrix->GetElement(i,3);
    }
  plane->SetNormal(normal);
  plane->SetOrigin(origin);
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

  vtkDebugMacro("ProcessMRMLEvents: processing event " << event);

  if (event == vtkMRMLScene::SceneAboutToBeClosedEvent )
    {
    this->RemoveHierarchyObservers(0);
    this->RemoveModelObservers(0);
    this->RemoveCameraObservers();
    }
  else if (event == vtkMRMLScene::SceneClosedEvent )
    {
    this->SceneClosing = true;
    this->CameraNodeWasCreated = 0;
    this->RemoveModelProps();
    this->RemoveHierarchyObservers(1);
    this->RemoveModelObservers(1);
    this->RemoveCameraObservers();
    this->UpdateFromMRMLRequested = 1;
    this->RequestRender();
    }
  else if (event == vtkMRMLScene::SceneAboutToBeImportedEvent)
    {
    this->SetEnableRender(0);
    this->MainViewer->SetRenderModeToDisabled();
    }
  else if (event == vtkMRMLScene::SceneImportedEvent)
    {
    this->SetEnableRender(1);
    this->MainViewer->SetRenderModeToInteractive();
    this->RequestRender();
    } 
  else 
    {
    this->SceneClosing = false;
    }

  if (event == vtkMRMLScene::SceneRestoredEvent )
    {
    // Backward compatibility change: all scenes saved so far do have 
    // camera nodes with an empty or false active tag. Restoring a snapshot
    // will invalidate the current active camera. Try to catch that
    // by grabbing the first available camera. Sounds like a hack, but
    // too  much time was wasted on this thing.
    if (this->ViewNode)
      {
      vtkMRMLCameraNode *camera_node = this->CameraNode;
      if (!camera_node)
        {
        camera_node = vtkMRMLCameraNode::SafeDownCast(
          this->MRMLScene->GetNthNodeByClass(0, "vtkMRMLCameraNode"));
        camera_node->SetActiveTag(this->ViewNode->GetID());
        }
      else if (!camera_node->GetActiveTag())
        {
        camera_node->SetActiveTag(this->ViewNode->GetID());
        }
      }
    this->UpdateFromMRMLRequested = 1;
    }

  if ( vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene && 
       (event == vtkMRMLScene::NodeAddedEvent || 
        event == vtkMRMLScene::NodeRemovedEvent ) )
    {
    vtkMRMLNode *node = (vtkMRMLNode*) (callData);
    if (node != NULL && node->IsA("vtkMRMLDisplayableNode") )
      {
      vtkMRMLDisplayableNode *dnode = vtkMRMLDisplayableNode::SafeDownCast(node);
      if (event == vtkMRMLScene::NodeRemovedEvent)
        {
        this->RemoveDisplayable(dnode);
        }
      this->UpdateFromMRMLRequested = 1;
      this->RequestRender();
      }
    else if (node != NULL && node->IsA("vtkMRMLDisplayNode") )
      {
      this->UpdateFromMRMLRequested = 1;
      this->RequestRender();
      }
    else if (node != NULL && node->IsA("vtkMRMLModelHierarchyNode") )
      {
      this->UpdateModelHierarchies();
      this->UpdateFromMRMLRequested = 1;
      this->RequestRender();
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
      this->UpdateFromMRMLRequested = 1;
      this->RequestRender();
      }
    else if (node != NULL && node->IsA("vtkMRMLCameraNode") )
      {
      if (event == vtkMRMLScene::NodeAddedEvent)
        {
        vtkEventBroker::GetInstance()->AddObservation ( 
          node, vtkMRMLCameraNode::ActiveTagModifiedEvent, this, this->MRMLCallbackCommand );
        }
      else if (event == vtkMRMLScene::NodeRemovedEvent)
        {
        vtkEventBroker::GetInstance()->RemoveObservations ( 
          node, vtkMRMLCameraNode::ActiveTagModifiedEvent, this, this->MRMLCallbackCommand );
        this->RequestCameraNodeUpdate();
        }
      }
    }
  else if (vtkMRMLCameraNode::SafeDownCast(caller) != NULL &&
           (event == vtkCommand::ModifiedEvent ||
            event == vtkMRMLCameraNode::ActiveTagModifiedEvent))
    {
    vtkDebugMacro("ProcessingMRML: got a camera node modified event");
    this->RequestCameraNodeUpdate();
    this->RequestRender();
    }
  else if (vtkMRMLViewNode::SafeDownCast(caller) != NULL &&
           event == vtkCommand::ModifiedEvent)
    {
    vtkDebugMacro("ProcessingMRML: got a view node modified event");
    this->UpdateViewNode();
    this->RequestRender();
    }
  else if (vtkMRMLDisplayableNode::SafeDownCast(caller) != NULL)
    {
    // check for events on a model node
    vtkMRMLDisplayableNode *modelNode = vtkMRMLDisplayableNode::SafeDownCast(caller);
    if (event == vtkCommand::ModifiedEvent ||
        event == vtkMRMLDisplayableNode::PolyDataModifiedEvent ||
        event == vtkMRMLDisplayableNode::DisplayModifiedEvent) 
      {
      // if the node is already cached with an actor process only this one
      // if it was not visible and is still not visible do nothing
      std::vector< vtkMRMLDisplayNode *> dnodes = this->GetDisplayNode(modelNode);
      bool updateModel = false;
      bool updateMRML = false;
      for (unsigned int i=0; i<dnodes.size(); i++)
        {
        vtkMRMLDisplayNode *dnode = dnodes[i];
        int visibility = dnode->GetVisibility();
        //if (hdnode)
        //  {
        //  visibility = hdnode->GetVisibility();
        //  }
        if (this->DisplayedActors.find(dnode->GetID()) == this->DisplayedActors.end())
          {
          updateMRML = true;
          break;
          }
        if (!(dnode && visibility == 0 && this->GetDisplayedModelsVisibility(dnode) == 0))
          {
          updateModel = true;
          break;
          }
        }
      if (updateModel) 
        {
        this->UpdateClipSlicesFromMRML();
        this->UpdateModifiedModel(modelNode);
        this->RequestRender( );
        }
      if (updateMRML)
        {
        this->UpdateFromMRMLRequested = 1;
        this->RequestRender();
        }
      }
    else
      {
      this->UpdateFromMRMLRequested = 1;
      this->RequestRender();
      }
    }
  else if (vtkMRMLClipModelsNode::SafeDownCast(caller) != NULL &&
           event == vtkCommand::ModifiedEvent)
    {
    this->UpdateFromMRMLRequested = 1;
    this->RequestRender();
    }
  else if (vtkMRMLSliceNode::SafeDownCast(caller) != NULL &&
           event == vtkCommand::ModifiedEvent && (this->UpdateClipSlicesFromMRML() || this->ClippingOn))
    {
    this->UpdateFromMRMLRequested = 1;
    this->RequestRender();
    }
  else if (vtkMRMLModelHierarchyNode::SafeDownCast(caller) &&
           event == vtkCommand::ModifiedEvent)
    {
    this->UpdateModelHierarchies();
    this->UpdateFromMRMLRequested = 1;
    this->RequestRender();
    }
  else
    {
    vtkDebugMacro("ProcessMRMLEvents: unhandled event " << event << " " << ((event == 31) ? "ModifiedEvent" : "not ModifiedEvent"));
    if (vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene) { vtkDebugMacro("\ton the mrml scene"); }
    if (vtkMRMLNode::SafeDownCast(caller) != NULL) { vtkDebugMacro("\tmrml node id = " << vtkMRMLNode::SafeDownCast(caller)->GetID()); }
    }
  
  this->ProcessingMRMLEvent = 0;
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::RequestCameraNodeUpdate()
{
  // Followers are not synched up with a new Camera
  // if UpdateCameraNode (which calls UpdateAxis) is called during a
  // Render. UpdateAxis does not complete under these circumstances
  // and the scene is respecting multiple cameras. To avoid this, 
  // this method schedules UpdateCameraNode for when render completes.
  if (this->UpdatingAxis || this->IsRendering)
    {
    this->CameraNodeUpdatePending = 1;
   }
  else
    {
    this->UpdateCameraNode();
    }
}


//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UpdateCameraNode()
{
  if (this->SceneClosing)
    {
    return;
    }
  if (this->MRMLScene == NULL)
    {
    vtkErrorMacro("UpdateCameraNode: viewer widget does not have a scene set, can't find camera nodes");
    return;
    }
  if (this->ViewNode == NULL)
    {
    vtkErrorMacro("UpdateCameraNode: viewer widget does not have a view node!");
    return;
    }

  if ( this->CameraNodeUpdatePending )
    {
    this->CameraNodeUpdatePending = 0;
    }

  const char *defaultCameraName = "Default Scene Camera";
  
  // how many cameras are in the scene?
  int numCameraNodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLCameraNode");
  // how many view nodes are in the scene?
  int numViewNodes = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLViewNode");
  vtkDebugMacro("UpdateCamera: num camera nodes = :" << numCameraNodes << ", num view nodes = " << numViewNodes);
  
  vtkMRMLCameraNode *camera_node = NULL;
  vtkMRMLCameraNode *unassignedCamera = NULL;
  vtkMRMLCameraNode  *pruneDefaultCamera = NULL;
  int foundDefaultCamera = 0;
  if (this->ViewNode && this->ViewNode->GetName())
    {
    std::vector<vtkMRMLNode *> cnodes;
    int nnodes = this->MRMLScene->GetNodesByClass("vtkMRMLCameraNode", cnodes);
    vtkMRMLCameraNode *node = NULL;
    for (int n=0; n<nnodes; n++)
      {
      node = vtkMRMLCameraNode::SafeDownCast (cnodes[n]);
      if (node)
        {
        // does the node point to my view node?
        if (node->GetActiveTag())
          {
          if (node->GetName() &&
              !strcmp(node->GetName(), defaultCameraName) &&
              !foundDefaultCamera)
            {
            // is this a default camera node that we created when the view
            // node was created?
            vtkDebugMacro("UpdateCamera: found a default camera node pointing to my view node");
            pruneDefaultCamera = node;
            // If there is a new camera with a non-null ActiveTag,
            // We want to be sure to null-ify the Active Tag of the
            // camera created upon view node creation (given the
            // default name = defaultCameraName.
            //
            // Logic here will deactivate this->ViewNode's original camera
            // (the one with defaultCameraName) marked with ActiveTag 
            // ONLY IF another camera node is found marked with a non-null ActiveTag
            // matching this->ViewNode.
            //
            // BUT: a new scene may be loaded which has another camera with the
            // name = defaultCameraName. If this is parameterized with a non-null
            // ActiveTag, then we want to be sure NOT to deactivate it instead of
            // the original camera by mistake.
            //
            // Add this flag once we find the first camera. Hacky: oh yes, but...
            foundDefaultCamera = 1;
            }
          if (!strcmp(node->GetActiveTag(), this->ViewNode->GetID()))
            {
            // We found a node with a non-null ActiveTag. Might be original
            // camera with defaultCameraName, or might be a new camera
            // also with defaultCameraName, or might be a new camera
            // with some unique name.
            vtkDebugMacro("UpdateCamera: found a camera pointing to me with id = " << (node->GetID() ? node->GetID() : "NULL"));
            camera_node = node;
            // Whichever camera is the last found pointing at the view node
            // will be the selected "ActiveCamera"
            }
          else
            {
            // it points to another view node
            }
          }
        else
          {
          // steal it!
          unassignedCamera = node;
          }
        }
      }
    }

  if (pruneDefaultCamera != NULL)
    {
    // is there a camera node that's not the default that wants to point to my
    // view node
    if (camera_node != NULL &&
        camera_node != pruneDefaultCamera)
      {
      // unhook the default node
      vtkDebugMacro("UpdateCamera: pruning default camera node, set it's active tag to null");
      pruneDefaultCamera->SetActiveTag(NULL);
      }
    }
  // if a camera node already points to me
  // do I already have it?
  if (camera_node != NULL &&
      this->CameraNode != NULL &&
      this->CameraNode == camera_node)
    {
    // I'm already pointing to it
    vtkDebugMacro("UpdateCamera: camera node " << camera_node->GetID() << " is already pointing to my view node and I'm observing it, CameraNode = " << this->CameraNode->GetID());
    return;
    }

  // do I have a camera node?
  if (this->CameraNode == NULL)
    {
    if (camera_node != NULL)
      {
      // I'm not observing the camera node that is using my view node's id as
      // it's active tag
      vtkDebugMacro("UpdateCamera: am not observing any camera nodes, now observe the camera node that's pointing at my view node");
      this->SetAndObserveCameraNode(camera_node);
      }
    else
      {
      // is there an unasigned camera node?
      if (unassignedCamera != NULL)
        {
        // use it!
        vtkDebugMacro("UpdateCamera: setting active tag on unassinged camera");
        unassignedCamera->SetActiveTag(this->ViewNode->GetID());
        vtkDebugMacro("UpdateCamera: setting and observing unassigned camera");
        this->SetAndObserveCameraNode(unassignedCamera);
        }
      else
        {
        // create one
        unassignedCamera =  vtkMRMLCameraNode::New();
        vtkDebugMacro("Viewer widget: Created new unassigned camera");
        unassignedCamera->SetName(defaultCameraName);
        //this->MRMLScene->GetUniqueNameByString(camera_node->GetNodeTagName()));
        unassignedCamera->SetActiveTag(
                                       this->ViewNode ? this->ViewNode->GetID() : NULL);
        this->MRMLScene->AddNode(unassignedCamera);
        this->SetAndObserveCameraNode(unassignedCamera);
        unassignedCamera->Delete();
        }
      }
    }
  else
    {
    if (camera_node != NULL)
      {
      // I'm not observing the camera node that is using my view node's id as
      // it's active tag
      vtkDebugMacro("Resetting to observe the camera node that's pointing at my view node");
      this->SetAndObserveCameraNode(camera_node);
      }
    else
      {
      // can get here if a view node steals my camera node
      vtkDebugMacro("I don't have a camera node, nothing is pointing to my view node");
      // can I swap for an unassigned one?
      if (unassignedCamera != NULL)
        {
        // swap!
        vtkWarningMacro("Stealing an unasigned camera node " << unassignedCamera->GetID());
        unassignedCamera->SetActiveTag(this->ViewNode->GetID());
        this->SetAndObserveCameraNode(unassignedCamera);
        }
      else
        {
        vtkDebugMacro("No unassigned camera in the scene to steal!");
        }
      }
    }
  //  if (this->CameraNode != NULL && 
  //      this->MRMLScene->GetNodeByID(this->CameraNode->GetID()) == NULL)
  //    {
  // local node not in the scene (what? how is that even possible??)
  //    this->SetAndObserveCameraNode (NULL);

  // No change? Bail.
/*
  if (this->CameraNode == camera_node)
    {
    if (this->CameraNode || this->CameraNodeWasCreated)
      {
      // is there a new valid camera?
      if (!this->MRMLScene)
        {
        return;
        }
      int numCameras = this->MRMLScene->GetNumberOfNodesByClass("vtkMRMLCameraNode");
      if (numCameras > 1)
        {
        // yes: delete our camera node
        vtkWarningMacro("We created an unnecessary new camera node, deleting it");
        vtkCollection *col = this->MRMLScene->GetNodesByName(defaultCameraName);
        if (col)
          {
          int numNodes = col->GetNumberOfItems();
          if (numNodes > 0)
            {
            for (int n = 0; n < numNodes; n++)
              {
              vtkMRMLCameraNode *camNode = vtkMRMLCameraNode::SafeDownCast(col->GetItemAsObject(n));
              if (camNode)
                {
                vtkWarningMacro("Removing camera node " << camNode->GetID());
                this->MRMLScene->RemoveNode(camNode);
                }
              }
            this->CameraNodeWasCreated = 0;
            }
          col->RemoveAllItems();
          col->Delete();
          }
        }
      return;
      }
    vtkWarningMacro("Viewer widget camera node is equal to input camera node. my camera node is " << (this->CameraNode == NULL ? "NULL" : "not null") << ", camera_node = " << (camera_node == NULL ? "NULL" : "not null"));
    // no camera in the scene after it's done loading, create an active camera
    this->CameraNodeWasCreated = 1;
    camera_node = vtkMRMLCameraNode::New();
    vtkWarningMacro("Viewer widget: Created new camera");
    camera_node->SetName(defaultCameraName);
    //this->MRMLScene->GetUniqueNameByString(camera_node->GetNodeTagName()));
    camera_node->SetActiveTag(
                              this->ViewNode ? this->ViewNode->GetID() : NULL);
    this->MRMLScene->AddNode(camera_node);
    camera_node->Delete();
    }
 
  this->SetAndObserveCameraNode(camera_node);
*/
  vtkCamera *cam = this->CameraNode ? this->CameraNode->GetCamera() : NULL;
  this->MainViewer->GetRenderer()->SetActiveCamera(cam);
  if (cam) 
    {
    // do not call if no camera otherwise it will create a new one without a node
    this->MainViewer->GetRenderer()->ResetCameraClippingRange();
    // do we need to update axis actors etc?
    }
 
  vtkRenderWindowInteractor *rwi = 
    this->MainViewer->GetRenderWindowInteractor();
  if (rwi)
    {
    vtkInteractorObserver *iobs = rwi->GetInteractorStyle();
    vtkSlicerViewerInteractorStyle *istyle = 
      vtkSlicerViewerInteractorStyle::SafeDownCast(iobs);
    if (istyle)
      {
      istyle->SetCameraNode(this->CameraNode);
      if (istyle->GetApplicationLogic() == NULL &&
          this->GetApplicationLogic() != NULL)
        {
        vtkDebugMacro("Updating interactor style's application logic, since it was null");
        istyle->SetApplicationLogic(this->GetApplicationLogic());
        }
      }
    }

  if (cam) 
    {
    // make sure the axis actors follow the new camera
    // NOTE: 
    // a call to this method is made asyncrhonously when processing
    // an ActiveTagModifiedEvent triggered by the cameranode.
    // But this can be called when IsRendering = 1. This causes
    // the UpdateAxis method to dump out early, and the AxisLabelActors'
    // camera is never set to synch up with a new ActiveCamera.
    this->UpdateAxis();
    }

  this->InvokeEvent(vtkSlicerViewerWidget::ActiveCameraChangedEvent, NULL);
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::AddCameraObservers()
{
  if (!this->MRMLScene || this->SceneClosing)
    {
    return;
    }

  vtkEventBroker *broker = vtkEventBroker::GetInstance();
  std::vector< vtkObservation *> observations;
  std::vector<vtkMRMLNode*> cnodes;
  int nnodes = this->MRMLScene->GetNodesByClass("vtkMRMLCameraNode", cnodes);
  for (int n = 0; n < nnodes; n++)
    {
    vtkMRMLCameraNode *node = vtkMRMLCameraNode::SafeDownCast(cnodes[n]);
    observations = broker->GetObservations( 
      node, vtkMRMLCameraNode::ActiveTagModifiedEvent, this, this->MRMLCallbackCommand );
    if ( node && observations.size() == 0 )
      {
      vtkEventBroker::GetInstance()->AddObservation ( 
        node, vtkMRMLCameraNode::ActiveTagModifiedEvent, this, this->MRMLCallbackCommand );
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::RemoveCameraObservers()
{
  if (!this->MRMLScene)
    {
    return;
    }

  vtkEventBroker *broker = vtkEventBroker::GetInstance();
  std::vector< vtkObservation *> observations;
  std::vector<vtkMRMLNode*> cnodes;
  int nnodes = this->MRMLScene->GetNodesByClass("vtkMRMLCameraNode", cnodes);
  for (int n = 0; n < nnodes; n++)
    {
    vtkMRMLCameraNode *node = vtkMRMLCameraNode::SafeDownCast(cnodes[n]);
    observations = broker->GetObservations( 
      node, vtkMRMLCameraNode::ActiveTagModifiedEvent, this, this->MRMLCallbackCommand );
    if ( node && observations.size() != 0 )
      {
      vtkEventBroker::GetInstance()->RemoveObservations ( observations );
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UpdateViewNode()
{
#if 0
  // This is not needed anymore, there is not one single vtkMRMLViewNode
  // but multiple, therefore it's up to whoever created that widget
  // to assign it the ViewNode properly, not this widget to pick the
  // first one...

  if (this->SceneClosing || !this->MRMLScene)
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
#endif
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

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  this->ViewerFrame = vtkKWFrame::New ( );
  this->ViewerFrame->SetParent ( this->GetParent ( ) );
  this->ViewerFrame->Create ( );

  //double *c = slicerStyle->GetHighLightColor();
  double c[3] = { 0.4, 0.0, 0.6 };
  this->ViewerFrame->SetConfigurationOptionAsInt("-highlightthickness", 2);
  this->ViewerFrame->SetConfigurationOptionAsColor("-highlightcolor",
                                                   c[0], c[1], c[2]);

  
  // Revert back to KW superclass renderwidget to address
  // window corruption on some linux boxes:
  //this->MainViewer = vtkSlicerRenderWidget::New ( );  
  this->MainViewer = vtkKWRenderWidget::New ( );  
  
  // added to enable active/passive stereo mode.  If user started slicer with --stereo
  // command line option, then stereo might be enabled sometime during this session.  Therefore
  // we need to request the correct OpenGL visual BEFORE the window is opened.  Quadbuffered
  // stereo has to be requested before the first Render() call on the window.
  
  if (vtkSlicerApplication::GetInstance()->GetStereoEnabled())
    {
    vtkDebugMacro("Opening Stereo Capable Window");
    vtkRenderWindow* renWin = this->MainViewer->GetRenderWindow();
    renWin->SetStereoCapableWindow(1);
    renWin->SetStereoTypeToCrystalEyes();
    }
  
  this->MainViewer->SetParent (this->ViewerFrame );
  this->MainViewer->Create ( );
  // don't use vtkKWRenderWidget's built-in ExposeEvent handler.  
  // It will call ProcessPendingEvents (update) even though it may already be inside
  // a call to update.  It also calls Render directly, which will pull the vtk pipeline chain.
  // Instead, use the RequestRender method to render when idle.
  this->MainViewer->GetVTKWidget()->RemoveBinding("<Expose>");
  this->MainViewer->GetVTKWidget()->AddBinding("<Expose>", this, "RequestRender");


  this->MainViewer->SetRendererBackgroundColor(
    app->GetSlicerTheme()->GetSlicerColors()->ViewerBlue );

  // tell the render widget not to respond to the Render() method
  // - this class turns on rendering explicitly when it's own
  //   Render() method is called.  This avoids redundant renders
  //   when, for example, the annotation is changed.
  //  TODO: this is disabled for until there is a way to observe 
  //  for direct render requests to the renderer (so they can be placed
  //  in the event queue)
  //this->MainViewer->RenderStateOff();

  // make a Slicer viewer interactor style to process our events
  // look at the InteractorStyle to get our events
  vtkRenderWindowInteractor *rwi = this->MainViewer->GetRenderWindowInteractor();
  if (rwi)
    {
    vtkSlicerViewerInteractorStyle *iStyle = vtkSlicerViewerInteractorStyle::New();
    iStyle->SetViewerWidget( this );
    if (this->ApplicationLogic != NULL)
      {
      iStyle->SetApplicationLogic ( this->ApplicationLogic );
      }
    else
      {
      vtkDebugMacro("Not setting interactor style's application logic to null.");
      }

    rwi->SetInteractorStyle (iStyle);
    iStyle->Delete();
    }


  // Set the viewer's minimum dimension to be the modifiedState as that for
  // the three main Slice viewers.
  vtkCamera *camera = this->MainViewer->GetRenderer()->IsActiveCameraCreated() ? 
    this->MainViewer->GetRenderer()->GetActiveCamera() : NULL;
  if (camera)
    {
    camera->ParallelProjectionOff();
    }
  if ( this->GetApplication() != NULL )
    {
    app = (vtkSlicerApplication *)this->GetApplication();
    this->MainViewer->SetWidth ( app->GetDefaultGeometry()->GetSliceViewerMinDim() );
    this->MainViewer->SetHeight ( app->GetDefaultGeometry()->GetSliceViewerMinDim() );
    }
    

  // observe scene for add/remove nodes
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneAboutToBeClosedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneAboutToBeImportedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneImportedEvent);
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneRestoredEvent);
  this->SetAndObserveMRMLSceneEvents(this->MRMLScene, events);
  events->Delete();

  this->CreateClipSlices();
  this->CreateAxis();
  this->MainViewer->ResetCamera ( );
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UpdateFromMRML()
{
  // not called anymore; code stubbed out.
  this->UpdateViewNode();

  this->AddCameraObservers();
  this->RequestCameraNodeUpdate();

  this->AddAxisActors();

  this->UpdateClipSlicesFromMRML();

  this->RemoveModelProps ( );
  
  this->UpdateModelsFromMRML();

  this->RequestRender ( );

  this->UpdateFromMRMLRequested = 0;
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UpdateModelsFromMRML()
{
  vtkMRMLScene *scene = this->GetMRMLScene();
  vtkMRMLNode *node = NULL;
  std::vector<vtkMRMLDisplayableNode *> slices;

  // find volume slices
  bool clearDisplayedModels = false;
  
  std::vector<vtkMRMLNode *> dnodes;
  int nnodes = scene->GetNodesByClass("vtkMRMLDisplayableNode", dnodes);
  for (int n=0; n<nnodes; n++)
    {
    node = dnodes[n];
    vtkMRMLDisplayableNode *model = vtkMRMLDisplayableNode::SafeDownCast(node);
    // render slices last so that transparent objects are rendered in front of them
    if (!strcmp(model->GetName(), "Red Volume Slice") ||
        !strcmp(model->GetName(), "Green Volume Slice") ||
        !strcmp(model->GetName(), "Yellow Volume Slice"))
      {
      slices.push_back(model);
      vtkMRMLDisplayNode *dnode = model->GetDisplayNode();
      if (dnode && this->DisplayedActors.find(dnode->GetID()) == this->DisplayedActors.end() )
        {
        clearDisplayedModels = true;
        }
      }
    }

  if (clearDisplayedModels)
    {
    std::map<std::string, vtkProp3D *>::iterator iter;
    for (iter = this->DisplayedActors.begin(); iter != this->DisplayedActors.end(); iter++)
      {
      this->MainViewer->RemoveViewProp(iter->second);
      }
    //this->MainViewer->RemoveAllViewProps();
    this->RemoveModelObservers(1);
    this->RemoveHierarchyObservers(1);
    this->DisplayedActors.clear();
    this->DisplayedNodes.clear();
    this->DisplayedClipState.clear();
    this->DisplayedVisibility.clear();
    this->AddAxisActors();
    this->UpdateModelHierarchies();
    }

  // render slices first
  for (unsigned int i=0; i<slices.size(); i++)
    {
    vtkMRMLDisplayableNode *model = slices[i];
    // add nodes that are not in the list yet
    vtkMRMLDisplayNode *dnode = model->GetDisplayNode();
    if (dnode && this->DisplayedActors.find(dnode->GetID()) == this->DisplayedActors.end() )
      {
      this->UpdateModel(model);
      } 
    this->SetModelDisplayProperty(model);
    }
  
  // render the rest of the models
  //int nmodels = scene->GetNumberOfNodesByClass("vtkMRMLDisplayableNode");
  for (int n=0; n<nnodes; n++)
    {
    vtkMRMLDisplayableNode *model = vtkMRMLDisplayableNode::SafeDownCast(dnodes[n]);
    // render slices last so that transparent objects are rendered in fron of them
    if (model)
      {
      if (!strcmp(model->GetName(), "Red Volume Slice") ||
          !strcmp(model->GetName(), "Green Volume Slice") ||
          !strcmp(model->GetName(), "Yellow Volume Slice"))
        {
        continue;
        }
      this->UpdateModifiedModel(model);
      }
    } // end while
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UpdateModifiedModel(vtkMRMLDisplayableNode *model)
{

  this->UpdateModelHierarchyDisplay(model);
  this->UpdateModel(model);
  this->SetModelDisplayProperty(model);
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UpdateModelPolyData(vtkMRMLDisplayableNode *model)
{
  std::vector< vtkMRMLDisplayNode *> displayNodes = this->GetDisplayNode(model);
  vtkMRMLDisplayNode *hdnode = this->GetHierarchyDisplayNode(model);
  
  for (unsigned int i=0; i<displayNodes.size(); i++)
    {
    vtkMRMLDisplayNode *modelDisplayNode = displayNodes[i];
    vtkProp3D* prop = NULL;
    bool hasPolyData = true;

    int clipping = modelDisplayNode->GetClipping();
    int visibility = modelDisplayNode->GetVisibility();
    vtkPolyData *poly = modelDisplayNode->GetPolyData();

    if (hdnode) 
      {
      clipping = hdnode->GetClipping();
      //visibility = hdnode->GetVisibility();
      poly = hdnode->GetPolyData();
      }  
    // hierarchy display nodes may not have poly data pointer
    if (poly == NULL)
      {
      poly = model->GetPolyData();
      }
    if (poly == NULL)
      {
      hasPolyData = false;
      }
     
    std::map<std::string, vtkProp3D *>::iterator ait;
    ait = this->DisplayedActors.find(modelDisplayNode->GetID());
    if (ait == this->DisplayedActors.end() )
      {
      if ( poly )
        {
#ifdef USE_IMAGE_ACTOR
        if ( polyData->GetNumberOfCells() == 1 )
          {
          prop = vtkImageActor::New();
          }
#endif
        }
      if ( !prop )
        {
        prop = vtkActor::New();
        }
      }
    else
      {
      prop = (*ait).second;
      std::map<std::string, int>::iterator cit = this->DisplayedClipState.find(modelDisplayNode->GetID());
      if (modelDisplayNode && cit != this->DisplayedClipState.end() && cit->second == clipping )
        {
        this->DisplayedVisibility[modelDisplayNode->GetID()] = visibility;
        // make sure that we are looking at the current polydata (most of the code in here 
        // assumes a display node will never change what polydata it wants to view and hence
        // caches information to skip steps if the display node has already rendered. but we
        // can have rendered a display node but not rendered its current polydata.
        vtkActor *actor = vtkActor::SafeDownCast(prop);
        if (actor)
          {
          vtkPolyDataMapper *mapper = vtkPolyDataMapper::SafeDownCast(actor->GetMapper());
          if (mapper && mapper->GetInput() != poly && !(this->ClippingOn && clipping))
            {
            mapper->SetInput(poly);
            }
          }
        vtkMRMLTransformNode* tnode = model->GetParentTransformNode();
        // clipped model could be transformed
        if (clipping == 0 || tnode == NULL || !tnode->IsLinear())
          {
          continue;
          }
        }
      }

    vtkActor *actor;
    vtkClipPolyData *clipper = NULL;
    actor = vtkActor::SafeDownCast(prop);
    if ( actor )
      {
      if (this->ClippingOn && modelDisplayNode != NULL && clipping)
        {
        clipper = this->CreateTransformedClipper(model);
        }

      vtkPolyDataMapper *mapper = vtkPolyDataMapper::New ();
     
      if (clipper)
        {
        clipper->SetInput ( poly );
        clipper->Update();
        mapper->SetInput ( clipper->GetOutput() );
        }
      else
        {
        mapper->SetInput ( poly );
        }

   
      actor->SetMapper( mapper );
      mapper->Delete();
      }

    if (hasPolyData && ait == this->DisplayedActors.end())
      {
      this->MainViewer->AddViewProp( prop );
      this->DisplayedActors[modelDisplayNode->GetID()] = prop;
      this->DisplayedNodes[std::string(modelDisplayNode->GetID())] = modelDisplayNode;
    
      if (modelDisplayNode)
        {
        this->DisplayedVisibility[modelDisplayNode->GetID()] = visibility;
        }
      else
        {
        this->DisplayedVisibility[modelDisplayNode->GetID()] = 1;
        }
      
      if (clipper)
        {
        this->DisplayedClipState[modelDisplayNode->GetID()] = 1;
        clipper->Delete();
        }
      else
        {
        this->DisplayedClipState[modelDisplayNode->GetID()] = 0;
        }
      prop->Delete();
      }
    else if (!hasPolyData)
      {
      prop->Delete();
      }
    else 
      {
      if (clipper)
        {
        this->DisplayedClipState[modelDisplayNode->GetID()] = 1;
        clipper->Delete();
        }
      else 
        {
        this->DisplayedClipState[modelDisplayNode->GetID()] = 0;
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::UpdateModel(vtkMRMLDisplayableNode *model)
{
  this->UpdateModelPolyData(model);

  vtkEventBroker *broker = vtkEventBroker::GetInstance();
  std::vector< vtkObservation *> observations;
  // observe polydata
  observations = broker->GetObservations( model, vtkMRMLDisplayableNode::PolyDataModifiedEvent, this, this->MRMLCallbackCommand );
  if ( observations.size() == 0 )
    {
    broker->AddObservation( model, vtkMRMLDisplayableNode::PolyDataModifiedEvent, this, this->MRMLCallbackCommand );
    DisplayableNodes[model->GetID()] = model;
    }
  // observe display node
  observations = broker->GetObservations( model, vtkMRMLDisplayableNode::DisplayModifiedEvent, this, this->MRMLCallbackCommand );
  if ( observations.size() == 0 )
    {
    broker->AddObservation( model, vtkMRMLDisplayableNode::DisplayModifiedEvent, this, this->MRMLCallbackCommand );
    }

  observations = broker->GetObservations( model, vtkMRMLTransformableNode::TransformModifiedEvent, this, this->MRMLCallbackCommand );
  if ( observations.size() == 0 )
    {
    broker->AddObservation( model, vtkMRMLTransformableNode::TransformModifiedEvent, this, this->MRMLCallbackCommand );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::CheckModelHierarchies()
{
  if (this->MRMLScene == NULL || this->ModelHierarchyLogic == NULL)
    {
    return;
    }
  this->ModelHierarchyLogic->SetMRMLScene(this->MRMLScene);
  int nnodes = this->ModelHierarchyLogic->GetNumberOfModelsInHierarchy();
  this->ModelHierarchiesPresent = nnodes > 0 ? true:false;
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::AddHierarchiyObservers()
{
  if (this->MRMLScene == NULL)
    {
    return;
    }
  vtkMRMLModelHierarchyNode *node;
  
  vtkEventBroker *broker = vtkEventBroker::GetInstance();
  std::vector<vtkMRMLNode *> hnodes;
  int nnodes = this->MRMLScene->GetNodesByClass("vtkMRMLModelHierarchyNode", hnodes);

  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLModelHierarchyNode::SafeDownCast (hnodes[n]);
    bool found = false;
    std::map<std::string, int>::iterator iter;
    // search for matching string (can't use find, since it would look for 
    // matching pointer not matching content)
    for(iter=this->RegisteredModelHierarchies.begin(); iter != this->RegisteredModelHierarchies.end(); iter++) 
      {
      if ( iter->first.c_str() && !strcmp( iter->first.c_str(), node->GetID() ) )
        {
        found = true;
        break;
        }
      }
    if (!found)
      {
      broker->AddObservation( node, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
      this->RegisteredModelHierarchies[node->GetID()] = 0;
      }
    }
}

//----------------------------
void vtkSlicerViewerWidget::UpdateModelHierarchyVisibility(vtkMRMLModelHierarchyNode* mhnode, int visibility )
{
  vtkMRMLDisplayNode* dnode = mhnode->GetDisplayNode();
  if (dnode)
    {
    std::map<std::string, vtkProp3D *>::iterator iter = this->DisplayedActors.find(dnode->GetID());
    if (iter != this->DisplayedActors.end())
      {
      vtkProp3D *actor = iter->second;
      actor->SetVisibility(visibility);
      this->DisplayedVisibility[dnode->GetID()] = visibility;
      }
    }
}

//----------------------------
void vtkSlicerViewerWidget::UpdateModelHierarchyDisplay(vtkMRMLDisplayableNode *model)
{
  if (model && this->ModelHierarchyLogic)
    {
    vtkMRMLModelHierarchyNode* mhnode = this->ModelHierarchyLogic->GetModelHierarchyNode(model->GetID());

    if (mhnode) 
      {
      // turn off visibility of this node
      int ndnodes = model->GetNumberOfDisplayNodes();
      for (int i=0; i<ndnodes; i++)
        {
        vtkMRMLDisplayNode *dnode = model->GetNthDisplayNode(i);
        if (dnode)
          {
          std::map<std::string, vtkProp3D *>::iterator iter = this->DisplayedActors.find(dnode->GetID());
          if (iter != this->DisplayedActors.end())
            {
            vtkProp3D *actor = iter->second;
            actor->SetVisibility(0);
            this->DisplayedVisibility[dnode->GetID()] = 0;
            }
          }
        }

      // turn off visibility for hierarchy nodes in the tree
      vtkMRMLModelHierarchyNode *parent = mhnode;
      do 
        {
        this->UpdateModelHierarchyVisibility(parent, 0);
        parent = vtkMRMLModelHierarchyNode::SafeDownCast(parent->GetParentNode());
        }
      while (parent != NULL);
      }
    }
}

//----------------------------
std::vector< vtkMRMLDisplayNode* > vtkSlicerViewerWidget::GetDisplayNode(vtkMRMLDisplayableNode *model)
{
  std::vector< vtkMRMLDisplayNode* > dnodes;
  vtkMRMLDisplayNode* dnode = NULL;

  int ndnodes = model->GetNumberOfDisplayNodes();
  for (int i=0; i<ndnodes; i++)
    {
    dnode = model->GetNthDisplayNode(i);
    if (dnode)
      {
      dnodes.push_back(dnode);
      }
    }
    
  return dnodes;
}

//----------------------------
vtkMRMLDisplayNode*  vtkSlicerViewerWidget::GetHierarchyDisplayNode(vtkMRMLDisplayableNode *model)
{
  vtkMRMLDisplayNode* dnode = NULL;
  if (this->ModelHierarchiesPresent)
    {
    vtkMRMLModelHierarchyNode* mhnode = NULL;
    vtkMRMLModelHierarchyNode* phnode = NULL;
    mhnode = this->ModelHierarchyLogic->GetModelHierarchyNode(model->GetID());
    if (mhnode) 
      {
      phnode = mhnode->GetUnExpandedParentNode();
      }
    if (phnode) 
      {
      dnode = phnode->GetDisplayNode();
      }
    }
  return dnode;
}


//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::RequestRender()
{
  // move the render to the last thing on the idle processing queue
  this->Script("after cancel { if {[info command %s] != {}} {%s Render} }", this->GetTclName(), this->GetTclName());
  this->Script("after idle { if {[info command %s] != {}} {%s Render} }", this->GetTclName(), this->GetTclName());
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::Render()
{
  if (this->UpdateFromMRMLRequested)
    {
    this->UpdateFromMRML();
    this->UpdateAxis();
    }
  // *** added code to check the RenderState and restore to whatever it
  // was before the specific request to render, instead of just setting
  // renderState to OFF.
  if (this->GetEnableRender())
    {
    int currentRenderState = this->MainViewer->GetRenderState();
    this->MainViewer->RenderStateOn();
    if (this->MainViewer->GetRenderer()->IsActiveCameraCreated())
      {
      this->IsRendering = 1;
      this->MainViewer->Render();
      this->IsRendering = 0;
      }
    vtkDebugMacro("vtkSlicerViewerWidget::Render called render" << endl);
    this->MainViewer->SetRenderState(currentRenderState);
    } 

  //--- 
  // Axis Label Followers are not synched up with a new Camera
  // if UpdateCameraNode (which calls UpdateAxis) is called during a
  // Render. UpdateAxis does not complete under these circumstances
  // leaving axis actors in the scene respecting the wrong camera. To avoid this, 
  // we schedule UpdateCameraNode for post-render, so that
  // UpdateCameraNode and its call to UpdateAxis completes.
  if ( this->CameraNodeUpdatePending )
    {
    this->RequestCameraNodeUpdate();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::RemoveModelProps()
{
  std::map<std::string, vtkProp3D *>::iterator iter;
  std::map<std::string, int>::iterator clipIter;
  std::vector<std::string> removedIDs;
  for(iter=this->DisplayedActors.begin(); iter != this->DisplayedActors.end(); iter++) 
    {
    vtkMRMLDisplayNode *modelDisplayNode = vtkMRMLDisplayNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(iter->first));
    if (modelDisplayNode == NULL)
      {
      this->MainViewer->RemoveViewProp(iter->second);
      removedIDs.push_back(iter->first);
      }
    else
      {
      int clipModel = 0;
      if (modelDisplayNode != NULL)
        {
        clipModel = modelDisplayNode->GetClipping();
        }
      clipIter = this->DisplayedClipState.find(iter->first);
      if (clipIter == this->DisplayedClipState.end())
        {
        vtkErrorMacro ("vtkSlicerViewerWidget::RemoveModelProps() Unknown clip state\n");
        }
      else 
        {
        if ((clipIter->second && !this->ClippingOn) ||
            (this->ClippingOn && clipIter->second != clipModel))
          {
          this->MainViewer->RemoveViewProp(iter->second);
          removedIDs.push_back(iter->first);
          }     
        }
      }
    }
  for (unsigned int i=0; i< removedIDs.size(); i++)
    {
    this->RemoveDispalyedID(removedIDs[i]);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::RemoveDisplayable(vtkMRMLDisplayableNode* model)
{
  if (!model)
    {
    return;
    }  
  int ndnodes = model->GetNumberOfDisplayNodes();
  std::map<std::string, vtkProp3D *>::iterator iter;
  std::vector<std::string> removedIDs;
  for (int i=0; i<ndnodes; i++)
    {
    vtkMRMLDisplayNode *displayNode = model->GetNthDisplayNode(i);
    if (displayNode)
      {
      iter = this->DisplayedActors.find(displayNode->GetID());
      if (iter != this->DisplayedActors.end())
        {
        this->MainViewer->RemoveViewProp(iter->second);
        removedIDs.push_back(iter->first);
        }
      }
    }

  for (unsigned int i=0; i< removedIDs.size(); i++)
    {
    this->RemoveDispalyedID(removedIDs[i]);
    }
  this->RemoveModelObservers(model);
  this->DisplayableNodes.erase(model->GetID());
}

void vtkSlicerViewerWidget::RemoveDispalyedID(std::string &id)
{
  std::map<std::string, vtkMRMLDisplayNode *>::iterator modelIter;
  this->DisplayedActors.erase(id);
  this->DisplayedClipState.erase(id);
  this->DisplayedVisibility.erase(id);
  modelIter = this->DisplayedNodes.find(id);
  if(modelIter != this->DisplayedNodes.end())
    {
    //this->RemoveModelObservers(modelIter->second);
    this->DisplayedNodes.erase(modelIter->first);
    }
}

int vtkSlicerViewerWidget::GetDisplayedModelsVisibility(vtkMRMLDisplayNode *model)
{
  int visibility = 1;
  
  std::map<std::string, int>::iterator iter;
  iter = this->DisplayedVisibility.find(model->GetID());
  if (iter != this->DisplayedVisibility.end())
    {
    visibility = iter->second;
    }

  return visibility;
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::RemoveMRMLObservers()
{
  this->RemoveModelObservers(1);
  this->RemoveHierarchyObservers(1);
  this->RemoveCameraObservers();

  this->SetAndObserveMRMLScene(NULL);
  this->SetAndObserveViewNode (NULL);
  this->SetAndObserveCameraNode(NULL);
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::RemoveModelObservers(int clearCache)
{
  std::map<std::string, vtkMRMLDisplayableNode *>::iterator iter;

  for (iter=this->DisplayableNodes.begin(); iter!=this->DisplayableNodes.end(); iter++)
    {
    this->RemoveModelObservers(iter->second);
    }
  if (clearCache)
    {
    this->DisplayableNodes.clear();
    this->DisplayedActors.clear();
    this->DisplayedNodes.clear();
    this->DisplayedClipState.clear();
    this->DisplayedVisibility.clear();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::RemoveModelObservers( vtkMRMLDisplayableNode *model)
{
  vtkEventBroker *broker = vtkEventBroker::GetInstance();
  std::vector< vtkObservation *> observations;
  if (model != NULL)
    {
    observations = broker->GetObservations( 
      model, vtkMRMLDisplayableNode::PolyDataModifiedEvent, this, this->MRMLCallbackCommand );
    broker->RemoveObservations( observations );
    observations = broker->GetObservations( 
      model, vtkMRMLDisplayableNode::DisplayModifiedEvent, this, this->MRMLCallbackCommand );
    broker->RemoveObservations( observations );
    observations = broker->GetObservations( 
      model, vtkMRMLTransformableNode::TransformModifiedEvent, this, this->MRMLCallbackCommand );
    broker->RemoveObservations( observations );
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::RemoveHierarchyObservers(int clearCache)
{
  vtkEventBroker *broker = vtkEventBroker::GetInstance();
  std::vector< vtkObservation *> observations;
  std::map<std::string, int>::iterator iter;
  for(iter=this->RegisteredModelHierarchies.begin(); iter != this->RegisteredModelHierarchies.end(); iter++) 
    {
    vtkMRMLModelHierarchyNode *node = vtkMRMLModelHierarchyNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(iter->first));
    if (node)
      {
      observations = broker->GetObservations( 
        node, vtkCommand::ModifiedEvent, this, this->MRMLCallbackCommand );
      broker->RemoveObservations( observations );
      }
    }
  if (clearCache)
    {
    RegisteredModelHierarchies.clear();
    }
}

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::SetModelDisplayProperty(vtkMRMLDisplayableNode *model)
{
  vtkMRMLTransformNode* tnode = model->GetParentTransformNode();
  vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
  transformToWorld->Identity();
  if (tnode != NULL && tnode->IsLinear())
    {
    vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
    lnode->GetMatrixTransformToWorld(transformToWorld);
    }
 
  std::vector<vtkMRMLDisplayNode *> dnodes = this->GetDisplayNode(model);
  vtkMRMLDisplayNode *hdnode = this->GetHierarchyDisplayNode(model);
  
  for (unsigned int i=0; i<dnodes.size(); i++)
    {
    vtkMRMLDisplayNode *dnode = dnodes[i];
    vtkMRMLDisplayNode *mdnode = dnode;
    if (dnode != NULL)
      {
      vtkProp3D *prop = this->GetActorByID(dnode->GetID());
      if (prop == NULL)
        {
        continue;
        }
      // use hierarchy dispaly node if it exists
      if (hdnode)
        {
        dnode = hdnode;
        }
        
      vtkActor *actor = vtkActor::SafeDownCast(prop);
      vtkImageActor *imageActor = vtkImageActor::SafeDownCast(prop);
      prop->SetUserMatrix(transformToWorld);

      prop->SetVisibility(mdnode->GetVisibility());
      this->DisplayedVisibility[dnode->GetID()] = mdnode->GetVisibility();

      if (actor)
        {
        actor->GetMapper()->SetScalarVisibility(mdnode->GetScalarVisibility());
        // if the scalars are visible, set active scalars, try to get the lookup
        // table
        if (mdnode->GetScalarVisibility())
          {
          if (mdnode->GetColorNode() != NULL)
            {
            if (mdnode->GetColorNode()->GetLookupTable() != NULL)
              {
              actor->GetMapper()->SetLookupTable(mdnode->GetColorNode()->GetLookupTable());
              }
            else if (mdnode->GetColorNode()->IsA("vtkMRMLProceduralColorNode") &&
                     vtkMRMLProceduralColorNode::SafeDownCast(mdnode->GetColorNode())->GetColorTransferFunction() != NULL)
              {
              actor->GetMapper()->SetLookupTable((vtkScalarsToColors*)(vtkMRMLProceduralColorNode::SafeDownCast(mdnode->GetColorNode())->GetColorTransferFunction()));
              }
            }

          int cellScalarsActive = 0;
          if (mdnode->GetActiveScalarName() == NULL)
            {
            // see if there are scalars on the poly data that are not set as
            // active on the display node
            vtkMRMLModelNode *mnode = vtkMRMLModelNode::SafeDownCast(model);
            if (mnode)
              {
              std::string pointScalarName = std::string(mnode->GetActivePointScalarName("scalars"));
              std::string cellScalarName = std::string(mnode->GetActiveCellScalarName("scalars"));
              vtkDebugMacro("Display node active scalar name was null, but the node says active point scalar name = '" << pointScalarName.c_str() << "', cell = '" << cellScalarName.c_str() << "'");
              if (pointScalarName.compare("") != 0)
                {
                vtkDebugMacro("Setting the display node's active scalar to " << pointScalarName.c_str());
                dnode->SetActiveScalarName(pointScalarName.c_str());
                }
              else
                {
                if (cellScalarName.compare("") != 0)
                  {
                  vtkDebugMacro("Setting the display node's active scalar to " << cellScalarName.c_str());
                  dnode->SetActiveScalarName(cellScalarName.c_str());
                  }
                else
                  {
                  vtkDebugMacro("No active scalars");
                  }
                }
              }
            }
          if (mdnode->GetActiveScalarName() != NULL)
            {
            vtkMRMLModelNode *mnode = vtkMRMLModelNode::SafeDownCast(model);
            if (mnode)
              {
              mnode->SetActiveScalars(mdnode->GetActiveScalarName(), "Scalars");
              if (strcmp(mdnode->GetActiveScalarName(), mnode->GetActiveCellScalarName("scalars")) == 0)
                {
                cellScalarsActive = 1;
                }
              }
            actor->GetMapper()->SelectColorArray(mdnode->GetActiveScalarName());
            }
          if (!cellScalarsActive)
            {
            // set the scalar range
            actor->GetMapper()->SetScalarRange(mdnode->GetScalarRange());
            //if (!(dnode->IsA("vtkMRMLFiberBundleDisplayNode")))
            //  {
            // WHY need this, does not show glyph colors otherwise
            //actor->GetMapper()->SetScalarModeToUsePointFieldData();
            // }
            actor->GetMapper()->SetScalarModeToUsePointData();            
            actor->GetMapper()->SetColorModeToMapScalars();            
            actor->GetMapper()->UseLookupTableScalarRangeOff();
            actor->GetMapper()->SetScalarRange(mdnode->GetScalarRange());
            }
          else
            {
            actor->GetMapper()->SetScalarModeToUseCellFieldData();
            actor->GetMapper()->SetColorModeToDefault();
            actor->GetMapper()->UseLookupTableScalarRangeOff();
            actor->GetMapper()->SetScalarRange(mdnode->GetScalarRange());
            }
          }
        //// }
        actor->GetProperty()->SetBackfaceCulling(dnode->GetBackfaceCulling());

        if (mdnode)
          {
          if (mdnode->GetSelected())
            {
            vtkDebugMacro("Model display node " << mdnode->GetName() << " is selected...");
            actor->GetProperty()->SetColor(mdnode->GetSelectedColor());
            actor->GetProperty()->SetAmbient(mdnode->GetSelectedAmbient());
            actor->GetProperty()->SetSpecular(mdnode->GetSelectedSpecular());
            }
          else
            {
            //vtkWarningMacro("Model display node " << mdnode->GetName() << " is not selected...");
            actor->GetProperty()->SetColor(dnode->GetColor());
            actor->GetProperty()->SetAmbient(dnode->GetAmbient());
            actor->GetProperty()->SetSpecular(dnode->GetSpecular());
            }
          }
        actor->GetProperty()->SetOpacity(dnode->GetOpacity());
        actor->GetProperty()->SetDiffuse(dnode->GetDiffuse());
        actor->GetProperty()->SetSpecularPower(dnode->GetPower());
        if (dnode->GetTextureImageData() != NULL)
          {
          if (actor->GetTexture() == NULL)
            {
            vtkTexture *texture = vtkTexture::New();
            texture->SetInterpolate(0);
            actor->SetTexture(texture);
            texture->Delete();
            }
          actor->GetTexture()->SetInput(dnode->GetTextureImageData());
          actor->GetProperty()->SetColor(1., 1., 1.);
          }
        else
          {
          actor->SetTexture(NULL);
          }
        }
      else if (imageActor)
        {
        if (dnode->GetTextureImageData() != NULL)
          {
          imageActor->SetInput(dnode->GetTextureImageData());
          }
        else
          {
          imageActor->SetInput(NULL);
          }
        imageActor->SetDisplayExtent(-1, 0, 0, 0, 0, 0);
        }
      }
    }

}

//---------------------------------------------------------------------------
// Description:
// return the current actor corresponding to a give MRML ID
vtkProp3D *
vtkSlicerViewerWidget::GetActorByID (const char *id)
{
  if ( !id )
    {
    return (NULL);
    }

  std::map<std::string, vtkProp3D *>::iterator iter;
  // search for matching string (can't use find, since it would look for 
  // matching pointer not matching content)
  for(iter=this->DisplayedActors.begin(); iter != this->DisplayedActors.end(); iter++) 
    {
    if ( iter->first.c_str() && !strcmp( iter->first.c_str(), id ) )
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
vtkSlicerViewerWidget::GetIDByActor (vtkProp3D *actor)
{
  if ( !actor )
    {
    return (NULL);
    }

  std::map<std::string, vtkProp3D *>::iterator iter;
  for(iter=this->DisplayedActors.begin(); iter != this->DisplayedActors.end(); iter++) 
    {
    if ( iter->second && ( iter->second == actor ) )
      {
      return (iter->first.c_str());
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
void vtkSlicerViewerWidget::GridSpanWidget ( vtkKWFrame *f, int row, int col, int rowspan, int colspan )
{
  this->Script ( "grid configure %s -in %s -row %d -column %d -stick news -padx 0 -pady 0 -rowspan %d -columnspan %d",
                 this->ViewerFrame->GetWidgetName(), f->GetWidgetName(), row, col, rowspan, colspan );
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

//---------------------------------------------------------------------------
void vtkSlicerViewerWidget::ResetPick()
{
  double zero[3] = {0.0, 0.0, 0.0};
  this->PickedNodeName = std::string("");
  this->SetPickedRAS(zero);
  this->SetPickedCellID(-1);
  this->SetPickedPointID(-1);
}

//---------------------------------------------------------------------------
int vtkSlicerViewerWidget::Pick(int x, int y)
{
  double RASPoint[3] = {0.0, 0.0, 0.0};
  double pickPoint[3] = {0.0, 0.0, 0.0};

  // reset the pick vars
  this->ResetPick();
  
  vtkRenderer *ren;
  if (this->GetMainViewer() != NULL)
    {
    ren = this->GetMainViewer()->GetRenderer();
    }
  else
    {
    vtkErrorMacro("Pick: unable to get renderer\n");
    return 0;
    }
  // get the current renderer's size
  int *renSize = ren->GetSize();
  // resize the interactor?
  
  // pass the event's display point to the world point picker
  double displayPoint[3];
  displayPoint[0] = x;
  displayPoint[1] = renSize[1] - y;
  displayPoint[2] = 0.0;

  if (this->CellPicker->Pick(displayPoint[0], displayPoint[1], displayPoint[2], ren))
    {
    this->CellPicker->GetPickPosition(pickPoint);
    this->SetPickedCellID(this->CellPicker->GetCellId());
    // get the pointer to the poly data that the cell was in
    vtkPolyData *polyData = vtkPolyData::SafeDownCast(this->CellPicker->GetDataSet());
    if (polyData != NULL)
      {
      // now find the model this poly data belongs to
      std::map<std::string, vtkMRMLDisplayNode *>::iterator modelIter;
      for (modelIter = this->DisplayedNodes.begin();
           modelIter != this->DisplayedNodes.end();
           modelIter++)
        {
        vtkDebugMacro("Checking model " << modelIter->first.c_str() << "'s polydata");
        if (modelIter->second != NULL)
          {
          if (modelIter->second->GetPolyData() == polyData)
            {
            vtkDebugMacro("Found matching poly data, pick was on model " << modelIter->first.c_str());
            this->PickedNodeName = modelIter->first;
            
            // figure out the closest vertex in the picked cell to the picked RAS
            // point. Only doing this on model nodes for now.
            vtkCell *cell = polyData->GetCell(this->GetPickedCellID());
            if (cell != NULL)
              {
              int numPoints = cell->GetNumberOfPoints();
              int closestPointId = -1;
              double closestDistance = 0.0l;
              for (int p = 0; p < numPoints; p++)
                {
                int pointId = cell->GetPointId(p);
                double *pointCoords = polyData->GetPoint(pointId);
                if (pointCoords != NULL)
                  {
                  double distance = sqrt(pow(pointCoords[0]-pickPoint[0], 2) +
                                         pow(pointCoords[1]-pickPoint[1], 2) +
                                         pow(pointCoords[2]-pickPoint[2], 2));
                  if (p == 0 ||
                      distance < closestDistance)
                    {
                    closestDistance = distance;
                    closestPointId = pointId;
                    }
                  }
                }
              vtkDebugMacro("Pick: found closest point id = " << closestPointId << ", distance = " << closestDistance);
              this->SetPickedPointID(closestPointId);
              }
            continue;
            }
          }
        }
      }
    }
  else
    {
    return 0;
    }
  /**
  if (this->PropPicker->PickProp(x, y, ren))
    {
    this->PropPicker->GetPickPosition(pickPoint);
    }
  else
    {
    return 0;
    }
    **/

  // world point picker's Pick always returns 0
  /**
  this->WorldPointPicker->Pick(displayPoint, ren);
  this->WorldPointPicker->GetPickPosition(pickPoint);
  **/

  // translate world to RAS
  for (int p = 0; p < 3; p++)
    {
    RASPoint[p] = pickPoint[p];
    }
  
  // now set up the class vars
  this->SetPickedRAS( RASPoint );

  return 1;
}     

void vtkSlicerViewerWidget::SetBoxWidgetInteractor ()
{
  if ( this->GetMainViewer() &&  
       this->GetMainViewer()->GetRenderWindow() &&
       this->GetMainViewer()->GetRenderWindow()->GetInteractor() )
    {
    vtkRenderWindowInteractor *interactor = this->GetMainViewer()->GetRenderWindow()->GetInteractor();
    this->BoxWidget->SetInteractor(interactor);
    }
}

vtkClipPolyData* vtkSlicerViewerWidget::CreateTransformedClipper (vtkMRMLDisplayableNode *model)
{
  vtkClipPolyData *clipper = vtkClipPolyData::New();
  clipper->SetValue( 0.0);

  vtkMRMLTransformNode* tnode = model->GetParentTransformNode();
  vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
  transformToWorld->Identity();
  if (tnode != NULL && tnode->IsLinear())
    {
    vtkMRMLLinearTransformNode *lnode = vtkMRMLLinearTransformNode::SafeDownCast(tnode);
    lnode->GetMatrixTransformToWorld(transformToWorld);

    vtkImplicitBoolean *slicePlanes =   vtkImplicitBoolean::New();
    if (this->ClipType == vtkMRMLClipModelsNode::ClipIntersection) 
      {
      slicePlanes->SetOperationTypeToIntersection();
      }
    else if (this->ClipType == vtkMRMLClipModelsNode::ClipUnion) 
      {
      slicePlanes->SetOperationTypeToUnion();
      }

    vtkPlane *redSlicePlane = vtkPlane::New();
    vtkPlane *greenSlicePlane = vtkPlane::New();
    vtkPlane *yellowSlicePlane = vtkPlane::New();


    if (this->RedSliceClipState != vtkMRMLClipModelsNode::ClipOff)
      {
      slicePlanes->AddFunction(redSlicePlane);
      }

    if (this->GreenSliceClipState != vtkMRMLClipModelsNode::ClipOff)
      {
      slicePlanes->AddFunction(greenSlicePlane);
      }

    if (this->YellowSliceClipState != vtkMRMLClipModelsNode::ClipOff)
      {
      slicePlanes->AddFunction(yellowSlicePlane);
      }

    vtkMatrix4x4 *sliceMatrix = NULL;
    vtkSmartPointer<vtkMatrix4x4> mat = vtkSmartPointer<vtkMatrix4x4>::New();
    int planeDirection = 1;
    transformToWorld->Invert();

    sliceMatrix = this->RedSliceNode->GetSliceToRAS();
    mat->Identity();
    vtkMatrix4x4::Multiply4x4(transformToWorld, sliceMatrix, mat);
    planeDirection = (this->RedSliceClipState == vtkMRMLClipModelsNode::ClipNegativeSpace) ? -1 : 1;
    this->SetClipPlaneFromMatrix(mat, planeDirection, redSlicePlane);

    sliceMatrix = this->GreenSliceNode->GetSliceToRAS();
    mat->Identity();
    vtkMatrix4x4::Multiply4x4(transformToWorld, sliceMatrix, mat);
    planeDirection = (this->GreenSliceClipState == vtkMRMLClipModelsNode::ClipNegativeSpace) ? -1 : 1;
    this->SetClipPlaneFromMatrix(mat, planeDirection, greenSlicePlane);

    sliceMatrix = this->YellowSliceNode->GetSliceToRAS();
    mat->Identity();
    vtkMatrix4x4::Multiply4x4(transformToWorld, sliceMatrix, mat);
    planeDirection = (this->YellowSliceClipState == vtkMRMLClipModelsNode::ClipNegativeSpace) ? -1 : 1;
    this->SetClipPlaneFromMatrix(mat, planeDirection, yellowSlicePlane);

    clipper->SetClipFunction(slicePlanes);
    
    slicePlanes->Delete();
    redSlicePlane->Delete();
    greenSlicePlane->Delete();
    yellowSlicePlane->Delete();

    }
  else 
    {
    clipper->SetClipFunction(this->SlicePlanes);
    }
  return clipper;
}
