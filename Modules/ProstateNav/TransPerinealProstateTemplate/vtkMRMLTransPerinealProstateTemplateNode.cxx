/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLTransPerinealProstateTemplateNode.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include "vtkObjectFactory.h"

#include "vtkMRMLTransPerinealProstateTemplateNode.h"
#include "vtkMRMLScene.h"

#include "vtkOpenIGTLinkIFGUI.h"
#include "vtkOpenIGTLinkIFLogic.h"
#include "vtkIGTLToMRMLCoordinate.h"

#include "vtkMRMLIGTLConnectorNode.h"

#include "vtkZFrameRobotToImageRegistration2.h"

#include "vtkProstateNavTargetDescriptor.h"

#include "vtkTriangleFilter.h"

#include "vtkSphereSource.h"


//
// Models and Transforms
//    + ZFrameTransformNode (position of the center of Z-frame)
//      + ZFrameModel
//      + TemplateTransformNode (center of hole (0, 0) on the front surface, in the Z-frame coordinate)
//          + TemplateModel

// Hole (0, 0) position (surface)
#define TEMPLATE_HOLE_OFFSET_FROM_ZFRAME_X  35.0
#define TEMPLATE_HOLE_OFFSET_FROM_ZFRAME_Y  25.0
#define TEMPLATE_HOLE_OFFSET_FROM_ZFRAME_Z  30.0

// Template corner position
#define TEMPLATE_BLOCK_OFFSET_FROM_HOLE_X   15.0
#define TEMPLATE_BLOCK_OFFSET_FROM_HOLE_y   15.0
#define TEMPLATE_BLOCK_OFFSET_FROM_HOLE_Z   0.0

#define TEMPLATE_WIDTH   100.0  // dimension in x direction (mm)
#define TEMPLATE_HEIGHT  120.0  // dimension in y direction (mm)
#define TEMPLATE_DEPTH   25.0   // dimension in z direction (mm)
#define TEMPLATE_HOLE_RADIUS 1.0 // (only for visualization)

#define TEMPLATE_GRID_PITCH_X -5.0
#define TEMPLATE_GRID_PITCH_Y -5.0
#define TEMPLATE_NUMBER_OF_GRIDS_X 15  // must be odd number
#define TEMPLATE_NUMBER_OF_GRIDS_y 15


//-------------------------------------

static const unsigned int STATUS_SCANNER=1;

static const char STATUS_SCANNER_OFF[]="Scanner: OFF";
static const char STATUS_SCANNER_ON[]="Scanner: ON";

//------------------------------------------------------------------------------
vtkMRMLTransPerinealProstateTemplateNode* vtkMRMLTransPerinealProstateTemplateNode::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLTransPerinealProstateTemplateNode");
  if(ret)
    {
      return (vtkMRMLTransPerinealProstateTemplateNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTransPerinealProstateTemplateNode;
}

//----------------------------------------------------------------------------
vtkMRMLTransPerinealProstateTemplateNode* vtkMRMLTransPerinealProstateTemplateNode::CreateNodeInstance()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRMLTransPerinealProstateTemplateNode");
  if(ret)
    {
      return (vtkMRMLTransPerinealProstateTemplateNode*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRMLTransPerinealProstateTemplateNode;
}

//----------------------------------------------------------------------------
vtkMRMLTransPerinealProstateTemplateNode::vtkMRMLTransPerinealProstateTemplateNode()
{
  // Node references

  this->ScannerConnectorNodeID=NULL;
  this->ScannerConnectorNode=NULL;

  this->ZFrameModelNodeID=NULL;
  this->ZFrameModelNode=NULL;

  this->ZFrameTransformNodeID=NULL;
  this->ZFrameTransformNode=NULL;  

  this->TemplateModelNodeID=NULL;
  this->TemplateModelNode=NULL;

  //this->TemplateTransformNodeID=NULL;
  //this->TemplateTransformNode=NULL;  

  this->WorkspaceModelNodeID=NULL;
  this->WorkspaceModelNode=NULL;

  this->ActiveNeedleModelNodeID = NULL;
  this->ActiveNeedleModelNode   = NULL;

  this->ActiveNeedleTransformNodeID = NULL;
  this->ActiveNeedleTransformNode   = NULL;

  // Other

  this->ScreenMessage = NULL;

  this->ScannerWorkPhase     = -1;
  this->ScannerConnectedFlag = 0;

  StatusDescriptor s;
  s.indicator=StatusOff;
  s.text=STATUS_SCANNER_OFF;
  this->StatusDescriptors.push_back(s); // STATUS_SCANNER=1

  // Needle guiding template information
  this->TemplateGridPitch[0] = TEMPLATE_GRID_PITCH_X;
  this->TemplateGridPitch[1] = TEMPLATE_GRID_PITCH_Y;
  this->TemplateNumGrids[0]  = TEMPLATE_NUMBER_OF_GRIDS_X;
  this->TemplateNumGrids[1]  = TEMPLATE_NUMBER_OF_GRIDS_y;
  this->TemplateOffset[0]    = TEMPLATE_HOLE_OFFSET_FROM_ZFRAME_X;
  this->TemplateOffset[1]    = TEMPLATE_HOLE_OFFSET_FROM_ZFRAME_Y;
  this->TemplateOffset[2]    = TEMPLATE_HOLE_OFFSET_FROM_ZFRAME_Z;

}


//----------------------------------------------------------------------------
vtkMRMLTransPerinealProstateTemplateNode::~vtkMRMLTransPerinealProstateTemplateNode()
{

  if (this->ScannerConnectorNodeID) 
    {
    SetAndObserveScannerConnectorNodeID(NULL);
    }
  if (this->ZFrameModelNodeID) 
    {
    SetAndObserveZFrameModelNodeID(NULL);
    }
  if (this->ZFrameTransformNodeID) 
    {
    SetAndObserveZFrameTransformNodeID(NULL);
    }
  if (this->TemplateModelNodeID) 
    {
    SetAndObserveTemplateModelNodeID(NULL);
    }
  //if (this->TemplateTransformNodeID) 
  //  {
  //  SetAndObserveTemplateTransformNodeID(NULL);
  //  }
  if (this->ActiveNeedleModelNodeID)
    {
    SetAndObserveActiveNeedleModelNodeID(NULL);
    }
  if (this->ActiveNeedleTransformNodeID)
    {
    SetAndObserveActiveNeedleTransformNodeID(NULL);
    }

}


//----------------------------------------------------------------------------
void vtkMRMLTransPerinealProstateTemplateNode::WriteXML(ostream& of, int nIndent)
{
  // Start by having the superclass write its information
  Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->ScannerConnectorNodeID != NULL) 
    {
    of << indent << " ScannerConnectorNodeRef=\"" << this->ScannerConnectorNodeID << "\"";
    }
  if (this->ZFrameModelNodeID != NULL) 
    {
    of << indent << " ZFrameModelNodeRef=\"" << this->ZFrameModelNodeID << "\"";
    }
  if (this->ZFrameTransformNodeID != NULL) 
    {
    of << indent << " ZFrameTransformNodeRef=\"" << this->ZFrameTransformNodeID << "\"";
    }

  //switch (this->Type)
  //  {
  //  case TYPE_SERVER:
  //    of << " connectorType=\"" << "SERVER" << "\" ";
  //    break;
  //  case TYPE_CLIENT:
  //    of << " connectorType=\"" << "CLIENT" << "\" ";
  //    of << " serverHostname=\"" << this->ServerHostname << "\" ";
  //    break;
  //  default:
  //    of << " connectorType=\"" << "NOT_DEFINED" << "\" ";
  //    break;
  //  }
  //
  //of << " serverPort=\"" << this->ServerPort << "\" ";
  //of << " restrictDeviceName=\"" << this->RestrictDeviceName << "\" ";


}


//----------------------------------------------------------------------------
void vtkMRMLTransPerinealProstateTemplateNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);

  // Read all MRML node attributes from two arrays of names and values
  const char* attName;
  const char* attValue;

  while (*atts != NULL)
    {
    attName  = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "ScannerConnectorNodeRef")) 
      {
      this->SetAndObserveScannerConnectorNodeID(attValue);
      }
    if (!strcmp(attName, "ZFrameModelNodeRef")) 
      {
      this->SetAndObserveZFrameModelNodeID(attValue);
      }
    if (!strcmp(attName, "ZFrameTransformNodeRef")) 
      {
      this->SetAndObserveZFrameTransformNodeID(attValue);
      }

    }


  /*
  switch(type)
    {
    case TYPE_SERVER:
      this->SetTypeServer(port);
      this->SetRestrictDeviceName(restrictDeviceName);
      break;
    case TYPE_CLIENT:
      this->SetTypeClient(serverHostname, port);
      this->SetRestrictDeviceName(restrictDeviceName);
      break;
    default: // not defined
      // do nothing
      break;
    }
  */

}


