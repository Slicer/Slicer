/*=auto=======================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
  Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRIBiasFieldCorrectionGUI.cxx,v $
  Date:      $Date: 2006/03/17 15:10:10 $
  Version:   $Revision: 1.2 $

=======================================================================auto=*/

#include "vtkMRIBiasFieldCorrectionGUI.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerModuleCollapsibleFrame.h"

// KWWidgets includes
#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWScale.h"
#include "vtkKWMenu.h"
#include "vtkKWEntry.h"
#include "vtkKWFrame.h"
#include "vtkKWPushButton.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWSpinBoxWithLabel.h"
#include "vtkKWSpinBox.h"
#include "vtkKWScale.h"

// MRML includes
#include "vtkMRMLSliceNode.h"

// VTK includes
#include "vtkCommand.h"
#include "vtkObjectFactory.h"

// STL includes
#include <string>
#include <iostream>
#include <sstream>

//----------------------------------------------------------------------------
vtkMRIBiasFieldCorrectionGUI* vtkMRIBiasFieldCorrectionGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret =
    vtkObjectFactory::CreateInstance("vtkMRIBiasFieldCorrectionGUI");
  if(ret)
  {
    return (vtkMRIBiasFieldCorrectionGUI*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRIBiasFieldCorrectionGUI;
}


//----------------------------------------------------------------------------
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

  this->ParametersFrame = vtkKWFrameWithLabel::New();
  this->AdvancedParametersFrame = vtkKWFrameWithLabel::New();

  this->ShrinkFactor = vtkKWScaleWithEntry::New();
  this->MaxNumOfIt   = vtkKWScaleWithEntry::New();
  this->NumOfFitLev  = vtkKWScaleWithEntry::New();
  this->WienFilNoi   = vtkKWScaleWithEntry::New();
  this->BiasField    = vtkKWScaleWithEntry::New();
  this->Conv         = vtkKWScaleWithEntry::New();

  this->SlidePositionScale = vtkKWScaleWithEntry::New();
  this->BiasIntensityScale = vtkKWScaleWithEntry::New();
}

//----------------------------------------------------------------------------
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

  if( this->MaxNumOfIt )
  {
    this->MaxNumOfIt->SetParent(NULL);
    this->MaxNumOfIt->Delete();
    this->MaxNumOfIt = NULL;
  }

  if( this->NumOfFitLev )
  {
    this->NumOfFitLev->SetParent(NULL);
    this->NumOfFitLev->Delete();
    this->NumOfFitLev = NULL;
  }

  if( this->WienFilNoi )
  {
    this->WienFilNoi->SetParent(NULL);
    this->WienFilNoi->Delete();
    this->WienFilNoi = NULL;
  }

  if( this->BiasField )
  {
    this->BiasField->SetParent(NULL);
    this->BiasField->Delete();
    this->BiasField = NULL;
  }

  if( this->Conv )
  {
    this->Conv->SetParent(NULL);
    this->Conv->Delete();
    this->Conv = NULL;
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

  this->SetLogic(NULL);
  vtkSetMRMLNodeMacro(this->MRIBiasFieldCorrectionNode, NULL);
}

