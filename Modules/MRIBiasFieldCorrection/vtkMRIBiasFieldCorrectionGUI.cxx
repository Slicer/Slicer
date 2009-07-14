/*=auto====================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
  Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRIBiasFieldCorrectionGUI.cxx,v $
  Date:      $Date: 2006/03/17 15:10:10 $
  Version:   $Revision: 1.2 $
  Author:    $Nicolas Rannou (BWH), Sylvain Jaume (MIT)

====================================================================auto=*/

#include "vtkMRIBiasFieldCorrectionGUI.h"

#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkKWScale.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerNodeSelectorWidget.h"

#include "vtkMRMLSliceNode.h"

//-------------------------------------------------------------------------
vtkMRIBiasFieldCorrectionGUI* vtkMRIBiasFieldCorrectionGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance(
      "vtkMRIBiasFieldCorrectionGUI");

  if (ret)
    {
    return (vtkMRIBiasFieldCorrectionGUI*)ret;
    }

  // If the factory was unable to create the object, then create it here.
  return new vtkMRIBiasFieldCorrectionGUI;
}

//-------------------------------------------------------------------------
vtkMRIBiasFieldCorrectionGUI::vtkMRIBiasFieldCorrectionGUI()
{
  this->BiasIntensityScale = NULL;
  this->NumberOfIterationsScale    = vtkKWScaleWithEntry::New();
  this->SagittalScale              = vtkKWScaleWithEntry::New();
  this->CoronalScale               = vtkKWScaleWithEntry::New();
  this->AxialScale                 = vtkKWScaleWithEntry::New();
  this->MaskThresholdScale         = vtkKWScaleWithEntry::New();
  this->ShrinkFactorScale          = vtkKWScaleWithEntry::New();
  this->NumberOfIterationsScale    = vtkKWScaleWithEntry::New();
  this->NumberOfFittingLevelsScale = vtkKWScaleWithEntry::New();
  this->WienerFilterNoiseScale     = vtkKWScaleWithEntry::New();
  this->BiasFieldScale             = vtkKWScaleWithEntry::New();
  this->ConvergenceThresholdScale  = vtkKWScaleWithEntry::New();

  this->VolumeSelector             = vtkSlicerNodeSelectorWidget::New();
  this->OutVolumeSelector          = vtkSlicerNodeSelectorWidget::New();
  this->StorageVolumeSelector      = vtkSlicerNodeSelectorWidget::New();
  this->MaskVolumeSelector         = vtkSlicerNodeSelectorWidget::New();
  this->NodeSelector               = vtkSlicerNodeSelectorWidget::New();

  this->ApplyButton                = vtkKWPushButton::New();
  this->CatchButton                = vtkKWPushButton::New();

  this->VolumeSelectionFrame       = vtkKWFrameWithLabel::New();
  this->ParametersFrame            = vtkKWFrameWithLabel::New();
  this->AdvancedParametersFrame    = vtkKWFrameWithLabel::New();

  this->Logic                      = NULL;
  this->MRIBiasFieldCorrectionNode = NULL;
}

