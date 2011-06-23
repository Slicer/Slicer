/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/


#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTransform.h"

#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"

#include "vtkIGTToolSelectorLogic.h"

#include "vtkSmartPointer.h"
#include "vtkAppendPolyData.h"
#include "vtkAxes.h"
#include "vtkConeSource.h"
#include "vtkCubeSource.h"
#include "vtkCylinderSource.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkSphereSource.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkRotationalExtrusionFilter.h"
#include "vtkLinearExtrusionFilter.h"


vtkCxxRevisionMacro(vtkIGTToolSelectorLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkIGTToolSelectorLogic);

//---------------------------------------------------------------------------
vtkIGTToolSelectorLogic::vtkIGTToolSelectorLogic()
{

  // Timer Handling

  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkIGTToolSelectorLogic::DataCallback);

  this->ToolTransformNodeID = "";
  this->ToolPolyData = NULL;
  this->ToolTipPolyData = NULL;
  this->ToolProjPolyData = NULL;
  this->ToolType = -1;
  this->ShowAxesFlag = 0;
  this->CurrentShowAxesFlag = 0;
  this->ShowProjectionFlag = 0;
  this->CurrentShowProjectionFlag = 0;
  this->ShowToolTipFlag = 0;
  this->CurrentShowAxesFlag = 0;

  this->NewParameters = 1;

  this->projectionLength = 100.0;
  this->projectionDiameter = 1.0;
  this->toolTipDiameter = 1.5;

}


//---------------------------------------------------------------------------
vtkIGTToolSelectorLogic::~vtkIGTToolSelectorLogic()
{

  if (this->DataCallbackCommand)
    {
    this->DataCallbackCommand->Delete();
    }
  // vtkSmartPointers implemented.  These are not needed now.
//  if(this->ToolPolyData)
//  {
//    this->ToolPolyData->Delete();
//  }
//  if(this->ToolTipPolyData)
//  {
//    this->ToolTipPolyData->Delete();
//  }
//  if(this->ToolProjPolyData)
//  {
//    this->ToolProjPolyData->Delete();
//  }

}


//---------------------------------------------------------------------------
void vtkIGTToolSelectorLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->vtkObject::PrintSelf(os, indent);
  os << indent << "vtkIGTToolSelectorLogic:             " << this->GetClassName() << "\n";

}


//---------------------------------------------------------------------------
void vtkIGTToolSelectorLogic::DataCallback(vtkObject *caller, 
                                       unsigned long eid, void *clientData, void *callData)
{
  vtkIGTToolSelectorLogic *self = reinterpret_cast<vtkIGTToolSelectorLogic *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkIGTToolSelectorLogic DataCallback");
  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkIGTToolSelectorLogic::UpdateAll()
{

}

int vtkIGTToolSelectorLogic::SetToolTransformNode(const char* nodeID)
{
  vtkMRMLLinearTransformNode* node =
    vtkMRMLLinearTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(nodeID));

  if (node && strcmp(node->GetNodeTagName(), "LinearTransform") == 0)
    {
    this->ToolTransformNodeID = nodeID;
    return 1;
    }

  return 0;
}

void vtkIGTToolSelectorLogic::SetShowAxes(int sw)
{
  this->ShowAxesFlag = sw;
}

void vtkIGTToolSelectorLogic::SetShowProjection(int sw)
{
  this->ShowProjectionFlag = sw;
}

void vtkIGTToolSelectorLogic::SetShowToolTip(int sw)
{
  this->ShowToolTipFlag = sw;
}

void vtkIGTToolSelectorLogic::SetProjectionLength(double val)
{
  if(val > 0.0)
  {
    this->projectionLength = val;
    this->NewParameters = 1;
  }
}

void vtkIGTToolSelectorLogic::SetProjectionDiameter(double val)
{
  if(val > 0.0)
  {
    this->projectionDiameter = val;
    this->NewParameters = 1;
  }
}

void vtkIGTToolSelectorLogic::SetToolTipDiameter(double val)
{
  if(val > 0.0)
  {
    this->toolTipDiameter = val;
    this->NewParameters = 1;
  }
}