//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
void vtkMRMLTransPerinealProstateTemplateNode::Copy(vtkMRMLNode *anode)
{  
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLTransPerinealProstateTemplateNode *node = vtkMRMLTransPerinealProstateTemplateNode::SafeDownCast(anode);
  if (node!=NULL)
  {
    this->SetScannerConnectorNodeID(node->ScannerConnectorNodeID);
    this->SetZFrameModelNodeID(node->ZFrameModelNodeID);
    this->SetZFrameTransformNodeID(node->ZFrameTransformNodeID);
    this->SetTemplateModelNodeID(node->TemplateModelNodeID);
    //this->SetTemplateTransformNodeID(node->TemplateTransformNodeID);
    this->SetActiveNeedleModelNodeID(node->ActiveNeedleModelNodeID);
    this->SetActiveNeedleTransformNodeID(node->ActiveNeedleTransformNodeID);
  }
  else
  {
    vtkErrorMacro("Invalid node");
  }

  this->StatusDescriptors.clear();
  for (unsigned int i=0; i<node->StatusDescriptors.size(); i++)
  {    
    this->StatusDescriptors.push_back(node->StatusDescriptors[i]);
  }

  this->EndModify(disabledModify);
}

//-----------------------------------------------------------
void vtkMRMLTransPerinealProstateTemplateNode::UpdateReferences()
{
  Superclass::UpdateReferences();

  if (this->ScannerConnectorNodeID != NULL && this->Scene->GetNodeByID(this->ScannerConnectorNodeID) == NULL)
    {
    this->SetAndObserveScannerConnectorNodeID(NULL);
    }
  if (this->ZFrameModelNodeID != NULL && this->Scene->GetNodeByID(this->ZFrameModelNodeID) == NULL)
    {
    this->SetAndObserveZFrameModelNodeID(NULL);
    }
  if (this->ZFrameTransformNodeID != NULL && this->Scene->GetNodeByID(this->ZFrameTransformNodeID) == NULL)
    {
    this->SetAndObserveZFrameTransformNodeID(NULL);
    }
  if (this->TemplateModelNodeID != NULL && this->Scene->GetNodeByID(this->TemplateModelNodeID) == NULL)
    {
    this->SetAndObserveTemplateModelNodeID(NULL);
    }
  //if (this->TemplateTransformNodeID != NULL && this->Scene->GetNodeByID(this->TemplateTransformNodeID) == NULL)
  //  {
  //  this->SetAndObserveTemplateTransformNodeID(NULL);
  //  }
  if (this->ActiveNeedleModelNodeID != NULL && this->Scene->GetNodeByID(this->ActiveNeedleModelNodeID) == NULL)
    {
    this->SetAndObserveActiveNeedleModelNodeID(NULL);
    }
  if (this->ActiveNeedleTransformNodeID != NULL && this->Scene->GetNodeByID(this->ActiveNeedleTransformNodeID) == NULL)
    {
    this->SetAndObserveActiveNeedleTransformNodeID(NULL);
    }


}

//----------------------------------------------------------------------------
void vtkMRMLTransPerinealProstateTemplateNode::UpdateReferenceID(const char *oldID, const char *newID)
{
  Superclass::UpdateReferenceID(oldID, newID);
  if (this->ScannerConnectorNodeID && !strcmp(oldID, this->ScannerConnectorNodeID))
    {
    this->SetAndObserveScannerConnectorNodeID(newID);
    }
  if (this->ZFrameModelNodeID && !strcmp(oldID, this->ZFrameModelNodeID))
    {
    this->SetAndObserveZFrameModelNodeID(newID);
    }
  if (this->ZFrameTransformNodeID && !strcmp(oldID, this->ZFrameTransformNodeID))
    {
    this->SetAndObserveZFrameTransformNodeID(newID);
    }
  if (this->TemplateModelNodeID && !strcmp(oldID, this->TemplateModelNodeID))
    {
    this->SetAndObserveTemplateModelNodeID(newID);
    }
  //if (this->TemplateTransformNodeID && !strcmp(oldID, this->TemplateTransformNodeID))
  //  {
  //  this->SetAndObserveTemplateTransformNodeID(newID);
  //  }

  if (this->ActiveNeedleModelNodeID && !strcmp(oldID, this->ActiveNeedleModelNodeID))
    {
    this->SetAndObserveActiveNeedleModelNodeID(newID);
    }
  if (this->ActiveNeedleTransformNodeID && !strcmp(oldID, this->ActiveNeedleTransformNodeID))
    {
    this->SetAndObserveActiveNeedleTransformNodeID(newID);
    }

}

