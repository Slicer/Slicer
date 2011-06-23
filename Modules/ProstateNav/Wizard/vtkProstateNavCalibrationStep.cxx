/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
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

#include "vtkMRMLBrpRobotCommandNode.h"

#include "vtkMRMLRobotNode.h"

#include "vtkKWMatrixWidget.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkProstateNavCalibrationStep);
vtkCxxRevisionMacro(vtkProstateNavCalibrationStep, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkProstateNavCalibrationStep::vtkProstateNavCalibrationStep()
{
  this->SetTitle("Calibration");  
  this->SetDescription("Perform Z-frame calibration.");

  this->SelectImageFrame  = NULL;
  //this->SelectImageButton = NULL;
  this->ZFrameImageSelectorWidget = NULL;
  this->SliceRangeMatrix = NULL;
  this->CalibrateButton   = NULL;

  this->ZFrameSettingFrame       = NULL;
  this->ShowZFrameCheckButton    = NULL;
  this->ShowWorkspaceCheckButton = NULL;

}

//----------------------------------------------------------------------------
vtkProstateNavCalibrationStep::~vtkProstateNavCalibrationStep()
{
  if (this->SelectImageFrame)
    {
    this->SelectImageFrame->SetParent(NULL);
    this->SelectImageFrame->Delete();
    }
  //if (this->SelectImageButton)
  //  {
  //  this->SelectImageButton->SetParent(NULL);
  //  this->SelectImageButton->Delete();
  //  }
  if (this->ZFrameImageSelectorWidget)
    {
    this->ZFrameImageSelectorWidget->SetParent(NULL);
    this->ZFrameImageSelectorWidget->Delete();
    }
  if (this->SliceRangeMatrix)
    {
    this->SliceRangeMatrix->SetParent(NULL);
    this->SliceRangeMatrix->Delete();
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
  if (this->ShowWorkspaceCheckButton)
    {
    this->ShowWorkspaceCheckButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand );
    this->ShowWorkspaceCheckButton->SetParent(NULL);
    this->ShowWorkspaceCheckButton->Delete();
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

  //if (!this->SelectImageButton)
  //  {
  //  this->SelectImageButton = vtkKWLoadSaveButtonWithLabel::New();
  //  this->SelectImageButton->SetParent(this->SelectImageFrame);
  //  this->SelectImageButton->Create();
  //  this->SelectImageButton->SetWidth(50);
  //  this->SelectImageButton->GetWidget()->SetText ("Browse Image File");
  //  /*
  //  this->SelectImageButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes(
  //    "{ {ProstateNav} {*.dcm} }");
  //  */
  //  this->SelectImageButton->GetWidget()->GetLoadSaveDialog()
  //    ->RetrieveLastPathFromRegistry("OpenPath");
  //  }
  //
  //this->Script("pack %s -side left -anchor w -fill x -padx 2 -pady 2", 
  //             this->SelectImageButton->GetWidgetName());

  if (!this->ZFrameImageSelectorWidget)
    {
    this->ZFrameImageSelectorWidget = vtkSlicerNodeSelectorWidget::New() ;
    this->ZFrameImageSelectorWidget->SetParent(this->SelectImageFrame);
    this->ZFrameImageSelectorWidget->Create(); 
    this->ZFrameImageSelectorWidget->AddNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, NULL);
    this->ZFrameImageSelectorWidget->SetMRMLScene(this->MRMLScene);
    this->ZFrameImageSelectorWidget->SetBorderWidth(2);
    this->ZFrameImageSelectorWidget->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->ZFrameImageSelectorWidget->GetWidget()->GetWidget()->SetWidth(24);
    this->ZFrameImageSelectorWidget->SetLabelText( "ZFrame Image: ");
    this->ZFrameImageSelectorWidget->NewNodeEnabledOn();
    this->ZFrameImageSelectorWidget->SetBalloonHelpString("Select Z-frame image node");
    this->ZFrameImageSelectorWidget->SetEnabled(1);
    }

  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
               this->ZFrameImageSelectorWidget->GetWidgetName());


  if (!this->SliceRangeMatrix)
    {
    this->SliceRangeMatrix = vtkKWMatrixWidgetWithLabel::New();
    this->SliceRangeMatrix->SetParent(this->SelectImageFrame);
    this->SliceRangeMatrix->Create();
    this->SliceRangeMatrix->SetLabelText("Slice range:");
    this->SliceRangeMatrix->ExpandWidgetOff();
    this->SliceRangeMatrix->GetLabel()->SetWidth(18);
    this->SliceRangeMatrix->SetBalloonHelpString("Set the needle position");

    vtkKWMatrixWidget *matrix =  this->SliceRangeMatrix->GetWidget();
    matrix->SetNumberOfColumns(2);
    matrix->SetNumberOfRows(1);
    matrix->SetElementWidth(12);
    matrix->SetRestrictElementValueToInteger();
    matrix->SetElementChangedCommandTriggerToAnyChange();
    matrix->SetElementValueAsInt(0, 0, 1);
    matrix->SetElementValueAsInt(0, 1, 12);
    }

  this->Script("pack %s -side top -anchor w -padx 2 -pady 2", 
               this->SliceRangeMatrix->GetWidgetName());


  if (!this->CalibrateButton)
    {
    this->CalibrateButton = vtkKWPushButton::New();
    this->CalibrateButton->SetParent (this->SelectImageFrame);
    this->CalibrateButton->Create ( );
    this->CalibrateButton->SetText ("Perform Calibration");
    this->CalibrateButton->SetBalloonHelpString("Send Calibration Data to the Robot");
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
    }

  this->Script("pack %s -side top -anchor w -padx 2 -pady 2", 
               this->ShowZFrameCheckButton->GetWidgetName());

  if (!this->ShowWorkspaceCheckButton)
    {
    this->ShowWorkspaceCheckButton = vtkKWCheckButton::New();
    this->ShowWorkspaceCheckButton->SetParent(this->ZFrameSettingFrame);
    this->ShowWorkspaceCheckButton->Create();
    this->ShowWorkspaceCheckButton->SelectedStateOff();
    this->ShowWorkspaceCheckButton->SetText("Show Range of Motion");
    }

  this->Script("pack %s -side top -anchor w -padx 2 -pady 2", 
               this->ShowWorkspaceCheckButton->GetWidgetName());

  this->AddGUIObservers();
}