//-------------------------------------------------------------------------
vtkMRIBiasFieldCorrectionGUI::~vtkMRIBiasFieldCorrectionGUI()
{
  if (this->BiasIntensityScale)
    {
    this->BiasIntensityScale->SetParent(NULL);
    this->BiasIntensityScale->Delete();
    this->BiasIntensityScale = NULL;
    }

  if (this->NumberOfIterationsScale)
    {
    this->NumberOfIterationsScale->SetParent(NULL);
    this->NumberOfIterationsScale->Delete();
    this->NumberOfIterationsScale = NULL;
    }

  if (this->NumberOfFittingLevelsScale)
    {
    this->NumberOfFittingLevelsScale->SetParent(NULL);
    this->NumberOfFittingLevelsScale->Delete();
    this->NumberOfFittingLevelsScale = NULL;
    }

  if (this->WienerFilterNoiseScale)
    {
    this->WienerFilterNoiseScale->SetParent(NULL);
    this->WienerFilterNoiseScale->Delete();
    this->WienerFilterNoiseScale = NULL;
    }

  if (this->BiasFieldScale)
  {
    this->BiasFieldScale->SetParent(NULL);
    this->BiasFieldScale->Delete();
    this->BiasFieldScale = NULL;
    }

  if (this->ConvergenceThresholdScale)
    {
    this->ConvergenceThresholdScale->SetParent(NULL);
    this->ConvergenceThresholdScale->Delete();
    this->ConvergenceThresholdScale = NULL;
    }

  if (this->MaskThresholdScale)
    {
    this->MaskThresholdScale->SetParent(NULL);
    this->MaskThresholdScale->Delete();
    this->MaskThresholdScale = NULL;
    }

  if (this->ShrinkFactorScale)
    {
    this->ShrinkFactorScale->SetParent(NULL);
    this->ShrinkFactorScale->Delete();
    this->ShrinkFactorScale = NULL;
    }

  if (this->ParametersFrame)
    {
    this->ParametersFrame->SetParent(NULL);
    this->ParametersFrame->Delete();
    this->ParametersFrame = NULL;
    }

  if (this->AdvancedParametersFrame)
    {
    this->AdvancedParametersFrame->SetParent(NULL);
    this->AdvancedParametersFrame->Delete();
    this->AdvancedParametersFrame = NULL;
    }

  if (this->VolumeSelector)
    {
    this->VolumeSelector->SetParent(NULL);
    this->VolumeSelector->Delete();
    this->VolumeSelector = NULL;
    }

  if (this->OutVolumeSelector)
    {
    this->OutVolumeSelector->SetParent(NULL);
    this->OutVolumeSelector->Delete();
    this->OutVolumeSelector = NULL;
    }

  if (this->StorageVolumeSelector)
    {
    this->StorageVolumeSelector->SetParent(NULL);
    this->StorageVolumeSelector->Delete();
    this->StorageVolumeSelector = NULL;
    }

  if (this->MaskVolumeSelector)
    {
    this->MaskVolumeSelector->SetParent(NULL);
    this->MaskVolumeSelector->Delete();
    this->MaskVolumeSelector = NULL;
    }

  if (this->NodeSelector)
    {
    this->NodeSelector->SetParent(NULL);
    this->NodeSelector->Delete();
    this->NodeSelector = NULL;
    }

  if (this->ApplyButton)
    {
    this->ApplyButton->SetParent(NULL);
    this->ApplyButton->Delete();
    this->ApplyButton = NULL;
    }

  if (this->CatchButton)
    {
    this->CatchButton->SetParent(NULL);
    this->CatchButton->Delete();
    this->CatchButton = NULL;
    }

  if (this->VolumeSelectionFrame)
    {
    this->VolumeSelectionFrame->SetParent(NULL);
    this->VolumeSelectionFrame->Delete();
    this->VolumeSelectionFrame = NULL;
    }

  if (this->SagittalScale)
    {
    this->SagittalScale->SetParent(NULL);
    this->SagittalScale->Delete();
    this->SagittalScale = NULL;
    }

  if (this->CoronalScale)
    {
    this->CoronalScale->SetParent(NULL);
    this->CoronalScale->Delete();
    this->CoronalScale = NULL;
    }

  if( this->AxialScale )
    {
    this->AxialScale->SetParent(NULL);
    this->AxialScale->Delete();
    this->AxialScale = NULL;
    }

  this->SetLogic(NULL);
  vtkSetMRMLNodeMacro(this->MRIBiasFieldCorrectionNode,NULL);
}

//-------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::PrintSelf(ostream& os, vtkIndent
    indent)
{
}