//-----------------------------------------------------------
void vtkMRMLTransPerinealProstateTemplateNode::UpdateScene(vtkMRMLScene *scene)
{
   Superclass::UpdateScene(scene);
   this->SetAndObserveScannerConnectorNodeID(this->GetScannerConnectorNodeID());
   this->SetAndObserveZFrameModelNodeID(this->GetZFrameModelNodeID());
   this->SetAndObserveZFrameTransformNodeID(this->GetZFrameTransformNodeID());
   this->SetAndObserveTemplateModelNodeID(this->GetTemplateModelNodeID());
   //this->SetAndObserveTemplateTransformNodeID(this->GetTemplateTransformNodeID());
   this->SetAndObserveActiveNeedleModelNodeID(this->GetActiveNeedleModelNodeID());
   this->SetAndObserveActiveNeedleTransformNodeID(this->GetActiveNeedleTransformNodeID());
}

//-----------------------------------------------------------
void vtkMRMLTransPerinealProstateTemplateNode::ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);

  if (this->StatusDescriptors.size()<=STATUS_SCANNER)
  {
    vtkErrorMacro("StatusDescriptors does not contain scanner and robot status");
  }
  else
  {
    if (this->GetScannerConnectorNode() && this->GetScannerConnectorNode() == vtkMRMLIGTLConnectorNode::SafeDownCast(caller))
      {
      switch (event)
        {
        case vtkMRMLIGTLConnectorNode::ConnectedEvent:
          this->StatusDescriptors[STATUS_SCANNER].indicator=StatusOk;
          this->StatusDescriptors[STATUS_SCANNER].text=STATUS_SCANNER_ON;
          this->InvokeEvent(vtkMRMLRobotNode::ChangeStatusEvent);
          break;
        case vtkMRMLIGTLConnectorNode::DisconnectedEvent:
          this->StatusDescriptors[STATUS_SCANNER].indicator=StatusOff;
          this->StatusDescriptors[STATUS_SCANNER].text=STATUS_SCANNER_OFF;
          this->InvokeEvent(vtkMRMLRobotNode::ChangeStatusEvent);
          break;
        }
      }
  }
}


//----------------------------------------------------------------------------
void vtkMRMLTransPerinealProstateTemplateNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}


int vtkMRMLTransPerinealProstateTemplateNode::Init(vtkSlicerApplication* app)
{ 
  this->Superclass::Init(app);

  vtkOpenIGTLinkIFGUI* igtlGUI = vtkOpenIGTLinkIFGUI::SafeDownCast(app->GetModuleGUIByName("OpenIGTLink IF"));
  if (igtlGUI)
    {
    igtlGUI->Enter();    
    //igtlGUI->GetLogic()->RegisterMessageConverter(this->CoordinateConverter);
    //igtlGUI->GetLogic()->RegisterMessageConverter(this->CommandConverter);
    }

  // Z-Frame transform node
  if (GetZFrameTransformNode()==NULL)
  {
    vtkMRMLLinearTransformNode* ztnode = vtkMRMLLinearTransformNode::New();
    ztnode->SetName("ZFrameTransform");
    vtkMatrix4x4* ztransform = vtkMatrix4x4::New();
    ztransform->Identity();
    //transformNode->SetAndObserveImageData(transform);
    ztnode->ApplyTransform(ztransform);
    ztnode->SetScene(this->Scene);
    ztransform->Delete();
    this->Scene->AddNode(ztnode);
    SetAndObserveZFrameTransformNodeID(ztnode->GetID());
  }

  // ZFrame model
  // This part should be moved to Robot Display Node.
  if (GetZFrameModelNode()==NULL)
  {
    const char* nodeID = AddZFrameModel("ZFrame");
    vtkMRMLModelNode*  modelNode = vtkMRMLModelNode::SafeDownCast(this->Scene->GetNodeByID(nodeID));
    if (modelNode)
      {
      vtkMRMLDisplayNode* displayNode = modelNode->GetDisplayNode();
      displayNode->SetVisibility(0);
      modelNode->Modified();
      this->Scene->Modified();
      modelNode->SetAndObserveTransformNodeID(GetZFrameTransformNodeID());
      SetAndObserveZFrameModelNodeID(nodeID);
      }
  }

  // Workspace model
  // This part should be moved to Robot Display Node.
  if (GetWorkspaceModelNode()==NULL)
  {
    const char* nodeID = AddWorkspaceModel("Range of Motion");
    vtkMRMLModelNode*  modelNode = vtkMRMLModelNode::SafeDownCast(this->Scene->GetNodeByID(nodeID));
    if (modelNode)
      {
      vtkMRMLDisplayNode* displayNode = modelNode->GetDisplayNode();
      displayNode->SetVisibility(0);
      modelNode->Modified();
      this->Scene->Modified();
      modelNode->SetAndObserveTransformNodeID(GetZFrameTransformNodeID());
      SetAndObserveWorkspaceModelNodeID(nodeID);
      }
  }


  //// Template transform node
  //if (GetTemplateTransformNode()==NULL)
  //  {
  //  vtkMRMLLinearTransformNode* ttnode = vtkMRMLLinearTransformNode::New();
  //  ttnode->SetName("TemplateTransform");
  //  vtkMatrix4x4* ztransform = vtkMatrix4x4::New();
  //  ztransform->Identity();
  //  ztransform->SetElement(0, 3, this->TemplateOffset[0]);
  //  ztransform->SetElement(1, 3, this->TemplateOffset[1]);
  //  ztransform->SetElement(2, 3, this->TemplateOffset[2]);
  //
  //  //transformNode->SetAndObserveImageData(transform);
  //  ttnode->ApplyTransform(ztransform);
  //  ttnode->SetScene(this->Scene);
  //  ztransform->Delete();
  //  this->Scene->AddNode(ttnode);
  //  SetAndObserveTemplateTransformNodeID(ttnode->GetID());
  //
  //  if (GetZFrameTransformNodeID())
  //    {
  //    ttnode->SetAndObserveTransformNodeID(GetZFrameTransformNodeID());
  //    }
  //}

  // Template model
  // This part should be moved to Robot Display Node.
  if (GetTemplateModelNode()==NULL)
  {
    const char* nodeID = AddTemplateModel("Template");
    vtkMRMLModelNode*  modelNode = vtkMRMLModelNode::SafeDownCast(this->Scene->GetNodeByID(nodeID));
    if (modelNode)
      {
      vtkMRMLDisplayNode* displayNode = modelNode->GetDisplayNode();
      displayNode->SetVisibility(0);
      modelNode->Modified();
      this->Scene->Modified();
      //modelNode->SetAndObserveTransformNodeID(GetTemplateTransformNodeID());
      modelNode->SetAndObserveTransformNodeID(GetZFrameTransformNodeID());
      SetAndObserveTemplateModelNodeID(nodeID);
      }
  }

  // Active needle transform node
  if (GetActiveNeedleTransformNode()==NULL)
  {
    vtkMRMLLinearTransformNode* atnode = vtkMRMLLinearTransformNode::New();
    atnode->SetName("ActiveNeedleTransform");
    vtkMatrix4x4* ztransform = vtkMatrix4x4::New();
    ztransform->Identity();
    //transformNode->SetAndObserveImageData(transform);
    atnode->ApplyTransform(ztransform);
    atnode->SetScene(this->Scene);
    ztransform->Delete();
    this->Scene->AddNode(atnode);
    SetAndObserveActiveNeedleTransformNodeID(atnode->GetID());
  }

  // Active Needle model
  // This part should be moved to Robot Display Node.
  if (GetActiveNeedleModelNode()==NULL)
  {
  const char* nodeID = AddNeedleModel("ActiveNeedle", 200.0, 4.0);
    vtkMRMLModelNode*  modelNode = vtkMRMLModelNode::SafeDownCast(this->Scene->GetNodeByID(nodeID));
    if (modelNode)
      {
      vtkMRMLDisplayNode* displayNode = modelNode->GetDisplayNode();
      displayNode->SetVisibility(0);
      modelNode->Modified();
      this->Scene->Modified();
      modelNode->SetAndObserveTransformNodeID(GetActiveNeedleTransformNodeID());
      SetAndObserveActiveNeedleModelNodeID(nodeID);
      }
  }

  return 1;
}


