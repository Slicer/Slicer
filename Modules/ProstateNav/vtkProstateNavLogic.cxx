/*=auto=========================================================================

  Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: $
  Date:      $Date: $
  Version:   $Revision: $

=========================================================================auto=*/

#include "vtkObjectFactory.h"
#include "vtkCallbackCommand.h"

#include "vtkProstateNavLogic.h"

#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerColorLogic.h"


// for DICOM read
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImage.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkGDCMImageIO.h"
#include "itkSpatialOrientationAdapter.h"

#include "vtkMRMLBrpRobotCommandNode.h"

#include "vtkProstateNavGUI.h"

vtkCxxRevisionMacro(vtkProstateNavLogic, "$Revision: 1.9.12.1 $");
vtkStandardNewMacro(vtkProstateNavLogic);

//---------------------------------------------------------------------------
vtkProstateNavLogic::vtkProstateNavLogic()
{
  this->RobotWorkPhase       = -1;
  this->ScannerWorkPhase     = -1;
  
  // Timer Handling
  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkProstateNavLogic::DataCallback);

  this->RobotCommandNodeID = "";

  this->TimerOn = 0;

}


//---------------------------------------------------------------------------
vtkProstateNavLogic::~vtkProstateNavLogic()
{

    if (this->DataCallbackCommand)
    {
      this->DataCallbackCommand->Delete();
    }

}

//---------------------------------------------------------------------------
void vtkProstateNavLogic::PrintSelf(ostream& os, vtkIndent indent)
{
    this->vtkObject::PrintSelf(os, indent);

    os << indent << "vtkProstateNavLogic:             " << this->GetClassName() << "\n";

}

//---------------------------------------------------------------------------
void vtkProstateNavLogic::DataCallback(vtkObject *caller, 
                                       unsigned long eid, void *clientData, void *callData)
{
    vtkProstateNavLogic *self = reinterpret_cast<vtkProstateNavLogic *>(clientData);
    vtkDebugWithObjectMacro(self, "In vtkProstateNavLogic DataCallback");
    self->UpdateAll();
}

//---------------------------------------------------------------------------
void vtkProstateNavLogic::UpdateAll()
{

}


int vtkProstateNavLogic::Enter()
{

  vtkMRMLBrpRobotCommandNode *linxnode = vtkMRMLBrpRobotCommandNode::New(); 
  this->GetMRMLScene()->RegisterNodeClass( linxnode );
  linxnode->Delete();
  
//  std::cerr << "checking IsA() .." << std::endl;
  if (linxnode->IsA("vtkMRMLBrpRobotCommandNode"))
    {
    std::cerr << "OK" << std::endl;
    }
  else
    {
    std::cerr << "NOT OK" << std::endl;
    }

  std::vector<vtkMRMLNode*> nodes;
  this->GetMRMLScene()->GetNodesByClass("vtkMRMLBrpRobotCommandNode", nodes);

  // Command node
  vtkMRMLBrpRobotCommandNode* node = vtkMRMLBrpRobotCommandNode::New();
  node->SetName("BRPRobotCommand");
  node->SetScene(this->GetMRMLScene());

  this->GetMRMLScene()->SaveStateForUndo();
  this->GetMRMLScene()->AddNode(node);

  node->Modified();
  this->GetMRMLScene()->Modified();

  this->RobotCommandNodeID = node->GetID();
  node->Delete();

  // Target node
  vtkMRMLLinearTransformNode* tnode = vtkMRMLLinearTransformNode::New();
  tnode->SetName("BRPRobotTarget");

  vtkMatrix4x4* transform = vtkMatrix4x4::New();
  transform->Identity();
  //transformNode->SetAndObserveImageData(transform);
  tnode->ApplyTransform(transform);
  tnode->SetScene(this->MRMLScene);
  transform->Delete();

  this->GetMRMLScene()->AddNode(tnode);
  this->RobotTargetNodeID = tnode->GetID();

  tnode->Delete();

  // Z-Frame node
  vtkMRMLLinearTransformNode* ztnode = vtkMRMLLinearTransformNode::New();
  ztnode->SetName("ZFrameTransform");

  vtkMatrix4x4* ztransform = vtkMatrix4x4::New();
  ztransform->Identity();
  //transformNode->SetAndObserveImageData(transform);
  ztnode->ApplyTransform(ztransform);
  ztnode->SetScene(this->MRMLScene);
  ztransform->Delete();
  this->GetMRMLScene()->AddNode(ztnode);
  this->ZFrameTransformNodeID = ztnode->GetID();

  ztnode->Delete();

  // ZFrame
  this->ZFrameModelNodeID = AddZFrameModel("ZFrame");
  vtkMRMLModelNode*  modelNode =
    vtkMRMLModelNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->ZFrameModelNodeID.c_str()));
  
  if (modelNode)
    {
    vtkMRMLDisplayNode* displayNode = modelNode->GetDisplayNode();
    displayNode->SetVisibility(0);
    modelNode->Modified();
    this->MRMLScene->Modified();

    modelNode->SetAndObserveTransformNodeID(this->ZFrameTransformNodeID.c_str());
    }