//----------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::AddGUIObservers()
{
  this->VolumeSelector->AddObserver(
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
      (vtkCommand*) this->GUICallbackCommand );

  this->OutVolumeSelector->AddObserver(
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
      (vtkCommand*) this->GUICallbackCommand );

  this->StorageVolumeSelector->AddObserver(
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
      (vtkCommand*) this->GUICallbackCommand );

  this->MaskVolumeSelector->AddObserver(
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
      (vtkCommand*) this->GUICallbackCommand );

  this->GADNodeSelector->AddObserver(
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
      (vtkCommand*) this->GUICallbackCommand );

  this->ApplyButton->AddObserver(
      vtkKWPushButton::InvokedEvent,
      (vtkCommand*) this->GUICallbackCommand );

  this->CatchButton->AddObserver(
      vtkKWPushButton::InvokedEvent,
      (vtkCommand*) this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::RemoveGUIObservers()
{
  this->VolumeSelector->RemoveObservers(
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
      (vtkCommand*) this->GUICallbackCommand );

  this->MaskVolumeSelector->RemoveObservers(
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
      (vtkCommand*) this->GUICallbackCommand );

  this->OutVolumeSelector->RemoveObservers(
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
      (vtkCommand*) this->GUICallbackCommand );

  this->StorageVolumeSelector->RemoveObservers(
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
      (vtkCommand*) this->GUICallbackCommand );

  this->GADNodeSelector->RemoveObservers(
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
      (vtkCommand*) this->GUICallbackCommand );

  this->ApplyButton->RemoveObservers(
      vtkKWPushButton::InvokedEvent,
      (vtkCommand*) this->GUICallbackCommand );

  this->CatchButton->RemoveObservers(
      vtkKWPushButton::InvokedEvent,
      (vtkCommand*) this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::ProcessGUIEvents(
    vtkObject *caller,
    unsigned long event,
    void *vtkNotUsed(callData))
{
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast(caller);
  vtkSlicerNodeSelectorWidget *selector =
    vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

  if (selector == this->VolumeSelector &&
      event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->VolumeSelector->GetSelected() != NULL)
  {
    this->UpdateMRML();
    this->UpdateGUI();
  }
  else if (selector == this->OutVolumeSelector &&
      event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->OutVolumeSelector->GetSelected() != NULL)
  {
    this->UpdateMRML();
    this->UpdateGUI();
  }
  else if (selector == this->MaskVolumeSelector &&
      event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->MaskVolumeSelector->GetSelected() != NULL)
  {
    this->UpdateMRML();
    this->UpdateGUI();
  }
  else if (selector == this->StorageVolumeSelector &&
      event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->StorageVolumeSelector->GetSelected() != NULL)
  {
    this->UpdateMRML();
    this->UpdateGUI();
  }
  if (selector == this->GADNodeSelector &&
      event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->GADNodeSelector->GetSelected() != NULL)
  {
    vtkMRMLMRIBiasFieldCorrectionNode* n =
      vtkMRMLMRIBiasFieldCorrectionNode::SafeDownCast(
          this->GADNodeSelector->GetSelected());

    this->Logic->SetAndObserveMRIBiasFieldCorrectionNode(n);
    vtkSetAndObserveMRMLNodeMacro( this->MRIBiasFieldCorrectionNode, n);
    this->UpdateGUI();
  }
  else if (b == this->ApplyButton && event == vtkKWPushButton::InvokedEvent )
  {
    this->UpdateMRML();
    this->Logic->Apply();
    this->UpdateGUI();
  }
  else if (b == this->CatchButton && event == vtkKWPushButton::InvokedEvent )
  {
    this->UpdateMRML();

    vtkSlicerApplicationGUI *applicationGUI = this->GetApplicationGUI();

    double oldSliceSetting[3];
    oldSliceSetting[0] = double(applicationGUI->GetMainSliceGUI("Red")->
        GetSliceController()->GetOffsetScale()->GetValue());
    oldSliceSetting[1] = double(applicationGUI->GetMainSliceGUI("Yellow")->
        GetSliceController()->GetOffsetScale()->GetValue());
    oldSliceSetting[2] = double(applicationGUI->GetMainSliceGUI("Green")->
        GetSliceController()->GetOffsetScale()->GetValue());

    double oldSliceSetting2[3];
    oldSliceSetting2[0] = double(applicationGUI->GetApplicationLogic()->
        GetSliceLogic("Red")->GetSliceOffset());
    oldSliceSetting2[1] = double(applicationGUI->GetApplicationLogic()->
        GetSliceLogic("Yellow")->GetSliceOffset());
    oldSliceSetting2[2] = double(applicationGUI->GetApplicationLogic()->
        GetSliceLogic("Green")->GetSliceOffset());

    // GET DATA FROM SLICES
    vtkImageData *image = vtkImageData::New();
    image->DeepCopy( applicationGUI->GetApplicationLogic()->
        GetSliceLogic("Red")->GetBackgroundLayer()->GetSlice()->GetOutput());

   int extent[6];
   double bounds[6];

   image->UpdateInformation();
   image->GetBounds(bounds);
   image->GetWholeExtent(extent);

   double ijkPt[4];
   double xyPt[4] = { 10, 10, 0, 1 };

   vtkTransform *xyToijk = applicationGUI->GetApplicationLogic()->
      GetSliceLogic("Red")->GetBackgroundLayer()->GetXYToIJKTransform();

   xyToijk->MultiplyPoint(xyPt,ijkPt);

   vtkMRMLSliceNode *snode = applicationGUI->GetMainSliceGUI("Red")->
      GetSliceNode();

  unsigned int dimensions[3];
  snode->GetDimensions(dimensions);
  this->Logic->SliceProcess(xyToijk,dimensions[0],dimensions[1]);
  }
}

//---------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::UpdateMRML()
{
  vtkMRMLMRIBiasFieldCorrectionNode* n =
    this->GetMRIBiasFieldCorrectionNode();
  if (n == NULL)
  {
    // no parameter node selected yet, create new
    this->GADNodeSelector->
      SetSelectedNew("vtkMRMLMRIBiasFieldCorrectionNode");
    this->GADNodeSelector->ProcessNewNodeCommand(
        "vtkMRMLMRIBiasFieldCorrectionNode", "GADParameters");

    n = vtkMRMLMRIBiasFieldCorrectionNode::SafeDownCast(
        this->GADNodeSelector->GetSelected());

    // set an observe new node in Logic
    this->Logic->SetAndObserveMRIBiasFieldCorrectionNode(n);
    vtkSetAndObserveMRMLNodeMacro(this->MRIBiasFieldCorrectionNode, n);
  }

  // save node parameters for Undo
  this->GetLogic()->GetMRMLScene()->SaveStateForUndo(n);

  n->SetShrink((int)floor(this->ShrinkFactor->GetValue()));
  n->SetMax((int)floor(this->MaxNumOfIt->GetValue()));
  n->SetNum(this->NumOfFitLev->GetValue());
  n->SetWien(this->WienFilNoi->GetValue());
  n->SetField(this->BiasField->GetValue());
  n->SetCon(this->Conv->GetValue());

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
    n->SetStorageVolumeRef(
        this->StorageVolumeSelector->GetSelected()->GetID());
  }
  if (this->MaskVolumeSelector->GetSelected() != NULL)
  {
    n->SetMaskVolumeRef(this->MaskVolumeSelector->GetSelected()->GetID());
  }
}

//---------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::UpdateGUI()
{
  vtkMRMLMRIBiasFieldCorrectionNode* n =
    this->GetMRIBiasFieldCorrectionNode();

  if (n)
  {
    // set GUI widgest from parameter node

    this->ShrinkFactor->SetValue(n->GetShrink());
    this->MaxNumOfIt->SetValue(n->GetMax());
    this->NumOfFitLev->SetValue(n->GetNum());
    this->WienFilNoi->SetValue(n->GetWien());
    this->BiasField->SetValue(n->GetField());
    this->Conv->SetValue(n->GetCon());
  }
}

//---------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::ProcessMRMLEvents(
    vtkObject *caller, unsigned long vtkNotUsed(event), void *vtkNotUsed(callData))
{
  // if parameter node has been changed externally, update GUI widgets with
  // new values
  vtkMRMLMRIBiasFieldCorrectionNode* node =
    vtkMRMLMRIBiasFieldCorrectionNode::SafeDownCast(caller);

  if (node != NULL && this->GetMRIBiasFieldCorrectionNode() == node)
  {
    this->UpdateGUI();
    //this->Logic->Cut();
  }
}

//---------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::BuildGUI()
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  vtkMRMLMRIBiasFieldCorrectionNode* gadNode =
    vtkMRMLMRIBiasFieldCorrectionNode::New();

  this->Logic->GetMRMLScene()->RegisterNodeClass(gadNode);
  gadNode->Delete();

  this->UIPanel->AddPage("MRIBiasFieldCorrection",
      "MRIBiasFieldCorrection", NULL);

  // Define your help text and build the help frame here.
  std::string help = "Documentation and screenshots are available at ";
  help += "<a>http://wiki.slicer.org/slicerWiki/index.php/Modules:";
  help += "MRIBiasFieldCorrection-Documentation-3.5</a>\n\nInstructions:\n";
  help += "- Create a mask volume using the Editor module (the Threshold";
  help += " tool should give a good result)\n";
  help += "- Select the MRIBiasFieldCorrection module\n";
  help += "- Select the Input Volume\n";
  help += "- Select the Mask Volume\n";
  help += "- In Preview Volume, select Create New Volume\n";
  help += "- In Output Volume, select Create New Volume\n";
  help += "- Modify parameter values if desired (The default parameters gave";
  help += " good results during our experiments)\n";
  help += "- Click on Apply\n\n";
  help += "Based on our experiments, it takes 32 min to process a ";
  help += "512x512x30 voxel volume on a Mac laptop.";

  std::string about = "The MRIBiasFieldCorrection module was developed by ";
  about += "Nicolas Rannou (BWH) and Sylvain Jaume (MIT).\n";
  about += "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the ";
  about += "Slicer Community. See <a>http://www.slicer.org</a> for details. ";

  vtkKWWidget *page = this->UIPanel->GetPageWidget("MRIBiasFieldCorrection");
  this->BuildHelpAndAboutFrame( page, help.c_str(), about.c_str() );

  vtkSlicerModuleCollapsibleFrame *moduleFrame =
    vtkSlicerModuleCollapsibleFrame::New();
  moduleFrame->SetParent(
      this->UIPanel->GetPageWidget("MRIBiasFieldCorrection") );
  moduleFrame->Create();
  moduleFrame->SetLabelText("MRI Bias Correction Module");
  moduleFrame->ExpandFrame();
  app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
      moduleFrame->GetWidgetName(),
      this->UIPanel->GetPageWidget("MRIBiasFieldCorrection")->
      GetWidgetName());

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


  this->VolumeSelector->
    SetNodeClass("vtkMRMLScalarVolumeNode",NULL,NULL,NULL);
  this->VolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->VolumeSelector->Create();
  this->VolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->VolumeSelector->UpdateMenu();

  this->VolumeSelector->SetBorderWidth(2);
  this->VolumeSelector->SetLabelText("Input Volume:");
  this->VolumeSelector->SetBalloonHelpString(
      "select an input volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4",
      this->VolumeSelector->GetWidgetName());

  this->MaskVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode",
      NULL, NULL, NULL);
  this->MaskVolumeSelector->SetParent(moduleFrame->GetFrame());
  this->MaskVolumeSelector->Create();
  this->MaskVolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->MaskVolumeSelector->UpdateMenu();

  this->MaskVolumeSelector->SetBorderWidth(2);
  this->MaskVolumeSelector->SetLabelText("Mask Volume:");
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
  this->StorageVolumeSelector->SetLabelText("Preview Volume: ");
  this->StorageVolumeSelector->SetBalloonHelpString(
      "select an output volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4",
      this->StorageVolumeSelector->GetWidgetName());

  this->OutVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode",NULL,NULL,
      "GADVolumeOut");
  this->OutVolumeSelector->SetNewNodeEnabled(1);
  this->OutVolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->OutVolumeSelector->Create();
  this->OutVolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->OutVolumeSelector->UpdateMenu();

  this->OutVolumeSelector->SetBorderWidth(2);
  this->OutVolumeSelector->SetLabelText("Output Volume:");
  this->OutVolumeSelector->SetBalloonHelpString(
      "select an output volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4",
      this->OutVolumeSelector->GetWidgetName());

  if (!this->AdvancedParametersFrame)
  {
    this->AdvancedParametersFrame = vtkKWFrameWithLabel::New();
  }
  if (!this->AdvancedParametersFrame->IsCreated())
  {
    this->AdvancedParametersFrame->SetParent(moduleFrame->GetFrame());
    this->AdvancedParametersFrame->Create();
    //this->AdvancedParametersFrame->CollapseFrame();
    this->AdvancedParametersFrame->SetLabelText("Parameters");
  }

  this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 2", 
    this->AdvancedParametersFrame->GetWidgetName());

  vtkKWFrame* aparametersFrame = this->AdvancedParametersFrame->GetFrame();