int vtkMRMLTransPerinealProstateTemplateNode::OnTimer()
{
    return 1;
}


//---------------------------------------------------------------------------
int vtkMRMLTransPerinealProstateTemplateNode::SendZFrame()
{

  std::cerr << "int vtkProstateNavLogic::SendZFrame(): " << this->GetZFrameTransformNodeID() << std::endl;

  return 1;
  
}


//----------------------------------------------------------------------------
const char* vtkMRMLTransPerinealProstateTemplateNode::AddWorkspaceModel(const char* nodeName)
{
  vtkMRMLModelNode           *workspaceModel;
  vtkMRMLModelDisplayNode    *workspaceDisp;
  
  workspaceModel = vtkMRMLModelNode::New();
  workspaceDisp  = vtkMRMLModelDisplayNode::New();

  this->Scene->SaveStateForUndo();
  this->Scene->AddNode(workspaceDisp);
  this->Scene->AddNode(workspaceModel);

  workspaceDisp->SetScene(this->Scene);
  workspaceModel->SetName(nodeName);
  workspaceModel->SetScene(this->Scene);
  workspaceModel->SetAndObserveDisplayNodeID(workspaceDisp->GetID());
  workspaceModel->SetHideFromEditors(0);

  // construct Z-frame model
  
  // Parameters
  // offset from z-frame -- this will be a class variable to let users configure it in the future.
  const double offsetFromZFrame[] = {0, 22.76, 150.0};
  const double length = 200.0;

  //----- cylinder 1 (R-A) -----
  vtkCylinderSource *cylinder1 = vtkCylinderSource::New();
  cylinder1->SetResolution(24);
  cylinder1->SetRadius(25.0);
  cylinder1->SetHeight(length);
  cylinder1->SetCenter(0, 0, 0);
  cylinder1->Update();
  
  vtkTransform* trans1 =   vtkTransform::New();
  trans1->Translate(offsetFromZFrame);
  trans1->RotateX(90.0);
  trans1->Update();

  vtkTransformPolyDataFilter *tfilter1 = vtkTransformPolyDataFilter::New();
  tfilter1->SetInput(cylinder1->GetOutput());
  tfilter1->SetTransform(trans1);
  tfilter1->Update();


  vtkAppendPolyData *apd = vtkAppendPolyData::New();
  apd->AddInput(tfilter1->GetOutput());
  //apd->AddInput(tfilter2->GetOutput());
  apd->Update();

  vtkSmartPointer<vtkTriangleFilter> cleaner=vtkSmartPointer<vtkTriangleFilter>::New();
  cleaner->SetInputConnection(apd->GetOutputPort());
  
  workspaceModel->SetAndObservePolyData(cleaner->GetOutput());

  double color[3];
  color[0] = 0.5;
  color[1] = 0.5;
  color[2] = 1.0;
  workspaceDisp->SetPolyData(workspaceModel->GetPolyData());
  workspaceDisp->SetColor(color);
  workspaceDisp->SetOpacity(0.5);
  
  trans1->Delete();
  tfilter1->Delete();
  cylinder1->Delete();
  apd->Delete();

  const char* modelID = workspaceModel->GetID();

  workspaceDisp->Delete();
  workspaceModel->Delete();

  return modelID;

}