//-------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::AddGUIObservers()
{
  this->VolumeSelector->AddObserver( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->OutVolumeSelector->AddObserver( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->StorageVolumeSelector->AddObserver( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->MaskVolumeSelector->AddObserver( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->NodeSelector->AddObserver( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->ApplyButton->AddObserver( vtkKWPushButton::InvokedEvent,
      (vtkCommand*)this->GUICallbackCommand );

  this->CatchButton->AddObserver( vtkKWPushButton::InvokedEvent,
      (vtkCommand*)this->GUICallbackCommand );
}

//-------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::RemoveGUIObservers()
{
  this->VolumeSelector->RemoveObservers( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->MaskVolumeSelector->RemoveObservers( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->OutVolumeSelector->RemoveObservers( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->StorageVolumeSelector->RemoveObservers(
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand*)
      this->GUICallbackCommand );

  this->NodeSelector->RemoveObservers( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->ApplyButton->RemoveObservers( vtkKWPushButton::InvokedEvent,
      (vtkCommand*)this->GUICallbackCommand );

  this->CatchButton->RemoveObservers( vtkKWPushButton::InvokedEvent,
      (vtkCommand*)this->GUICallbackCommand );
}

//-------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::ProcessGUIEvents( vtkObject *caller,
  unsigned long event, void *callData )
{
  vtkKWPushButton *button = vtkKWPushButton::SafeDownCast(caller);

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
  if( selector == this->NodeSelector && event ==
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->NodeSelector->GetSelected() != NULL)
    {
    vtkMRMLMRIBiasFieldCorrectionNode *node =
      vtkMRMLMRIBiasFieldCorrectionNode::SafeDownCast(
          this->NodeSelector->GetSelected());
    this->Logic->SetAndObserveMRIBiasFieldCorrectionNode(node);
    vtkSetAndObserveMRMLNodeMacro( this->MRIBiasFieldCorrectionNode,
        node);
    this->UpdateGUI();
    }
  else if( button == this->ApplyButton && event == vtkKWPushButton::
      InvokedEvent )
    {
    this->UpdateMRML();
    this->Logic->Apply();
    this->UpdateGUI();
    }
  else if( button == this->CatchButton && event == vtkKWPushButton::
      InvokedEvent )
    {
    this->UpdateMRML();

    vtkSlicerApplicationGUI *applicationGUI = this->GetApplicationGUI();

    // Get data from slices

    vtkImageData* image = vtkImageData::New();
    image->DeepCopy( applicationGUI->GetApplicationLogic()->GetSliceLogic(
          "Red")->GetBackgroundLayer()->GetSlice()->GetOutput());
    image->UpdateInformation();

    vtkTransform* xyToijk = vtkTransform::New();
    xyToijk = applicationGUI->GetApplicationLogic()->GetSliceLogic("Red")->
      GetBackgroundLayer()->GetXYToIJKTransform();

    vtkMRMLSliceNode *snode = applicationGUI->GetMainSliceGUI("Red")->
      GetSliceNode();

    unsigned int dimensions[3];
    snode->GetDimensions(dimensions);
    this->Logic->SliceProcess(xyToijk,dimensions[0],dimensions[1]);
    }
}

//-------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::UpdateMRML()
{
  vtkMRMLMRIBiasFieldCorrectionNode* node = this->
    GetMRIBiasFieldCorrectionNode();

  if (node == NULL)
    {
    // no parameter node selected yet, create new
    this->NodeSelector->SetSelectedNew(
      "vtkMRMLMRIBiasFieldCorrectionNode");
    this->NodeSelector->ProcessNewNodeCommand(
      "vtkMRMLMRIBiasFieldCorrectionNode", "BiasFieldParameters");
    node = vtkMRMLMRIBiasFieldCorrectionNode::SafeDownCast(this->
      NodeSelector->GetSelected());

    // set an observe new node in Logic
    this->Logic->SetAndObserveMRIBiasFieldCorrectionNode(node);
    vtkSetAndObserveMRMLNodeMacro(this->MRIBiasFieldCorrectionNode,node);
   }

  // save node parameters for Undo
  this->GetLogic()->GetMRMLScene()->SaveStateForUndo(node);

  double maskThreshold        = this->MaskThresholdScale->GetValue();
  double shrinkFactor         = this->ShrinkFactorScale->GetValue();
  double numIterations        = this->NumberOfIterationsScale->GetValue();
  double numFittingLevels     = this->NumberOfFittingLevelsScale->GetValue();
  double WienerFilterNoise    = this->WienerFilterNoiseScale->GetValue();
  double biasField            = this->BiasFieldScale->GetValue();
  double convergenceThreshold = this->ConvergenceThresholdScale->GetValue();

  node->SetMaskThreshold(maskThreshold);
  node->SetShrinkFactor(shrinkFactor);
  node->SetNumberOfIterations((unsigned int)numIterations);
  node->SetNumberOfFittingLevels((unsigned int)numFittingLevels);
  node->SetWienerFilterNoise(WienerFilterNoise);
  node->SetBiasField(biasField);
  node->SetConvergenceThreshold(convergenceThreshold);

  if (this->VolumeSelector->GetSelected() != NULL)
    {
    node->SetInputVolumeRef(this->VolumeSelector->GetSelected()->GetID());
    }

  if (this->OutVolumeSelector->GetSelected() != NULL)
    {
    node->SetOutputVolumeRef(this->OutVolumeSelector->GetSelected()->GetID());
    }

  if (this->StorageVolumeSelector->GetSelected() != NULL)
    {
    node->SetStorageVolumeRef(this->StorageVolumeSelector->GetSelected()->
      GetID());
    }

  if (this->MaskVolumeSelector->GetSelected() != NULL)
    {
    node->SetMaskVolumeRef(this->MaskVolumeSelector->GetSelected()->GetID());
    }
}

//--------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::UpdateGUI()
{
  vtkMRMLMRIBiasFieldCorrectionNode* node = this->
    GetMRIBiasFieldCorrectionNode();

  if (node != NULL)
    {
    // Set GUI widget from parameter node
    vtkDebugMacro("Update GUI");

    double shrinkFactor         = node->GetShrinkFactor();
    double numIterations        = node->GetNumberOfIterations();
    double numFittingLevels     = node->GetNumberOfFittingLevels();
    double WienerFilterNoise    = node->GetWienerFilterNoise();
    double biasField            = node->GetBiasField();
    double convergenceThreshold = node->GetConvergenceThreshold();

    this->ShrinkFactorScale->SetValue(shrinkFactor);
    this->NumberOfIterationsScale->SetValue(numIterations);
    this->NumberOfFittingLevelsScale->SetValue(numFittingLevels);
    this->WienerFilterNoiseScale->SetValue(WienerFilterNoise);
    this->BiasFieldScale->SetValue(biasField);
    this->ConvergenceThresholdScale->SetValue(convergenceThreshold);
    }
}

//--------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::ProcessMRMLEvents(vtkObject *caller,
  unsigned long event, void *callData)
{
  // if parameter node has been changed externally, update GUI widgets with
  // new values
  vtkMRMLMRIBiasFieldCorrectionNode* node =
    vtkMRMLMRIBiasFieldCorrectionNode::SafeDownCast(caller);

  if (node != NULL && this->GetMRIBiasFieldCorrectionNode() == node)
    {
    this->UpdateGUI();
    //this->Logic->Cut();
    vtkDebugMacro("MRML changed");
    }
}

//--------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::BuildGUI()
{
  vtkSlicerApplication *app = (vtkSlicerApplication*)this->GetApplication();

  vtkMRMLMRIBiasFieldCorrectionNode* node =
    vtkMRMLMRIBiasFieldCorrectionNode::New();

  this->Logic->GetMRMLScene()->RegisterNodeClass(node);
  node->Delete();

  this->UIPanel->AddPage("MRIBiasFieldCorrection","MRIBiasFieldCorrection",
      NULL);

  // MODULE GUI FRAME
  // Define your help text and build the help frame here.
  std::string help  = "The MRIBiasFieldCorrection module is built upon ";
  help += "the non-uniform non-parametric inhomogeneity correction method of";
  help += " Styner et al.  The implementation is inspired by the code of ";
  help += "Nick Tustison classes.";

  std::string about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, ";
  about += "and the Slicer Community. See <a>http://www.slicer.org</a> for ";
  about += "details. ";

  vtkKWWidget *page = this->UIPanel->GetPageWidget(
      "MRIBiasFieldCorrection");
  this->BuildHelpAndAboutFrame( page, help.c_str(), about.c_str() );

  vtkSlicerModuleCollapsibleFrame *moduleFrame =
    vtkSlicerModuleCollapsibleFrame::New();
  moduleFrame->SetParent(this->UIPanel->GetPageWidget(
        "MRIBiasFieldCorrection") );
  moduleFrame->Create();
  moduleFrame->SetLabelText("MRI Bias Correction Module");
  moduleFrame->ExpandFrame();
  app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
    moduleFrame->GetWidgetName(), this->UIPanel->GetPageWidget(
      "MRIBiasFieldCorrection")->GetWidgetName());

  this->NodeSelector->SetNodeClass("vtkMRMLMRIBiasFieldCorrectionNode",
    NULL, NULL, "BiasFieldParameters");
  this->NodeSelector->SetNewNodeEnabled(1);
  this->NodeSelector->NoneEnabledOn();
  this->NodeSelector->SetShowHidden(1);
  this->NodeSelector->SetParent( moduleFrame->GetFrame() );
  this->NodeSelector->Create();
  this->NodeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->NodeSelector->UpdateMenu();

  this->NodeSelector->SetBorderWidth(2);
  this->NodeSelector->SetLabelText( "IN Parameters");
  this->NodeSelector->SetBalloonHelpString(
    "select a bias field node from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4",
    this->NodeSelector->GetWidgetName());

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
  this->MaskVolumeSelector->SetLabelText("Mask Volume: ");
  this->MaskVolumeSelector->SetBalloonHelpString(
    "select an output volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4",
    this->MaskVolumeSelector->GetWidgetName());

  this->StorageVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode",
    NULL, NULL, "VolumeStorage");
  this->StorageVolumeSelector->SetNewNodeEnabled(1);
  this->StorageVolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->StorageVolumeSelector->Create();
  this->StorageVolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->StorageVolumeSelector->UpdateMenu();

  this->StorageVolumeSelector->SetBorderWidth(2);
  this->StorageVolumeSelector->SetLabelText("Preview Volume: ");
  this->StorageVolumeSelector->SetBalloonHelpString(
    "select an output volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4",
    this->StorageVolumeSelector->GetWidgetName());

  this->OutVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode",
    NULL, NULL, "VolumeOut");
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

  if( !this->ShrinkFactorScale )
    {
    this->ShrinkFactorScale = vtkKWScaleWithEntry::New();
    }

  if( !this->ShrinkFactorScale->IsCreated() )
    {
    this->ShrinkFactorScale->SetParent(aparametersFrame);
    this->ShrinkFactorScale->Create();
    this->ShrinkFactorScale->SetRange(0,10);
    this->ShrinkFactorScale->SetResolution(1);
    this->ShrinkFactorScale->SetValue(3);
    this->ShrinkFactorScale->SetLabelText("Set Shrink Factor");
    }

  this->Script("pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->ShrinkFactorScale->GetWidgetName());

  if (!this->NumberOfIterationsScale)
    {
    this->NumberOfIterationsScale = vtkKWScaleWithEntry::New();
    }

  if (!this->NumberOfIterationsScale->IsCreated())
    {
    this->NumberOfIterationsScale->SetParent(aparametersFrame);
    this->NumberOfIterationsScale->Create();
    this->NumberOfIterationsScale->SetRange(0,100);
    this->NumberOfIterationsScale->SetResolution(1);
    this->NumberOfIterationsScale->SetValue(50);
    this->NumberOfIterationsScale->SetLabelText("Set Number of Iterations");
    }

  this->Script("pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->NumberOfIterationsScale->GetWidgetName());

  if (!this->NumberOfFittingLevelsScale)
    {
    this->NumberOfFittingLevelsScale = vtkKWScaleWithEntry::New();
    }

  if (!this->NumberOfFittingLevelsScale->IsCreated())
    {
    this->NumberOfFittingLevelsScale->SetParent(aparametersFrame);
    this->NumberOfFittingLevelsScale->Create();
    this->NumberOfFittingLevelsScale->SetRange(0,20);
    this->NumberOfFittingLevelsScale->SetResolution(1);
    this->NumberOfFittingLevelsScale->SetValue(4);
    this->NumberOfFittingLevelsScale->SetLabelText(
        "Set Number of Fitting Levels");
    }

  this->Script("pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->NumberOfFittingLevelsScale->GetWidgetName());

  if (!this->WienerFilterNoiseScale)
    {
    this->WienerFilterNoiseScale = vtkKWScaleWithEntry::New();
    }

  if (!this->WienerFilterNoiseScale->IsCreated())
    {
    this->WienerFilterNoiseScale->SetParent(aparametersFrame);
    this->WienerFilterNoiseScale->Create();
    this->WienerFilterNoiseScale->SetRange(0,10);
    this->WienerFilterNoiseScale->SetResolution(0.1);
    this->WienerFilterNoiseScale->SetValue(0.1);
    this->WienerFilterNoiseScale->SetLabelText("Set Wiener Filter Noise");
    }

  this->Script("pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->WienerFilterNoiseScale->GetWidgetName());

  if (!this->BiasFieldScale)
    {
    this->BiasFieldScale = vtkKWScaleWithEntry::New();
    }

  if (!this->BiasFieldScale->IsCreated())
    {
    this->BiasFieldScale->SetParent(aparametersFrame);
    this->BiasFieldScale->Create();
    this->BiasFieldScale->SetRange(0,10);
    this->BiasFieldScale->SetResolution(0.05);
    this->BiasFieldScale->SetValue(0.15);
    this->BiasFieldScale->SetLabelText("Set Bias Field Full.");
    }

  this->Script("pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->BiasFieldScale->GetWidgetName());

  if (!this->ConvergenceThresholdScale)
    {
    this->ConvergenceThresholdScale = vtkKWScaleWithEntry::New();
    }

  if (!this->ConvergenceThresholdScale->IsCreated())
    {
    this->ConvergenceThresholdScale->SetParent(aparametersFrame);
    this->ConvergenceThresholdScale->Create();
    this->ConvergenceThresholdScale->SetRange(0,10);
    this->ConvergenceThresholdScale->SetResolution(0.001);
    this->ConvergenceThresholdScale->SetValue(0.001);
    this->ConvergenceThresholdScale->SetLabelText(
        "Set Convergence Threshold");
    }

  this->Script("pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->ConvergenceThresholdScale->GetWidgetName());

  //--------------------------------------------------------------------------
  this->CatchButton->SetParent(moduleFrame->GetFrame());
  this->CatchButton->Create();
  this->CatchButton->SetText("Preview");
  this->CatchButton->SetWidth(8);

  app->Script("pack %s -side top -anchor e -padx 20 -pady 10",
    this->CatchButton->GetWidgetName());

  //--------------------------------------------------------------------------
  this->ApplyButton->SetParent(moduleFrame->GetFrame());
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetWidth(8);

  app->Script("pack %s -side top -anchor e -padx 20 -pady 10",
    this->ApplyButton->GetWidgetName());
}

