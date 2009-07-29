/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#include "vtkProstateNavCalibrationStep.h"

#include "vtkProstateNavGUI.h"
#include "vtkProstateNavLogic.h"

#include "vtkKWFrame.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkCylinderSource.h"
#include "vtkTransformPolyDataFilter.h"
#include "vtkTransform.h"
#include "vtkAppendPolyData.h"

#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerVolumesLogic.h"

#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLLinearTransformNode.h"

#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLVolumeHeaderlessStorageNode.h"
#include "vtkMRMLVolumeArchetypeStorageNode.h"
#include "vtkCollection.h"

#include "vtkSlicerColorLogic.h"

#include "vtkZFrameRobotToImageRegistration.h"


//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkProstateNavCalibrationStep);
vtkCxxRevisionMacro(vtkProstateNavCalibrationStep, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkProstateNavCalibrationStep::vtkProstateNavCalibrationStep()
{
  //this->SetName("Calibration");
  this->SetTitle("Calibration");  
  this->SetDescription("Perform Z-frame calibration.");

  this->SelectImageFrame  = NULL;
  this->SelectImageButton = NULL;
  this->CalibrateButton   = NULL;

  this->ZFrameSettingFrame = NULL;
  this->ShowZFrameCheckButton = NULL;

}

//----------------------------------------------------------------------------
vtkProstateNavCalibrationStep::~vtkProstateNavCalibrationStep()
{
  if (this->SelectImageFrame)
    {
    this->SelectImageFrame->SetParent(NULL);
    this->SelectImageFrame->Delete();
    }
  if (this->SelectImageButton)
    {
    this->SelectImageButton->SetParent(NULL);
    this->SelectImageButton->Delete();
    }
  if (this->CalibrateButton)
    {
    this->CalibrateButton->SetParent(NULL);
    this->CalibrateButton->Delete();
    }
  if (this->ZFrameSettingFrame)
    {
    this->ZFrameSettingFrame->SetParent(NULL);
    this->ZFrameSettingFrame->Delete();
    }
  if (this->ShowZFrameCheckButton)
    {
    this->ShowZFrameCheckButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand );
    this->ShowZFrameCheckButton->SetParent(NULL);
    this->ShowZFrameCheckButton->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::ShowUserInterface()
{

  this->Superclass::ShowUserInterface();

  vtkKWWizardWidget *wizardWidget = this->GetGUI()->GetWizardWidget();
  vtkKWWidget *parent = wizardWidget->GetClientArea();

  // Create frame
  if (!this->SelectImageFrame)
    {
    this->SelectImageFrame = vtkKWFrame::New();
    this->SelectImageFrame->SetParent(parent);
    this->SelectImageFrame->Create();
    }

  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
               this->SelectImageFrame->GetWidgetName());

  if (!this->SelectImageButton)
    {
    this->SelectImageButton = vtkKWLoadSaveButtonWithLabel::New();
    this->SelectImageButton->SetParent(this->SelectImageFrame);
    this->SelectImageButton->Create();
    this->SelectImageButton->SetWidth(50);
    this->SelectImageButton->GetWidget()->SetText ("Browse Image File");
    /*
    this->SelectImageButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes(
      "{ {ProstateNav} {*.dcm} }");
    */
    this->SelectImageButton->GetWidget()->GetLoadSaveDialog()
      ->RetrieveLastPathFromRegistry("OpenPath");
    }

  this->Script("pack %s -side left -anchor w -fill x -padx 2 -pady 2", 
               this->SelectImageButton->GetWidgetName());

  if (!this->CalibrateButton)
    {
    this->CalibrateButton = vtkKWPushButton::New();
    this->CalibrateButton->SetParent (this->SelectImageFrame);
    this->CalibrateButton->Create ( );
    this->CalibrateButton->SetText ("Perform Calibration");
    this->CalibrateButton->SetBalloonHelpString("Send Calibration Data to the Robot");
    this->CalibrateButton->AddObserver(vtkKWPushButton::InvokedEvent,
                                       (vtkCommand *)this->GUICallbackCommand);
    }

  this->Script("pack %s -side top -anchor w -padx 2 -pady 2", 
               this->CalibrateButton->GetWidgetName());

  
  if (!this->ZFrameSettingFrame)
    {
    this->ZFrameSettingFrame = vtkKWFrame::New();
    this->ZFrameSettingFrame->SetParent(parent);
    this->ZFrameSettingFrame->Create();
    }

  this->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 2",
               this->ZFrameSettingFrame->GetWidgetName());
  
  if (!this->ShowZFrameCheckButton)
    {
    this->ShowZFrameCheckButton = vtkKWCheckButton::New();
    this->ShowZFrameCheckButton->SetParent(this->ZFrameSettingFrame);
    this->ShowZFrameCheckButton->Create();
    this->ShowZFrameCheckButton->SelectedStateOff();
    this->ShowZFrameCheckButton->SetText("Show ZFrame");
    this->ShowZFrameCheckButton
      ->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    
    }

  this->Script("pack %s -side top -anchor w -padx 2 -pady 2", 
               this->ShowZFrameCheckButton->GetWidgetName());

}


