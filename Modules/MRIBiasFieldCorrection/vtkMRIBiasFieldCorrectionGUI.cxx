/*=auto====================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRIBiasFieldCorrectionGUI.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $
Author:    $Nicolas Rannou (BWH), Sylvain Jaume (MIT)

====================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRIBiasFieldCorrectionGUI.h"

#include "vtkCommand.h"
#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWScale.h"
#include "vtkKWMenu.h"
#include "vtkKWEntry.h"
#include "vtkKWFrame.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkKWPushButton.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWSpinBoxWithLabel.h"
#include "vtkKWSpinBox.h"

#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkKWScale.h"

#include "vtkMRMLSliceNode.h"
#include "vtkSlicerSliceGUI.h"

//-------------------------------------------------------------------------
vtkMRIBiasFieldCorrectionGUI* vtkMRIBiasFieldCorrectionGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance(
      "vtkMRIBiasFieldCorrectionGUI");

  if(ret)
    {
    return (vtkMRIBiasFieldCorrectionGUI*)ret;
    }

  // If the factory was unable to create the object, then create it here.
  return new vtkMRIBiasFieldCorrectionGUI;
}

//-------------------------------------------------------------------------
vtkMRIBiasFieldCorrectionGUI::vtkMRIBiasFieldCorrectionGUI()
{
  this->TimeStepScale = vtkKWScaleWithEntry::New();
  this->NumberOfIterationsScale = vtkKWScaleWithEntry::New();
  this->VolumeSelector = vtkSlicerNodeSelectorWidget::New();
  this->OutVolumeSelector = vtkSlicerNodeSelectorWidget::New();
  this->StorageVolumeSelector = vtkSlicerNodeSelectorWidget::New();
  this->MaskVolumeSelector = vtkSlicerNodeSelectorWidget::New();

  this->GADNodeSelector = vtkSlicerNodeSelectorWidget::New();
  this->ApplyButton = vtkKWPushButton::New();
  this->CatchButton = vtkKWPushButton::New();
  this->Logic = NULL;
  this->MRIBiasFieldCorrectionNode = NULL;

  this->Sagittal2Scale = vtkKWScaleWithEntry::New();

  this->Coronal1Scale = vtkKWScaleWithEntry::New();
  this->Coronal2Scale = vtkKWScaleWithEntry::New();

  this->VolumeSelectionFrame = vtkKWFrameWithLabel::New();

  this->ParametersFrame         = vtkKWFrameWithLabel::New();
  this->AdvancedParametersFrame = vtkKWFrameWithLabel::New();

  this->ShrinkFactor          = vtkKWScaleWithEntry::New();
  this->MaxNumberOfIterations = vtkKWScaleWithEntry::New();
  this->NumOfFitLev           = vtkKWScaleWithEntry::New();
  this->WienFilterNoise       = vtkKWScaleWithEntry::New();
  this->BiasField             = vtkKWScaleWithEntry::New();
  this->Convergence           = vtkKWScaleWithEntry::New();

  this->SlidePositionScale = vtkKWScaleWithEntry::New();
  this->BiasIntensityScale = vtkKWScaleWithEntry::New();
}

//-------------------------------------------------------------------------
vtkMRIBiasFieldCorrectionGUI::~vtkMRIBiasFieldCorrectionGUI()
{
  if( this->BiasIntensityScale )
    {
    this->BiasIntensityScale->SetParent(NULL);
    this->BiasIntensityScale->Delete();
    this->BiasIntensityScale = NULL;
    }

  if( this->SlidePositionScale )
    {
    this->SlidePositionScale->SetParent(NULL);
    this->SlidePositionScale->Delete();
    this->SlidePositionScale = NULL;
    }

  if( this->MaxNumberOfIterations )
    {
    this->MaxNumberOfIterations->SetParent(NULL);
    this->MaxNumberOfIterations->Delete();
    this->MaxNumberOfIterations = NULL;
    }

  if( this->NumOfFitLev )
    {
    this->NumOfFitLev->SetParent(NULL);
    this->NumOfFitLev->Delete();
    this->NumOfFitLev = NULL;
    }

  if( this->WienFilterNoise )
    {
    this->WienFilterNoise->SetParent(NULL);
    this->WienFilterNoise->Delete();
    this->WienFilterNoise = NULL;
    }

  if( this->BiasField )
    {
    this->BiasField->SetParent(NULL);
    this->BiasField->Delete();
    this->BiasField = NULL;
    }

  if( this->Convergence )
    {
    this->Convergence->SetParent(NULL);
    this->Convergence->Delete();
    this->Convergence = NULL;
    }

  if( this->ShrinkFactor )
    {
    this->ShrinkFactor->SetParent(NULL);
    this->ShrinkFactor->Delete();
    this->ShrinkFactor = NULL;
    }

  if( this->ParametersFrame )
    {
    this->ParametersFrame->SetParent(NULL);
    this->ParametersFrame->Delete();
    this->ParametersFrame = NULL;
    }

  if( this->AdvancedParametersFrame )
    {
    this->AdvancedParametersFrame->SetParent(NULL);
    this->AdvancedParametersFrame->Delete();
    this->AdvancedParametersFrame = NULL;
    }

  if( this->TimeStepScale )
    {
    this->TimeStepScale->SetParent(NULL);
    this->TimeStepScale->Delete();
    this->TimeStepScale = NULL;
    }

  if( this->NumberOfIterationsScale )
    {
    this->NumberOfIterationsScale->SetParent(NULL);
    this->NumberOfIterationsScale->Delete();
    this->NumberOfIterationsScale = NULL;
    }

  if( this->VolumeSelector )
    {
    this->VolumeSelector->SetParent(NULL);
    this->VolumeSelector->Delete();
    this->VolumeSelector = NULL;
    }

  if( this->OutVolumeSelector )
    {
    this->OutVolumeSelector->SetParent(NULL);
    this->OutVolumeSelector->Delete();
    this->OutVolumeSelector = NULL;
    }

  if( this->StorageVolumeSelector )
    {
    this->StorageVolumeSelector->SetParent(NULL);
    this->StorageVolumeSelector->Delete();
    this->StorageVolumeSelector = NULL;
    }

  if( this->MaskVolumeSelector )
    {
    this->MaskVolumeSelector->SetParent(NULL);
    this->MaskVolumeSelector->Delete();
    this->MaskVolumeSelector = NULL;
    }

  if( this->GADNodeSelector )
    {
    this->GADNodeSelector->SetParent(NULL);
    this->GADNodeSelector->Delete();
    this->GADNodeSelector = NULL;
    }

  if( this->ApplyButton )
    {
    this->ApplyButton->SetParent(NULL);
    this->ApplyButton->Delete();
    this->ApplyButton = NULL;
    }

  if( this->CatchButton )
    {
    this->CatchButton->SetParent(NULL);
    this->CatchButton->Delete();
    this->CatchButton = NULL;
    }

  if( this->VolumeSelectionFrame )
    {
    this->VolumeSelectionFrame->SetParent(NULL);
    this->VolumeSelectionFrame->Delete();
    this->VolumeSelectionFrame = NULL;
    }

  if( this->Sagittal2Scale )
    {
    this->Sagittal2Scale->SetParent(NULL);
    this->Sagittal2Scale->Delete();
    this->Sagittal2Scale = NULL;
    }

  if( this->Coronal1Scale )
    {
    this->Coronal1Scale->SetParent(NULL);
    this->Coronal1Scale->Delete();
    this->Coronal1Scale = NULL;
    }

  if( this->Coronal2Scale )
    {
    this->Coronal2Scale->SetParent(NULL);
    this->Coronal2Scale->Delete();
    this->Coronal2Scale = NULL;
    }

  this->SetLogic (NULL);
  vtkSetMRMLNodeMacro(this->MRIBiasFieldCorrectionNode, NULL);
}

//-------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::PrintSelf(ostream& os, vtkIndent indent)
{
}

//-------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::AddGUIObservers()
{
  this->VolumeSelector->AddObserver( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->OutVolumeSelector->AddObserver( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->StorageVolumeSelector->AddObserver( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->MaskVolumeSelector->AddObserver( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->GADNodeSelector->AddObserver( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->ApplyButton->AddObserver( vtkKWPushButton::InvokedEvent,
      (vtkCommand *)this->GUICallbackCommand );

  this->CatchButton->AddObserver( vtkKWPushButton::InvokedEvent,
      (vtkCommand *)this->GUICallbackCommand );
}

//-------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::RemoveGUIObservers()
{
  this->VolumeSelector->RemoveObservers( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->MaskVolumeSelector->RemoveObservers( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->OutVolumeSelector->RemoveObservers( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->StorageVolumeSelector->RemoveObservers(
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)
      this->GUICallbackCommand );

  this->GADNodeSelector->RemoveObservers( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->ApplyButton->RemoveObservers( vtkKWPushButton::InvokedEvent,
      (vtkCommand *)this->GUICallbackCommand );

  this->CatchButton->RemoveObservers( vtkKWPushButton::InvokedEvent,
      (vtkCommand *)this->GUICallbackCommand );
}

//-------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::ProcessGUIEvents ( vtkObject *caller,
  unsigned long event, void *callData )
{
  vtkKWPushButton     *b = vtkKWPushButton::SafeDownCast(caller);

  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::
    SafeDownCast(caller);

  if( selector == this->VolumeSelector && event ==
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->VolumeSelector->GetSelected() != NULL)
    {
    this->UpdateMRML();
    this->UpdateGUI();
    }
  else if( selector == this->OutVolumeSelector && event ==
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->OutVolumeSelector->GetSelected() != NULL)
    {
    this->UpdateMRML();
    this->UpdateGUI();
    }
  else if( selector == this->MaskVolumeSelector && event ==
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->MaskVolumeSelector->GetSelected() != NULL)
    {
    this->UpdateMRML();
    this->UpdateGUI();
    }
   else if( selector == this->StorageVolumeSelector && event ==
       vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
       this->StorageVolumeSelector->GetSelected() != NULL)
    {
    this->UpdateMRML();
    this->UpdateGUI();
    }
  if( selector == this->GADNodeSelector && event ==
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->GADNodeSelector->GetSelected() != NULL)
    {
    vtkMRMLMRIBiasFieldCorrectionNode *n =
      vtkMRMLMRIBiasFieldCorrectionNode::SafeDownCast(
          this->GADNodeSelector->GetSelected());
    this->Logic->SetAndObserveMRIBiasFieldCorrectionNode(n);
    vtkSetAndObserveMRMLNodeMacro( this->MRIBiasFieldCorrectionNode, n);
    this->UpdateGUI();
    }
  else if( b == this->ApplyButton && event == vtkKWPushButton::
      InvokedEvent )
    {
    this->UpdateMRML();
    this->Logic->Apply();
    this->UpdateGUI();
    }
  else if( b == this->CatchButton && event == vtkKWPushButton::
      InvokedEvent )
    {
    this->UpdateMRML();

    vtkSlicerApplicationGUI *applicationGUI = this->GetApplicationGUI();

    // GET DATA FROM SLICES

    vtkImageData* image = vtkImageData::New();
    image->DeepCopy( applicationGUI->GetApplicationLogic()->
      GetSliceLogic("Red")->GetBackgroundLayer()->GetSlice()->
      GetOutput());

    image->UpdateInformation();

    vtkTransform* xyToijk = vtkTransform::New();
    xyToijk = applicationGUI->GetApplicationLogic()->GetSliceLogic(
      "Red")->GetBackgroundLayer()->GetXYToIJKTransform();

    vtkMRMLSliceNode *snode = applicationGUI->GetMainSliceGUI("Red")
      ->GetSliceNode();

    unsigned int dimensions[3];
    snode->GetDimensions(dimensions);
    this->Logic->SliceProcess(xyToijk,dimensions[0],dimensions[1]);
    }
}

//-------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::UpdateMRML ()
{
  vtkMRMLMRIBiasFieldCorrectionNode* n = this->
    GetMRIBiasFieldCorrectionNode();

  if(n == NULL)
    {
    // no parameter node selected yet, create new
    this->GADNodeSelector->SetSelectedNew(
      "vtkMRMLMRIBiasFieldCorrectionNode");
    this->GADNodeSelector->ProcessNewNodeCommand(
      "vtkMRMLMRIBiasFieldCorrectionNode", "GADParameters");
    n = vtkMRMLMRIBiasFieldCorrectionNode::SafeDownCast(this->
      GADNodeSelector->GetSelected());

    // set an observe new node in Logic
    this->Logic->SetAndObserveMRIBiasFieldCorrectionNode(n);
    vtkSetAndObserveMRMLNodeMacro(this->MRIBiasFieldCorrectionNode, n);
   }

  // save node parameters for Undo
  this->GetLogic()->GetMRMLScene()->SaveStateForUndo(n);

  n->SetShrink((int)floor(this->ShrinkFactor->GetValue()));

  n->SetMax((int)floor(this->MaxNumberOfIterations->GetValue()));

  n->SetNum(this->NumOfFitLev->GetValue());

  n->SetWien(this->WienFilterNoise->GetValue());

  n->SetField(this->BiasField->GetValue());

  n->SetCon(this->Convergence->GetValue());

  if (this->VolumeSelector->GetSelected() != NULL)
    {
    n->SetInputVolumeRef(this->VolumeSelector->GetSelected()->GetID());
    }

  if (this->OutVolumeSelector->GetSelected() != NULL)
    {
    n->SetOutputVolumeRef(this->OutVolumeSelector->GetSelected()->GetID());
    }
  if (this->StorageVolumeSelector->GetSelected() != NULL)
    {
    n->SetStorageVolumeRef(this->StorageVolumeSelector->GetSelected()->
      GetID());
    }
  if (this->MaskVolumeSelector->GetSelected() != NULL)
    {
    n->SetMaskVolumeRef(this->MaskVolumeSelector->GetSelected()->GetID());
    }
}

//--------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::UpdateGUI()
{
  vtkMRMLMRIBiasFieldCorrectionNode* n = this->
    GetMRIBiasFieldCorrectionNode();

  if( n != NULL )
    {
    std::cout<<"Update GUI"<<std::endl;
    // set GUI widgest from parameter node

    this->ShrinkFactor->SetValue(n->GetShrink());

    this->MaxNumberOfIterations->SetValue(n->GetMax());

    this->NumOfFitLev->SetValue(n->GetNum());

    this->WienFilterNoise->SetValue(n->GetWien());

    this->BiasField->SetValue(n->GetField());

    this->Convergence->SetValue(n->GetCon());
    }
}

//--------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::ProcessMRMLEvents ( vtkObject *caller,
  unsigned long event, void *callData )
{
  // if parameter node has been changed externally, update GUI
  // widgets with new values
  vtkMRMLMRIBiasFieldCorrectionNode* node =
    vtkMRMLMRIBiasFieldCorrectionNode::SafeDownCast(caller);

  if (node != NULL && this->GetMRIBiasFieldCorrectionNode() == node)
    {
    this->UpdateGUI();
    //this->Logic->Cut();
    std::cout<<"MRML changed"<<std::endl;
    }
}

//--------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::BuildGUI()
{
  vtkSlicerApplication *app = (vtkSlicerApplication*)this->
    GetApplication();

  vtkMRMLMRIBiasFieldCorrectionNode* gadNode =
    vtkMRMLMRIBiasFieldCorrectionNode::New();

  this->Logic->GetMRMLScene()->RegisterNodeClass(gadNode);
  gadNode->Delete();

  this->UIPanel->AddPage ( "MRIBiasFieldCorrection",
      "MRIBiasFieldCorrection", NULL );

  // MODULE GUI FRAME
  // Define your help text and build the help frame here.
  const char *help  = "The MRIBiasFieldCorrection module is built upon the non-uniform non-parametric inhomogeneity correction method of Styner et al. The implementation is inspired by the code of Nick Tustison classes.";

  const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. ";

  vtkKWWidget *page = this->UIPanel->GetPageWidget(
    "MRIBiasFieldCorrection" );
  this->BuildHelpAndAboutFrame( page, help, about );

  vtkSlicerModuleCollapsibleFrame *moduleFrame =
    vtkSlicerModuleCollapsibleFrame::New();
  moduleFrame->SetParent( this->UIPanel->GetPageWidget(
    "MRIBiasFieldCorrection" ) );
  moduleFrame->Create();
  moduleFrame->SetLabelText("MRI Bias Correction Module");
  moduleFrame->ExpandFrame();
  app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
    moduleFrame->GetWidgetName(), this->UIPanel->GetPageWidget(
      "MRIBiasFieldCorrection")->GetWidgetName());

  this->GADNodeSelector->SetNodeClass("vtkMRMLMRIBiasFieldCorrectionNode",
    NULL, NULL, "GADParameters");
  this->GADNodeSelector->SetNewNodeEnabled(1);
  this->GADNodeSelector->NoneEnabledOn();
  this->GADNodeSelector->SetShowHidden(1);
  this->GADNodeSelector->SetParent( moduleFrame->GetFrame() );
  this->GADNodeSelector->Create();
  this->GADNodeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->GADNodeSelector->UpdateMenu();

  this->GADNodeSelector->SetBorderWidth(2);
  this->GADNodeSelector->SetLabelText( "IN Parameters");
  this->GADNodeSelector->SetBalloonHelpString(
    "select a GAD node from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4",
    this->GADNodeSelector->GetWidgetName());

  this->VolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL,
    NULL, NULL);
  this->VolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->VolumeSelector->Create();
  this->VolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->VolumeSelector->UpdateMenu();

  this->VolumeSelector->SetBorderWidth(2);
  this->VolumeSelector->SetLabelText("Input Volume: ");
  this->VolumeSelector->SetBalloonHelpString(
    "select an input volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4",
    this->VolumeSelector->GetWidgetName());

  this->MaskVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode",
      NULL, NULL, NULL);
  this->MaskVolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->MaskVolumeSelector->Create();
  this->MaskVolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->MaskVolumeSelector->UpdateMenu();

  this->MaskVolumeSelector->SetBorderWidth(2);
  this->MaskVolumeSelector->SetLabelText( "Mask Volume: ");
  this->MaskVolumeSelector->SetBalloonHelpString(
    "select an output volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4",
    this->MaskVolumeSelector->GetWidgetName());

  this->StorageVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode",
    NULL, NULL, "GADVolumeStorage");
  this->StorageVolumeSelector->SetNewNodeEnabled(1);
  this->StorageVolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->StorageVolumeSelector->Create();
  this->StorageVolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->StorageVolumeSelector->UpdateMenu();

  this->StorageVolumeSelector->SetBorderWidth(2);
  this->StorageVolumeSelector->SetLabelText( "Preview Volume: ");
  this->StorageVolumeSelector->SetBalloonHelpString(
    "select an output volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4",
    this->StorageVolumeSelector->GetWidgetName());

  this->OutVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode",
    NULL, NULL, "GADVolumeOut");
  this->OutVolumeSelector->SetNewNodeEnabled(1);
  this->OutVolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->OutVolumeSelector->Create();
  this->OutVolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->OutVolumeSelector->UpdateMenu();

  this->OutVolumeSelector->SetBorderWidth(2);
  this->OutVolumeSelector->SetLabelText( "Output Volume: ");
  this->OutVolumeSelector->SetBalloonHelpString(
    "select an output volume from the current mrml scene.");

  app->Script("pack %s -side top -anchor e -padx 20 -pady 4",
    this->OutVolumeSelector->GetWidgetName());

  /////////////////////////////////////////////////////////////////////////
  if( !this->AdvancedParametersFrame )
    {
    this->AdvancedParametersFrame = vtkKWFrameWithLabel::New();
    }

  if( !this->AdvancedParametersFrame->IsCreated() )
    {
    this->AdvancedParametersFrame->SetParent(moduleFrame->GetFrame());
    this->AdvancedParametersFrame->Create();

    this->AdvancedParametersFrame->SetLabelText("Parameters");
    }

  this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 2",
    this->AdvancedParametersFrame->GetWidgetName());

  vtkKWFrame* aparametersFrame = this->AdvancedParametersFrame->GetFrame();
  /////////////////////////////////////////////////////////////////////

  if( !this->ShrinkFactor )
    {
    this->ShrinkFactor = vtkKWScaleWithEntry::New();
    }

  if( !this->ShrinkFactor->IsCreated() )
    {
    this->ShrinkFactor->SetParent(aparametersFrame);
    this->ShrinkFactor->Create();
    this->ShrinkFactor->SetRange(0,10);
    this->ShrinkFactor->SetResolution(1);
    this->ShrinkFactor->SetValue(3);
    this->ShrinkFactor->SetLabelText("Set Shrink Factor");
    }

  this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->ShrinkFactor->GetWidgetName());

  if (!this->MaxNumberOfIterations)
    {
    this->MaxNumberOfIterations = vtkKWScaleWithEntry::New();
    }

  if( !this->MaxNumberOfIterations->IsCreated() )
    {
    this->MaxNumberOfIterations->SetParent(aparametersFrame);
    this->MaxNumberOfIterations->Create();
    this->MaxNumberOfIterations->SetRange(0,100);
    this->MaxNumberOfIterations->SetResolution(1);
    this->MaxNumberOfIterations->SetValue(50);
    this->MaxNumberOfIterations->SetLabelText(
      "Set Max Number of Iterations");
    }

  this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->MaxNumberOfIterations->GetWidgetName());

  if (!this->NumOfFitLev)
    {
    this->NumOfFitLev = vtkKWScaleWithEntry::New();
    }

  if (!this->NumOfFitLev->IsCreated())
    {
    this->NumOfFitLev->SetParent(aparametersFrame);
    this->NumOfFitLev->Create();
    this->NumOfFitLev->SetRange(0,20);
    this->NumOfFitLev->SetResolution(1);
    this->NumOfFitLev->SetValue(4);
    this->NumOfFitLev->SetLabelText("Set Number of Fitting Level");
    }

  this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->NumOfFitLev->GetWidgetName());

  if (!this->WienFilterNoise)
    {
    this->WienFilterNoise = vtkKWScaleWithEntry::New();
    }

  if (!this->WienFilterNoise->IsCreated())
    {
    this->WienFilterNoise->SetParent(aparametersFrame);
    this->WienFilterNoise->Create();
    this->WienFilterNoise->SetRange(0,10);
    this->WienFilterNoise->SetResolution(0.1);
    this->WienFilterNoise->SetValue(0.1);
    this->WienFilterNoise->SetLabelText("Set Wien Filter Noise");
    }

  this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->WienFilterNoise->GetWidgetName());

  if( !this->BiasField )
    {
    this->BiasField = vtkKWScaleWithEntry::New();
    }

  if( !this->BiasField->IsCreated() )
    {
    this->BiasField->SetParent(aparametersFrame);
    this->BiasField->Create();
    this->BiasField->SetRange(0,10);
    this->BiasField->SetResolution(0.05);
    this->BiasField->SetValue(0.15);
    this->BiasField->SetLabelText("Set Bias Field Full.");
    }

  this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->BiasField->GetWidgetName());

  if(!this->Convergence)
    {
    this->Convergence = vtkKWScaleWithEntry::New();
    }

  if(!this->Convergence->IsCreated())
    {
    this->Convergence->SetParent(aparametersFrame);
    this->Convergence->Create();
    this->Convergence->SetRange(0,10);
    this->Convergence->SetResolution(0.001);
    this->Convergence->SetValue(0.001);
    this->Convergence->SetLabelText(
      "Set Convergenceergence Threshold");
    }

  this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->Convergence->GetWidgetName());

  ////////////////////////////////////////////////////////////////////////

  this->CatchButton->SetParent(moduleFrame->GetFrame());
  this->CatchButton->Create();
  this->CatchButton->SetText("Preview");
  this->CatchButton->SetWidth( 8 );

  app->Script("pack %s -side top -anchor e -padx 20 -pady 10",
    this->CatchButton->GetWidgetName());

  this->ApplyButton->SetParent(moduleFrame->GetFrame());
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetWidth ( 8 );

  app->Script("pack %s -side top -anchor e -padx 20 -pady 10",
    this->ApplyButton->GetWidgetName());
}

