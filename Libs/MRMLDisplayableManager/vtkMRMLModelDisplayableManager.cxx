/*=========================================================================

  Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.slicer.org/Slicer3/trunk/Base/GUI/vtkMRMLModelDisplayableManager.cxx $
  Date:      $Date: 2010-05-27 14:32:23 -0400 (Thu, 27 May 2010) $
  Version:   $Revision: 13525 $

==========================================================================*/

// MRMLLogic includes
#include <vtkMRMLModelHierarchyLogic.h>

// MRMLDisplayableManager includes
#include "vtkMRMLModelDisplayableManager.h"
#include "vtkDisplayableManagerInteractorStyle.h"

// MRML includes
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLDisplayNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLClipModelsNode.h>
#include <vtkMRMLModelHierarchyNode.h>
#include <vtkMRMLColorNode.h>
#include <vtkMRMLProceduralColorNode.h>

// VTK includes
#include <vtkObject.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkMatrix4x4.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkMath.h>
#include <vtkProp3D.h>
#include <vtkActor.h>
#include <vtkImageActor.h>
#include <vtkFollower.h>
#include <vtkProperty.h>
#include <vtkTexture.h>
#include <vtkRenderer.h>
#include <vtkCamera.h>
#include <vtkPolyDataMapper.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkOutlineSource.h>
#include <vtkVectorText.h>
#include <vtkRenderWindow.h>
#include <vtkImplicitBoolean.h>
#include <vtkPlane.h>
#include <vtkClipPolyData.h>
#include <vtkBoundingBox.h>

// for picking
#include <vtkWorldPointPicker.h>
#include <vtkPropPicker.h>
#include <vtkCellPicker.h>
#include <vtkPointPicker.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>

#include <vtkDisplayableManagerBoxWidget2.h>
#include <vtkDisplayableManagerBoxRepresentation.h>

// STD includes
#include <string>
#include <sstream>
#include <cassert>

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkMRMLModelDisplayableManager );
vtkCxxRevisionMacro ( vtkMRMLModelDisplayableManager, "$Revision: 13525 $");

//---------------------------------------------------------------------------
vtkMRMLModelDisplayableManager::vtkMRMLModelDisplayableManager()
{
  this->ProcessingMRMLEvent = 0;
  this->UpdateFromMRMLRequested = 0;

  //this->CameraNode = 0;
  //this->CameraNodeWasCreated = 0;

  this->ClipModelsNode = 0;
  this->RedSliceNode = 0;
  this->GreenSliceNode = 0;
  this->YellowSliceNode = 0;

  this->SlicePlanes = 0;
  this->RedSlicePlane = 0;
  this->GreenSlicePlane = 0;
  this->YellowSlicePlane = 0;

  //this->ViewNode = 0;
  //this->BoxAxisActor = 0;
  //this->BoxAxisBoundingBox = new vtkBoundingBox;

  this->SceneClosing = false;

  this->ModelHierarchiesPresent = false;
  this->ModelHierarchyLogic = 0;

  this->WorldPointPicker = vtkWorldPointPicker::New();
  this->PropPicker = vtkPropPicker::New();
  this->CellPicker = vtkCellPicker::New();
  this->CellPicker->SetTolerance(0.00001);
  this->PointPicker = vtkPointPicker::New();
  this->ResetPick();

  this->PickedCellID = 0;
  this->PickedPointID = 0;

    
  this->BoxWidget = vtkDisplayableManagerBoxWidget2::New();
  this->BoxWidgetRepresentation = vtkDisplayableManagerBoxRepresentation::New();
  this->BoxWidget->SetRepresentation(this->BoxWidgetRepresentation);
  
  this->EnableRender = 1;
  this->UpdatingAxis = 0;
  this->IsRendering = 0;
  this->Created = 0;

  this->CreateClipSlices();
}