//----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::HideUserInterface()
{
  Superclass::HideUserInterface();
  RemoveGUIObservers();
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
    ShowZFrameModel(checked);
    }
  if (this->ShowWorkspaceCheckButton == vtkKWCheckButton::SafeDownCast(caller) 
           && event == vtkKWCheckButton::SelectedStateChangedEvent )
    {
    int checked = this->ShowWorkspaceCheckButton->GetSelectedState(); 
    ShowWorkspaceModel(checked);
    }
  if (this->CalibrateButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
    vtkKWMatrixWidget *matrix =  this->SliceRangeMatrix->GetWidget();
    int s_index = matrix->GetElementValueAsInt(0, 0);
    int e_index = matrix->GetElementValueAsInt(0, 1);

    //const char *filename = this->SelectImageButton->GetWidget()->GetFileName();
    vtkMRMLScalarVolumeNode *volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(this->ZFrameImageSelectorWidget->GetSelected());

    // shift the index (on GUI, the index starts at 1)
    s_index --;
    e_index --;
    if (s_index < 1) s_index = 1;

    //PerformZFrameCalibration(filename);
    PerformZFrameCalibration(volumeNode, s_index, e_index);
    }
}


//----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::ShowZFrameModel(bool show)
{

  //  vtkMRMLModelNode*  modelNode =
  //  vtkMRMLModelNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->ZFrameModelNodeID.c_str()));
  //  vtkMRMLModelNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetLogic()->GetZFrameModelNodeID()));

  vtkMRMLModelNode*  modelNode = vtkMRMLModelNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetProstateNavManager()->GetRobotNode()->GetCalibrationObjectModelId()));
  vtkMRMLDisplayNode* displayNode = modelNode->GetDisplayNode();
  displayNode->SetVisibility(show);
  modelNode->Modified();
  this->MRMLScene->Modified();
}


//----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::ShowWorkspaceModel(bool show)
{

  vtkMRMLModelNode*   modelNode = vtkMRMLModelNode::SafeDownCast(this->MRMLScene->GetNodeByID(this->GetProstateNavManager()->GetRobotNode()->GetWorkspaceObjectModelId()));
  if (modelNode)
    {
    vtkMRMLDisplayNode* displayNode = modelNode->GetDisplayNode();
    displayNode->SetVisibility(show);
    modelNode->Modified();
    this->MRMLScene->Modified();
    }
}


/*
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
*/

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
      this->GetProstateNavManager()->GetRobotNode()->PerformRegistration(volumeNode);
    }
  }
  else
  {
    std::cerr << "Couldn't find ZFrame image in the MRML scene." << std::endl;
  }
}


//----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::PerformZFrameCalibration(vtkMRMLScalarVolumeNode* node, int s_index, int e_index)
{
  if (node)
    {
    this->GetProstateNavManager()->GetRobotNode()->PerformRegistration(node, s_index, e_index);
    }

}


//-----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::AddGUIObservers()
{
  this->RemoveGUIObservers();

  if (this->CalibrateButton)
    {
    this->CalibrateButton->AddObserver(vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallbackCommand);
    }
  if (this->ShowZFrameCheckButton)
    {
    this->ShowZFrameCheckButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
  if (this->ShowWorkspaceCheckButton)
    {
    this->ShowWorkspaceCheckButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
    }
}

//-----------------------------------------------------------------------------
void vtkProstateNavCalibrationStep::RemoveGUIObservers()
{
  if (this->CalibrateButton)
    {
    this->CalibrateButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->ShowZFrameCheckButton)
    {
    this->ShowZFrameCheckButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
  if (this->ShowWorkspaceCheckButton)
    {
    this->ShowWorkspaceCheckButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }
}
