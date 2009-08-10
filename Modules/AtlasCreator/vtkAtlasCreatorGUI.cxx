/*=auto=======================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights
  Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkAtlasCreatorGUI.cxx,v $
  Date:      $Date: 2006/03/17 15:10:10 $
  Version:   $Revision: 1.2 $
  Author:    $Sylvain Jaume (MIT)$

=======================================================================auto=*/

#include "vtkAtlasCreatorGUI.h"

#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkKWScale.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWRenderWidget.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerNodeSelectorWidget.h"

#include "vtkMRMLSliceNode.h"

#include "vtkImageData.h"
#include "vtkImageDataGeometryFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkRenderer.h"
#include "vtkActor.h"
#include "vtkImageResample.h"
#include "vtkImageThreshold.h"
#include "vtkImageSeedConnectivity.h"
#include "vtkImageViewer2.h"

//----------------------------------------------------------------------------
vtkAtlasCreatorGUI* vtkAtlasCreatorGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkAtlasCreatorGUI");

  if (ret)
    {
    return (vtkAtlasCreatorGUI*)ret;
    }

  // If the factory was unable to create the object, then create it here.
  return new vtkAtlasCreatorGUI;
}

//----------------------------------------------------------------------------
vtkAtlasCreatorGUI::vtkAtlasCreatorGUI()
{
  this->SecondLabelMapThresholdScale = vtkKWScaleWithEntry::New();
  this->OutputSizeScale              = vtkKWScaleWithEntry::New();
  this->SliceScale                   = vtkKWScaleWithEntry::New();

  this->VolumeSelector               = vtkSlicerNodeSelectorWidget::New();
  this->OutVolumeSelector            = vtkSlicerNodeSelectorWidget::New();
  this->ThirdLabelMapVolumeSelector  = vtkSlicerNodeSelectorWidget::New();
  this->SecondLabelMapVolumeSelector = vtkSlicerNodeSelectorWidget::New();
  this->NodeSelector                 = vtkSlicerNodeSelectorWidget::New();

  this->ImageResample                = vtkImageResample::New();
  this->ImageThreshold               = vtkImageThreshold::New();
  this->ImageSeedConnectivity        = vtkImageSeedConnectivity::New();
  this->ImageViewer2                 = vtkImageViewer2::New();
  this->RenderWidget                 = vtkKWRenderWidget::New();

  this->ApplyButton                  = vtkKWPushButton::New();

  this->VolumeSelectionFrame         = vtkKWFrameWithLabel::New();
  this->ParametersFrame              = vtkKWFrameWithLabel::New();
  this->AdvancedParametersFrame      = vtkKWFrameWithLabel::New();

  this->Logic                        = NULL;
  this->AtlasCreatorNode             = NULL;
}