//---------------------------------------------------------------------------
vtkMRMLModelDisplayableManager::~vtkMRMLModelDisplayableManager()
{
  //delete this->BoxAxisBoundingBox;

  this->SetModelHierarchyLogic(0);

  this->RemoveMRMLObservers();

  vtkSetMRMLNodeMacro(this->ClipModelsNode, 0);
  //vtkSetMRMLNodeMacro(this->CameraNode, 0);
  //vtkSetMRMLNodeMacro(this->ViewNode, 0);

  vtkSetMRMLNodeMacro(this->RedSliceNode, 0);
  vtkSetMRMLNodeMacro(this->GreenSliceNode, 0);
  vtkSetMRMLNodeMacro(this->YellowSliceNode, 0);

  if (this->GetInteractor())
    {
    vtkDisplayableManagerInteractorStyle *iStyle;
    iStyle = vtkDisplayableManagerInteractorStyle::SafeDownCast (this->GetInteractor()->GetInteractorStyle());
    this->SetMRMLScene(0);
    }

  this->SlicePlanes->Delete();
  this->SlicePlanes = 0;
  this->RedSlicePlane->Delete();
  this->RedSlicePlane = 0;
  this->GreenSlicePlane->Delete();
  this->GreenSlicePlane = 0;
  this->YellowSlicePlane->Delete();
  this->YellowSlicePlane = 0;

  /*if (this->BoxAxisActor)
    {
    this->BoxAxisActor->Delete();
    this->BoxAxisActor = 0;
    }
  for (unsigned int i=0; i<this->AxisLabelActors.size(); i++)
    {
    this->AxisLabelActors[i]->SetCamera(0);
    this->AxisLabelActors[i]->Delete();
    }
  this->AxisLabelActors.clear();
  */
  // release the DisplayedModelActors
  this->DisplayedActors.clear();
  
  if (this->WorldPointPicker)
    {
    this->WorldPointPicker->Delete();
    this->WorldPointPicker = 0;
    }
  if (this->PropPicker)
    {
    this->PropPicker->Delete();
    this->PropPicker = 0;
    }
  if (this->CellPicker)
    {
    this->CellPicker->Delete();
    this->CellPicker = 0;
    }
  if (this->PointPicker)
    {
    this->PointPicker->Delete();
    this->PointPicker = 0;
    }

  if (this->ModelHierarchyLogic)
    {
    this->ModelHierarchyLogic->Delete();
    }
  
  this->BoxWidget->SetRepresentation(0);
  this->BoxWidgetRepresentation->Delete();
  this->BoxWidget->Delete();
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayableManager::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "vtkMRMLModelDisplayableManager: " << this->GetClassName() << "\n";

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

/*
//---------------------------------------------------------------------------
void vtkMRMLModelDisplayableManager::ProcessWidgetEvents(vtkObject *vtkNotUsed(caller),
                                                unsigned long vtkNotUsed(event),
                                                void *vtkNotUsed(callData))
{
  // Not currently used... 
  this->RequestRender();
} 
*/

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayableManager::CreateClipSlices()
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
int vtkMRMLModelDisplayableManager::UpdateClipSlicesFromMRML()
{
  if (this->GetMRMLScene() == 0)
    {
    return 0;
    }

  // update ClipModels node
  vtkMRMLClipModelsNode *clipNode = vtkMRMLClipModelsNode::SafeDownCast(this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLClipModelsNode"));
  if (clipNode != this->ClipModelsNode) 
    {
    vtkSetAndObserveMRMLNodeMacro(this->ClipModelsNode, clipNode);
    }

  if (this->ClipModelsNode == 0)
    {
    return 0;
    }

  // update Slice nodes
  vtkMRMLSliceNode *node= 0;
  vtkMRMLSliceNode *nodeRed= 0;
  vtkMRMLSliceNode *nodeGreen= 0;
  vtkMRMLSliceNode *nodeYellow= 0;
  
  std::vector<vtkMRMLNode *> snodes;
  int nnodes = this->GetMRMLScene()->GetNodesByClass("vtkMRMLSliceNode", snodes);
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
    node = 0;
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

  if (this->RedSliceNode == 0 || this->GreenSliceNode == 0 || this->YellowSliceNode == 0)
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
  vtkMatrix4x4 *sliceMatrix = 0;
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
void vtkMRMLModelDisplayableManager::SetClipPlaneFromMatrix(vtkMatrix4x4 *sliceMatrix, 
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
void vtkMRMLModelDisplayableManager::ProcessMRMLEvents ( vtkObject *caller,
                                                unsigned long event, 
                                                void *callData )
{
  if (this->ProcessingMRMLEvent != 0 )
    {
    return;
    }

  this->ProcessingMRMLEvent = event;

  vtkDebugMacro("ProcessMRMLEvents: processing event " << event);
  if (event == vtkMRMLScene::SceneClosingEvent )
    {
    this->RemoveHierarchyObservers(0);
    this->RemoveModelObservers(0);
    //this->RemoveCameraObservers();
    }
  else if (event == vtkMRMLScene::SceneCloseEvent )
    {
    this->SceneClosing = true;
    //this->CameraNodeWasCreated = 0;
    this->RemoveModelProps();
    this->RemoveHierarchyObservers(1);
    this->RemoveModelObservers(1);
    //this->RemoveCameraObservers();
    this->UpdateFromMRMLRequested = 1;
    this->RequestRender();
    }
  else if (event == vtkMRMLScene::SceneLoadStartEvent)
    {
    this->SetEnableRender(0);
    //this->GetRenderer()->SetRenderModeToDisabled();
    }
  else if (event == vtkMRMLScene::SceneLoadEndEvent)
    {
    this->SetEnableRender(1);
    //this->GetRenderer()->SetRenderModeToInteractive();
    ///WJPTEST
    ///Scene has loaded, nodes have been added.
    ///Since node added events occurred before the observers
    ///were put on camera nodes, so camera setup may not have
    //been completed when it got added.
    //this->UpdateCameraNode();
    this->RequestRender();
    } 
  else 
    {
    this->SceneClosing = false;
    }

  if (event == vtkMRMLScene::SceneRestoredEvent )
    {
    /*
    // Backward compatibility change: all scenes saved so far do have 
    // camera nodes with an empty or false active tag. Restoring a snapshot
    // will invalidate the current active camera. Try to catch that
    // by grabbing the first available camera. Sounds like a hack, but
    // too  much time was wasted on this thing.
    if (this->GetMRMLViewNode())
      {
      vtkMRMLCameraNode *camera_node = this->CameraNode;
      if (!camera_node)
        {
        camera_node = vtkMRMLCameraNode::SafeDownCast(
          this->GetMRMLScene()->GetNthNodeByClass(0, "vtkMRMLCameraNode"));
        camera_node->SetActiveTag(this->GetMRMLViewNode()->GetID());
        }
      else if (!camera_node->GetActiveTag())
        {
        camera_node->SetActiveTag(this->GetMRMLViewNode()->GetID());
        }
      }
      */
    this->UpdateFromMRMLRequested = 1;
    }

  if ( vtkMRMLScene::SafeDownCast(caller) == this->GetMRMLScene() &&
       (event == vtkMRMLScene::NodeAddedEvent || 
        event == vtkMRMLScene::NodeRemovedEvent) )
    {
    std::cout << "vtkMRMLModelDisplayableManager::ProcessMRMLEvents - ADDED/REMOVED" << std::endl;
    vtkMRMLNode *node = (vtkMRMLNode*) (callData);
    if (node != 0 && node->IsA("vtkMRMLDisplayableNode") )
      {
      vtkMRMLDisplayableNode *dnode = vtkMRMLDisplayableNode::SafeDownCast(node);
      if (event == vtkMRMLScene::NodeRemovedEvent)
        {
        this->RemoveDisplayable(dnode);
        }
      this->UpdateFromMRMLRequested = 1;
      this->RequestRender();
      }
    else if (node != 0 && node->IsA("vtkMRMLDisplayNode") )
      {
      this->UpdateFromMRMLRequested = 1;
      this->RequestRender();
      }
    else if (node != 0 && node->IsA("vtkMRMLModelHierarchyNode") )
      {
      this->UpdateModelHierarchies();
      this->UpdateFromMRMLRequested = 1;
      this->RequestRender();
      }
    else if (node != 0 && node->IsA("vtkMRMLClipModelsNode") )
      {
      if (event == vtkMRMLScene::NodeAddedEvent)
        {
        vtkSetAndObserveMRMLNodeMacro(this->ClipModelsNode, node);
        }
      else if (event == vtkMRMLScene::NodeRemovedEvent)
        {
        vtkSetMRMLNodeMacro(this->ClipModelsNode, 0);
        }
      this->UpdateFromMRMLRequested = 1;
      this->RequestRender();
      }
    /*
    else if (node != 0 && node->IsA("vtkMRMLCameraNode") )
      {
      if (event == vtkMRMLScene::NodeAddedEvent)
        {
        vtkEventBroker::GetInstance()->AddObservation ( 
          node, vtkMRMLCameraNode::ActiveTagModifiedEvent, this, this->GetMRMLCallbackCommand());
        }
      else if (event == vtkMRMLScene::NodeRemovedEvent)
        {
        vtkEventBroker::GetInstance()->RemoveObservations ( 
          node, vtkMRMLCameraNode::ActiveTagModifiedEvent, this, this->GetMRMLCallbackCommand() );
        this->UpdateCameraNode();
        }
      }*/
    }
  /*
  else if (vtkMRMLCameraNode::SafeDownCast(caller) != 0 &&
           (event == vtkCommand::ModifiedEvent ||
            event == vtkMRMLCameraNode::ActiveTagModifiedEvent))
    {
    vtkDebugMacro("ProcessingMRML: got a camera node modified event");
    this->UpdateCameraNode();
    this->RequestRender();
    }*/
  else if (vtkMRMLViewNode::SafeDownCast(caller) != 0 &&
           event == vtkCommand::ModifiedEvent)
    {
    vtkDebugMacro("ProcessingMRML: got a view node modified event");
    this->RequestRender();
    }
  else if (vtkMRMLDisplayableNode::SafeDownCast(caller) != 0)
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
        this->RequestRender();
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
  else if (vtkMRMLClipModelsNode::SafeDownCast(caller) != 0 &&
           event == vtkCommand::ModifiedEvent)
    {
    this->UpdateFromMRMLRequested = 1;
    this->RequestRender();
    }
  else if (vtkMRMLSliceNode::SafeDownCast(caller) != 0 &&
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
    if (vtkMRMLScene::SafeDownCast(caller) == this->GetMRMLScene()) { vtkDebugMacro("\ton the mrml scene"); }
    if (vtkMRMLNode::SafeDownCast(caller) != 0) { vtkDebugMacro("\tmrml node id = " << vtkMRMLNode::SafeDownCast(caller)->GetID()); }
    }
  
  this->ProcessingMRMLEvent = 0;
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayableManager::Create()
{
  if (this->IsCreated())
    {
    vtkErrorMacro(<< this->GetClassName() << " already created");
    return;
    }
    
  assert(this->GetRenderer());
  if (!this->GetRenderer())
    {
    return;
    }
  
  // Call the superclass to create the whole widget
  
  //double *c = slicerStyle->GetHighLightColor()

  // added to enable active/passive stereo mode.  If user started slicer with --stereo
  // command line option, then stereo might be enabled sometime during this session.  Therefore
  // we need to request the correct OpenGL visual BEFORE the window is opened.  Quadbuffered
  // stereo has to be requested before the first Render() call on the window.
  
  /***
  if (vtkSlicerApplication::GetInstance()->GetStereoEnabled())
    {
    vtkDebugMacro("Opening Stereo Capable Window");
    vtkRenderWindow* renWin = this->MainViewer->GetRenderWindow();
    renWin->SetStereoCapableWindow(1);
    renWin->SetStereoTypeToCrystalEyes();
    }
    ***/
  
  // don't use vtkKWRenderWidget's built-in ExposeEvent handler.  
  // It will call ProcessPendingEvents (update) even though it may already be inside
  // a call to update.  It also calls Render directly, which will pull the vtk pipeline chain.
  // Instead, use the RequestRender method to render when idle.
  //this->MainViewer->GetVTKWidget()->RemoveBinding("<Expose>");
  //this->MainViewer->GetVTKWidget()->AddBinding("<Expose>", this, "RequestRender");


  //this->GetRenderer()->SetRendererBackgroundColor(
  //  app->GetSlicerTheme()->GetSlicerColors()->ViewerBlue );

  // tell the render widget not to respond to the Render() method
  // - this class turns on rendering explicitly when it's own
  //   Render() method is called.  This avoids redundant renders
  //   when, for example, the annotation is changed.
  //  TODO: this is disabled for until there is a way to observe 
  //  for direct render requests to the renderer (so they can be placed
  //  in the event queue)
  //this->MainViewer->RenderStateOff();

  /*
  // make a Slicer viewer interactor style to process our events
  // look at the InteractorStyle to get our events
  vtkRenderWindowInteractor *rwi = this->GetInteractor();
  if (rwi)
    {
    vtkDisplayableManagerInteractorStyle *iStyle = vtkDisplayableManagerInteractorStyle::New();
    iStyle->SetViewerWidget(this);

    //if (this->ApplicationLogic != 0)
    //  {
    //  iStyle->SetApplicationLogic(this->ApplicationLogic);
    //  }
    //else
    //  {
    //  vtkDebugMacro("Not setting interactor style's application logic to 0.");
    //  }

    rwi->SetInteractorStyle (iStyle);
    iStyle->Delete();
    }
  */


  // Set the viewer's minimum dimension to be the modifiedState as that for
  // the three main Slice viewers.
  /*vtkCamera *camera = this->GetRenderer()->IsActiveCameraCreated() ?
    this->GetRenderer()->GetActiveCamera() : 0;
  if (camera)
    {
    camera->ParallelProjectionOff();
    }
*/
  //if ( this->GetApplication() != 0 )
  //  {
  //  app = (vtkSlicerApplication *)this->GetApplication();
  //  this->GetRenderer()->SetWidth ( app->GetDefaultGeometry()->GetSliceViewerMinDim() );
  //  this->GetRenderer()->SetHeight ( app->GetDefaultGeometry()->GetSliceViewerMinDim() );
  //  }

  // observe scene for add/remove nodes
  vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosingEvent);
  events->InsertNextValue(vtkMRMLScene::SceneLoadStartEvent);
  events->InsertNextValue(vtkMRMLScene::SceneLoadEndEvent);
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneRestoredEvent);
  this->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
  events->Delete();

  //this->CreateClipSlices();
  //this->CreateAxis();
  //this->GetRenderer()->ResetCamera();

  this->Created = 1;
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayableManager::UpdateFromMRML()
{
  //this->AddCameraObservers();
  //this->UpdateCameraNode();

  //this->AddAxisActors();

  this->UpdateClipSlicesFromMRML();

  this->RemoveModelProps();
  
  this->UpdateModelsFromMRML();

  //this->RequestRender();

  this->UpdateFromMRMLRequested = 0;
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayableManager::UpdateModelsFromMRML()
{
  std::cout << "vtkMRMLModelDisplayableManager::UpdateModelsFromMRML" << std::endl;
  vtkMRMLScene *scene = this->GetMRMLScene();
  vtkMRMLNode *node = 0;
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
      this->GetRenderer()->RemoveViewProp(iter->second);
      }
    //this->MainViewer->RemoveAllViewProps();
    this->RemoveModelObservers(1);
    this->RemoveHierarchyObservers(1);
    this->DisplayedActors.clear();
    this->DisplayedNodes.clear();
    this->DisplayedClipState.clear();
    this->DisplayedVisibility.clear();
    //this->AddAxisActors();
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
void vtkMRMLModelDisplayableManager::UpdateModifiedModel(vtkMRMLDisplayableNode *model)
{

  this->UpdateModelHierarchyDisplay(model);
  this->UpdateModel(model);
  this->SetModelDisplayProperty(model);
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayableManager::UpdateModelPolyData(vtkMRMLDisplayableNode *model)
{
  std::vector< vtkMRMLDisplayNode *> displayNodes = this->GetDisplayNode(model);
  vtkMRMLDisplayNode *hdnode = this->GetHierarchyDisplayNode(model);
  
  for (unsigned int i=0; i<displayNodes.size(); i++)
    {
    vtkMRMLDisplayNode *modelDisplayNode = displayNodes[i];
    vtkProp3D* prop = 0;
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
    if (poly == 0)
      {
      poly = model->GetPolyData();
      }
    if (poly == 0)
      {
      hasPolyData = false;
      }
     
    std::map<std::string, vtkProp3D *>::iterator ait;
    ait = this->DisplayedActors.find(modelDisplayNode->GetID());
    if (ait == this->DisplayedActors.end() )
      {
      if(poly)
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
        if (clipping == 0 || tnode == 0 || !tnode->IsLinear())
          {
          continue;
          }
        }
      }

    vtkActor *actor;
    vtkClipPolyData *clipper = 0;
    actor = vtkActor::SafeDownCast(prop);
    if(actor)
      {
      if (this->ClippingOn && modelDisplayNode != 0 && clipping)
        {
        clipper = this->CreateTransformedClipper(model);
        }

      vtkPolyDataMapper *mapper = vtkPolyDataMapper::New ();
     
      if (clipper)
        {
        clipper->SetInput(poly);
        clipper->Update();
        mapper->SetInput ( clipper->GetOutput() );
        }
      else
        {
        mapper->SetInput(poly);
        }

   
      actor->SetMapper(mapper);
      mapper->Delete();
      }

    if (hasPolyData && ait == this->DisplayedActors.end())
      {
      this->GetRenderer()->AddViewProp(prop);
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
void vtkMRMLModelDisplayableManager::UpdateModel(vtkMRMLDisplayableNode *model)
{
  this->UpdateModelPolyData(model);

  vtkEventBroker *broker = vtkEventBroker::GetInstance();
  std::vector< vtkObservation *> observations;
  // observe polydata
  observations = broker->GetObservations( model, vtkMRMLDisplayableNode::PolyDataModifiedEvent, this, this->GetMRMLCallbackCommand() );
  if ( observations.size() == 0 )
    {
    broker->AddObservation( model, vtkMRMLDisplayableNode::PolyDataModifiedEvent, this, this->GetMRMLCallbackCommand() );
    DisplayableNodes[model->GetID()] = model;
    }
  // observe display node
  observations = broker->GetObservations( model, vtkMRMLDisplayableNode::DisplayModifiedEvent, this, this->GetMRMLCallbackCommand() );
  if ( observations.size() == 0 )
    {
    broker->AddObservation( model, vtkMRMLDisplayableNode::DisplayModifiedEvent, this, this->GetMRMLCallbackCommand() );
    }

  observations = broker->GetObservations( model, vtkMRMLTransformableNode::TransformModifiedEvent, this, this->GetMRMLCallbackCommand() );
  if ( observations.size() == 0 )
    {
    broker->AddObservation( model, vtkMRMLTransformableNode::TransformModifiedEvent, this, this->GetMRMLCallbackCommand() );
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayableManager::CheckModelHierarchies()
{
  if (this->GetMRMLScene() == 0 || this->ModelHierarchyLogic == 0)
    {
    return;
    }
  this->ModelHierarchyLogic->SetMRMLScene(this->GetMRMLScene());
  int nnodes = this->ModelHierarchyLogic->GetNumberOfModelsInHierarchy();
  this->ModelHierarchiesPresent = nnodes > 0 ? true:false;
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayableManager::AddHierarchiyObservers()
{
  if (this->GetMRMLScene() == 0)
    {
    return;
    }
  vtkMRMLModelHierarchyNode *node;
  
  vtkEventBroker *broker = vtkEventBroker::GetInstance();
  std::vector<vtkMRMLNode *> hnodes;
  int nnodes = this->GetMRMLScene()->GetNodesByClass("vtkMRMLModelHierarchyNode", hnodes);

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
      broker->AddObservation( node, vtkCommand::ModifiedEvent, this, this->GetMRMLCallbackCommand() );
      this->RegisteredModelHierarchies[node->GetID()] = 0;
      }
    }
}

//----------------------------
void vtkMRMLModelDisplayableManager::UpdateModelHierarchyVisibility(vtkMRMLModelHierarchyNode* mhnode, int visibility )
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
void vtkMRMLModelDisplayableManager::UpdateModelHierarchyDisplay(vtkMRMLDisplayableNode *model)
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
      while (parent != 0);
      }
    }
}

//----------------------------
std::vector< vtkMRMLDisplayNode* > vtkMRMLModelDisplayableManager::GetDisplayNode(vtkMRMLDisplayableNode *model)
{
  std::vector< vtkMRMLDisplayNode* > dnodes;
  vtkMRMLDisplayNode* dnode = 0;

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
vtkMRMLDisplayNode*  vtkMRMLModelDisplayableManager::GetHierarchyDisplayNode(vtkMRMLDisplayableNode *model)
{
  vtkMRMLDisplayNode* dnode = 0;
  if (this->ModelHierarchiesPresent)
    {
    vtkMRMLModelHierarchyNode* mhnode = 0;
    vtkMRMLModelHierarchyNode* phnode = 0;
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
void vtkMRMLModelDisplayableManager::RequestRender()
{
  if (!this->GetMRMLScene()->GetIsClosed())
    {
    this->Render();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayableManager::Render()
{
  if (this->UpdateFromMRMLRequested)
    {
    this->UpdateFromMRML();
    //this->UpdateAxis();
    }

  std::cout << "vtkMRMLModelDisplayableManager::Render" << std::endl;

  // *** added code to check the RenderState and restore to whatever it
  // was before the specific request to render, instead of just setting
  // renderState to OFF.
  if (this->GetEnableRender())
    {
    std::cout << "vtkMRMLModelDisplayableManager::Render - 0" << std::endl;

    //int currentRenderState = this->MainViewer->GetRenderState();
    //this->MainViewer->RenderStateOn();
    if (this->GetRenderer()->IsActiveCameraCreated())
      {
      std::cout << "vtkMRMLModelDisplayableManager::Render - 1" << std::endl;
      this->IsRendering = 1;
      this->GetRenderer()->Render();
      this->IsRendering = 0;
      }
    vtkDebugMacro("vtkMRMLModelDisplayableManager::Render called render" << endl);
    //this->GetRenderer()->SetRenderState(currentRenderState);
    } 
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayableManager::RemoveModelProps()
{
  std::map<std::string, vtkProp3D *>::iterator iter;
  std::map<std::string, int>::iterator clipIter;
  std::vector<std::string> removedIDs;
  for(iter=this->DisplayedActors.begin(); iter != this->DisplayedActors.end(); iter++) 
    {
    vtkMRMLDisplayNode *modelDisplayNode = vtkMRMLDisplayNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(iter->first));
    if (modelDisplayNode == 0)
      {
      this->GetRenderer()->RemoveViewProp(iter->second);
      removedIDs.push_back(iter->first);
      }
    else
      {
      int clipModel = 0;
      if (modelDisplayNode != 0)
        {
        clipModel = modelDisplayNode->GetClipping();
        }
      clipIter = this->DisplayedClipState.find(iter->first);
      if (clipIter == this->DisplayedClipState.end())
        {
        vtkErrorMacro ("vtkMRMLModelDisplayableManager::RemoveModelProps() Unknown clip state\n");
        }
      else 
        {
        if ((clipIter->second && !this->ClippingOn) ||
            (this->ClippingOn && clipIter->second != clipModel))
          {
          this->GetRenderer()->RemoveViewProp(iter->second);
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
void vtkMRMLModelDisplayableManager::RemoveDisplayable(vtkMRMLDisplayableNode* model)
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
        this->GetRenderer()->RemoveViewProp(iter->second);
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

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayableManager::RemoveDispalyedID(std::string &id)
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

//---------------------------------------------------------------------------
int vtkMRMLModelDisplayableManager::GetDisplayedModelsVisibility(vtkMRMLDisplayNode *model)
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
void vtkMRMLModelDisplayableManager::RemoveMRMLObservers()
{
  this->RemoveModelObservers(1);
  this->RemoveHierarchyObservers(1);
  //this->RemoveCameraObservers();

  //this->SetAndObserveMRMLScene(0);
  //this->SetAndObserveMRMLViewNode(0);
  //this->SetAndObserveCameraNode(0);
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayableManager::RemoveModelObservers(int clearCache)
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
void vtkMRMLModelDisplayableManager::RemoveModelObservers(vtkMRMLDisplayableNode *model)
{
  vtkEventBroker *broker = vtkEventBroker::GetInstance();
  std::vector< vtkObservation *> observations;
  if (model != 0)
    {
    observations = broker->GetObservations( 
      model, vtkMRMLDisplayableNode::PolyDataModifiedEvent, this, this->GetMRMLCallbackCommand() );
    broker->RemoveObservations(observations);
    observations = broker->GetObservations( 
      model, vtkMRMLDisplayableNode::DisplayModifiedEvent, this, this->GetMRMLCallbackCommand() );
    broker->RemoveObservations(observations);
    observations = broker->GetObservations( 
      model, vtkMRMLTransformableNode::TransformModifiedEvent, this, this->GetMRMLCallbackCommand() );
    broker->RemoveObservations(observations);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayableManager::RemoveHierarchyObservers(int clearCache)
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
        node, vtkCommand::ModifiedEvent, this, this->GetMRMLCallbackCommand() );
      broker->RemoveObservations(observations);
      }
    }
  if (clearCache)
    {
    RegisteredModelHierarchies.clear();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayableManager::SetModelDisplayProperty(vtkMRMLDisplayableNode *model)
{
  vtkMRMLTransformNode* tnode = model->GetParentTransformNode();
  vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
  transformToWorld->Identity();
  if (tnode != 0 && tnode->IsLinear())
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
    if (dnode != 0)
      {
      vtkProp3D *prop = this->GetActorByID(dnode->GetID());
      if (prop == 0)
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
          if (mdnode->GetColorNode() != 0)
            {
            if (mdnode->GetColorNode()->GetLookupTable() != 0)
              {
              actor->GetMapper()->SetLookupTable(mdnode->GetColorNode()->GetLookupTable());
              }
            else if (mdnode->GetColorNode()->IsA("vtkMRMLProceduralColorNode") &&
                     vtkMRMLProceduralColorNode::SafeDownCast(mdnode->GetColorNode())->GetColorTransferFunction() != 0)
              {
              actor->GetMapper()->SetLookupTable((vtkScalarsToColors*)(vtkMRMLProceduralColorNode::SafeDownCast(mdnode->GetColorNode())->GetColorTransferFunction()));
              }
            }

          int cellScalarsActive = 0;
          if (mdnode->GetActiveScalarName() == 0)
            {
            // see if there are scalars on the poly data that are not set as
            // active on the display node
            vtkMRMLModelNode *mnode = vtkMRMLModelNode::SafeDownCast(model);
            if (mnode)
              {
              std::string pointScalarName = std::string(mnode->GetActivePointScalarName("scalars"));
              std::string cellScalarName = std::string(mnode->GetActiveCellScalarName("scalars"));
              vtkDebugMacro("Display node active scalar name was 0, but the node says active point scalar name = '" << pointScalarName.c_str() << "', cell = '" << cellScalarName.c_str() << "'");
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
          if (mdnode->GetActiveScalarName() != 0)
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
        if (dnode->GetTextureImageData() != 0)
          {
          if (actor->GetTexture() == 0)
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
          actor->SetTexture(0);
          }
        }
      else if (imageActor)
        {
        if (dnode->GetTextureImageData() != 0)
          {
          imageActor->SetInput(dnode->GetTextureImageData());
          }
        else
          {
          imageActor->SetInput(0);
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
vtkMRMLModelDisplayableManager::GetActorByID (const char *id)
{
  if ( !id )
    {
    return (0);
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
  return (0);
}

//---------------------------------------------------------------------------
// Description:
// return the ID for the given actor 
const char *
vtkMRMLModelDisplayableManager::GetIDByActor (vtkProp3D *actor)
{
  if ( !actor )
    {
    return (0);
    }

  std::map<std::string, vtkProp3D *>::iterator iter;
  for(iter=this->DisplayedActors.begin(); iter != this->DisplayedActors.end(); iter++) 
    {
    if ( iter->second && ( iter->second == actor ) )
      {
      return (iter->first.c_str());
      }
    }
  return (0);
}



//---------------------------------------------------------------------------
void vtkMRMLModelDisplayableManager::ResetPick()
{
  double zero[3] = {0.0, 0.0, 0.0};
  this->PickedNodeName = std::string("");
  this->SetPickedRAS(zero);
  this->SetPickedCellID(-1);
  this->SetPickedPointID(-1);
}

//---------------------------------------------------------------------------
int vtkMRMLModelDisplayableManager::Pick(int x, int y)
{
  double RASPoint[3] = {0.0, 0.0, 0.0};
  double pickPoint[3] = {0.0, 0.0, 0.0};

  // reset the pick vars
  this->ResetPick();
  
  vtkRenderer *ren;
  if (this->GetRenderer() != 0)
    {
    ren = this->GetRenderer();
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
    if (polyData != 0)
      {
      // now find the model this poly data belongs to
      std::map<std::string, vtkMRMLDisplayNode *>::iterator modelIter;
      for (modelIter = this->DisplayedNodes.begin();
           modelIter != this->DisplayedNodes.end();
           modelIter++)
        {
        vtkDebugMacro("Checking model " << modelIter->first.c_str() << "'s polydata");
        if (modelIter->second != 0)
          {
          if (modelIter->second->GetPolyData() == polyData)
            {
            vtkDebugMacro("Found matching poly data, pick was on model " << modelIter->first.c_str());
            this->PickedNodeName = modelIter->first;
            
            // figure out the closest vertex in the picked cell to the picked RAS
            // point. Only doing this on model nodes for now.
            vtkCell *cell = polyData->GetCell(this->GetPickedCellID());
            if (cell != 0)
              {
              int numPoints = cell->GetNumberOfPoints();
              int closestPointId = -1;
              double closestDistance = 0.0l;
              for (int p = 0; p < numPoints; p++)
                {
                int pointId = cell->GetPointId(p);
                double *pointCoords = polyData->GetPoint(pointId);
                if (pointCoords != 0)
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
  this->SetPickedRAS(RASPoint);

  return 1;
}     

//---------------------------------------------------------------------------
void vtkMRMLModelDisplayableManager::SetBoxWidgetInteractor ()
{
  if ( this->GetRenderer() &&  
       this->GetInteractor() )
    {
    vtkRenderWindowInteractor *interactor = this->GetInteractor();
    this->BoxWidget->SetInteractor(interactor);
    }
}

//---------------------------------------------------------------------------
vtkClipPolyData* vtkMRMLModelDisplayableManager::CreateTransformedClipper (vtkMRMLDisplayableNode *model)
{
  vtkClipPolyData *clipper = vtkClipPolyData::New();
  clipper->SetValue( 0.0);

  vtkMRMLTransformNode* tnode = model->GetParentTransformNode();
  vtkSmartPointer<vtkMatrix4x4> transformToWorld = vtkSmartPointer<vtkMatrix4x4>::New();
  transformToWorld->Identity();
  if (tnode != 0 && tnode->IsLinear())
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

    vtkMatrix4x4 *sliceMatrix = 0;
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