//----------------------------------------------------------------------------
const char* vtkMRMLTransPerinealProstateTemplateNode::AddZFrameModel(const char* nodeName)
{
  vtkMRMLModelNode           *zframeModel;
  vtkMRMLModelDisplayNode    *zframeDisp;

  zframeModel = vtkMRMLModelNode::New();
  zframeDisp = vtkMRMLModelDisplayNode::New();

  this->Scene->SaveStateForUndo();
  this->Scene->AddNode(zframeDisp);
  this->Scene->AddNode(zframeModel);  

  zframeDisp->SetScene(this->Scene);
  zframeModel->SetName(nodeName);
  zframeModel->SetScene(this->Scene);
  zframeModel->SetAndObserveDisplayNodeID(zframeDisp->GetID());
  zframeModel->SetHideFromEditors(0);

  // construct Z-frame model
  const double length = 60; // mm

  //----- cylinder 1 (R-A) -----
  vtkCylinderSource *cylinder1 = vtkCylinderSource::New();
  cylinder1->SetRadius(1.5);
  cylinder1->SetHeight(length);
  cylinder1->SetCenter(0, 0, 0);
  cylinder1->Update();
  
  vtkTransformPolyDataFilter *tfilter1 = vtkTransformPolyDataFilter::New();
  vtkTransform* trans1 =   vtkTransform::New();
  trans1->Translate(length/2.0, length/2.0, 0.0);
  trans1->RotateX(90.0);
  trans1->Update();
  tfilter1->SetInput(cylinder1->GetOutput());
  tfilter1->SetTransform(trans1);
  tfilter1->Update();


  //----- cylinder 2 (R-center) -----
  vtkCylinderSource *cylinder2 = vtkCylinderSource::New();
  cylinder2->SetRadius(1.5);
  cylinder2->SetHeight(length*1.4142135);
  cylinder2->SetCenter(0, 0, 0);
  cylinder2->Update();

  vtkTransformPolyDataFilter *tfilter2 = vtkTransformPolyDataFilter::New();
  vtkTransform* trans2 =   vtkTransform::New();
  trans2->Translate(length/2.0, 0.0, 0.0);
  trans2->RotateX(90.0);
  //trans2->RotateX(-45.0);
  trans2->RotateX(45.0);
  trans2->Update();
  tfilter2->SetInput(cylinder2->GetOutput());
  tfilter2->SetTransform(trans2);
  tfilter2->Update();


  //----- cylinder 3 (R-P) -----
  vtkCylinderSource *cylinder3 = vtkCylinderSource::New();
  cylinder3->SetRadius(1.5);
  cylinder3->SetHeight(length);
  cylinder3->SetCenter(0, 0, 0);
  cylinder3->Update();

  vtkTransformPolyDataFilter *tfilter3 = vtkTransformPolyDataFilter::New();
  vtkTransform* trans3 =   vtkTransform::New();
  trans3->Translate(length/2.0, -length/2.0, 0.0);
  trans3->RotateX(90.0);
  trans3->Update();
  tfilter3->SetInput(cylinder3->GetOutput());
  tfilter3->SetTransform(trans3);
  tfilter3->Update();


  //----- cylinder 4 (center-P) -----  
  vtkCylinderSource *cylinder4 = vtkCylinderSource::New();
  cylinder4->SetRadius(1.5);
  cylinder4->SetHeight(length*1.4142135);
  cylinder4->SetCenter(0, 0, 0);
  cylinder4->Update();

  vtkTransformPolyDataFilter *tfilter4 = vtkTransformPolyDataFilter::New();
  vtkTransform* trans4 =   vtkTransform::New();
  trans4->Translate(0.0, length/2.0, 0.0);
  trans4->RotateX(90.0);
  //trans4->RotateZ(-45.0);
  trans4->RotateZ(-45.0);
  trans4->Update();
  tfilter4->SetInput(cylinder4->GetOutput());
  tfilter4->SetTransform(trans4);
  tfilter4->Update();


  //----- cylinder 5 (L-P) -----  
  vtkCylinderSource *cylinder5 = vtkCylinderSource::New();
  cylinder5->SetRadius(1.5);
  cylinder5->SetHeight(length);
  cylinder5->SetCenter(0, 0, 0);
  cylinder5->Update();

  vtkTransformPolyDataFilter *tfilter5 = vtkTransformPolyDataFilter::New();
  vtkTransform* trans5 =   vtkTransform::New();
  trans5->Translate(-length/2.0, -length/2.0, 0.0);
  trans5->RotateX(90.0);
  trans5->Update();
  tfilter5->SetInput(cylinder5->GetOutput());
  tfilter5->SetTransform(trans5);
  tfilter5->Update();


  //----- cylinder 6 (L-center) -----  
  vtkCylinderSource *cylinder6 = vtkCylinderSource::New();
  cylinder6->SetRadius(1.5);
  cylinder6->SetHeight(length*1.4142135);
  cylinder6->SetCenter(0, 0, 0);
  cylinder6->Update();

  vtkTransformPolyDataFilter *tfilter6 = vtkTransformPolyDataFilter::New();
  vtkTransform* trans6 =   vtkTransform::New();
  trans6->Translate(-length/2.0, 0.0, 0.0);
  trans6->RotateX(90.0);
  //trans6->RotateX(45.0);
  trans6->RotateX(-45.0);
  trans6->Update();
  tfilter6->SetInput(cylinder6->GetOutput());
  tfilter6->SetTransform(trans6);
  tfilter6->Update();


  //----- cylinder 7 (L-A) -----  
  vtkCylinderSource *cylinder7 = vtkCylinderSource::New();
  cylinder7->SetRadius(1.5);
  cylinder7->SetHeight(length);
  cylinder7->SetCenter(0, 0, 0);
  cylinder7->Update();

  vtkTransformPolyDataFilter *tfilter7 = vtkTransformPolyDataFilter::New();
  vtkTransform* trans7 =   vtkTransform::New();
  trans7->Translate(-length/2.0, length/2.0, 0.0);
  trans7->RotateX(90.0);
  trans7->Update();
  tfilter7->SetInput(cylinder7->GetOutput());
  tfilter7->SetTransform(trans7);
  tfilter7->Update();

  vtkAppendPolyData *apd = vtkAppendPolyData::New();
  apd->AddInput(tfilter1->GetOutput());
  apd->AddInput(tfilter2->GetOutput());
  apd->AddInput(tfilter3->GetOutput());
  apd->AddInput(tfilter4->GetOutput());
  apd->AddInput(tfilter5->GetOutput());
  apd->AddInput(tfilter6->GetOutput());
  apd->AddInput(tfilter7->GetOutput());
  apd->Update();
  
  zframeModel->SetAndObservePolyData(apd->GetOutput());

  double color[3];
  color[0] = 1.0;
  color[1] = 1.0;
  color[2] = 0.0;
  zframeDisp->SetPolyData(zframeModel->GetPolyData());
  zframeDisp->SetColor(color);
  
  trans1->Delete();
  trans2->Delete();
  trans3->Delete();
  trans4->Delete();
  trans5->Delete();
  trans6->Delete();
  trans7->Delete();
  tfilter1->Delete();
  tfilter2->Delete();
  tfilter3->Delete();
  tfilter4->Delete();
  tfilter5->Delete();
  tfilter6->Delete();
  tfilter7->Delete();
  cylinder1->Delete();
  cylinder2->Delete();
  cylinder3->Delete();
  cylinder4->Delete();
  cylinder5->Delete();
  cylinder6->Delete();
  cylinder7->Delete();

  apd->Delete();

  const char* modelID = zframeModel->GetID();

  zframeDisp->Delete();
  zframeModel->Delete();  

  return modelID;
}


//----------------------------------------------------------------------------
const char* vtkMRMLTransPerinealProstateTemplateNode::AddTemplateModel(const char* nodeName)
{

  vtkMRMLModelNode           *model;
  vtkMRMLModelDisplayNode    *disp;
  
  model = vtkMRMLModelNode::New();
  disp  = vtkMRMLModelDisplayNode::New();

  this->Scene->SaveStateForUndo();
  this->Scene->AddNode(disp);
  this->Scene->AddNode(model);

  disp->SetScene(this->Scene);
  model->SetName(nodeName);
  model->SetScene(this->Scene);
  model->SetAndObserveDisplayNodeID(disp->GetID());
  model->SetHideFromEditors(0);

  vtkAppendPolyData *apd = vtkAppendPolyData::New();

  for (int i = 0; i < this->TemplateNumGrids[0]; i ++)
    {
    for (int j = 0; j < this->TemplateNumGrids[1]; j ++)
      {
      double offset[3];

      offset[0] = this->TemplateOffset[0] + i * this->TemplateGridPitch[0];
      offset[1] = this->TemplateOffset[1] + j * this->TemplateGridPitch[1];
      offset[2] = this->TemplateOffset[2] + TEMPLATE_DEPTH/2;

      vtkCylinderSource *cylinder = vtkCylinderSource::New();
      cylinder->SetResolution(24);
      cylinder->SetRadius(1.0);
      cylinder->SetHeight(TEMPLATE_DEPTH);
      cylinder->SetCenter(0, 0, 0);
      cylinder->Update();
      
      vtkTransform* trans =   vtkTransform::New();
      trans->Translate(offset);
      trans->RotateX(90.0);
      trans->Update();
      
      vtkTransformPolyDataFilter *tfilter = vtkTransformPolyDataFilter::New();
      tfilter->SetInput(cylinder->GetOutput());
      tfilter->SetTransform(trans);
      tfilter->Update();

      apd->AddInput(tfilter->GetOutput());
      apd->Update();

      cylinder->Delete();
      trans->Delete();
      tfilter->Delete();
      }
    }
  
  


  model->SetAndObservePolyData(apd->GetOutput());

  double color[3];
  color[0] = 0.2;
  color[1] = 0.2;
  color[2] = 1.0;

  disp->SetPolyData(model->GetPolyData());
  disp->SetColor(color);
  disp->SetOpacity(0.5);
  
  apd->Delete();

  const char* modelID = model->GetID();

  disp->Delete();
  model->Delete();

  return modelID;

}