int vtkIGTToolSelectorLogic::EnableTool(int tool, int sw)
{
  vtkMRMLModelNode* mnode;

  if (sw == 1) // turn on
  {
    //********************************************
    // turn on the main tool.
    //********************************************
    this->EnableToolFlag = 1;  // set flag in case we need to know this.
    // get the MRML Node and turn on the tool.
    mnode = SetVisibilityOfTool("IGTTool", tool, 1);
    // find the linear transform node given by the name id.
    vtkMRMLLinearTransformNode* tnode =
        vtkMRMLLinearTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->ToolTransformNodeID));
    if (!tnode)
    {
      return 0;
    }
    mnode->SetAndObserveTransformNodeID(tnode->GetID());
    mnode->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);

    //********************************************
    // if projection needed, turn it on.
    //********************************************
    if(this->ShowProjectionFlag)
    {
      mnode = SetVisibilityOfTool("IGTProj", tool, 1);
      // find the linear transform node given by the name id.
      vtkMRMLLinearTransformNode* tnode =
          vtkMRMLLinearTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->ToolTransformNodeID));
      if (!tnode)
      {
        return 0;
      }
      mnode->SetAndObserveTransformNodeID(tnode->GetID());
      mnode->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
      this->CurrentShowProjectionFlag = 1;
    }
    else
    {
      SetVisibilityOfTool("IGTProj", tool, 0);
      this->CurrentShowProjectionFlag = 0;
    }

    //********************************************
    // if tool tip needed, turn it on.
    //********************************************
    if(this->ShowToolTipFlag)
    {
      mnode = SetVisibilityOfTool("IGTTip", tool, 1);
      // find the linear transform node given by the name id.
      vtkMRMLLinearTransformNode* tnode =
          vtkMRMLLinearTransformNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(this->ToolTransformNodeID));
      if (!tnode)
      {
        return 0;
      }
      mnode->SetAndObserveTransformNodeID(tnode->GetID());
      mnode->InvokeEvent(vtkMRMLTransformableNode::TransformModifiedEvent);
      this->CurrentShowTipFlag = 1;
    }
    else
    {
      SetVisibilityOfTool("IGTTip", tool, 0);
      this->CurrentShowTipFlag = 0;
    }
  }
  else // turn off
  {
    this->EnableToolFlag=0;
    SetVisibilityOfTool("IGTTool", tool, 0);
    SetVisibilityOfTool("IGTProj", tool, 0);
    SetVisibilityOfTool("IGTTip", tool, 0);
    this->CurrentShowTipFlag = 0;
  }
  return 1;
}

vtkMRMLModelNode* vtkIGTToolSelectorLogic::SetVisibilityOfTool(const char* nodeName, int tool, int v)
{
  vtkMRMLModelNode*   toolModel;
  vtkMRMLModelDisplayNode* toolModelDisp;
  vtkMRMLDisplayNode* toolDisp;

  // Check if any node with the specified name exists
  vtkMRMLScene*  scene = this->GetApplicationLogic()->GetMRMLScene();
  vtkCollection* collection = scene->GetNodesByName(nodeName);

  if (collection != NULL && collection->GetNumberOfItems() == 0)
    {
    // if a node doesn't exist
    toolModel = AddToolModel(nodeName);
    }
  else
    {
    toolModel = vtkMRMLModelNode::SafeDownCast(collection->GetItemAsObject(0));
    }

  if (this->ToolType != tool || this->CurrentShowAxesFlag != this->ShowAxesFlag || this->NewParameters)
  {
    this->BuildToolPolyData(tool);
  }

  if (toolModel)
    {
    // set the colour.
    double color[3];
    if (strcmp(nodeName, "IGTTool") == 0)
    {
      //blue
      color[0] = 0.0; color[1] = 0.0; color[2] = 1.0;
      toolModel->SetAndObservePolyData(this->ToolPolyData->GetOutput());
    }
    else if (strcmp(nodeName, "IGTTip") == 0)
    {
      // yellow.
      color[0] = 1.0; color[1] = 1.0; color[2] = 0.0;
      toolModel->SetAndObservePolyData(this->ToolTipPolyData->GetOutput());
    }
    else if (strcmp(nodeName, "IGTProj") == 0)
    {
      //green.
      color[0] = 0.0; color[1] = 1.0; color[2] = 0.0;
      toolModel->SetAndObservePolyData(this->ToolProjPolyData->GetOutput());
    }
    else
    {
      std::cerr << "Not expecting this tool name in the color selection. Set to cyan." << std::endl;
    }

    toolModelDisp = toolModel->GetModelDisplayNode();
    toolModelDisp->SetPolyData(toolModel->GetPolyData());
    toolModelDisp->SetColor(color);

    // set the visibility.
    toolDisp = toolModel->GetDisplayNode();
    toolDisp->SetVisibility(v);
    toolModel->Modified();
    this->GetApplicationLogic()->GetMRMLScene()->Modified();
    }

  return toolModel;
}