//----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}


//----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::ProcessGUIEvents(vtkObject *caller,
                                           unsigned long event, void *callData)
{

  if (this->ShowZFrameCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
           && event == vtkKWCheckButton::SelectedStateChangedEvent )
    {
    int checked = this->ShowZFrameCheckButton->GetSelectedState(); 
    if (checked)
      {
      ShowZFrameModel();
      }
    else
      {
      HideZFrameModel();
      }
    }
  if (this->CalibrateButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
    const char *filename = this->SelectImageButton->GetWidget()->GetFileName();
    PerformZFrameCalibration(filename);
    }
}


//----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::ShowZFrameModel()
{

  vtkMRMLModelNode*  modelNode =
    //vtkMRMLModelNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->ZFrameModelNodeID.c_str()));
    vtkMRMLModelNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetLogic()->GetZFrameModelNodeID()));

  vtkMRMLDisplayNode* displayNode = modelNode->GetDisplayNode();
  displayNode->SetVisibility(1);
  modelNode->Modified();
  this->MRMLScene->Modified();
  

}


//----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::HideZFrameModel()
{

  vtkMRMLModelNode*  modelNode =
    vtkMRMLModelNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetLogic()->GetZFrameModelNodeID()));

  if (modelNode)
    {
    vtkMRMLDisplayNode* displayNode = modelNode->GetDisplayNode();
    displayNode->SetVisibility(0);
    modelNode->Modified();
    this->MRMLScene->Modified();
    }
}


//----------------------------------------------------------------------------
const char* vtkProstateNavCalibrationStep::AddZFrameModel(const char* nodeName)
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


//----------------------------------------------------------------------------
const char* vtkProstateNavCalibrationStep::AddZFrameTransform(const char* nodeName)
{
  std::string rstr;

  vtkMRMLLinearTransformNode* transformNode;

  transformNode = vtkMRMLLinearTransformNode::New();
  transformNode->SetName(nodeName);
  transformNode->SetDescription("Created by ProstateNav module");

  vtkMatrix4x4* transform = vtkMatrix4x4::New();
  transform->Identity();
  transformNode->ApplyTransform(transform);
  transform->Delete();

  this->MRMLScene->AddNode(transformNode);  

  rstr = transformNode->GetID();
  transformNode->Delete();
  
  return rstr.c_str();

}

//----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::PerformZFrameCalibration(const char* filename)
{
  std::cerr << "Loading " << filename << std::endl;

  vtkSlicerModuleGUI *m = vtkSlicerApplication::SafeDownCast(this->GetApplication())->GetModuleGUIByName("Volumes"); 
  if ( m != NULL ) 
    {
    vtkSlicerVolumesLogic* volume_logic = 
      vtkSlicerVolumesGUI::SafeDownCast(m)->GetLogic();
    volume_logic->AddArchetypeVolume(filename, "ZFrameImage", 0x0004);

    vtkMRMLScalarVolumeNode* volumeNode = NULL;   // Event Source MRML node 
    vtkCollection* collection = this->MRMLScene->GetNodesByName("ZFrameImage");
    int nItems = collection->GetNumberOfItems();
    for (int i = 0; i < nItems; i ++)
      {
      vtkMRMLNode* node = vtkMRMLNode::SafeDownCast(collection->GetItemAsObject(i));
      if (strcmp(node->GetNodeTagName(), "Volume") == 0)
        {
        volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(node);
        break;
        }
      }

    if (volumeNode)
      {
      vtkZFrameRobotToImageRegistration* registration = vtkZFrameRobotToImageRegistration::New();
      registration->SetFiducialVolume(volumeNode);

      vtkMRMLNode* node = this->MRMLScene->GetNodeByID(this->GetLogic()->GetZFrameTransformNodeID());
      vtkMRMLLinearTransformNode* transformNode;
      if (node != NULL)
        {
        transformNode = vtkMRMLLinearTransformNode::SafeDownCast(node);
        registration->SetRobotToImageTransform(transformNode);

        registration->DoRegistration();

        this->GetLogic()->SendZFrame();
        }
      else
        {
        std::cerr << "Couldn't find zframe transform node" << std::endl;
        }
      }
    else
      {
      std::cerr << "Couldn't find ZFrame image in the MRML scene." << std::endl;
      }
    }
}