//  std::cerr << "Robot = " <<  this->RobotCommandNodeID << std::endl;
//  std::cerr << "Target = " <<  this->RobotTargetNodeID  << std::endl;
//  std::cerr << "ZFrame = " <<  this->ZFrameModelNodeID  << std::endl;


  return 1;
}


//----------------------------------------------------------------------------
void vtkProstateNavLogic::TimerHandler()
{
  if (this->TimerOn)
    {
    vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(this->RobotCommandNodeID.c_str());
    vtkMRMLBrpRobotCommandNode* cnode = vtkMRMLBrpRobotCommandNode::SafeDownCast(node);
    if (cnode)
      {
      //cnode->PushOutgoingCommand("GET_COORDINATE");
      cnode->PushOutgoingCommand("GET_COORDINA");
      cnode->InvokeEvent(vtkCommand::ModifiedEvent);
      }
//    std::cerr << "void vtkProstateNavLogic::TimerHandler() is called" << std::endl;
    vtkKWTkUtilities::CreateTimerHandler(this->GetGUI()->GetApplication(), 200, this, "TimerHandler");
    }
}


//---------------------------------------------------------------------------
int vtkProstateNavLogic::ConnectTracker(const char* filename)
{

    return 1;
}


//---------------------------------------------------------------------------
int vtkProstateNavLogic::DisconnectTracker()
{

    return 1;
}


//---------------------------------------------------------------------------
int vtkProstateNavLogic::RobotStop()
{

  std::cerr << "vtkProstateNavLogic::RobotStop()" << std::endl;
  return 1;

}


//---------------------------------------------------------------------------
int vtkProstateNavLogic::RobotMoveTo(float px, float py, float pz,
                                     float nx, float ny, float nz,
                                     float tx, float ty, float tz)
{

  std::cerr << "vtkProstateNavLogic::RobotMoveTo()" << std::endl;
  return 1;
}


//---------------------------------------------------------------------------
int vtkProstateNavLogic::RobotMoveTo(float position[3], float orientation[3])
{
  std::cerr << "vtkProstateNavLogic::RobotMoveTo()" << std::endl;

  return 1;
}


//---------------------------------------------------------------------------
int  vtkProstateNavLogic::RobotMoveTo()
{
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(this->RobotCommandNodeID.c_str());
  vtkMRMLBrpRobotCommandNode* cnode = vtkMRMLBrpRobotCommandNode::SafeDownCast(node);
  if (!cnode)
    {
    return 0;
    }

  cnode->SetTargetTransformNodeID(this->RobotTargetNodeID.c_str());
  cnode->PushOutgoingCommand("MOVE_TO");
  cnode->InvokeEvent(vtkCommand::ModifiedEvent);
  
  return 1;
}

//---------------------------------------------------------------------------
int vtkProstateNavLogic::SendZFrame()
{
  std::cerr << "int vtkProstateNavLogic::SendZFrame()" << std::endl;
  vtkMRMLNode* node = this->GetMRMLScene()->GetNodeByID(this->RobotCommandNodeID.c_str());
  vtkMRMLBrpRobotCommandNode* cnode = vtkMRMLBrpRobotCommandNode::SafeDownCast(node);
  if (!cnode)
    {
    return 0;
    }

  std::cerr << "int vtkProstateNavLogic::SendZFrame(): " << this->ZFrameTransformNodeID.c_str() << std::endl;

  cnode->SetZFrameTransformNodeID(this->ZFrameTransformNodeID.c_str());
  cnode->PushOutgoingCommand("SET_Z_FRAME");
  cnode->InvokeEvent(vtkCommand::ModifiedEvent);

  return 1;
  
}

//---------------------------------------------------------------------------
int vtkProstateNavLogic::ScanStart()
{

  return 1;
}

//---------------------------------------------------------------------------
int vtkProstateNavLogic::ScanPause()
{
  return 1;
}

//---------------------------------------------------------------------------
int vtkProstateNavLogic::ScanStop()
{

  return 1;
}


//----------------------------------------------------------------------------
const char* vtkProstateNavLogic::AddZFrameModel(const char* nodeName)
{
  std::string rstr;

  vtkMRMLModelNode           *zframeModel;
  vtkMRMLModelDisplayNode    *zframeDisp;

  zframeModel = vtkMRMLModelNode::New();
  zframeDisp = vtkMRMLModelDisplayNode::New();

  this->MRMLScene->SaveStateForUndo();
  this->MRMLScene->AddNode(zframeDisp);
  this->MRMLScene->AddNode(zframeModel);  

  zframeDisp->SetScene(this->MRMLScene);
  zframeModel->SetName(nodeName);
  zframeModel->SetScene(this->MRMLScene);
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
  trans2->RotateX(-45.0);
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
  trans4->Translate(0.0, -length/2.0, 0.0);
  trans4->RotateX(90.0);
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
  trans6->RotateX(45.0);
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

  rstr = zframeModel->GetID();
  
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

  zframeDisp->Delete();
  zframeModel->Delete();

  return rstr.c_str();

}