//---------------------------------------------------------------------------
vtkMRMLModelNode* vtkIGTToolSelectorLogic::AddToolModel(const char* nodeName)
{

  vtkSmartPointer<vtkMRMLModelNode> locatorModel;
  vtkSmartPointer<vtkMRMLModelDisplayNode> locatorDisp;

  locatorModel = vtkSmartPointer<vtkMRMLModelNode>::New();
  locatorDisp = vtkSmartPointer<vtkMRMLModelDisplayNode>::New();

  GetMRMLScene()->SaveStateForUndo();
  GetMRMLScene()->AddNode(locatorDisp);
  GetMRMLScene()->AddNode(locatorModel);

  locatorDisp->SetScene(this->GetMRMLScene());

  locatorModel->SetName(nodeName);
  locatorModel->SetScene(this->GetMRMLScene());
  locatorModel->SetAndObserveDisplayNodeID(locatorDisp->GetID());
  locatorModel->SetHideFromEditors(0);

  //locatorDisp->Delete();

  return locatorModel;
}

void vtkIGTToolSelectorLogic::BuildToolPolyData(int tool)
{
   //std::cout << "DEBUG: IGTToolSelectorLogic: BuildToolPolyData was called." << std::endl;
  // check to see if the current tool type is already been built.
  if(this->ToolType == tool && this->CurrentShowAxesFlag == this->ShowAxesFlag && !this->NewParameters)
  {
    //std::cout << "DEBUG: IGTToolSelectorLogic: Tried to build the same tool, leave tool as is." << std::endl;
    return;
  }
  // either allocate the memory or remove all the existing inputs.
  if(!this->ToolPolyData)
  {
    this->ToolPolyData = vtkSmartPointer<vtkAppendPolyData>::New();
  }
  else
  {
    this->ToolPolyData->RemoveAllInputs();
  }

  if(!this->ToolTipPolyData)
  {
    this->ToolTipPolyData = vtkSmartPointer<vtkAppendPolyData>::New();
  }
  else
  {
    this->ToolTipPolyData->RemoveAllInputs();
  }

  if(!this->ToolProjPolyData)
  {
    this->ToolProjPolyData = vtkSmartPointer<vtkAppendPolyData>::New();
  }
  else
  {
    this->ToolProjPolyData->RemoveAllInputs();
  }


  // let's build the tool.  The OpenIGTLink tool is the default.
 if (tool == NEEDLE)
  {   
   this->BuildNeedlePolyData();
  }
 else if (tool == POINTER)
 {
   this->BuildPointerPolyData();
 }
  else
 {
    this->BuildDefaultToolPolyData();

  } // end tool type if.

  if(this->ShowAxesFlag)
  {
    vtkSmartPointer<vtkAxes> axes = vtkSmartPointer<vtkAxes>::New();
    axes->SetOrigin(0.0, 0.0, 0.0);
    axes->SetScaleFactor(100.0);
    axes->Update();
    this->ToolPolyData->AddInput(axes->GetOutput());

    // debug code to figure out the orientation of the tool.
    //vtkSmartPointer<vtkSphereSource> testSphere = vtkSmartPointer<vtkSphereSource>::New();
    //testSphere->SetRadius(5.0);
    //testSphere->SetCenter(0, 0, 10);
    //testSphere->Update();
    //this->ToolPolyData->AddInput(testSphere->GetOutput());
  }
  //-------------
  // Build the tip location.
  //-------------
  // Sphere represents the locator tip
  vtkSmartPointer<vtkSphereSource> sphereTip = vtkSmartPointer<vtkSphereSource>::New();
  sphereTip->SetRadius(this->toolTipDiameter/2.0);
  sphereTip->SetCenter(0, 0, 0);
  sphereTip->Update();
  this->ToolTipPolyData->AddInput(sphereTip->GetOutput());

  this->ToolPolyData->Update();
  this->ToolProjPolyData->Update();
  this->ToolTipPolyData->Update();

  // keep track of the tools;
  this->ToolType = tool;
  this->CurrentShowAxesFlag = this->ShowAxesFlag;
  this->NewParameters = 0;
}