////////////////////////////////////////////////////////////////////////////////////////////

  if (!this->ShrinkFactor)
  {
    this->ShrinkFactor = vtkKWScaleWithEntry::New();
  }

  if (!this->ShrinkFactor->IsCreated())
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


  if( !this->MaxNumOfIt )
  {
    this->MaxNumOfIt = vtkKWScaleWithEntry::New();
  }
  if( !this->MaxNumOfIt->IsCreated() )
  {
    this->MaxNumOfIt->SetParent(aparametersFrame);
    this->MaxNumOfIt->Create();
    this->MaxNumOfIt->SetRange(0,100);
    this->MaxNumOfIt->SetResolution(1);
    this->MaxNumOfIt->SetValue(20);
    this->MaxNumOfIt->SetLabelText("Set Max Number of Iterations");
  }

  this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6", 
    this->MaxNumOfIt->GetWidgetName());

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


  if (!this->WienFilNoi)
  {
    this->WienFilNoi = vtkKWScaleWithEntry::New();
  }
  if (!this->WienFilNoi->IsCreated())
  {
    this->WienFilNoi->SetParent(aparametersFrame);
    this->WienFilNoi->Create();
    this->WienFilNoi->SetRange(0,10);
    this->WienFilNoi->SetResolution(0.1);
    this->WienFilNoi->SetValue(0.1);
    this->WienFilNoi->SetLabelText("Set Wien Filter Noise");
  }

  this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->WienFilNoi->GetWidgetName());


  if (!this->BiasField)
  {
    this->BiasField = vtkKWScaleWithEntry::New();
  }
  if (!this->BiasField->IsCreated())
  {
    this->BiasField->SetParent(aparametersFrame);
    this->BiasField->Create();
    this->BiasField->SetRange(0,10);
    this->BiasField->SetResolution(0.05);
    this->BiasField->SetValue(4.1);
    this->BiasField->SetLabelText("Set Bias Field Full.");
  }

  this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6", 
    this->BiasField->GetWidgetName());

  if (!this->Conv)
  {
    this->Conv = vtkKWScaleWithEntry::New();
  }
  if (!this->Conv->IsCreated())
  {
    this->Conv->SetParent(aparametersFrame);
    this->Conv->Create();
    this->Conv->SetRange(0,10);
    this->Conv->SetResolution(0.001);
    this->Conv->SetValue(0.001);
    this->Conv->SetLabelText("Set Convergence Threshold");
  }

  this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->Conv->GetWidgetName());

  this->CatchButton->SetParent(moduleFrame->GetFrame());
  this->CatchButton->Create();
  this->CatchButton->SetText("Preview");
  this->CatchButton->SetWidth(8);
  app->Script("pack %s -side top -anchor e -padx 20 -pady 10",
      this->CatchButton->GetWidgetName());

  this->ApplyButton->SetParent(moduleFrame->GetFrame());
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetWidth(8);

  app->Script("pack %s -side top -anchor e -padx 20 -pady 10",
      this->ApplyButton->GetWidgetName());

  moduleFrame->Delete();
}