//----------------------------------------------------------------------------
vtkAtlasCreatorGUI::~vtkAtlasCreatorGUI()
{
  if (this->SecondLabelMapThresholdScale)
    {
    this->SecondLabelMapThresholdScale->SetParent(NULL);
    this->SecondLabelMapThresholdScale->Delete();
    this->SecondLabelMapThresholdScale = NULL;
    }

  if (this->SliceScale)
    {
    this->SliceScale->SetParent(NULL);
    this->SliceScale->Delete();
    this->SliceScale = NULL;
    }

  if (this->ApplyButton)
    {
    this->ApplyButton->SetParent(NULL);
    this->ApplyButton->Delete();
    this->ApplyButton = NULL;
    }

  if (this->OutputSizeScale)
    {
    this->OutputSizeScale->SetParent(NULL);
    this->OutputSizeScale->Delete();
    this->OutputSizeScale = NULL;
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

  if (this->ThirdLabelMapVolumeSelector)
    {
    this->ThirdLabelMapVolumeSelector->SetParent(NULL);
    this->ThirdLabelMapVolumeSelector->Delete();
    this->ThirdLabelMapVolumeSelector = NULL;
    }

  if (this->SecondLabelMapVolumeSelector)
    {
    this->SecondLabelMapVolumeSelector->SetParent(NULL);
    this->SecondLabelMapVolumeSelector->Delete();
    this->SecondLabelMapVolumeSelector = NULL;
    }

  if (this->NodeSelector)
    {
    this->NodeSelector->SetParent(NULL);
    this->NodeSelector->Delete();
    this->NodeSelector = NULL;
    }

  if (this->ImageResample)
    {
    this->ImageResample->Delete();
    this->ImageResample = NULL;
    }

  if (this->ImageThreshold)
    {
    this->ImageThreshold->Delete();
    this->ImageThreshold = NULL;
    }

  if (this->ImageSeedConnectivity)
    {
    this->ImageSeedConnectivity->Delete();
    this->ImageSeedConnectivity = NULL;
    }

  if (this->ImageViewer2)
    {
    this->ImageViewer2->Delete();
    this->ImageViewer2 = NULL;
    }

  if (this->RenderWidget)
    {
    this->RenderWidget->SetParent(NULL);
    this->RenderWidget->Delete();
    this->RenderWidget = NULL;
    }

  if (this->VolumeSelectionFrame)
    {
    this->VolumeSelectionFrame->SetParent(NULL);
    this->VolumeSelectionFrame->Delete();
    this->VolumeSelectionFrame = NULL;
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

  this->SetLogic(NULL);
  vtkSetMRMLNodeMacro(this->AtlasCreatorNode,NULL);
}

//----------------------------------------------------------------------------
void vtkAtlasCreatorGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkAtlasCreatorGUI::AddGUIObservers()
{
  this->VolumeSelector->AddObserver(vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->OutVolumeSelector->AddObserver(vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->ThirdLabelMapVolumeSelector->AddObserver(vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->SecondLabelMapVolumeSelector->AddObserver(vtkSlicerNodeSelectorWidget
      ::NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->NodeSelector->AddObserver(vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->ApplyButton->AddObserver(vtkKWPushButton::InvokedEvent,
      (vtkCommand*)this->GUICallbackCommand );

  this->SliceScale->AddObserver(vtkKWScale::ScaleValueChangingEvent,
      (vtkCommand*)this->GUICallbackCommand);
}

//----------------------------------------------------------------------------
void vtkAtlasCreatorGUI::RemoveGUIObservers()
{
  this->VolumeSelector->RemoveObservers( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->SecondLabelMapVolumeSelector->RemoveObservers(
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand*)this->
      GUICallbackCommand );

  this->OutVolumeSelector->RemoveObservers( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->ThirdLabelMapVolumeSelector->RemoveObservers(
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand*)this->
      GUICallbackCommand );

  this->NodeSelector->RemoveObservers( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->ApplyButton->RemoveObservers( vtkKWPushButton::InvokedEvent,
      (vtkCommand*)this->GUICallbackCommand );

  this->SliceScale->RemoveObservers( vtkKWScale::ScaleValueChangingEvent,
      (vtkCommand*)this->GUICallbackCommand );
}

//----------------------------------------------------------------------------
void vtkAtlasCreatorGUI::ProcessGUIEvents( vtkObject *caller, unsigned long
    event, void *callData )
{
  vtkKWPushButton     *button = vtkKWPushButton::SafeDownCast(caller);
  vtkKWScaleWithEntry *scale  = vtkKWScaleWithEntry::SafeDownCast(caller);

  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::
    SafeDownCast(caller);

  if( selector == this->VolumeSelector && event == vtkSlicerNodeSelectorWidget
      ::NodeSelectedEvent && this->VolumeSelector->GetSelected() != NULL )
    {
    this->UpdateMRML();
    this->UpdateGUI();
    }
  else if( selector == this->OutVolumeSelector && event ==
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->OutVolumeSelector->GetSelected() != NULL )
    {
    this->UpdateMRML();
    this->UpdateGUI();
    }
  else if( selector == this->SecondLabelMapVolumeSelector && event ==
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->SecondLabelMapVolumeSelector->GetSelected() != NULL )
    {
    this->UpdateMRML();
    this->UpdateGUI();
    }
  else if( selector == this->ThirdLabelMapVolumeSelector && event ==
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->ThirdLabelMapVolumeSelector->GetSelected() != NULL )
    {
    this->UpdateMRML();
    this->UpdateGUI();
    }

  if( selector == this->NodeSelector && event ==
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->NodeSelector->GetSelected() != NULL )
    {
    vtkMRMLAtlasCreatorNode *node = vtkMRMLAtlasCreatorNode::SafeDownCast(
        this->NodeSelector->GetSelected());
    this->Logic->SetAndObserveAtlasCreatorNode(node);
    vtkSetAndObserveMRMLNodeMacro(this->AtlasCreatorNode,node);
    this->UpdateGUI();
    }
  else if( button == this->ApplyButton && event == vtkKWPushButton::
      InvokedEvent )
    {
    this->UpdateMRML();
    this->Logic->Apply();
    this->UpdateGUI();
    }
  else if( (scale == this->SliceScale || scale == this->
        SecondLabelMapThresholdScale) && event == vtkKWScale::
      ScaleValueChangingEvent )
    {
    double sliceNormalized     = this->SliceScale->GetValue();
    double thresholdNormalized = this->SecondLabelMapThresholdScale->
      GetValue();

    std::string volumeId = this->AtlasCreatorNode->
      GetFirstLabelMapRef();

    vtkMRMLScalarVolumeNode *inVolume = vtkMRMLScalarVolumeNode::SafeDownCast(
        this->GetMRMLScene()->GetNodeByID(volumeId) );

    if (inVolume == NULL)
      {
      vtkErrorMacro("No input volume");
      return;
      }

    if (inVolume->GetImageData() == NULL)
      {
      vtkErrorMacro("No image data for the input volume");
      return;
      }

    int numComponents = inVolume->GetImageData()->
      GetNumberOfScalarComponents();

    if (numComponents != 1)
      {
      vtkErrorMacro("Input image has " << numComponents << " components. "
          << "This module only operates on 1-component images.");
      return;
      }

    bool isNew = this->ImageResample->GetInput() != inVolume->GetImageData();
    int extent[6];

    if (isNew)
      {
      inVolume->GetImageData()->GetWholeExtent(extent);

      if (extent[1]-extent[0] > 63)
        {
        double factor = 63.0 / (extent[1]-extent[0]);
        this->ImageResample->SetAxisMagnificationFactor(0,factor);
        }

      if (extent[3]-extent[2] > 63)
        {
        double factor = 63.0 / (extent[3]-extent[2]);
        this->ImageResample->SetAxisMagnificationFactor(0,factor);
        }

      if (extent[5]-extent[4] > 63)
        {
        double factor = 63.0 / (extent[5]-extent[4]);
        this->ImageResample->SetAxisMagnificationFactor(0,factor);
        }

      this->ImageResample->SetInput(inVolume->GetImageData());
      this->ImageResample->Update();
      }

    int dim[3];
    double scalarRange[2];

    this->ImageResample->GetOutput()->GetWholeExtent(extent);
    this->ImageResample->GetOutput()->GetDimensions(dim);
    this->ImageResample->GetOutput()->GetScalarRange(scalarRange);

    int slice = extent[4] + (int)((extent[5]-extent[4]) * sliceNormalized + 0.5);
    double threshold = scalarRange[0] + (scalarRange[1]-scalarRange[0]) *
      thresholdNormalized;

    vtkErrorMacro("\nsliceNormalized " << sliceNormalized
        << " -> slice " << slice
        << "\nthresholdNormalized " << thresholdNormalized
        << " -> threshold " << threshold
        << "\nextent " << extent[0] << " " << extent[1] << " " << extent[2]
        << " " << extent[3] << " " << extent[4] << " " <<extent[5]
        << "\nscalarRange " << scalarRange[0] << " " << scalarRange[1]);

    this->ImageThreshold->SetInput(this->ImageResample->GetOutput());
    this->ImageThreshold->ThresholdByUpper(threshold);

    this->ImageSeedConnectivity->SetInput(this->ImageThreshold->GetOutput());
    this->ImageSeedConnectivity->RemoveAllSeeds();

    int seedExtent[6];

    seedExtent[0] = extent[0]     + dim[0] / 4;
    seedExtent[1] = seedExtent[0] + dim[0] / 2;

    seedExtent[2] = extent[2]     + dim[1] / 4;
    seedExtent[3] = seedExtent[2] + dim[1] / 2;

    seedExtent[4] = extent[4]     + dim[2] / 4;
    seedExtent[5] = seedExtent[4] + dim[2] / 2;

    for(int k=seedExtent[4]; k<=seedExtent[5]; k++)
      {
      for(int j=seedExtent[2]; j<=seedExtent[3]; j++)
        {
        for(int i=seedExtent[0]; i<=seedExtent[1]; i++)
          {
          this->ImageSeedConnectivity->AddSeed(i,j,k);
          }
        }
      }

    this->ImageViewer2->SetInput(this->ImageSeedConnectivity->GetOutput());
    this->ImageViewer2->SetSlice(slice);
    this->ImageViewer2->SetRenderWindow(this->RenderWidget->
        GetRenderWindow());
    this->ImageViewer2->SetRenderer(this->RenderWidget->GetRenderer());
    this->ImageViewer2->SetupInteractor(this->RenderWidget->GetRenderWindow()
        ->GetInteractor());

    this->RenderWidget->ResetCamera();

    if (isNew)
      {
      double viewUp[3];
      this->RenderWidget->GetRenderer()->GetActiveCamera()->GetViewUp(viewUp);

      viewUp[0] = -viewUp[0];
      viewUp[1] = -viewUp[1];
      viewUp[2] = -viewUp[2];

      this->RenderWidget->GetRenderer()->GetActiveCamera()->SetViewUp(viewUp);
      }

    this->RenderWidget->GetRenderer()->GetActiveCamera()->Zoom(1.7);
    this->RenderWidget->Render();
    }
}

//----------------------------------------------------------------------------
void vtkAtlasCreatorGUI::UpdateMRML()
{
  vtkMRMLAtlasCreatorNode* node = this->GetAtlasCreatorNode();

  if (node == NULL)
    {
    // no parameter node selected yet, create new
    this->NodeSelector->SetSelectedNew("vtkMRMLAtlasCreatorNode");
    this->NodeSelector->ProcessNewNodeCommand("vtkMRMLAtlasCreatorNode",
        "AtlasCreatorFieldParameters");
    node = vtkMRMLAtlasCreatorNode::SafeDownCast(this->NodeSelector->
        GetSelected());

    // set an observe new node in Logic
    this->Logic->SetAndObserveAtlasCreatorNode(node);
    vtkSetAndObserveMRMLNodeMacro(this->AtlasCreatorNode,node);
   }

  // save node parameters for Undo
  this->GetLogic()->GetMRMLScene()->SaveStateForUndo(node);

  double maskThreshold = this->SecondLabelMapThresholdScale->GetValue();
  double outputSize    = this->OutputSizeScale->GetValue();

  node->SetSecondLabelMapThreshold(maskThreshold);
  node->SetOutputSize(outputSize);

  if (this->VolumeSelector->GetSelected() != NULL)
    {
    node->SetFirstLabelMapRef(this->VolumeSelector->GetSelected()->GetID());
    }

  if (this->OutVolumeSelector->GetSelected() != NULL)
    {
    node->SetOutputVolumeRef(this->OutVolumeSelector->GetSelected()->GetID());
    }

  if (this->ThirdLabelMapVolumeSelector->GetSelected() != NULL)
    {
    node->SetThirdLabelMapVolumeRef(this->ThirdLabelMapVolumeSelector->
        GetSelected()->GetID());
    }

  if (this->SecondLabelMapVolumeSelector->GetSelected() != NULL)
    {
    node->SetSecondLabelMapVolumeRef(this->SecondLabelMapVolumeSelector->
        GetSelected()->GetID());
    }
}

//----------------------------------------------------------------------------
void vtkAtlasCreatorGUI::UpdateGUI()
{
  vtkMRMLAtlasCreatorNode* node = this->GetAtlasCreatorNode();

  if (node)
    {
    // Set GUI widget from parameter node
    vtkDebugMacro("Update GUI");

    double maskThreshold        = node->GetSecondLabelMapThreshold();
    double outputSize           = node->GetOutputSize();

    this->SecondLabelMapThresholdScale->SetValue(maskThreshold);
    this->OutputSizeScale->SetValue(outputSize);
    }
}

//----------------------------------------------------------------------------
void vtkAtlasCreatorGUI::ProcessMRMLEvents(vtkObject *caller, unsigned long
    event, void *callData)
{
  // if parameter node has been changed externally, update GUI widgets with
  // new values
  vtkMRMLAtlasCreatorNode* node = vtkMRMLAtlasCreatorNode::SafeDownCast(
      caller);

  if (node != NULL && this->GetAtlasCreatorNode() == node)
    {
    this->UpdateGUI();
    vtkDebugMacro("MRML changed");
    }
}

//----------------------------------------------------------------------------
void vtkAtlasCreatorGUI::BuildGUI()
{
  vtkMRMLAtlasCreatorNode* node = vtkMRMLAtlasCreatorNode::New();

  this->Logic->GetMRMLScene()->RegisterNodeClass(node);
  node->Delete();

  this->UIPanel->AddPage("AtlasCreator","AtlasCreator", NULL);

  // MODULE GUI FRAME
  // Define your help text and build the help frame here.
  std::string help  = "The AtlasCreator module builds a statistical atlas ";
  help += "using pre-defined label maps.";

  std::string about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, ";
  about += "and the Slicer Community. See <a>http://www.slicer.org</a> for ";
  about += "details. ";

  vtkKWWidget *page = this->UIPanel->GetPageWidget("AtlasCreator");
  this->BuildHelpAndAboutFrame( page, help.c_str(), about.c_str() );

  vtkSlicerModuleCollapsibleFrame *moduleFrame =
    vtkSlicerModuleCollapsibleFrame::New();
  moduleFrame->SetParent(this->UIPanel->GetPageWidget("AtlasCreator"));
  moduleFrame->Create();
  moduleFrame->SetLabelText("Atlas Creator Module");
  moduleFrame->ExpandFrame();
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
    moduleFrame->GetWidgetName(), this->UIPanel->GetPageWidget("AtlasCreator")
    ->GetWidgetName());

  this->NodeSelector->SetNodeClass("vtkMRMLAtlasCreatorNode", NULL, NULL,
      "AtlasCreatorParameters");
  this->NodeSelector->SetNewNodeEnabled(1);
  this->NodeSelector->NoneEnabledOn();
  this->NodeSelector->SetShowHidden(1);
  this->NodeSelector->SetParent(moduleFrame->GetFrame());
  this->NodeSelector->Create();
  this->NodeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->NodeSelector->UpdateMenu();

  this->NodeSelector->SetBorderWidth(2);
  this->NodeSelector->SetLabelText("In Parameters");
  this->NodeSelector->SetBalloonHelpString(
    "select a node from the current mrml scene.");
/*
  this->Script("pack %s -side top -anchor e -padx 20 -pady 4",
    this->NodeSelector->GetWidgetName());
*/
  this->VolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL,
    NULL, NULL);
  this->VolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->VolumeSelector->Create();
  this->VolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->VolumeSelector->UpdateMenu();

  this->VolumeSelector->SetBorderWidth(2);
  this->VolumeSelector->SetLabelText("First Label Map Input:");
  this->VolumeSelector->SetBalloonHelpString(
    "select an input volume from the current mrml scene.");
  this->Script("pack %s -side top -anchor e -padx 20 -pady 4",
    this->VolumeSelector->GetWidgetName());

  this->SecondLabelMapVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode",
      NULL, NULL, NULL);
  this->SecondLabelMapVolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->SecondLabelMapVolumeSelector->Create();
  this->SecondLabelMapVolumeSelector->SetMRMLScene(this->Logic->
      GetMRMLScene());
  this->SecondLabelMapVolumeSelector->UpdateMenu();

  this->SecondLabelMapVolumeSelector->SetBorderWidth(2);
  this->SecondLabelMapVolumeSelector->SetLabelText("Second Label Map Input:");
  this->SecondLabelMapVolumeSelector->SetBalloonHelpString(
    "select an output volume from the current mrml scene.");
  this->Script("pack %s -side top -anchor e -padx 20 -pady 4",
    this->SecondLabelMapVolumeSelector->GetWidgetName());

  this->ThirdLabelMapVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode",
      NULL, NULL, "VolumeThirdLabelMap");
  this->ThirdLabelMapVolumeSelector->SetNewNodeEnabled(1);
  this->ThirdLabelMapVolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->ThirdLabelMapVolumeSelector->Create();
  this->ThirdLabelMapVolumeSelector->SetMRMLScene(this->Logic->
      GetMRMLScene());
  this->ThirdLabelMapVolumeSelector->UpdateMenu();

  this->ThirdLabelMapVolumeSelector->SetBorderWidth(2);
  this->ThirdLabelMapVolumeSelector->SetLabelText("Third Label Map Input:");
  this->ThirdLabelMapVolumeSelector->SetBalloonHelpString(
    "select an output volume from the current mrml scene.");
  this->Script("pack %s -side top -anchor e -padx 20 -pady 4",
    this->ThirdLabelMapVolumeSelector->GetWidgetName());

  this->OutVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL,
      "VolumeOut");
  this->OutVolumeSelector->SetNewNodeEnabled(1);
  this->OutVolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->OutVolumeSelector->Create();
  this->OutVolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->OutVolumeSelector->UpdateMenu();

  this->OutVolumeSelector->SetBorderWidth(2);
  this->OutVolumeSelector->SetLabelText("Statistical Atlas Output:");
  this->OutVolumeSelector->SetBalloonHelpString(
    "select an output volume from the current mrml scene.");

  this->Script("pack %s -side top -anchor e -padx 20 -pady 4",
    this->OutVolumeSelector->GetWidgetName());

  //--------------------------------------------------------------------------
  if (this->AdvancedParametersFrame == NULL)
    {
    this->AdvancedParametersFrame = vtkKWFrameWithLabel::New();
    }

  if (!this->AdvancedParametersFrame->IsCreated())
    {
    this->AdvancedParametersFrame->SetParent(moduleFrame->GetFrame());
    this->AdvancedParametersFrame->Create();
    this->AdvancedParametersFrame->SetLabelText("Parameters");
    }

  this->Script("pack %s -side top -anchor nw -fill both -padx 2 -pady 2",
    this->AdvancedParametersFrame->GetWidgetName());

  vtkKWFrame* aparametersFrame = this->AdvancedParametersFrame->GetFrame();

  //--------------------------------------------------------------------------
  if (this->SecondLabelMapThresholdScale == NULL)
    {
    this->SecondLabelMapThresholdScale = vtkKWScaleWithEntry::New();
    }

  if (!this->SecondLabelMapThresholdScale->IsCreated())
    {
    this->SecondLabelMapThresholdScale->SetParent(aparametersFrame);
    this->SecondLabelMapThresholdScale->Create();
    this->SecondLabelMapThresholdScale->SetRange(0,1);
    this->SecondLabelMapThresholdScale->SetResolution(0.01);
    this->SecondLabelMapThresholdScale->SetValue(0.5);
    this->SecondLabelMapThresholdScale->SetLabelText("Set Threshold");
    }

  this->Script("pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->SecondLabelMapThresholdScale->GetWidgetName());

  //--------------------------------------------------------------------------
  if (this->OutputSizeScale == NULL)
    {
    this->OutputSizeScale = vtkKWScaleWithEntry::New();
    }

  if (!this->OutputSizeScale->IsCreated())
    {
    this->OutputSizeScale->SetParent(aparametersFrame);
    this->OutputSizeScale->Create();
    this->OutputSizeScale->SetRange(0.0,1.0);
    this->OutputSizeScale->SetResolution(0.01);
    this->OutputSizeScale->SetValue(1.0);
    this->OutputSizeScale->SetLabelText("Set Output Size");
    }

  this->Script("pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->OutputSizeScale->GetWidgetName());

  //--------------------------------------------------------------------------
  if (this->SliceScale == NULL)
    {
    this->SliceScale = vtkKWScaleWithEntry::New();
    }

  if (!this->SliceScale->IsCreated())
    {
    this->SliceScale->SetParent(aparametersFrame);
    this->SliceScale->Create();
    this->SliceScale->SetRange(0,1);
    this->SliceScale->SetResolution(0.01);
    this->SliceScale->SetValue(0.5);
    this->SliceScale->SetLabelText("Set Slice for Preview");
    }

  this->Script("pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->SliceScale->GetWidgetName());

  //--------------------------------------------------------------------------
  this->RenderWidget->SetParent(moduleFrame->GetFrame());
  this->RenderWidget->Create();
  this->RenderWidget->SetWidth(200);
  this->RenderWidget->SetHeight(100);
  this->RenderWidget->GetRenderer()->GetRenderWindow()->GetInteractor()->
    Disable();

  this->Script("pack %s -expand n -fill none -anchor c",this->RenderWidget->
      GetWidgetName());

  this->ImageResample->SetInterpolationModeToNearestNeighbor();

  this->ImageThreshold->ThresholdByUpper(127.5);
  this->ImageThreshold->SetInValue(255);
  this->ImageThreshold->SetOutValue(0);
  this->ImageThreshold->SetOutputScalarTypeToUnsignedChar();

  this->ImageSeedConnectivity->SetInputConnectValue(255);
  this->ImageSeedConnectivity->SetOutputConnectedValue(255);
  this->ImageSeedConnectivity->SetOutputUnconnectedValue(0);
  this->ImageSeedConnectivity->SetDimensionality(2);

  //--------------------------------------------------------------------------
  this->ApplyButton->SetParent(moduleFrame->GetFrame());
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetWidth(8);

  this->Script("pack %s -side top -anchor e -padx 20 -pady 10",
    this->ApplyButton->GetWidgetName());

  moduleFrame->Delete();
}