void vtkIGTToolSelectorLogic::BuildDefaultToolPolyData()
{
  // this is taken from the OpenIGTLinkIF Module.

  //********************************************
  //  Build the tool proper.
  //********************************************

  // Cylinder represents the locator stick
  vtkSmartPointer<vtkCylinderSource> cylinder = vtkSmartPointer<vtkCylinderSource>::New();
  cylinder->SetRadius(1.5);
  cylinder->SetHeight(100);
  cylinder->SetCenter(0, 0, 0);
  cylinder->Update();

  // Rotate cylinder
  vtkSmartPointer<vtkTransformPolyDataFilter> tfilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  vtkSmartPointer<vtkTransform> trans =   vtkTransform::New();
  trans->RotateX(90.0);
  trans->Translate(0.0, -50.0, 0.0);
  trans->Update();
  tfilter->SetInput(cylinder->GetOutput());
  tfilter->SetTransform(trans);
  tfilter->Update();

  // Sphere represents the locator tip
  vtkSmartPointer<vtkSphereSource> sphere = vtkSmartPointer<vtkSphereSource>::New();
  sphere->SetRadius(3.0);
  sphere->SetCenter(0, 0, 0);
  sphere->Update();

  // append the objects together to get the polydata object.
  this->ToolPolyData->AddInput(sphere->GetOutput());
  //apd->AddInput(cylinder->GetOutput());
  this->ToolPolyData->AddInput(tfilter->GetOutput());

  //********************************************
  //  Build the tool projection.
  //********************************************

  vtkSmartPointer<vtkCylinderSource> proj = vtkSmartPointer<vtkCylinderSource>::New();
  vtkSmartPointer<vtkTransform>projXfrm = vtkSmartPointer<vtkTransform>::New();
  vtkSmartPointer<vtkTransformPolyDataFilter> projXfrmPolyData = vtkSmartPointer<vtkTransformPolyDataFilter>::New();

  // create needle shaft.
  proj->SetHeight(this->projectionLength);
  proj->SetRadius(this->projectionDiameter/2.0);
  proj->SetCenter(0.0, 0.0, 0.0);
  proj->SetResolution(5);
  proj->CappingOn();
  //move to relative location.
  projXfrm->RotateX(-90.0);
  projXfrm->Translate(0.0, -1*((this->projectionDiameter/2.0)), 0.0);
  projXfrm->Update();
  // create the polydata filter.
  projXfrmPolyData->SetInput(proj->GetOutput());
  projXfrmPolyData->SetTransform(projXfrm);
  projXfrmPolyData->Update();
  // append to the poly data
  this->ToolProjPolyData->AddInput(projXfrmPolyData->GetOutput());

}