//----------------------------------------------------------------------------
const char* vtkMRMLTransPerinealProstateTemplateNode::AddNeedleModel(const char* nodeName, double length, double diameter)
{

  vtkMRMLModelNode           *needleModel;
  vtkMRMLModelDisplayNode    *needleDisp;
  
  needleModel = vtkMRMLModelNode::New();
  needleDisp  = vtkMRMLModelDisplayNode::New();

  this->Scene->SaveStateForUndo();
  this->Scene->AddNode(needleDisp);
  this->Scene->AddNode(needleModel);

  needleDisp->SetScene(this->Scene);
  needleModel->SetName(nodeName);
  needleModel->SetScene(this->Scene);
  needleModel->SetAndObserveDisplayNodeID(needleDisp->GetID());
  needleModel->SetHideFromEditors(0);

  // Cylinder represents the locator stick
  vtkCylinderSource *cylinder = vtkCylinderSource::New();
  cylinder->SetRadius(1.5);
  cylinder->SetHeight(100);
  cylinder->SetCenter(0, 0, 0);
  cylinder->Update();

  // Rotate cylinder
  vtkTransformPolyDataFilter *tfilter = vtkTransformPolyDataFilter::New();
  vtkTransform* trans =   vtkTransform::New();
  trans->RotateX(90.0);
  trans->Translate(0.0, -50.0, 0.0);
  trans->Update();
  tfilter->SetInput(cylinder->GetOutput());
  tfilter->SetTransform(trans);
  tfilter->Update();
  
  // Sphere represents the locator tip 
  vtkSphereSource *sphere = vtkSphereSource::New();
  sphere->SetRadius(3.0);
  sphere->SetCenter(0, 0, 0);
  sphere->Update();
  
  vtkAppendPolyData *apd = vtkAppendPolyData::New();
  apd->AddInput(sphere->GetOutput());
  //apd->AddInput(cylinder->GetOutput());
  apd->AddInput(tfilter->GetOutput());
  apd->Update();
  
  vtkSmartPointer<vtkTriangleFilter> cleaner=vtkSmartPointer<vtkTriangleFilter>::New();
  cleaner->SetInputConnection(apd->GetOutputPort());
  
  needleModel->SetAndObservePolyData(cleaner->GetOutput());

  needleDisp->SetPolyData(needleModel->GetPolyData());

  double color[3];
  color[0] = 0.5;
  color[1] = 0.5;
  color[2] = 0.5;
  needleDisp->SetPolyData(needleModel->GetPolyData());
  needleDisp->SetColor(color);

  needleDisp->SetColor(color);
  needleDisp->SetOpacity(0.5);
  
  trans->Delete();
  tfilter->Delete();
  cylinder->Delete();
  sphere->Delete();
  apd->Delete();

  const char* modelID = needleModel->GetID();

  needleDisp->Delete();
  needleModel->Delete();

  return modelID;

}



//----------------------------------------------------------------------------
int vtkMRMLTransPerinealProstateTemplateNode::PerformRegistration(vtkMRMLScalarVolumeNode* volumeNode)
{
  vtkZFrameRobotToImageRegistration2* registration = vtkZFrameRobotToImageRegistration2::New();
  registration->SetFiducialVolume(volumeNode);

  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(this->Scene->GetNodeByID(this->GetZFrameTransformNodeID()));
  if (transformNode != NULL)
  {
    registration->SetRobotToImageTransform(transformNode);
    registration->DoRegistration();
  }

  return 1;
}


//----------------------------------------------------------------------------
int vtkMRMLTransPerinealProstateTemplateNode::PerformRegistration(vtkMRMLScalarVolumeNode* volumeNode, int param1, int param2)
{
  vtkZFrameRobotToImageRegistration2* registration = vtkZFrameRobotToImageRegistration2::New();
  registration->SetFiducialVolume(volumeNode);

  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(this->Scene->GetNodeByID(this->GetZFrameTransformNodeID()));
  if (transformNode != NULL)
  {
    registration->SetRobotToImageTransform(transformNode);
    registration->SetSliceRange(param1, param2);
    registration->DoRegistration();
  }

  return 1;
}


//----------------------------------------------------------------------------
int vtkMRMLTransPerinealProstateTemplateNode::MoveTo(const char *transformNodeId)
{
  vtkMRMLLinearTransformNode* transformNode = vtkMRMLLinearTransformNode::SafeDownCast(this->Scene->GetNodeByID(transformNodeId));
  vtkMatrix4x4* transform = transformNode->GetMatrixTransformToParent();
  double targetX = transform->GetElement(0, 3);
  double targetY = transform->GetElement(1, 3);
  double targetZ = transform->GetElement(2, 3);
  int i;
  int j;
  double depth;
  double errorX;
  double errorY;
  double errorZ;

  FindHole(targetX, targetY, targetZ, i, j, depth, errorX, errorY, errorZ);
  vtkMatrix4x4* matrix = vtkMatrix4x4::New();
  GetHoleTransform(i, j, matrix);
  
  vtkMRMLLinearTransformNode* needleTransformNode = 
    vtkMRMLLinearTransformNode::SafeDownCast(this->Scene->GetNodeByID(this->GetActiveNeedleTransformNodeID()));
  if (needleTransformNode != NULL)
    {
    vtkMatrix4x4* needleTransform = needleTransformNode->GetMatrixTransformToParent();
    double needleOffset[4];
    double needleTip[4];
    needleOffset[0] = 0.0;
    needleOffset[1] = 0.0;
    needleOffset[2] = depth;
    needleOffset[3] = 1.0;
    matrix->MultiplyPoint(needleOffset, needleTip);
    needleTransform->DeepCopy(matrix);
    needleTransform->SetElement(0, 3, needleTip[0]);
    needleTransform->SetElement(1, 3, needleTip[1]);
    needleTransform->SetElement(2, 3, needleTip[2]);
    needleTransformNode->Modified();

    std::ostringstream ss;
    //ss << "Grid:   (" << i << ", " << (char) ('A' + j) << ")" << std::endl;
    ss << "Grid:   (" << (char) ('A' + j) << ", " << (i - TEMPLATE_NUMBER_OF_GRIDS_X/2) << ")" << std::endl;
    ss << "Depth:  " << depth << " mm" << std::endl;
    ss << "Target: R=" << targetX << ", A=" << targetY << ", S=" << targetZ << std::endl;
    ss << "Error:  R=" << errorX  << ", A=" << errorY  << ", S=" << errorZ  << std::endl;
    SetScreenMessage(ss.str().c_str());

    Modified();
    } 

  return 1;
}


