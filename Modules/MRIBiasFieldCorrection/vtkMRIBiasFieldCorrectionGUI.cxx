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
  this->NumberOfIterationsScale    = vtkKWScaleWithEntry::New();
  this->MaskThresholdScale         = vtkKWScaleWithEntry::New();
  this->OutputSizeScale            = vtkKWScaleWithEntry::New();
  this->NumberOfIterationsScale    = vtkKWScaleWithEntry::New();
  this->NumberOfFittingLevelsScale = vtkKWScaleWithEntry::New();
  this->WienerFilterNoiseScale     = vtkKWScaleWithEntry::New();
  this->BiasFieldScale             = vtkKWScaleWithEntry::New();
  this->ConvergenceThresholdScale  = vtkKWScaleWithEntry::New();
  this->SliceScale                 = vtkKWScaleWithEntry::New();

  this->VolumeSelector             = vtkSlicerNodeSelectorWidget::New();
  this->OutVolumeSelector          = vtkSlicerNodeSelectorWidget::New();
  this->PreviewVolumeSelector      = vtkSlicerNodeSelectorWidget::New();
  this->MaskVolumeSelector         = vtkSlicerNodeSelectorWidget::New();
  this->NodeSelector               = vtkSlicerNodeSelectorWidget::New();

  this->ImageResample              = vtkImageResample::New();
  this->ImageThreshold             = vtkImageThreshold::New();
  this->ImageSeedConnectivity      = vtkImageSeedConnectivity::New();
  this->ImageViewer2               = vtkImageViewer2::New();
  this->RenderWidget               = vtkKWRenderWidget::New();

  this->ApplyButton                = vtkKWPushButton::New();
  this->PreviewButton              = vtkKWPushButton::New();

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

  if (this->SliceScale)
    {
    this->SliceScale->SetParent(NULL);
    this->SliceScale->Delete();
    this->SliceScale = NULL;
    }

  if (this->PreviewButton)
    {
    this->PreviewButton->SetParent(NULL);
    this->PreviewButton->Delete();
    this->PreviewButton = NULL;
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

  if (this->PreviewVolumeSelector)
    {
    this->PreviewVolumeSelector->SetParent(NULL);
    this->PreviewVolumeSelector->Delete();
    this->PreviewVolumeSelector = NULL;
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
  vtkSetMRMLNodeMacro(this->MRIBiasFieldCorrectionNode,NULL);
}

//-------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//-------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::AddGUIObservers()
{
  this->VolumeSelector->AddObserver( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->OutVolumeSelector->AddObserver( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->PreviewVolumeSelector->AddObserver( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->MaskVolumeSelector->AddObserver( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->NodeSelector->AddObserver( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->ApplyButton->AddObserver( vtkKWPushButton::InvokedEvent,
      (vtkCommand*)this->GUICallbackCommand );

  this->PreviewButton->AddObserver( vtkKWPushButton::InvokedEvent,
      (vtkCommand*)this->GUICallbackCommand );

  this->ConvergenceThresholdScale->AddObserver( vtkKWScale::
      ScaleValueChangingEvent, (vtkCommand*)this->GUICallbackCommand);

  this->SliceScale->AddObserver( vtkKWScale::ScaleValueChangingEvent,
      (vtkCommand*)this->GUICallbackCommand);
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

  this->PreviewVolumeSelector->RemoveObservers(
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand*)
      this->GUICallbackCommand );

  this->NodeSelector->RemoveObservers( vtkSlicerNodeSelectorWidget::
      NodeSelectedEvent, (vtkCommand*)this->GUICallbackCommand );

  this->ApplyButton->RemoveObservers( vtkKWPushButton::InvokedEvent,
      (vtkCommand*)this->GUICallbackCommand );

  this->PreviewButton->RemoveObservers( vtkKWPushButton::InvokedEvent,
      (vtkCommand*)this->GUICallbackCommand );

  this->ConvergenceThresholdScale->RemoveObservers( vtkKWScale::
      ScaleValueChangingEvent, (vtkCommand*)this->GUICallbackCommand );

  this->SliceScale->RemoveObservers( vtkKWScale::ScaleValueChangingEvent,
      (vtkCommand*)this->GUICallbackCommand );
}

//-------------------------------------------------------------------------
void vtkMRIBiasFieldCorrectionGUI::ProcessGUIEvents( vtkObject *caller,
  unsigned long event, void *callData )
{
  vtkKWPushButton     *button = vtkKWPushButton::SafeDownCast(caller);
  vtkKWScaleWithEntry *scale  = vtkKWScaleWithEntry::SafeDownCast(caller);

  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::
    SafeDownCast(caller);

  if( selector == this->VolumeSelector && event ==
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->VolumeSelector->GetSelected() != NULL )
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
  else if( selector == this->MaskVolumeSelector && event ==
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->MaskVolumeSelector->GetSelected() != NULL )
    {
    this->UpdateMRML();
    this->UpdateGUI();
    }
  else if( selector == this->PreviewVolumeSelector && event ==
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->PreviewVolumeSelector->GetSelected() != NULL )
    {
    this->UpdateMRML();
    this->UpdateGUI();
    }
  if( selector == this->NodeSelector && event ==
      vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
      this->NodeSelector->GetSelected() != NULL )
    {
    vtkMRMLMRIBiasFieldCorrectionNode *node =
      vtkMRMLMRIBiasFieldCorrectionNode::SafeDownCast(
          this->NodeSelector->GetSelected());
    this->Logic->SetAndObserveMRIBiasFieldCorrectionNode(node);
    vtkSetAndObserveMRMLNodeMacro(this->MRIBiasFieldCorrectionNode,node);
    this->UpdateGUI();
    }
  else if( button == this->ApplyButton && event == vtkKWPushButton::
      InvokedEvent )
    {
    this->UpdateMRML();
    this->Logic->Apply();
    this->UpdateGUI();
    }
  else if( button == this->PreviewButton && event == vtkKWPushButton::
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
    image->Delete();
    }
  else if( (scale == this->SliceScale || scale == this->MaskThresholdScale) &&
      event == vtkKWScale::ScaleValueChangingEvent )
    {
    double sliceNormalized     = this->SliceScale->GetValue();
    double thresholdNormalized = this->MaskThresholdScale->GetValue();

    //vtkMRMLMRIBiasFieldCorrectionNode *node =
    //vtkMRMLMRIBiasFieldCorrectionNode::SafeDownCast(
    //  this->NodeSelector->GetSelected());

    std::string volumeId = this->MRIBiasFieldCorrectionNode->
      GetInputVolumeRef();

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

    int slice = extent[4] + (extent[5]-extent[4]) * sliceNormalized;
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
  /*
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

    vtkImageData *demoImage = node->GetDemoImage();
    this->RenderWidget();
    this->Logic           % inVolume
    }
  */
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
  double outputSize           = this->OutputSizeScale->GetValue();
  double numIterations        = this->NumberOfIterationsScale->GetValue();
  double numFittingLevels     = this->NumberOfFittingLevelsScale->GetValue();
  double WienerFilterNoise    = this->WienerFilterNoiseScale->GetValue();
  double biasField            = this->BiasFieldScale->GetValue();
  double convergenceThreshold = this->ConvergenceThresholdScale->GetValue();

  node->SetMaskThreshold(maskThreshold);
  node->SetOutputSize(outputSize);
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

  if (this->PreviewVolumeSelector->GetSelected() != NULL)
    {
    node->SetPreviewVolumeRef(this->PreviewVolumeSelector->GetSelected()->
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

  if (node)
    {
    // Set GUI widget from parameter node
    vtkDebugMacro("Update GUI");

    double maskThreshold        = node->GetMaskThreshold();
    double outputSize           = node->GetOutputSize();
    double numIterations        = node->GetNumberOfIterations();
    double numFittingLevels     = node->GetNumberOfFittingLevels();
    double WienerFilterNoise    = node->GetWienerFilterNoise();
    double biasField            = node->GetBiasField();
    double convergenceThreshold = node->GetConvergenceThreshold();

    this->MaskThresholdScale->SetValue(maskThreshold);
    this->OutputSizeScale->SetValue(outputSize);
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
  //vtkSlicerApplication *app = (vtkSlicerApplication*)this->GetApplication();

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
  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
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
  this->Script("pack %s -side top -anchor e -padx 20 -pady 4",
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
  this->Script("pack %s -side top -anchor e -padx 20 -pady 4",
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
  this->Script("pack %s -side top -anchor e -padx 20 -pady 4",
    this->MaskVolumeSelector->GetWidgetName());

  this->PreviewVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode",
    NULL, NULL, "VolumePreview");
  this->PreviewVolumeSelector->SetNewNodeEnabled(1);
  this->PreviewVolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->PreviewVolumeSelector->Create();
  this->PreviewVolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->PreviewVolumeSelector->UpdateMenu();

  this->PreviewVolumeSelector->SetBorderWidth(2);
  this->PreviewVolumeSelector->SetLabelText("Preview Volume: ");
  this->PreviewVolumeSelector->SetBalloonHelpString(
    "select an output volume from the current mrml scene.");
  this->Script("pack %s -side top -anchor e -padx 20 -pady 4",
    this->PreviewVolumeSelector->GetWidgetName());

  this->OutVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode",
    NULL, NULL, "VolumeOut");
  this->OutVolumeSelector->SetNewNodeEnabled(1);
  this->OutVolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->OutVolumeSelector->Create();
  this->OutVolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->OutVolumeSelector->UpdateMenu();

  this->OutVolumeSelector->SetBorderWidth(2);
  this->OutVolumeSelector->SetLabelText("Output Volume: ");
  this->OutVolumeSelector->SetBalloonHelpString(
    "select an output volume from the current mrml scene.");

  this->Script("pack %s -side top -anchor e -padx 20 -pady 4",
    this->OutVolumeSelector->GetWidgetName());

  //-------------------------------------------------------------------------
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

  //-------------------------------------------------------------------------
  if (this->MaskThresholdScale == NULL)
    {
    this->MaskThresholdScale = vtkKWScaleWithEntry::New();
    }

  if (!this->MaskThresholdScale->IsCreated())
    {
    this->MaskThresholdScale->SetParent(aparametersFrame);
    this->MaskThresholdScale->Create();
    this->MaskThresholdScale->SetRange(0,1);
    this->MaskThresholdScale->SetResolution(0.01);
    this->MaskThresholdScale->SetValue(0.5);
    this->MaskThresholdScale->SetLabelText("Set Mask Threshold");
    }

  this->Script("pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->MaskThresholdScale->GetWidgetName());

  //-------------------------------------------------------------------------
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

  //-------------------------------------------------------------------------
  if (this->NumberOfIterationsScale == NULL)
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

  //-------------------------------------------------------------------------
  if (this->NumberOfFittingLevelsScale == NULL)
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

  //-------------------------------------------------------------------------
  if (this->WienerFilterNoiseScale == NULL)
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
    this->ConvergenceThresholdScale->SetRange(0.0,1.0);//10);
    this->ConvergenceThresholdScale->SetResolution(0.001);
    this->ConvergenceThresholdScale->SetValue(0.001);
    this->ConvergenceThresholdScale->SetLabelText(
        "Set Convergence Threshold");
    }

  this->Script("pack %s -side top -anchor e -expand n -padx 2 -pady 6",
    this->ConvergenceThresholdScale->GetWidgetName());

  //-------------------------------------------------------------------------
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
  this->PreviewButton->SetParent(moduleFrame->GetFrame());
  this->PreviewButton->Create();
  this->PreviewButton->SetText("Preview");
  this->PreviewButton->SetWidth(8);

  this->Script("pack %s -side top -anchor e -padx 20 -pady 10",
    this->PreviewButton->GetWidgetName());

  //--------------------------------------------------------------------------
  this->ApplyButton->SetParent(moduleFrame->GetFrame());
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetWidth(8);

  this->Script("pack %s -side top -anchor e -padx 20 -pady 10",
    this->ApplyButton->GetWidgetName());
}