void vtkIGTToolSelectorLogic::BuildNeedlePolyData()
{
  // from Robarts Atamai Viewer Definitions.
  //double cannualDia = 1.27; //not implemented.
  double needleDia = 1.0;
  double taperAtTip = 3.0;
  //double cannulaLength = 200 - taperAtTip; //not implemented.
  double needleLength = 200 - taperAtTip;
  //double sphereDiameter = 1.5;
  //int showSphereAtTip = 0; //not implemented.


  //********************************************
  //  Build the tool proper.
  //********************************************

  //-------------
  // tip:
  //-------------
  vtkSmartPointer<vtkConeSource> tip =  vtkSmartPointer<vtkConeSource>::New();
  vtkSmartPointer<vtkTransform> tipXfrm =  vtkSmartPointer<vtkTransform>::New();
  vtkSmartPointer<vtkTransformPolyDataFilter> tipXfrmPolyData =  vtkSmartPointer<vtkTransformPolyDataFilter>::New();

  // create tip.
  tip->SetHeight(taperAtTip);
  tip->SetRadius(needleDia/2.0);
  tip->SetResolution(5);
  tip->Update();
  // tip drawn along x-axis rotate and translate.
  tipXfrm->RotateY(-90.0);
  tipXfrm->Translate(-1*taperAtTip/2.0, 0.0, 0.0);
  tipXfrm->Update();
  // create the polydata filter.
  tipXfrmPolyData->SetInput(tip->GetOutput());
  tipXfrmPolyData->SetTransform(tipXfrm);
  tipXfrmPolyData->Update();
  // append to the poly data
  this->ToolPolyData->AddInput(tipXfrmPolyData->GetOutput());

  //--------------
  // needle shaft:
  //--------------
  vtkSmartPointer<vtkCylinderSource> needle = vtkSmartPointer<vtkCylinderSource>::New();
  vtkSmartPointer<vtkTransform> needleXfrm = vtkSmartPointer<vtkTransform>::New();
  vtkSmartPointer<vtkTransformPolyDataFilter> needleXfrmPolyData = vtkSmartPointer<vtkTransformPolyDataFilter>::New();

  // create needle shaft.
  needle->SetHeight(needleLength);
  needle->SetRadius(needleDia/2.0);
  needle->SetCenter(0.0, 0.0, 0.0);
  needle->SetResolution(5);
  needle->CappingOn();
  //move to relative location.
  needleXfrm->RotateX(-90.0);
  needleXfrm->Translate(0.0, (needleLength/2.0)+taperAtTip, 0.0);
  needleXfrm->Update();
  // create the polydata filter.
  needleXfrmPolyData->SetInput(needle->GetOutput());
  needleXfrmPolyData->SetTransform(needleXfrm);
  needleXfrmPolyData->Update();
  // append to the poly data
  this->ToolPolyData->AddInput(needleXfrmPolyData->GetOutput());

  //--------------
  // hub:
  //--------------
  vtkSmartPointer<vtkCubeSource> hub = vtkSmartPointer<vtkCubeSource>::New();
  vtkSmartPointer<vtkTransform> hubXfrm = vtkSmartPointer<vtkTransform>::New();
  vtkSmartPointer<vtkTransformPolyDataFilter> hubXfrmPolyData = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  hub->SetXLength(10.0);
  hub->SetYLength(30.0);
  hub->SetZLength(10.0);
  hub->SetCenter(0.0, 0.0, 0.0);
  hub->Update();
  // move to relative location similar to needle.
  hubXfrm->RotateX(-90.0);
  hubXfrm->Translate(0.0, needleLength+taperAtTip, 0.0);
  hubXfrm->Update();
  // create the polydata filter.
  hubXfrmPolyData->SetInput(hub->GetOutput());
  hubXfrmPolyData->SetTransform(hubXfrm);
  hubXfrmPolyData->Update();
  this->ToolPolyData->AddInput(hubXfrmPolyData->GetOutput());

  //********************************************
  //  Build the tool projection.
  //********************************************

  vtkSmartPointer<vtkConeSource> tipProj = vtkSmartPointer<vtkConeSource>::New();
  vtkSmartPointer<vtkTransform> tipProjXfrm = vtkSmartPointer<vtkTransform>::New();
  vtkSmartPointer<vtkTransformPolyDataFilter> tipProjXfrmPolyData = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  // create tip.
  tipProj->SetHeight(taperAtTip);
  tipProj->SetRadius(this->projectionDiameter/2.0);
  tipProj->SetResolution(5);
  tipProj->Update();
  // tip drawn along x-axis rotate and translate.
  tipProjXfrm->RotateY(90.0);
  tipProjXfrm->Translate(-1*taperAtTip/2.0, 0.0, 0.0);
  tipProjXfrm->Update();
  // create the polydata filter.
  tipProjXfrmPolyData->SetInput(tipProj->GetOutput());
  tipProjXfrmPolyData->SetTransform(tipProjXfrm);
  tipProjXfrmPolyData->Update();
  // append to the poly data
  this->ToolProjPolyData->AddInput(tipProjXfrmPolyData->GetOutput());

  //--------------
  // needle shaft:
  //--------------
  vtkSmartPointer<vtkCylinderSource> needleProj = vtkSmartPointer<vtkCylinderSource>::New();
  vtkSmartPointer<vtkTransform> needleProjXfrm = vtkSmartPointer<vtkTransform>::New();
  vtkSmartPointer<vtkTransformPolyDataFilter> needleProjXfrmPolyData = vtkSmartPointer<vtkTransformPolyDataFilter>::New();

  // create needle shaft.
  needleProj->SetHeight(this->projectionLength);
  needleProj->SetRadius(this->projectionDiameter/2.0);
  needleProj->SetCenter(0.0, 0.0, 0.0);
  needleProj->SetResolution(5);
  needleProj->CappingOn();
  //move to relative location.
  needleProjXfrm->RotateX(-90.0);
  needleProjXfrm->Translate(0.0, -1*((this->projectionLength/2.0)+taperAtTip), 0.0);
  needleProjXfrm->Update();
  // create the polydata filter.
  needleProjXfrmPolyData->SetInput(needleProj->GetOutput());
  needleProjXfrmPolyData->SetTransform(needleProjXfrm);
  needleProjXfrmPolyData->Update();
  // append to the poly data
  this->ToolProjPolyData->AddInput(needleProjXfrmPolyData->GetOutput());
}