//----------------------------------------------------------------------------
void vtkMRMLTransPerinealProstateTemplateNode::SwitchStep(const char *stepName)
{
}


//----------------------------------------------------------------------------
void vtkMRMLTransPerinealProstateTemplateNode::SetAndObserveScannerConnectorNodeID(const char *nodeId)
{
  if (nodeId==this->GetScannerConnectorNodeID())
  {
    // no change
    return;
  }
  if (this->GetScannerConnectorNodeID()!=NULL && nodeId!=NULL && strcmp(nodeId, this->GetScannerConnectorNodeID())==0)
  {
    // no change
    return;
  }
  vtkSetAndObserveMRMLObjectMacro(this->ScannerConnectorNode, NULL);
  this->SetScannerConnectorNodeID(nodeId);
  vtkMRMLIGTLConnectorNode *tnode = this->GetScannerConnectorNode();
  vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
  events->InsertNextValue(vtkMRMLIGTLConnectorNode::ConnectedEvent);
  events->InsertNextValue(vtkMRMLIGTLConnectorNode::DisconnectedEvent);
  vtkSetAndObserveMRMLObjectEventsMacro(this->ScannerConnectorNode, tnode, events);
}


//----------------------------------------------------------------------------
vtkMRMLIGTLConnectorNode* vtkMRMLTransPerinealProstateTemplateNode::GetScannerConnectorNode()
{
  if (this->GetScene() && this->ScannerConnectorNodeID != NULL )
    {    
    return vtkMRMLIGTLConnectorNode::SafeDownCast(this->GetScene()->GetNodeByID(this->ScannerConnectorNodeID));
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkMRMLTransPerinealProstateTemplateNode::SetAndObserveZFrameModelNodeID(const char *nodeId)
{
  vtkSetAndObserveMRMLObjectMacro(this->ZFrameModelNode, NULL);
  this->SetZFrameModelNodeID(nodeId);
  vtkMRMLModelNode *tnode = this->GetZFrameModelNode();
  vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  vtkSetAndObserveMRMLObjectEventsMacro(this->ZFrameModelNode, tnode, events);
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLTransPerinealProstateTemplateNode::GetZFrameModelNode()
{
  if (this->GetScene() && this->ZFrameModelNodeID != NULL )
    {    
    return vtkMRMLModelNode::SafeDownCast(this->GetScene()->GetNodeByID(this->ZFrameModelNodeID));
    }
  return NULL;
}


//----------------------------------------------------------------------------
void vtkMRMLTransPerinealProstateTemplateNode::SetAndObserveWorkspaceModelNodeID(const char *nodeId)
{
  vtkSetAndObserveMRMLObjectMacro(this->WorkspaceModelNode, NULL);
  this->SetWorkspaceModelNodeID(nodeId);
  vtkMRMLModelNode *tnode = this->GetWorkspaceModelNode();
  vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  vtkSetAndObserveMRMLObjectEventsMacro(this->WorkspaceModelNode, tnode, events);
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLTransPerinealProstateTemplateNode::GetWorkspaceModelNode()
{
  if (this->GetScene() && this->WorkspaceModelNodeID != NULL )
    {    
    return vtkMRMLModelNode::SafeDownCast(this->GetScene()->GetNodeByID(this->WorkspaceModelNodeID));
    }
  return NULL;
}


//----------------------------------------------------------------------------
void vtkMRMLTransPerinealProstateTemplateNode::SetAndObserveZFrameTransformNodeID(const char *nodeId)
{
  vtkSetAndObserveMRMLObjectMacro(this->ZFrameTransformNode, NULL);
  this->SetZFrameTransformNodeID(nodeId);
  vtkMRMLLinearTransformNode *tnode = this->GetZFrameTransformNode();
  vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  vtkSetAndObserveMRMLObjectEventsMacro(this->ZFrameTransformNode, tnode, events);
}


//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransPerinealProstateTemplateNode::GetZFrameTransformNode()
{
  if (this->GetScene() && this->ZFrameTransformNodeID != NULL )
    {    
    return vtkMRMLLinearTransformNode::SafeDownCast(this->GetScene()->GetNodeByID(this->ZFrameTransformNodeID));
    }
  return NULL;
}

////----------------------------------------------------------------------------
//void vtkMRMLTransPerinealProstateTemplateNode::SetAndObserveTemplateTransformNodeID(const char *nodeId)
//{
//  vtkSetAndObserveMRMLObjectMacro(this->TemplateTransformNode, NULL);
//  this->SetTemplateTransformNodeID(nodeId);
//  vtkMRMLLinearTransformNode *tnode = this->GetTemplateTransformNode();
//  vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
//  events->InsertNextValue(vtkCommand::ModifiedEvent);
//  vtkSetAndObserveMRMLObjectEventsMacro(this->TemplateTransformNode, tnode, events);
//}


////----------------------------------------------------------------------------
//vtkMRMLLinearTransformNode* vtkMRMLTransPerinealProstateTemplateNode::GetTemplateTransformNode()
//{
//  if (this->GetScene() && this->TemplateTransformNodeID != NULL )
//    {    
//    return vtkMRMLLinearTransformNode::SafeDownCast(this->GetScene()->GetNodeByID(this->TemplateTransformNodeID));
//    }
//  return NULL;
//}


//----------------------------------------------------------------------------
void vtkMRMLTransPerinealProstateTemplateNode::SetAndObserveTemplateModelNodeID(const char *nodeId)
{
  vtkSetAndObserveMRMLObjectMacro(this->TemplateModelNode, NULL);
  this->SetTemplateModelNodeID(nodeId);
  vtkMRMLModelNode *tnode = this->GetTemplateModelNode();
  vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  vtkSetAndObserveMRMLObjectEventsMacro(this->TemplateModelNode, tnode, events);
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLTransPerinealProstateTemplateNode::GetTemplateModelNode()
{
  if (this->GetScene() && this->TemplateModelNodeID != NULL )
    {    
    return vtkMRMLModelNode::SafeDownCast(this->GetScene()->GetNodeByID(this->TemplateModelNodeID));
    }
  return NULL;
}


//----------------------------------------------------------------------------
void vtkMRMLTransPerinealProstateTemplateNode::SetAndObserveActiveNeedleModelNodeID(const char *nodeId)
{
  vtkSetAndObserveMRMLObjectMacro(this->ActiveNeedleModelNode, NULL);
  this->SetActiveNeedleModelNodeID(nodeId);
  vtkMRMLModelNode *tnode = this->GetActiveNeedleModelNode();
  vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  vtkSetAndObserveMRMLObjectEventsMacro(this->ActiveNeedleModelNode, tnode, events);
}

//----------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLTransPerinealProstateTemplateNode::GetActiveNeedleModelNode()
{
  if (this->GetScene() && this->ActiveNeedleModelNodeID != NULL )
    {    
    return vtkMRMLModelNode::SafeDownCast(this->GetScene()->GetNodeByID(this->ActiveNeedleModelNodeID));
    }
  return NULL;
}

//----------------------------------------------------------------------------
void vtkMRMLTransPerinealProstateTemplateNode::SetAndObserveActiveNeedleTransformNodeID(const char *nodeId)
{
  vtkSetAndObserveMRMLObjectMacro(this->ActiveNeedleTransformNode, NULL);
  this->SetActiveNeedleTransformNodeID(nodeId);
  vtkMRMLLinearTransformNode *tnode = this->GetActiveNeedleTransformNode();
  vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
  events->InsertNextValue(vtkCommand::ModifiedEvent);
  vtkSetAndObserveMRMLObjectEventsMacro(this->ActiveNeedleTransformNode, tnode, events);
}


//----------------------------------------------------------------------------
vtkMRMLLinearTransformNode* vtkMRMLTransPerinealProstateTemplateNode::GetActiveNeedleTransformNode()
{
  if (this->GetScene() && this->ActiveNeedleTransformNodeID != NULL )
    {    
    return vtkMRMLLinearTransformNode::SafeDownCast(this->GetScene()->GetNodeByID(this->ActiveNeedleTransformNodeID));
    }
  return NULL;
}




//----------------------------------------------------------------------------
bool vtkMRMLTransPerinealProstateTemplateNode::FindTargetingParams(vtkProstateNavTargetDescriptor *targetDesc)
{
  // this is used for coverage area computation (IsOutsideReach means that the target is outside the robot's coverage area)

  // :TODO: perform real targeting parameter computation  
  double *ras=targetDesc->GetRASLocation();
  const double center[3]={0,0,0};
  const double radius2=25*25;
  targetDesc->SetIsOutsideReach(
    (ras[0]-center[0])*(ras[0]-center[0])+
    (ras[1]-center[1])*(ras[1]-center[1])+
    (ras[2]-center[2])*(ras[2]-center[2])>radius2
    );
  return true;
}

//----------------------------------------------------------------------------
std::string vtkMRMLTransPerinealProstateTemplateNode::GetTargetInfoText(vtkProstateNavTargetDescriptor *targetDesc)
{
  // :TODO: construct a string that contains useful information for the active target (reachable, etc.)
  return "";
}


//----------------------------------------------------------------------------
int vtkMRMLTransPerinealProstateTemplateNode::GetHoleTransform(int i, int j, vtkMatrix4x4* matrix)
{
  if (matrix == NULL)
    {
    return 0;
    }

  if (i < 0 || i >= this->TemplateNumGrids[0] ||
      j < 0 || j >= this->TemplateNumGrids[1])
    {
    // the grid index is out of range
    return 0;
    }

  vtkMRMLLinearTransformNode *tnode = this->GetZFrameTransformNode(); 
  if (tnode == NULL)
    {
    return 0;
    }

  // offset from the Z-frame center
  double off[4];
  off[0] = this->TemplateOffset[0] + i * this->TemplateGridPitch[0];
  off[1] = this->TemplateOffset[1] + j * this->TemplateGridPitch[1];
  off[2] = this->TemplateOffset[2];
  off[3] = 1.0;

  vtkMatrix4x4* transform = tnode->GetMatrixTransformToParent();
  double coord[4];
  transform->MultiplyPoint(off, coord);

  matrix->DeepCopy(transform);
  matrix->SetElement(0, 3, coord[0]);
  matrix->SetElement(1, 3, coord[1]);
  matrix->SetElement(2, 3, coord[2]);
  matrix->SetElement(3, 3, 1.0);

  return 1;

}


//----------------------------------------------------------------------------
int vtkMRMLTransPerinealProstateTemplateNode::GetNeedleTransform(int i, int j, double length, vtkMatrix4x4* matrix)
{
  if (matrix == NULL)
    {
    return 0;
    }
  
  return 0; // not implemented
}
 

//----------------------------------------------------------------------------
int vtkMRMLTransPerinealProstateTemplateNode::FindHole(double targetX, double targetY, double targetZ,
                                                       int& nearest_i, int& nearest_j, double& nearest_depth,
                                                       double& errorX, double& errorY, double& errorZ)
{
  
  vtkMatrix4x4* matrix = vtkMatrix4x4::New();

  // NOTE: To find the hole for targeting, we will use the fact that
  // the optimal hole is nearest to the target. (It's not the best way, though...)
  double holeX;
  double holeY;
  double holeZ;

  nearest_i = 0;
  nearest_j = 0;
  GetHoleTransform(nearest_i, nearest_j, matrix);
  holeX = matrix->GetElement(0, 3);
  holeY = matrix->GetElement(1, 3);
  holeZ = matrix->GetElement(2, 3);
  nearest_depth = sqrt((holeX-targetX)*(holeX-targetX)+(holeY-targetY)*(holeY-targetY)+(holeZ-targetZ)*(holeZ-targetZ));

  for (int i = 0; i < this->TemplateNumGrids[0]; i ++)
    {
    for (int j = 0; j < this->TemplateNumGrids[1]; j ++)
      {
      GetHoleTransform(i, j, matrix);
      holeX = matrix->GetElement(0, 3);
      holeY = matrix->GetElement(1, 3);
      holeZ = matrix->GetElement(2, 3);
      double dist = sqrt((holeX-targetX)*(holeX-targetX)+(holeY-targetY)*(holeY-targetY)+(holeZ-targetZ)*(holeZ-targetZ));
      if (dist < nearest_depth)
        {
        nearest_depth = dist;
        nearest_i = i;
        nearest_j = j;
        }
      }
    }

  // use the distance as needle length
  double needleOffset[4];
  double needleTip[4];
  GetHoleTransform(nearest_i, nearest_j, matrix);
  needleOffset[0] = 0.0;
  needleOffset[1] = 0.0;
  needleOffset[2] = nearest_depth;
  needleOffset[3] = 1.0;
  matrix->MultiplyPoint(needleOffset, needleTip);

  // Calculate targeting error
  errorX = needleTip[0] - targetX;
  errorY = needleTip[1] - targetY;
  errorZ = needleTip[2] - targetZ;
  return 0;
}