void vtkIGTToolSelectorLogic::BuildPointerPolyData()
{
  // from Robarts Atamai Viewer Definitions.

  //********************************************
  //  Build the tool proper.
  //********************************************

  // metal tip.
  vtkSmartPointer<vtkPoints> tipPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> tipLine = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPolyData> tipProfile = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkRotationalExtrusionFilter> tipExtrude = vtkSmartPointer<vtkRotationalExtrusionFilter>::New();

  tipPoints->InsertNextPoint(0.0, 0.0, 0.0);
  tipPoints->InsertNextPoint(0.5, 0.0, -0.6);
  tipPoints->InsertNextPoint(1.6, 0.0, -4.3);
  tipPoints->InsertNextPoint(1.6, 0.0, -82.8);
  tipPoints->InsertNextPoint(3.5, 0.0, -83.0);
  tipPoints->InsertNextPoint(4.45, 0.0,-86.0);
  tipPoints->InsertNextPoint(4.45, 0.0, -110.0);
  tipPoints->InsertNextPoint(0.0, 0.0, -110.0);

  tipLine->InsertNextCell(8);
  tipLine->InsertCellPoint(0);
  tipLine->InsertCellPoint(1);
  tipLine->InsertCellPoint(2);
  tipLine->InsertCellPoint(3);
  tipLine->InsertCellPoint(4);
  tipLine->InsertCellPoint(5);
  tipLine->InsertCellPoint(6);
  tipLine->InsertCellPoint(7);

  tipProfile->SetPoints(tipPoints);
  tipProfile->SetLines(tipLine);

  tipExtrude->SetInput(tipProfile);
  tipExtrude->SetResolution(24);
  tipExtrude->Update();

  // append to the poly data
  this->ToolPolyData->AddInput(tipExtrude->GetOutput());

  // handle.

  vtkSmartPointer<vtkCylinderSource> handle = vtkSmartPointer<vtkCylinderSource>::New();
  vtkSmartPointer<vtkTransform> handleXfrm = vtkSmartPointer<vtkTransform>::New();
  vtkSmartPointer<vtkTransformPolyDataFilter> handleXfrmPolyData = vtkSmartPointer<vtkTransformPolyDataFilter>::New();

  handle->SetHeight(110.0);
  handle->SetRadius(7.5);
  handle->SetCenter(0.0,0.0,0.0);
  handle->SetResolution(20);
  handle->CappingOn();

  handleXfrm->RotateX(-90);
  handleXfrm->Translate(0.0, 150.0, -12);

  handleXfrmPolyData->SetInput(handle->GetOutput());
  handleXfrmPolyData->SetTransform(handleXfrm);

  // append to the poly data
  this->ToolPolyData->AddInput(handleXfrmPolyData->GetOutput());

  // joining parts (i.e. rhombus).
  vtkSmartPointer<vtkPoints> rhombusPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> rhombusLine = vtkSmartPointer<vtkCellArray>::New();
  vtkSmartPointer<vtkPolyData> rhombusProfile = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkLinearExtrusionFilter> rhombusExtrude = vtkSmartPointer<vtkLinearExtrusionFilter>::New();
  vtkSmartPointer<vtkTransform> rhombusXfrm = vtkSmartPointer<vtkTransform>::New();
  vtkSmartPointer<vtkTransformPolyDataFilter> rhombusXfrmPolyData = vtkSmartPointer<vtkTransformPolyDataFilter>::New();

  rhombusPoints->InsertNextPoint(4.4, 0.0, 0.0);
  rhombusPoints->InsertNextPoint(4.4, 0.0, 22.0);
  rhombusPoints->InsertNextPoint(-4.5, 0.0, 22.0);
  rhombusPoints->InsertNextPoint(-4.5, 0.0, 0.0);
  rhombusPoints->InsertNextPoint(5.0, 0.0, 0.0);

  rhombusLine->InsertNextCell(5);
  rhombusLine->InsertCellPoint(0);
  rhombusLine->InsertCellPoint(1);
  rhombusLine->InsertCellPoint(2);
  rhombusLine->InsertCellPoint(3);
  rhombusLine->InsertCellPoint(0);

  rhombusProfile->SetPoints(rhombusPoints);
  rhombusProfile->SetLines(rhombusLine);

  rhombusExtrude->SetInput(rhombusProfile);
  rhombusExtrude->SetVector(0.0, 15.0, 8.0);
  rhombusExtrude->CappingOn();
  rhombusExtrude->Update();

  rhombusXfrm->RotateX(180.0);
  rhombusXfrm->Translate(0.0, 0.0, 88.0);

  rhombusXfrmPolyData->SetInput(rhombusExtrude->GetOutput());
  rhombusXfrmPolyData->SetTransform(rhombusXfrm);

  // append to the poly data
  this->ToolPolyData->AddInput(rhombusXfrmPolyData->GetOutput());

  //********************************************
  //  Build the tool projection.
  //********************************************

  vtkSmartPointer<vtkCylinderSource> proj = vtkSmartPointer<vtkCylinderSource>::New();
  vtkSmartPointer<vtkTransform> projXfrm = vtkSmartPointer<vtkTransform>::New();
  vtkSmartPointer<vtkTransformPolyDataFilter> projXfrmPolyData = vtkSmartPointer<vtkTransformPolyDataFilter>::New();

  // create needle shaft.
  proj->SetHeight(this->projectionLength);
  proj->SetRadius(this->projectionDiameter/2.0);
  proj->SetCenter(0.0, 0.0, 0.0);
  proj->SetResolution(5);
  proj->CappingOn();
  //move to relative location.
  projXfrm->RotateX(-90.0);
  projXfrm->Translate(0.0, -1*((this->projectionDiameter/2.0)), 0.0);
  projXfrm->Update();
  // create the polydata filter.
  projXfrmPolyData->SetInput(proj->GetOutput());
  projXfrmPolyData->SetTransform(projXfrm);
  projXfrmPolyData->Update();
  // append to the poly data
  this->ToolProjPolyData->AddInput(projXfrmPolyData->GetOutput());
}
