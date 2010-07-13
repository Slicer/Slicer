//Slicer
#include "vtkSlicerVolumeRenderingHelper.h"
#include "vtkVolumeRenderingGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerColorDisplayWidget.h"
#include "vtkSlicerVolumePropertyWidget.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkSlicerVisibilityIcons.h"
#include "vtkSlicerVRMenuButtonColorMode.h"

//VTK
#include "vtkObjectFactory.h"
#include "vtkVolume.h"
#include "vtkTimerLog.h"
#include "vtkRenderer.h"
#include "vtkPointData.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkTexture.h"
#include "vtkImageGradientMagnitude.h"
#include "vtkInteractorStyle.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkPlanes.h"
#include "vtkProperty.h"
#include "vtkVolumeProperty.h"
#include "vtkMatrix4x4.h"

//KWWidgets
#include "vtkKWHistogram.h"
#include "vtkKWHistogramSet.h"
#include "vtkKWProgressGauge.h"
#include "vtkKWEvent.h"
#include "vtkKWMenuButtonWithSpinButtonsWithLabel.h"
#include "vtkKWProgressDialog.h"
#include "vtkKWScale.h"
#include "vtkKWScaleWithLabel.h"
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWCheckButton.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWColorTransferFunctionEditor.h"
#include "vtkKWPiecewiseFunctionEditor.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWPushButtonWithLabel.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWEntry.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWRange.h"
#include "vtkKWLoadSaveButton.h"

#include "vtkSlicerROIDisplayWidget.h"

//Compiler
#include <math.h>

vtkCxxRevisionMacro(vtkSlicerVolumeRenderingHelper, "$Revision: 1.46 $");
vtkStandardNewMacro(vtkSlicerVolumeRenderingHelper);

vtkSlicerVolumeRenderingHelper::vtkSlicerVolumeRenderingHelper(void)
{
  this->Gui = NULL;

  this->GUICallbackCommand = vtkCallbackCommand::New();
  this->GUICallbackCommand->SetClientData( reinterpret_cast<void *>(this) );
  this->GUICallbackCommand->SetCallback(vtkSlicerVolumeRenderingHelper::GUIEventsCallback);

  this->MB_PerformanceControl = NULL;
  
  this->MB_GPURayCastTechnique = NULL;

  this->MB_GPURayCastTechniqueII = NULL;
  this->MB_GPURayCastTechniqueIIFg = NULL;

  this->MB_GPURayCastTechnique3 = NULL;

  this->SC_GPURayCastDistanceColorBlending = NULL;
  this->SC_GPURayCastDepthPeelingThreshold = NULL;
  this->SC_GPURayCastICPEkt = NULL;
  this->SC_GPURayCastICPEks = NULL;
  this->MB_GPUMemorySize = NULL;
  this->MB_GPURayCastColorOpacityFusion = NULL;

  this->SC_ExpectedFPS = NULL;
  this->SC_GPURayCastIIFgBgRatio = NULL;
  this->MB_Mapper = NULL;

  this->FrameGPURayCasting = NULL;
  this->FramePolygonBlending = NULL;
  this->FrameCPURayCasting = NULL;
  this->FrameTechniques = NULL;

  this->PB_HideSurfaceModels = NULL;

  this->SVP_VolumePropertyWidget = NULL;
  this->SVP_VolumePropertyWidgetFg = NULL;
  this->LoadVolumePropertyButton = NULL;

  this->ROIWidget = NULL;
  this->CB_CroppingButton = NULL;
  this->PB_FitROIButton = NULL;

  this->CB_UseThreshold = NULL;
  this->FrameThresholding = NULL;
  this->RA_Threshold = NULL;
  this->SC_ThresholdOpacity = NULL;

  this->CB_UseThresholdFg = NULL;
  this->FrameThresholdingFg = NULL;
  this->RA_ThresholdFg = NULL;
  this->SC_ThresholdOpacityFg = NULL;

  this->CB_FollowVolumeDisplayNode = NULL;
  this->CB_FollowVolumeDisplayNodeFg = NULL;

  this->CB_UseSingleVolumeProperty = NULL;

  //PauseResume
  this->PB_PauseResume = NULL;
  this->VI_PauseResume = NULL;

  this->SetupGUIFromParametersNodeFlag = 0;

  this->RenderingPaused = 0;
}

vtkSlicerVolumeRenderingHelper::~vtkSlicerVolumeRenderingHelper(void)
{
  this->DestroyTechniquesTab();
  this->DestroyPropertyTab();
  this->DestroyROITab();
  this->DestroyMiscTab();

  if(this->GUICallbackCommand != NULL)
  {
    this->GUICallbackCommand->Delete();
    this->GUICallbackCommand = NULL;
  }

  if(this->NB_Details)
  {
    this->Script("pack forget %s", this->NB_Details->GetWidgetName());
    this->NB_Details->SetParent(NULL);
    this->NB_Details->Delete();
    this->NB_Details = NULL;
  }
}

void vtkSlicerVolumeRenderingHelper::GUIEventsCallback( vtkObject *caller, unsigned long eid, void *clientData, void *callData )
{
  vtkSlicerVolumeRenderingHelper *self = reinterpret_cast<vtkSlicerVolumeRenderingHelper *>(clientData);

  self->ProcessGUIEvents(caller, eid, callData);
}

void vtkSlicerVolumeRenderingHelper::Init(vtkVolumeRenderingGUI *gui)
{
  this->Gui = gui;
  this->SetApplication(this->Gui->GetApplication());

  BuildRenderingFrameGUI();
}

void vtkSlicerVolumeRenderingHelper::BuildRenderingFrameGUI()
{
  if (!this->Gui->GetRenderingFrame() )
    return;

  //Create a notebook
  this->NB_Details = vtkKWNotebook::New();
  this->NB_Details->SetParent(this->Gui->GetRenderingFrame()->GetFrame());
  this->NB_Details->UseFrameWithScrollbarsOn();
  this->NB_Details->Create();
  this->NB_Details->AddPage("Techniques","Volume rendering techniques and parameters.");
  this->NB_Details->AddPage("Volume Property","Color/opacity transfer functions, shading, interpolation etc.");
  this->NB_Details->AddPage("ROI","Define ROI for volume rendering.");
  this->NB_Details->AddPage("Misc","Misc options.");
  this->Script("pack %s -side top -anchor nw -fill both -expand y -padx 2 -pady 2", this->NB_Details->GetWidgetName());

  this->CreateTechniquesTab();
  this->CreatePropertyTab();
  this->CreateROITab();
  this->CreateMiscTab();

  this->SetupGUIFromParametersNode(this->Gui->GetCurrentParametersNode());
}

void vtkSlicerVolumeRenderingHelper::CreateTechniquesTab()
{
  this->FrameTechniques = vtkKWFrameWithLabel::New();
  this->FrameTechniques->SetParent(this->NB_Details->GetFrame("Techniques"));
  this->FrameTechniques->Create();
  this->FrameTechniques->AllowFrameToCollapseOff();
  this->FrameTechniques->SetLabelText("Techniques");
  this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FrameTechniques->GetWidgetName() );

  int labelWidth = 16;

  //mapper selection combobox
  {
    this->MB_Mapper = vtkKWMenuButtonWithLabel::New();
    this->MB_Mapper->SetParent(this->FrameTechniques->GetFrame());
    this->MB_Mapper->SetLabelText("Rendering Method");
    this->MB_Mapper->Create();
    this->MB_Mapper->SetLabelWidth(labelWidth);
    this->MB_Mapper->SetBalloonHelpString("Please select one rendering method");
    this->MB_Mapper->GetWidget()->GetMenu()->AddRadioButton("VTK CPU Ray Casting");
    this->MB_Mapper->GetWidget()->GetMenu()->SetItemCommand(0, this,"ProcessRenderingMethodEvents 0");
    this->MB_Mapper->GetWidget()->GetMenu()->AddRadioButton("VTK GPU Ray Casting");
    this->MB_Mapper->GetWidget()->GetMenu()->SetItemCommand(1, this,"ProcessRenderingMethodEvents 1");
    this->MB_Mapper->GetWidget()->GetMenu()->AddRadioButton("VTK OpenGL 3D Texture Mapping");
    this->MB_Mapper->GetWidget()->GetMenu()->SetItemCommand(2, this,"ProcessRenderingMethodEvents 2");
    this->MB_Mapper->GetWidget()->GetMenu()->AddRadioButton("NCI GPU Ray Casting");
    this->MB_Mapper->GetWidget()->GetMenu()->SetItemCommand(3, this,"ProcessRenderingMethodEvents 3");
    this->MB_Mapper->GetWidget()->GetMenu()->AddRadioButton("NCI GPU Ray Casting (Multi-Volume)");
    this->MB_Mapper->GetWidget()->GetMenu()->SetItemCommand(4, this,"ProcessRenderingMethodEvents 4");

    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->MB_Mapper->GetWidgetName() );
  }

  //GPU memory size
  {
    this->MB_GPUMemorySize = vtkKWMenuButtonWithLabel::New();
    this->MB_GPUMemorySize->SetParent(this->FrameTechniques->GetFrame());
    this->MB_GPUMemorySize->SetLabelText("GPU Memory Size");
    this->MB_GPUMemorySize->Create();
    this->MB_GPUMemorySize->SetLabelWidth(labelWidth);
    this->MB_GPUMemorySize->SetBalloonHelpString("Please specify size of your GPU memory. Generally the larger GPU memory the better rendering quality. Do not select memory size larger than physical GPU memory size.");
    this->MB_GPUMemorySize->GetWidget()->GetMenu()->AddRadioButton("128M");
    this->MB_GPUMemorySize->GetWidget()->GetMenu()->SetItemCommand(0, this,"ProcessGPUMemorySize 0");
    this->MB_GPUMemorySize->GetWidget()->GetMenu()->AddRadioButton("256M");
    this->MB_GPUMemorySize->GetWidget()->GetMenu()->SetItemCommand(1, this,"ProcessGPUMemorySize 1");
    this->MB_GPUMemorySize->GetWidget()->GetMenu()->AddRadioButton("512M");
    this->MB_GPUMemorySize->GetWidget()->GetMenu()->SetItemCommand(2, this,"ProcessGPUMemorySize 2");
    this->MB_GPUMemorySize->GetWidget()->GetMenu()->AddRadioButton("1024M");
    this->MB_GPUMemorySize->GetWidget()->GetMenu()->SetItemCommand(3, this,"ProcessGPUMemorySize 3");
    this->MB_GPUMemorySize->GetWidget()->GetMenu()->AddRadioButton("1.5G");
    this->MB_GPUMemorySize->GetWidget()->GetMenu()->SetItemCommand(4, this,"ProcessGPUMemorySize 4");
    this->MB_GPUMemorySize->GetWidget()->GetMenu()->AddRadioButton("2.0G");
    this->MB_GPUMemorySize->GetWidget()->GetMenu()->SetItemCommand(5, this,"ProcessGPUMemorySize 5");
    this->MB_GPUMemorySize->GetWidget()->GetMenu()->AddRadioButton("3.0G");
    this->MB_GPUMemorySize->GetWidget()->GetMenu()->SetItemCommand(6, this,"ProcessGPUMemorySize 6");
    this->MB_GPUMemorySize->GetWidget()->GetMenu()->AddRadioButton("4.0G");
    this->MB_GPUMemorySize->GetWidget()->GetMenu()->SetItemCommand(7, this,"ProcessGPUMemorySize 7");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->MB_GPUMemorySize->GetWidgetName() );
  }

  //Performance control
  {
    this->MB_PerformanceControl = vtkKWMenuButtonWithLabel::New();
    this->MB_PerformanceControl->SetParent(this->FrameTechniques->GetFrame());
    this->MB_PerformanceControl->SetLabelText("Quality Control");
    this->MB_PerformanceControl->Create();
    this->MB_PerformanceControl->SetLabelWidth(labelWidth);
    this->MB_PerformanceControl->SetBalloonHelpString("Define volume rendeing performance/quality control method. Adaptive: low/high qualty switching. Maximum Quality: force highest possible quality. Fixed Framerate: use user choice of interactive speed.");
    this->MB_PerformanceControl->GetWidget()->GetMenu()->AddRadioButton("Adaptive");
    this->MB_PerformanceControl->GetWidget()->GetMenu()->SetItemCommand(0, this,"ProcessPerformanceControl 0");
    this->MB_PerformanceControl->GetWidget()->GetMenu()->AddRadioButton("Maximum Quality");
    this->MB_PerformanceControl->GetWidget()->GetMenu()->SetItemCommand(1, this,"ProcessPerformanceControl 1");
    this->MB_PerformanceControl->GetWidget()->GetMenu()->AddRadioButton("Fixed Framerate");
    this->MB_PerformanceControl->GetWidget()->GetMenu()->SetItemCommand(2, this,"ProcessPerformanceControl 2");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4", this->MB_PerformanceControl->GetWidgetName() );    
  }
  
  //Framerate
  {
    this->SC_ExpectedFPS=vtkKWScaleWithEntry::New();
    this->SC_ExpectedFPS->SetParent(this->FrameTechniques->GetFrame());
    this->SC_ExpectedFPS->Create();
    this->SC_ExpectedFPS->SetLabelText("Interactive Speed");
    this->SC_ExpectedFPS->SetLabelWidth(labelWidth);
    this->SC_ExpectedFPS->SetBalloonHelpString("Adjust performance/quality. 1 fps: low performance/high quality. 20 fps: high performance/low quality.");
    this->SC_ExpectedFPS->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *) this->GUICallbackCommand);
    this->SC_ExpectedFPS->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *) this->GUICallbackCommand);
    this->SC_ExpectedFPS->SetEntryWidth(5);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->SC_ExpectedFPS->GetWidgetName() );
  }

  labelWidth = 24;
  //software ray casting
  {
    this->FrameCPURayCasting = vtkKWFrameWithLabel::New();
    this->FrameCPURayCasting->SetParent(this->FrameTechniques->GetFrame());
    this->FrameCPURayCasting->Create();
    this->FrameCPURayCasting->AllowFrameToCollapseOff();
    this->FrameCPURayCasting->SetLabelText("VTK CPU Ray Casting");
    this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FrameCPURayCasting->GetWidgetName() );

    //enable/disable cpu ray casting MIP rendering
    this->CB_CPURayCastMIP=vtkKWCheckButtonWithLabel::New();
    this->CB_CPURayCastMIP->SetParent(this->FrameCPURayCasting->GetFrame());
    this->CB_CPURayCastMIP->Create();
    this->CB_CPURayCastMIP->SetBalloonHelpString("Enable MIP rendering in CPU ray cast.");
    this->CB_CPURayCastMIP->SetLabelText("Maximum Intensity Projection");
    this->CB_CPURayCastMIP->SetLabelWidth(labelWidth);
    this->CB_CPURayCastMIP->GetWidget()->SetSelectedState(0);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->CB_CPURayCastMIP->GetWidgetName() );
    this->CB_CPURayCastMIP->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand*) this->GUICallbackCommand);
  }

  labelWidth = 16;
  //VTK GPU ray casting
  {
    this->FrameGPURayCasting3 = vtkKWFrameWithLabel::New();
    this->FrameGPURayCasting3->SetParent(this->FrameTechniques->GetFrame());
    this->FrameGPURayCasting3->Create();
    this->FrameGPURayCasting3->AllowFrameToCollapseOff();
    this->FrameGPURayCasting3->SetLabelText("VTK GPU Ray Casting");
    this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FrameGPURayCasting3->GetWidgetName() );

    //set technique
    this->MB_GPURayCastTechnique3 = vtkKWMenuButtonWithLabel::New();
    this->MB_GPURayCastTechnique3->SetParent(this->FrameGPURayCasting3->GetFrame());
    this->MB_GPURayCastTechnique3->SetLabelText("Technique (bg):");
    this->MB_GPURayCastTechnique3->Create();
    this->MB_GPURayCastTechnique3->SetLabelWidth(labelWidth);
    this->MB_GPURayCastTechnique3->SetBalloonHelpString("Select GPU ray casting technique for bg volume");
    this->MB_GPURayCastTechnique3->GetWidget()->GetMenu()->AddRadioButton("Composite");
    this->MB_GPURayCastTechnique3->GetWidget()->GetMenu()->SetItemCommand(0, this,"ProcessGPURayCastTechnique3 0");
    this->MB_GPURayCastTechnique3->GetWidget()->GetMenu()->AddRadioButton("Maximum Intensity Projection");
    this->MB_GPURayCastTechnique3->GetWidget()->GetMenu()->SetItemCommand(1, this,"ProcessGPURayCastTechnique3 1");
    //this->MB_GPURayCastTechnique3->GetWidget()->GetMenu()->AddRadioButton("Minimum Intensity Projection");
    //this->MB_GPURayCastTechnique3->GetWidget()->GetMenu()->SetItemCommand(2, this,"ProcessGPURayCastTechnique3 2");

    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->MB_GPURayCastTechnique3->GetWidgetName() );
  }
  
  //opengl 2D Polygon Texture 3D
  {
    this->FramePolygonBlending = vtkKWFrameWithLabel::New();
    this->FramePolygonBlending->SetParent(this->FrameTechniques->GetFrame());
    this->FramePolygonBlending->Create();
    this->FramePolygonBlending->AllowFrameToCollapseOff();
    this->FramePolygonBlending->SetLabelText("VTK OpenGL 3D Texture Mapping");
    this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FramePolygonBlending->GetWidgetName() );

    //currently no parameters
  }
  
  //GPU ray casting
  {
    this->FrameGPURayCasting = vtkKWFrameWithLabel::New();
    this->FrameGPURayCasting->SetParent(this->FrameTechniques->GetFrame());
    this->FrameGPURayCasting->Create();
    this->FrameGPURayCasting->AllowFrameToCollapseOff();
    this->FrameGPURayCasting->SetLabelText("NCI GPU Ray Casting");
    this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FrameGPURayCasting->GetWidgetName() );

    //set technique
    this->MB_GPURayCastTechnique = vtkKWMenuButtonWithLabel::New();
    this->MB_GPURayCastTechnique->SetParent(this->FrameGPURayCasting->GetFrame());
    this->MB_GPURayCastTechnique->SetLabelText("Rendering Technique");
    this->MB_GPURayCastTechnique->Create();
    this->MB_GPURayCastTechnique->SetLabelWidth(labelWidth);
    this->MB_GPURayCastTechnique->SetBalloonHelpString("Select different techniques in GPU ray casting");
    this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->AddRadioButton("Composite With Shading");
    this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->SetItemCommand(0, this,"ProcessGPURayCastTechnique 0");
    this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->AddRadioButton("Composite Psuedo Shading");
    this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->SetItemCommand(1, this,"ProcessGPURayCastTechnique 1");
    this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->AddRadioButton("Maximum Intensity Projection");
    this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->SetItemCommand(2, this,"ProcessGPURayCastTechnique 2");
    this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->AddRadioButton("Minimum Intensity Projection");
    this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->SetItemCommand(3, this,"ProcessGPURayCastTechnique 3");
    this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->AddRadioButton("Gradient Magnitude Opacity Modulation");
    this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->SetItemCommand(4, this,"ProcessGPURayCastTechnique 4");
    this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->AddRadioButton("Illustrative Context Preserving Exploration");
    this->MB_GPURayCastTechnique->GetWidget()->GetMenu()->SetItemCommand(5, this,"ProcessGPURayCastTechnique 5");

    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->MB_GPURayCastTechnique->GetWidgetName() );

    this->SC_GPURayCastDistanceColorBlending=vtkKWScaleWithEntry::New();
    this->SC_GPURayCastDistanceColorBlending->SetParent(this->FrameGPURayCasting->GetFrame());
    this->SC_GPURayCastDistanceColorBlending->Create();
    this->SC_GPURayCastDistanceColorBlending->SetLabelText("Dist. Color Blending:");
    this->SC_GPURayCastDistanceColorBlending->SetLabelWidth(labelWidth);
    this->SC_GPURayCastDistanceColorBlending->SetBalloonHelpString("Distance Color Blending. Voxels with longer distance to eye/camera would be more darker to reveal depth information in volume. Higher value indicates stronger darking effect. Setting the value to 0 will turn off the effect.");
    this->SC_GPURayCastDistanceColorBlending->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastDistanceColorBlending->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastDistanceColorBlending->SetEntryWidth(5);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->SC_GPURayCastDistanceColorBlending->GetWidgetName() );
    
    this->SC_GPURayCastICPEkt = vtkKWScaleWithEntry::New();
    this->SC_GPURayCastICPEkt->SetParent(this->FrameGPURayCasting->GetFrame());
    this->SC_GPURayCastICPEkt->Create();
    this->SC_GPURayCastICPEkt->SetLabelText("ICPE Scale:");
    this->SC_GPURayCastICPEkt->SetLabelWidth(labelWidth);
    this->SC_GPURayCastICPEkt->SetBalloonHelpString("Parameter scale (kt) for Illustrative Context Preserving Exploration, which simulates depth penerating into volume");
    this->SC_GPURayCastICPEkt->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastICPEkt->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastICPEkt->SetEntryWidth(5);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->SC_GPURayCastICPEkt->GetWidgetName() );

    this->SC_GPURayCastICPEks = vtkKWScaleWithEntry::New();
    this->SC_GPURayCastICPEks->SetParent(this->FrameGPURayCasting->GetFrame());
    this->SC_GPURayCastICPEks->Create();
    this->SC_GPURayCastICPEks->SetLabelText("ICPE Smoothness:");
    this->SC_GPURayCastICPEks->SetLabelWidth(labelWidth);
    this->SC_GPURayCastICPEks->SetBalloonHelpString("Parameter smoothness (ks) for Illustrative Context Preserving Exploration");
    this->SC_GPURayCastICPEks->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastICPEks->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastICPEks->SetEntryWidth(5);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->SC_GPURayCastICPEks->GetWidgetName() );

    this->SC_GPURayCastDepthPeelingThreshold=vtkKWScaleWithEntry::New();
    this->SC_GPURayCastDepthPeelingThreshold->SetParent(this->FrameGPURayCasting->GetFrame());
    this->SC_GPURayCastDepthPeelingThreshold->Create();
    this->SC_GPURayCastDepthPeelingThreshold->SetLabelText("Vol. Depth Peeling:");
    this->SC_GPURayCastDepthPeelingThreshold->SetLabelWidth(labelWidth);
    this->SC_GPURayCastDepthPeelingThreshold->SetBalloonHelpString("Set threshold for Volumetric Depth Peeling. Volume rendering starts after we have met scalar values higher than the threshold. Use with transfer functions together.");
    this->SC_GPURayCastDepthPeelingThreshold->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastDepthPeelingThreshold->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastDepthPeelingThreshold->SetEntryWidth(8);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->SC_GPURayCastDepthPeelingThreshold->GetWidgetName() );
  }

  //GPU ray casting II
  {
    this->FrameGPURayCastingII = vtkKWFrameWithLabel::New();
    this->FrameGPURayCastingII->SetParent(this->FrameTechniques->GetFrame());
    this->FrameGPURayCastingII->Create();
    this->FrameGPURayCastingII->AllowFrameToCollapseOff();
    this->FrameGPURayCastingII->SetLabelText("NCI GPU Ray Casting (Multi-Volume)");
    this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FrameGPURayCastingII->GetWidgetName() );

    //set technique
    this->MB_GPURayCastTechniqueII = vtkKWMenuButtonWithLabel::New();
    this->MB_GPURayCastTechniqueII->SetParent(this->FrameGPURayCastingII->GetFrame());
    this->MB_GPURayCastTechniqueII->SetLabelText("Technique (Bg):");
    this->MB_GPURayCastTechniqueII->Create();
    this->MB_GPURayCastTechniqueII->SetLabelWidth(labelWidth);
    this->MB_GPURayCastTechniqueII->SetBalloonHelpString("Select GPU ray casting technique for bg volume");
    this->MB_GPURayCastTechniqueII->GetWidget()->GetMenu()->AddRadioButton("Composite With Shading");
    this->MB_GPURayCastTechniqueII->GetWidget()->GetMenu()->SetItemCommand(0, this,"ProcessGPURayCastTechniqueII 0");
    this->MB_GPURayCastTechniqueII->GetWidget()->GetMenu()->AddRadioButton("Composite Psuedo Shading");
    this->MB_GPURayCastTechniqueII->GetWidget()->GetMenu()->SetItemCommand(1, this,"ProcessGPURayCastTechniqueII 1");
    this->MB_GPURayCastTechniqueII->GetWidget()->GetMenu()->AddRadioButton("Maximum Intensity Projection");
    this->MB_GPURayCastTechniqueII->GetWidget()->GetMenu()->SetItemCommand(2, this,"ProcessGPURayCastTechniqueII 2");
    this->MB_GPURayCastTechniqueII->GetWidget()->GetMenu()->AddRadioButton("Minimum Intensity Projection");
    this->MB_GPURayCastTechniqueII->GetWidget()->GetMenu()->SetItemCommand(3, this,"ProcessGPURayCastTechniqueII 3");
    this->MB_GPURayCastTechniqueII->GetWidget()->GetMenu()->AddRadioButton("Gradient Magnitude Opacity Modulation");
    this->MB_GPURayCastTechniqueII->GetWidget()->GetMenu()->SetItemCommand(4, this,"ProcessGPURayCastTechniqueII 4");

    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->MB_GPURayCastTechniqueII->GetWidgetName() );

    //set technique
    this->MB_GPURayCastTechniqueIIFg = vtkKWMenuButtonWithLabel::New();
    this->MB_GPURayCastTechniqueIIFg->SetParent(this->FrameGPURayCastingII->GetFrame());
    this->MB_GPURayCastTechniqueIIFg->SetLabelText("Technique (Fg):");
    this->MB_GPURayCastTechniqueIIFg->Create();
    this->MB_GPURayCastTechniqueIIFg->SetLabelWidth(labelWidth);
    this->MB_GPURayCastTechniqueIIFg->SetBalloonHelpString("Select GPU ray casting technique for fg volume");
    this->MB_GPURayCastTechniqueIIFg->GetWidget()->GetMenu()->AddRadioButton("Composite With Shading");
    this->MB_GPURayCastTechniqueIIFg->GetWidget()->GetMenu()->SetItemCommand(0, this,"ProcessGPURayCastTechniqueIIFg 0");
    this->MB_GPURayCastTechniqueIIFg->GetWidget()->GetMenu()->AddRadioButton("Composite Psuedo Shading");
    this->MB_GPURayCastTechniqueIIFg->GetWidget()->GetMenu()->SetItemCommand(1, this,"ProcessGPURayCastTechniqueIIFg 1");
    this->MB_GPURayCastTechniqueIIFg->GetWidget()->GetMenu()->AddRadioButton("Maximum Intensity Projection");
    this->MB_GPURayCastTechniqueIIFg->GetWidget()->GetMenu()->SetItemCommand(2, this,"ProcessGPURayCastTechniqueIIFg 2");
    this->MB_GPURayCastTechniqueIIFg->GetWidget()->GetMenu()->AddRadioButton("Minimum Intensity Projection");
    this->MB_GPURayCastTechniqueIIFg->GetWidget()->GetMenu()->SetItemCommand(3, this,"ProcessGPURayCastTechniqueIIFg 3");
    this->MB_GPURayCastTechniqueIIFg->GetWidget()->GetMenu()->AddRadioButton("Gradient Magnitude Opacity Modulation");
    this->MB_GPURayCastTechniqueIIFg->GetWidget()->GetMenu()->SetItemCommand(4, this,"ProcessGPURayCastTechniqueIIFg 4");

    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->MB_GPURayCastTechniqueIIFg->GetWidgetName() );

    this->MB_GPURayCastColorOpacityFusion = vtkKWMenuButtonWithLabel::New();
    this->MB_GPURayCastColorOpacityFusion->SetParent(this->FrameGPURayCastingII->GetFrame());
    this->MB_GPURayCastColorOpacityFusion->SetLabelText("Fusion:");
    this->MB_GPURayCastColorOpacityFusion->Create();
    this->MB_GPURayCastColorOpacityFusion->SetLabelWidth(labelWidth);
    this->MB_GPURayCastColorOpacityFusion->SetBalloonHelpString("Select color fusion method in multi-volume rendering");
    this->MB_GPURayCastColorOpacityFusion->GetWidget()->GetMenu()->AddRadioButton("Alpha Blending (OR)");
    this->MB_GPURayCastColorOpacityFusion->GetWidget()->GetMenu()->SetItemCommand(0, this,"ProcessGPURayCastColorOpacityFusion 0");
    this->MB_GPURayCastColorOpacityFusion->GetWidget()->GetMenu()->AddRadioButton("Alpha Blending (AND)");
    this->MB_GPURayCastColorOpacityFusion->GetWidget()->GetMenu()->SetItemCommand(1, this,"ProcessGPURayCastColorOpacityFusion 1");

    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->MB_GPURayCastColorOpacityFusion->GetWidgetName() );

    this->SC_GPURayCastIIFgBgRatio = vtkKWScaleWithEntry::New();
    this->SC_GPURayCastIIFgBgRatio->SetParent(this->FrameGPURayCastingII->GetFrame());
    this->SC_GPURayCastIIFgBgRatio->Create();
    this->SC_GPURayCastIIFgBgRatio->SetLabelText("Bg|Fg Ratio:");
    this->SC_GPURayCastIIFgBgRatio->SetLabelWidth(labelWidth);
    this->SC_GPURayCastIIFgBgRatio->SetEntryWidth(5);
    this->SC_GPURayCastIIFgBgRatio->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastIIFgBgRatio->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *) this->GUICallbackCommand);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->SC_GPURayCastIIFgBgRatio->GetWidgetName() );
  }
}

void vtkSlicerVolumeRenderingHelper::DestroyTechniquesTab()
{
  //remember user choice on rendering method and GPU memory size
  this->Gui->GetApplication()->SetRegistryValue(2,"VolumeRendering","MB_GPUMemorySize","%d",
      this->MB_GPUMemorySize->GetWidget()->GetMenu()->GetIndexOfSelectedItem());
  this->Gui->GetApplication()->SetRegistryValue(2,"VolumeRendering","MB_Mapper","%d",
      this->MB_Mapper->GetWidget()->GetMenu()->GetIndexOfSelectedItem());

  //now the boring part: delete all widgets one by one, should have smart ptr to handle this

  if(this->MB_PerformanceControl != NULL)
  {
    this->MB_PerformanceControl->SetParent(NULL);
    this->MB_PerformanceControl->Delete();
    this->MB_PerformanceControl=NULL;
  }
  
  if(this->MB_GPUMemorySize != NULL)
  {
    this->MB_GPUMemorySize->SetParent(NULL);
    this->MB_GPUMemorySize->Delete();
    this->MB_GPUMemorySize=NULL;
  }

  if(this->MB_GPURayCastTechnique != NULL)
  {
    this->MB_GPURayCastTechnique->SetParent(NULL);
    this->MB_GPURayCastTechnique->Delete();
    this->MB_GPURayCastTechnique = NULL;
  }

  if(this->MB_GPURayCastTechniqueII != NULL)
  {
    this->MB_GPURayCastTechniqueII->SetParent(NULL);
    this->MB_GPURayCastTechniqueII->Delete();
    this->MB_GPURayCastTechniqueII = NULL;
  }

  if(this->MB_GPURayCastTechniqueIIFg != NULL)
  {
    this->MB_GPURayCastTechniqueIIFg->SetParent(NULL);
    this->MB_GPURayCastTechniqueIIFg->Delete();
    this->MB_GPURayCastTechniqueIIFg = NULL;
  }
  if (this->MB_GPURayCastColorOpacityFusion != NULL)
    {
    this->MB_GPURayCastColorOpacityFusion->SetParent(NULL);
    this->MB_GPURayCastColorOpacityFusion->Delete();
    this->MB_GPURayCastColorOpacityFusion = NULL;
    }

  if(this->MB_GPURayCastTechnique3 != NULL)
  {
    this->MB_GPURayCastTechnique3->SetParent(NULL);
    this->MB_GPURayCastTechnique3->Delete();
    this->MB_GPURayCastTechnique3 = NULL;
  }

  if(this->SC_ExpectedFPS != NULL)
  {
    this->SC_ExpectedFPS->RemoveObservers(vtkKWScale::ScaleValueChangingEvent,(vtkCommand *) this->GUICallbackCommand);
    this->SC_ExpectedFPS->RemoveObservers(vtkKWScale::ScaleValueChangedEvent,(vtkCommand *) this->GUICallbackCommand);
    this->SC_ExpectedFPS->SetParent(NULL);
    this->SC_ExpectedFPS->Delete();
    this->SC_ExpectedFPS=NULL;
  }

  if(this->SC_GPURayCastDepthPeelingThreshold != NULL)
  {
    this->SC_GPURayCastDepthPeelingThreshold->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangingEvent,(vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastDepthPeelingThreshold->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent,(vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastDepthPeelingThreshold->SetParent(NULL);
    this->SC_GPURayCastDepthPeelingThreshold->Delete();
    this->SC_GPURayCastDepthPeelingThreshold=NULL;
  }

  if(this->SC_GPURayCastDistanceColorBlending != NULL)
  {
    this->SC_GPURayCastDistanceColorBlending->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangingEvent,(vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastDistanceColorBlending->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent,(vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastDistanceColorBlending->SetParent(NULL);
    this->SC_GPURayCastDistanceColorBlending->Delete();
    this->SC_GPURayCastDistanceColorBlending=NULL;
  }
  
  if(this->SC_GPURayCastICPEkt != NULL)
  {
    this->SC_GPURayCastICPEkt->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangingEvent,(vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastICPEkt->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent,(vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastICPEkt->SetParent(NULL);
    this->SC_GPURayCastICPEkt->Delete();
    this->SC_GPURayCastICPEkt=NULL;
  }

  if(this->SC_GPURayCastICPEks != NULL)
  {
    this->SC_GPURayCastICPEks->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangingEvent,(vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastICPEks->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent,(vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastICPEks->SetParent(NULL);
    this->SC_GPURayCastICPEks->Delete();
    this->SC_GPURayCastICPEks=NULL;
  }

  if(this->SC_GPURayCastIIFgBgRatio != NULL)
  {
    this->SC_GPURayCastIIFgBgRatio->RemoveObservers(vtkKWScale::ScaleValueChangingEvent,(vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastIIFgBgRatio->RemoveObservers(vtkKWScale::ScaleValueChangedEvent,(vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastIIFgBgRatio->SetParent(NULL);
    this->SC_GPURayCastIIFgBgRatio->Delete();
    this->SC_GPURayCastIIFgBgRatio=NULL;
  }

  if(this->CB_CPURayCastMIP != NULL)
  {
    this->CB_CPURayCastMIP->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*)this->GUICallbackCommand);
    this->CB_CPURayCastMIP->SetParent(NULL);
    this->CB_CPURayCastMIP->Delete();
    this->CB_CPURayCastMIP=NULL;
  }

  if(this->MB_Mapper != NULL)
  {
    this->MB_Mapper->SetParent(NULL);
    this->MB_Mapper->Delete();
    this->MB_Mapper=NULL;
  }

  if (this->FrameTechniques != NULL)
  {
    this->FrameTechniques->SetParent(NULL);
    this->FrameTechniques->Delete();
    this->FrameTechniques = NULL;
  }

  if (this->FrameGPURayCasting != NULL)
  {
    this->FrameGPURayCasting->SetParent(NULL);
    this->FrameGPURayCasting->Delete();
    this->FrameGPURayCasting = NULL;
  }
  if (this->FrameGPURayCastingII != NULL)
    {
    this->FrameGPURayCastingII->SetParent(NULL);
    this->FrameGPURayCastingII->Delete();
    this->FrameGPURayCastingII = NULL;
    }
  if (this->FrameGPURayCasting3 != NULL)
    {
    this->FrameGPURayCasting3->SetParent(NULL);
    this->FrameGPURayCasting3->Delete();
    this->FrameGPURayCasting3 = NULL;
    }
  if (this->FramePolygonBlending != NULL)
  {
    this->FramePolygonBlending->SetParent(NULL);
    this->FramePolygonBlending->Delete();
    this->FramePolygonBlending = NULL;
  }

  if (this->FrameCPURayCasting != NULL)
  {
    this->FrameCPURayCasting->SetParent(NULL);
    this->FrameCPURayCasting->Delete();
    this->FrameCPURayCasting = NULL;
  }
}

void vtkSlicerVolumeRenderingHelper::CreateROITab()
{
  vtkKWFrameWithLabel *mainFrame = vtkKWFrameWithLabel::New();
  mainFrame->SetParent(this->NB_Details->GetFrame("ROI"));
  mainFrame->SetLabelText("ROI (Clipping)");
  mainFrame->Create();
  this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", mainFrame->GetWidgetName() );

  this->PB_FitROIButton = vtkKWPushButton::New();
  this->PB_FitROIButton->SetParent( mainFrame->GetFrame() );
  this->PB_FitROIButton->Create();
  this->PB_FitROIButton->SetText("Fit ROI To Volume");
  this->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->PB_FitROIButton->GetWidgetName());

  this->PB_FitROIButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  vtkKWFrame *roiFrame = vtkKWFrame::New();
  roiFrame->SetParent(mainFrame->GetFrame());
  roiFrame->Create();
  roiFrame->SetReliefToGroove();
  roiFrame->SetBorderWidth(2);
  this->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2", roiFrame->GetWidgetName());

  this->ROIWidget = vtkSlicerROIDisplayWidget::New();
  this->ROIWidget->SetParent(roiFrame);
  this->ROIWidget->Create();

  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->ROIWidget->GetWidgetName());

  this->CB_CroppingButton = vtkKWCheckButtonWithLabel::New();
  this->CB_CroppingButton->SetParent ( mainFrame->GetFrame() );
  this->CB_CroppingButton->Create ( );
  this->CB_CroppingButton->SetLabelText("Cropping Enabled");
  this->CB_CroppingButton->SetBalloonHelpString("Enable cropping.");
  this->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->CB_CroppingButton->GetWidgetName());

  this->CB_CroppingButton->GetWidget()->AddObserver (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  roiFrame->Delete();
  mainFrame->Delete();

  this->UpdateROI();
}

void vtkSlicerVolumeRenderingHelper::DestroyROITab()
{
  if (this->CB_CroppingButton)
  {
    this->CB_CroppingButton->GetWidget()->RemoveObservers (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->CB_CroppingButton->SetParent(NULL);
    this->CB_CroppingButton->Delete();
    this->CB_CroppingButton = NULL;
  }

  if (this->PB_FitROIButton)
  {
    this->PB_FitROIButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->PB_FitROIButton->SetParent(NULL);
    this->PB_FitROIButton->Delete();
    this->PB_FitROIButton = NULL;
  }

  if (this->ROIWidget)
  {
    this->ROIWidget->SetParent(NULL);
    this->ROIWidget->Delete();
    this->ROIWidget = NULL;
  }
}

void vtkSlicerVolumeRenderingHelper::CreateMiscTab()
{
  vtkKWFrameWithLabel *mainFrame = vtkKWFrameWithLabel::New();
  mainFrame->SetParent(this->NB_Details->GetFrame("Misc"));
  mainFrame->Create();
  mainFrame->SetLabelText("Options");
  this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", mainFrame->GetWidgetName() );

  //pause/resume button
  this->VI_PauseResume = vtkSlicerVisibilityIcons::New();
  this->PB_PauseResume = vtkKWPushButtonWithLabel::New();
  this->PB_PauseResume->SetParent(mainFrame->GetFrame());
  this->PB_PauseResume->Create();
  this->PB_PauseResume->SetBalloonHelpString("Toggle the visibility of volume rendering.");
  this->PB_PauseResume->SetLabelText("Visiblity of Volume Rendering: ");
  this->PB_PauseResume->GetWidget()->SetImageToIcon(this->VI_PauseResume->GetVisibleIcon());
  this->Script("pack %s -side top -anchor n -padx 2 -pady 2", this->PB_PauseResume->GetWidgetName());
  this->PB_PauseResume->GetWidget()->SetCommand(this, "ProcessPauseResume");

  //Hide surface models pushbutton
  this->PB_HideSurfaceModels = vtkKWPushButton::New();
  this->PB_HideSurfaceModels->SetParent(mainFrame->GetFrame());
  this->PB_HideSurfaceModels->Create();
  this->PB_HideSurfaceModels->SetText("Hide Surface Models");
  this->PB_HideSurfaceModels->SetBalloonHelpString("Make all surface models invisible. Go to models module to enable, disable only some of them.");
//  this->PB_HideSurfaceModels->SetWidth(labelWidth);
  this->Script("pack %s -side top -anchor ne -fill x -padx 2 -pady 2",this->PB_HideSurfaceModels->GetWidgetName());

  this->PB_HideSurfaceModels->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *) this->GUICallbackCommand);

  mainFrame->Delete();
}

void vtkSlicerVolumeRenderingHelper::DestroyMiscTab()
{
  this->PB_HideSurfaceModels->RemoveObservers(vtkKWPushButton::InvokedEvent,(vtkCommand *) this->GUICallbackCommand);

  if(this->PB_PauseResume)
  {
    this->PB_PauseResume->SetParent(NULL);
    this->PB_PauseResume->Delete();
    this->PB_PauseResume = NULL;
  }

  if(this->VI_PauseResume)
  {
    this->VI_PauseResume->Delete();
    this->VI_PauseResume=NULL;
  }

  if(this->PB_HideSurfaceModels)
  {
    this->PB_HideSurfaceModels->SetParent(NULL);
    this->PB_HideSurfaceModels->Delete();
    this->PB_HideSurfaceModels = NULL;
  }
}

void vtkSlicerVolumeRenderingHelper::CreatePropertyTab()
{
  vtkKWFrameWithLabel *mainFrame = vtkKWFrameWithLabel::New();
  mainFrame->SetParent(this->NB_Details->GetFrame("Volume Property"));
  mainFrame->Create();
  mainFrame->SetLabelText("Background Volume");
  this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", mainFrame->GetWidgetName() );

  this->CB_UseSingleVolumeProperty = vtkKWCheckButtonWithLabel::New();
  this->CB_UseSingleVolumeProperty->SetParent(mainFrame->GetFrame());
  this->CB_UseSingleVolumeProperty->Create();
  this->CB_UseSingleVolumeProperty->SetLabelText("Apply To Fg Volume");
  this->CB_UseSingleVolumeProperty->SetBalloonHelpString("When checked, fg volume will use bg volume property. Useful in pre and post treatment image comparison.");
  this->CB_UseSingleVolumeProperty->SetLabelWidth(20);
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->CB_UseSingleVolumeProperty->GetWidgetName() );
  this->CB_UseSingleVolumeProperty->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand*) this->GUICallbackCommand);
  
  this->CB_FollowVolumeDisplayNode = vtkKWCheckButtonWithLabel::New();
  this->CB_FollowVolumeDisplayNode->SetParent(mainFrame->GetFrame());
  this->CB_FollowVolumeDisplayNode->Create();
  this->CB_FollowVolumeDisplayNode->SetLabelText("Follow Volumes Module");
  this->CB_FollowVolumeDisplayNode->SetBalloonHelpString("When checked, window/level and lookup table in the Volumes module will be used for color mapping in volume rendering and threshold in the Volumes module will be used for opacity mapping. Also when checked local GUI will disabled for following.");
  this->CB_FollowVolumeDisplayNode->SetLabelWidth(20);
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->CB_FollowVolumeDisplayNode->GetWidgetName() );
  this->CB_FollowVolumeDisplayNode->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand*) this->GUICallbackCommand);
  
  this->CB_UseThreshold = vtkKWCheckButtonWithLabel::New();
  this->CB_UseThreshold->SetParent(mainFrame->GetFrame());
  this->CB_UseThreshold->Create();
  this->CB_UseThreshold->SetBalloonHelpString("Enable/Disable thresholding.");
  this->CB_UseThreshold->SetLabelText("Use Thresholding");
  this->CB_UseThreshold->SetLabelWidth(20);
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->CB_UseThreshold->GetWidgetName() );
  this->CB_UseThreshold->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand*) this->GUICallbackCommand);

  {
    this->FrameThresholding = vtkKWFrameWithLabel::New();
    this->FrameThresholding->SetParent(mainFrame->GetFrame());
    this->FrameThresholding->Create();
    this->FrameThresholding->AllowFrameToCollapseOff();
    this->FrameThresholding->SetLabelText("Thresholding");
    this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FrameThresholding->GetWidgetName() );

    this->RA_Threshold = vtkKWRange::New();
    this->RA_Threshold->SetParent(this->FrameThresholding->GetFrame());
    this->RA_Threshold->Create();
    this->RA_Threshold->SetBalloonHelpString("Apply thresholds to the gray values of volume.");
    this->RA_Threshold->SetCommand(this, "ProcessThreshold");
    this->Script("pack %s -side top -anchor nw -expand yes -fill x -padx 2 -pady 2", this->RA_Threshold->GetWidgetName());

    this->SC_ThresholdOpacity = vtkKWScaleWithEntry::New();
    this->SC_ThresholdOpacity->SetParent(this->FrameThresholding->GetFrame());
    this->SC_ThresholdOpacity->Create();
    this->SC_ThresholdOpacity->SetLabelText("Opacity:");
    this->SC_ThresholdOpacity->SetLabelWidth(8);
    this->SC_ThresholdOpacity->SetBalloonHelpString("Opacity in thresholding mode.");
    this->SC_ThresholdOpacity->SetEntryWidth(5);
    this->SC_ThresholdOpacity->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *) this->GUICallbackCommand);
    this->SC_ThresholdOpacity->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *) this->GUICallbackCommand);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->SC_ThresholdOpacity->GetWidgetName() );

    this->FrameThresholding->CollapseFrame();
  }

  this->SVP_VolumePropertyWidget = vtkSlicerVolumePropertyWidget::New();
  this->SVP_VolumePropertyWidget->SetParent(mainFrame->GetFrame());
  this->SVP_VolumePropertyWidget->Create();
  this->SVP_VolumePropertyWidget->ScalarOpacityUnitDistanceVisibilityOff ();
  this->SVP_VolumePropertyWidget->InteractiveApplyModeOn ();

  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->SVP_VolumePropertyWidget->GetWidgetName());

  this->SVP_VolumePropertyWidget->AddObserver(vtkKWEvent::VolumePropertyChangingEvent, (vtkCommand*)this->GUICallbackCommand);

  mainFrame->Delete();

  //---------------fg volume property-----------------------------------
  vtkKWFrameWithLabel *mainFrameFg = vtkKWFrameWithLabel::New();
  mainFrameFg->SetParent(this->NB_Details->GetFrame("Volume Property"));
  mainFrameFg->Create();
  mainFrameFg->SetLabelText("Foreground Volume");
  this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", mainFrameFg->GetWidgetName() );

  this->CB_UseThresholdFg = vtkKWCheckButtonWithLabel::New();
  this->CB_UseThresholdFg->SetParent(mainFrameFg->GetFrame());
  this->CB_UseThresholdFg->Create();
  this->CB_UseThresholdFg->SetBalloonHelpString("Enable/Disable thresholding.");
  this->CB_UseThresholdFg->SetLabelText("Use Thresholding");
  this->CB_UseThresholdFg->SetLabelWidth(20);
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->CB_UseThresholdFg->GetWidgetName() );
  this->CB_UseThresholdFg->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand*) this->GUICallbackCommand);

  {
    this->FrameThresholdingFg = vtkKWFrameWithLabel::New();
    this->FrameThresholdingFg->SetParent(mainFrameFg->GetFrame());
    this->FrameThresholdingFg->Create();
    this->FrameThresholdingFg->AllowFrameToCollapseOff();
    this->FrameThresholdingFg->SetLabelText("Thresholding");
    this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FrameThresholdingFg->GetWidgetName() );

    this->RA_ThresholdFg = vtkKWRange::New();
    this->RA_ThresholdFg->SetParent(this->FrameThresholdingFg->GetFrame());
    this->RA_ThresholdFg->Create();
    this->RA_ThresholdFg->SetBalloonHelpString("Apply thresholds to the gray values of volume.");
    this->RA_ThresholdFg->SetCommand(this, "ProcessThresholdFg");
    this->Script("pack %s -side top -anchor nw -expand yes -fill x -padx 2 -pady 2", this->RA_ThresholdFg->GetWidgetName());

    this->SC_ThresholdOpacityFg = vtkKWScaleWithEntry::New();
    this->SC_ThresholdOpacityFg->SetParent(this->FrameThresholdingFg->GetFrame());
    this->SC_ThresholdOpacityFg->Create();
    this->SC_ThresholdOpacityFg->SetLabelText("Opacity:");
    this->SC_ThresholdOpacityFg->SetLabelWidth(8);
    this->SC_ThresholdOpacityFg->SetBalloonHelpString("Opacity in thresholding mode.");
    this->SC_ThresholdOpacityFg->SetEntryWidth(5);
    this->SC_ThresholdOpacityFg->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *) this->GUICallbackCommand);
    this->SC_ThresholdOpacityFg->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *) this->GUICallbackCommand);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->SC_ThresholdOpacityFg->GetWidgetName() );
    
    this->FrameThresholdingFg->CollapseFrame();
  }

  this->SVP_VolumePropertyWidgetFg = vtkSlicerVolumePropertyWidget::New();
  this->SVP_VolumePropertyWidgetFg->SetParent(mainFrameFg->GetFrame());
  this->SVP_VolumePropertyWidgetFg->Create();
  this->SVP_VolumePropertyWidgetFg->ScalarOpacityUnitDistanceVisibilityOff ();
  this->SVP_VolumePropertyWidgetFg->InteractiveApplyModeOn ();

  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->SVP_VolumePropertyWidgetFg->GetWidgetName());

  this->SVP_VolumePropertyWidgetFg->AddObserver(vtkKWEvent::VolumePropertyChangingEvent, (vtkCommand*)this->GUICallbackCommand);

  mainFrameFg->CollapseFrame();

  mainFrameFg->Delete();
  
   // ---
  // LOAD FRAME            
  vtkKWFrameWithLabel *loadFrame = vtkKWFrameWithLabel::New();
  loadFrame->SetParent(this->NB_Details->GetFrame("Volume Property"));
  loadFrame->Create();
  loadFrame->SetLabelText("Load Volume Property");
  this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", loadFrame->GetWidgetName() );


  // add a file browser 
  this->LoadVolumePropertyButton = vtkKWLoadSaveButton::New ( );
  this->LoadVolumePropertyButton->SetParent ( loadFrame->GetFrame() );
  this->LoadVolumePropertyButton->Create ( );
  this->LoadVolumePropertyButton->SetText ("Load Volume Property");
  this->LoadVolumePropertyButton->GetLoadSaveDialog()->RetrieveLastPathFromRegistry("OpenPath");
  this->LoadVolumePropertyButton->GetLoadSaveDialog()->SetFileTypes(
                                                                 "{ {volume property} {*.*} }");

  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2", 
                this->LoadVolumePropertyButton->GetWidgetName());
  this->LoadVolumePropertyButton->GetLoadSaveDialog()->AddObserver (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
  
  loadFrame->Delete();
 }


void vtkSlicerVolumeRenderingHelper::DestroyPropertyTab()
{
  if (this->LoadVolumePropertyButton)
  {
    this->LoadVolumePropertyButton->GetLoadSaveDialog()->RemoveObservers (vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand );
    this->LoadVolumePropertyButton->SetParent(NULL);
    this->LoadVolumePropertyButton->Delete ( );
    this->LoadVolumePropertyButton = NULL;
  }
  
  if(this->SVP_VolumePropertyWidget != NULL)
  {
    this->Gui->Script("pack forget %s", this->SVP_VolumePropertyWidget->GetWidgetName());
    this->SVP_VolumePropertyWidget->SetHistogramSet(NULL);
    this->SVP_VolumePropertyWidget->SetVolumeProperty(NULL);
    this->SVP_VolumePropertyWidget->SetDataSet(NULL);
    this->SVP_VolumePropertyWidget->SetParent(NULL);
    this->SVP_VolumePropertyWidget->RemoveObservers(vtkKWEvent::VolumePropertyChangingEvent, (vtkCommand*)this->GUICallbackCommand);
    this->SVP_VolumePropertyWidget->Delete();
    this->SVP_VolumePropertyWidget = NULL;
  }

  if(this->SVP_VolumePropertyWidgetFg != NULL)
  {
    this->Gui->Script("pack forget %s", this->SVP_VolumePropertyWidgetFg->GetWidgetName());
    this->SVP_VolumePropertyWidgetFg->SetHistogramSet(NULL);
    this->SVP_VolumePropertyWidgetFg->SetVolumeProperty(NULL);
    this->SVP_VolumePropertyWidgetFg->SetDataSet(NULL);
    this->SVP_VolumePropertyWidgetFg->SetParent(NULL);
    this->SVP_VolumePropertyWidgetFg->RemoveObservers(vtkKWEvent::VolumePropertyChangingEvent, (vtkCommand*)this->GUICallbackCommand);
    this->SVP_VolumePropertyWidgetFg->Delete();
    this->SVP_VolumePropertyWidgetFg = NULL;
  }

  if(this->CB_UseThreshold != NULL)
  {
    this->CB_UseThreshold->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*)this->GUICallbackCommand);
    this->CB_UseThreshold->SetParent(NULL);
    this->CB_UseThreshold->Delete();
    this->CB_UseThreshold = NULL;
  }

  if(this->CB_UseSingleVolumeProperty != NULL)
  {
    this->CB_UseSingleVolumeProperty->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*)this->GUICallbackCommand);
    this->CB_UseSingleVolumeProperty->SetParent(NULL);
    this->CB_UseSingleVolumeProperty->Delete();
    this->CB_UseSingleVolumeProperty = NULL;
  }
  
  if(this->CB_FollowVolumeDisplayNode != NULL)
  {
    this->CB_FollowVolumeDisplayNode->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*)this->GUICallbackCommand);
    this->CB_FollowVolumeDisplayNode->SetParent(NULL);
    this->CB_FollowVolumeDisplayNode->Delete();
    this->CB_FollowVolumeDisplayNode = NULL;
  }

  if(this->FrameThresholding != NULL)
  {
    this->FrameThresholding->SetParent(NULL);
    this->FrameThresholding->Delete();
    this->FrameThresholding = NULL;
  }

  if(this->RA_Threshold)
  {
    this->RA_Threshold->SetParent(NULL);
    this->RA_Threshold->Delete();
    this->RA_Threshold = NULL;
  }

  if(this->SC_ThresholdOpacity != NULL)
  {
    this->SC_ThresholdOpacity->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangingEvent,(vtkCommand *) this->GUICallbackCommand);
    this->SC_ThresholdOpacity->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent,(vtkCommand *) this->GUICallbackCommand);
    this->SC_ThresholdOpacity->SetParent(NULL);
    this->SC_ThresholdOpacity->Delete();
    this->SC_ThresholdOpacity = NULL;
  }
  
  if(this->CB_UseThresholdFg != NULL)
  {
    this->CB_UseThresholdFg->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*)this->GUICallbackCommand);
    this->CB_UseThresholdFg->SetParent(NULL);
    this->CB_UseThresholdFg->Delete();
    this->CB_UseThresholdFg = NULL;
  }

  if(this->FrameThresholdingFg != NULL)
  {
    this->FrameThresholdingFg->SetParent(NULL);
    this->FrameThresholdingFg->Delete();
    this->FrameThresholdingFg = NULL;
  }

  if(this->RA_ThresholdFg)
  {
    this->RA_ThresholdFg->SetParent(NULL);
    this->RA_ThresholdFg->Delete();
    this->RA_ThresholdFg = NULL;
  }

  if(this->SC_ThresholdOpacityFg != NULL)
  {
    this->SC_ThresholdOpacityFg->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangingEvent,(vtkCommand *) this->GUICallbackCommand);
    this->SC_ThresholdOpacityFg->GetWidget()->RemoveObservers(vtkKWScale::ScaleValueChangedEvent,(vtkCommand *) this->GUICallbackCommand);
    this->SC_ThresholdOpacityFg->SetParent(NULL);
    this->SC_ThresholdOpacityFg->Delete();
    this->SC_ThresholdOpacityFg = NULL;
  }
}

void vtkSlicerVolumeRenderingHelper::ProcessGUIEvents(vtkObject *caller,
                                                      unsigned long eid,
                                                      void *vtkNotUsed(callData))
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();

  vtkKWLoadSaveDialog *loadSaveDialog = vtkKWLoadSaveDialog::SafeDownCast(caller);
  
  if (loadSaveDialog && loadSaveDialog == this->LoadVolumePropertyButton->GetLoadSaveDialog() &&
      eid== vtkKWTopLevel::WithdrawEvent  )
    {
    // If a file has been selected for loading...
    const char *fileName = this->LoadVolumePropertyButton->GetFileName();
    if ( fileName ) 
    {      
      vtkMRMLVolumePropertyNode *vpNode = this->Gui->GetLogic()->AddVolumePropertyFromFile( fileName );

      if ( vpNode == NULL ) 
      {
        // TODO: generate an error...
        vtkErrorMacro("Unable to read volume property file " << fileName);
      }
      else
      {
        this->LoadVolumePropertyButton->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
      }
    }

    // reset the file browse button text
    this->LoadVolumePropertyButton->SetText ("Load Volume Property");
    return;

  }

  {//SVP
    vtkSlicerVolumePropertyWidget *callerObjectSVP = vtkSlicerVolumePropertyWidget::SafeDownCast(caller);
    if(callerObjectSVP == this->SVP_VolumePropertyWidget && eid == vtkKWEvent::VolumePropertyChangingEvent)
    {
      vtkMRMLVolumePropertyNode *vpNode = this->Gui->GetVolumePropertyNode();
      if (vpNode)
      {
        vpNode->SetModifiedSinceRead(1);
      }

      if (vspNode->GetCurrentVolumeMapper() == 4)//multi-volume
        this->Gui->GetLogic()->UpdateVolumePropertyGPURaycastII(vspNode);
        
      this->Gui->RequestRender();
      return;
    }
    if(callerObjectSVP == this->SVP_VolumePropertyWidgetFg && eid == vtkKWEvent::VolumePropertyChangingEvent)
    {
      vtkMRMLVolumePropertyNode *vpNode = this->Gui->GetFgVolumePropertyNode();
      if (vpNode)
      {
        vpNode->SetModifiedSinceRead(1);
      }

      this->Gui->GetLogic()->UpdateVolumePropertyGPURaycastII(vspNode);
      this->Gui->RequestRender();
      return;
    }
  }

  //scales
  {
    vtkKWScale *callerObjectSC = vtkKWScale::SafeDownCast(caller);
    if(callerObjectSC == this->SC_ExpectedFPS->GetWidget())
    {
      this->ProcessExpectedFPS();
      return;
    }

    if(callerObjectSC == this->SC_GPURayCastDistanceColorBlending->GetWidget())
    {
      vspNode->SetDistanceColorBlending(this->SC_GPURayCastDistanceColorBlending->GetWidget()->GetValue());

      this->Gui->GetLogic()->SetGPURaycastParameters(vspNode);
      this->Gui->RequestRender();
      return;
    }
    
    if(callerObjectSC == this->SC_GPURayCastDepthPeelingThreshold->GetWidget())
    {
      vspNode->SetDepthPeelingThreshold(this->SC_GPURayCastDepthPeelingThreshold->GetWidget()->GetValue());

      this->Gui->GetLogic()->SetGPURaycastParameters(vspNode);
      this->Gui->RequestRender();
      return;
    }

    if(callerObjectSC == this->SC_GPURayCastICPEkt->GetWidget())
    {
      vspNode->SetICPEScale(this->SC_GPURayCastICPEkt->GetWidget()->GetValue());

      this->Gui->GetLogic()->SetGPURaycastParameters(vspNode);
      this->Gui->RequestRender();
      return;
    }

    if(callerObjectSC == this->SC_GPURayCastICPEks->GetWidget())
    {
      vspNode->SetICPESmoothness(this->SC_GPURayCastICPEks->GetWidget()->GetValue());

      this->Gui->GetLogic()->SetGPURaycastParameters(vspNode);
      this->Gui->RequestRender();
      return;
    }

    if(callerObjectSC == this->SC_ThresholdOpacity->GetWidget())
    {
      this->ProcessThreshold(0,0);
      return;
    }

    if(callerObjectSC == this->SC_ThresholdOpacityFg->GetWidget())
    {
      this->ProcessThresholdFg(0,0);
      return;
    }
  }

  //scales with entry
  {
    vtkKWScaleWithEntry *callerObjectSC = vtkKWScaleWithEntry::SafeDownCast(caller);

    if(callerObjectSC == this->SC_GPURayCastIIFgBgRatio)
    {
      vspNode->SetGPURaycastIIBgFgRatio(this->SC_GPURayCastIIFgBgRatio->GetValue());

      this->Gui->GetLogic()->SetGPURaycastIIParameters(vspNode);
      this->Gui->RequestRender();
      return;
    }
  }

  //Check the checkbuttons
  {
    vtkKWCheckButton *callerObjectCheckButton = vtkKWCheckButton::SafeDownCast(caller);

    if (callerObjectCheckButton == this->CB_CPURayCastMIP->GetWidget())
    {
      vspNode->SetCPURaycastMode(this->CB_CPURayCastMIP->GetWidget()->GetSelectedState());

      this->Gui->GetLogic()->SetCPURaycastParameters(vspNode);
      this->Gui->RequestRender();
      return;
    }
    else if (callerObjectCheckButton == this->CB_CroppingButton->GetWidget())
    {
      vspNode->SetCroppingEnabled(this->CB_CroppingButton->GetWidget()->GetSelectedState());
      this->Gui->GetLogic()->SetROI(vspNode);
      this->Gui->RequestRender();
      return;
    }
    else if (callerObjectCheckButton == this->CB_UseSingleVolumeProperty->GetWidget())
    {
      vspNode->SetUseSingleVolumeProperty(this->CB_UseSingleVolumeProperty->GetWidget()->GetSelectedState());
        
      if (this->CB_UseSingleVolumeProperty->GetWidget()->GetSelectedState())
      {
        if (vspNode->GetUseFgThreshold())
          this->FrameThresholdingFg->CollapseFrame();
        else
          this->SVP_VolumePropertyWidgetFg->GetEditorFrame()->CollapseFrame();
        
        this->CB_UseThresholdFg->EnabledOff();
      }
      else
      {
        if (vspNode->GetUseFgThreshold())
          this->FrameThresholdingFg->ExpandFrame();
        else
          this->SVP_VolumePropertyWidgetFg->GetEditorFrame()->ExpandFrame();
        
        this->CB_UseThresholdFg->EnabledOn();
      }

      this->Gui->GetLogic()->UpdateVolumePropertyGPURaycastII(vspNode);
      this->Gui->RequestRender();
      
      return;
    }
    else if (callerObjectCheckButton == this->CB_FollowVolumeDisplayNode->GetWidget())
    {
      vspNode->SetFollowVolumeDisplayNode(this->CB_FollowVolumeDisplayNode->GetWidget()->GetSelectedState());

      if (this->CB_FollowVolumeDisplayNode->GetWidget()->GetSelectedState())
      {
        this->CB_UseThreshold->EnabledOff();
        this->SVP_VolumePropertyWidget->GetEditorFrame()->CollapseFrame();
        this->FrameThresholding->CollapseFrame();

        this->Gui->PauseRenderInteraction();
        
        this->Gui->GetLogic()->UpdateVolumePropertyByDisplayNode(vspNode);
        this->UpdateVolumeProperty();

        this->Gui->ResumeRenderInteraction();
        this->Gui->RequestRender();
      }
      else
      {
        this->CB_UseThreshold->EnabledOn();
        if (vspNode->GetUseThreshold())
          this->FrameThresholding->ExpandFrame();
        else
          this->SVP_VolumePropertyWidget->GetEditorFrame()->ExpandFrame();
      }
      
      return;
    }
    else if(callerObjectCheckButton == this->CB_UseThreshold->GetWidget())
    {
      vspNode->SetUseThreshold(this->CB_UseThreshold->GetWidget()->GetSelectedState());

      if (this->CB_UseThreshold->GetWidget()->GetSelectedState())
      {
        this->SVP_VolumePropertyWidget->GetEditorFrame()->CollapseFrame();
        this->FrameThresholding->ExpandFrame();
        this->ProcessThreshold(0, 0);//parameters not used
      }
      else
      {
        this->SVP_VolumePropertyWidget->GetEditorFrame()->ExpandFrame();
        this->SVP_VolumePropertyWidget->Update();
        this->FrameThresholding->CollapseFrame();
      }
      return;
    }
    else if(callerObjectCheckButton == this->CB_UseThresholdFg->GetWidget())
    {
      if (vspNode->GetFgVolumeNode() == NULL)
        return;
        
      vspNode->SetUseFgThreshold(this->CB_UseThresholdFg->GetWidget()->GetSelectedState());

      if (this->CB_UseThresholdFg->GetWidget()->GetSelectedState())
      {
        this->SVP_VolumePropertyWidgetFg->GetEditorFrame()->CollapseFrame();
        this->FrameThresholdingFg->ExpandFrame();
        this->ProcessThresholdFg(0, 0);//parameters not used
      }
      else
      {
        this->SVP_VolumePropertyWidgetFg->GetEditorFrame()->ExpandFrame();
        this->SVP_VolumePropertyWidgetFg->Update();
        this->FrameThresholdingFg->CollapseFrame();
      }
      return;
    }
  }

  {//push button
    if (vtkKWPushButton::SafeDownCast(caller) == this->PB_FitROIButton && eid == vtkKWPushButton::InvokedEvent )
    {
      this->Gui->GetLogic()->FitROIToVolume(vspNode);
      this->Gui->GetLogic()->SetROI(vspNode);
      this->SetROIRange(vspNode);
      // for the case when volume extents were outside the range
      this->ROIWidget->UpdateWidget();

      this->Gui->RequestRender();
      return;
    }
  }
}

void vtkSlicerVolumeRenderingHelper::SetROIRange(vtkMRMLVolumeRenderingParametersNode* vspNode)
{
  if (vspNode == NULL)
  {
    return;
  }
  vtkMRMLROINode *roiNode = vtkMRMLROINode::SafeDownCast(vspNode->GetROINode());
  vtkMRMLScalarVolumeNode *volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode());

  if (volumeNode && roiNode)
  {
    double xyz[3];
    double center[3];
    double bounds[6];

    roiNode->GetXYZ(center);
    roiNode->GetRadiusXYZ(xyz);

    for (int i = 0; i < 3; i++)
    {
      bounds[2*i  ] = center[i] - 1.1*xyz[i];
      bounds[2*i+1] = center[i] + 1.1*xyz[i];
    }
    this->ROIWidget->SetXRangeExtent(bounds[0], bounds[1]);
    this->ROIWidget->SetYRangeExtent(bounds[2], bounds[3]);
    this->ROIWidget->SetZRangeExtent(bounds[4], bounds[5]);

    this->ROIWidget->SetXResolution((bounds[1] - bounds[0])*0.01);
    this->ROIWidget->SetYResolution((bounds[3] - bounds[2])*0.01);
    this->ROIWidget->SetZResolution((bounds[5] - bounds[4])*0.01);
  }
}

void vtkSlicerVolumeRenderingHelper::SetupGUIFromParametersNode(vtkMRMLVolumeRenderingParametersNode* vspNode)
{
  this->SetupGUIFromParametersNodeFlag = 1;

  //fps
  this->SC_ExpectedFPS->GetWidget()->SetRange(1, 20);
  this->SC_ExpectedFPS->GetWidget()->SetResolution(1);
  this->SC_ExpectedFPS->SetValue(vspNode->GetExpectedFPS());

  //performance control
  switch(vspNode->GetPerformanceControl())
  {
    case 0:
      this->MB_PerformanceControl->GetWidget()->SetValue("Adaptive");
      break;
    case 1:
      this->MB_PerformanceControl->GetWidget()->SetValue("Maximum Quality");
      break;
    case 2:
      this->MB_PerformanceControl->GetWidget()->SetValue("Fixed Framerate");
      break;
  }
  
  //-------------------------techniques----------------------------
  this->CB_CPURayCastMIP->GetWidget()->SetSelectedState(vspNode->GetCPURaycastMode());

  switch(vspNode->GetGPURaycastTechnique())
  {
    case 0:
      this->MB_GPURayCastTechnique->GetWidget()->SetValue("Composite With Shading");
      break;
    case 1:
      this->MB_GPURayCastTechnique->GetWidget()->SetValue("Composite With Fast Psuedo Shading");
      break;
    case 2:
      this->MB_GPURayCastTechnique->GetWidget()->SetValue("Maximum Intensity Projection");
      break;
    case 3:
      this->MB_GPURayCastTechnique->GetWidget()->SetValue("Minimum Intensity Projection");
      break;
    case 4:
      this->MB_GPURayCastTechnique->GetWidget()->SetValue("Gradient Magnitude Opacity Modulation");
      break;
    case 5:
      this->MB_GPURayCastTechnique->GetWidget()->SetValue("Illustrative Context Preserving Exploration");
      break;
  }

  switch(vspNode->GetGPURaycastTechniqueII())
  {
    case 0:
      this->MB_GPURayCastTechniqueII->GetWidget()->SetValue("Composite With Shading");
      break;
    case 1:
      this->MB_GPURayCastTechniqueII->GetWidget()->SetValue("Composite With Fast Psuedo Shading");
      break;
    case 2:
      this->MB_GPURayCastTechniqueII->GetWidget()->SetValue("Maximum Intensity Projection");
      break;
    case 3:
      this->MB_GPURayCastTechniqueII->GetWidget()->SetValue("Minimum Intensity Projection");
      break;
    case 4:
      this->MB_GPURayCastTechniqueII->GetWidget()->SetValue("Gradient Magnitude Opacity Modulation");
      break;
    case 5:
      this->MB_GPURayCastTechniqueII->GetWidget()->SetValue("Illustrative Context Preserving Exploration");
      break;
  }

  switch(vspNode->GetGPURaycastTechniqueIIFg())
  {
    case 0:
      this->MB_GPURayCastTechniqueIIFg->GetWidget()->SetValue("Composite With Shading");
      break;
    case 1:
      this->MB_GPURayCastTechniqueIIFg->GetWidget()->SetValue("Composite With Fast Psuedo Shading");
      break;
    case 2:
      this->MB_GPURayCastTechniqueIIFg->GetWidget()->SetValue("Maximum Intensity Projection");
      break;
    case 3:
      this->MB_GPURayCastTechniqueIIFg->GetWidget()->SetValue("Minimum Intensity Projection");
      break;
    case 4:
      this->MB_GPURayCastTechniqueIIFg->GetWidget()->SetValue("Gradient Magnitude Opacity Modulation");
      break;
    case 5:
      this->MB_GPURayCastTechniqueIIFg->GetWidget()->SetValue("Illustrative Context Preserving Exploration");
      break;
  }

  switch(vspNode->GetGPURaycastIIFusion())
  {
    case 0:
      this->MB_GPURayCastColorOpacityFusion->GetWidget()->SetValue("Alpha Blending (OR)");
      break;
    case 1:
      this->MB_GPURayCastColorOpacityFusion->GetWidget()->SetValue("Alpha Blending (AND)");
      break;
    case 2:
      this->MB_GPURayCastColorOpacityFusion->GetWidget()->SetValue("Alpha Blending (NOT)");
      break;
  }

  switch(vspNode->GetGPURaycastTechnique3())
  {
    case 0:
      this->MB_GPURayCastTechnique3->GetWidget()->SetValue("Composite With Shading");
      break;
    case 1:
      this->MB_GPURayCastTechnique3->GetWidget()->SetValue("Composite With Fast Psuedo Shading");
      break;
    case 2:
      this->MB_GPURayCastTechnique3->GetWidget()->SetValue("Maximum Intensity Projection");
      break;
    case 3:
      this->MB_GPURayCastTechnique3->GetWidget()->SetValue("Minimum Intensity Projection");
      break;
    case 4:
      this->MB_GPURayCastTechnique3->GetWidget()->SetValue("Gradient Magnitude Opacity Modulation");
      break;
    case 5:
      this->MB_GPURayCastTechnique3->GetWidget()->SetValue("Illustrative Context Preserving Exploration");
      break;
  }

  this->SC_GPURayCastDistanceColorBlending->GetWidget()->SetRange(0, 1);
  this->SC_GPURayCastDistanceColorBlending->GetWidget()->SetResolution(0.01);
  this->SC_GPURayCastDistanceColorBlending->SetValue(vspNode->GetDistanceColorBlending());
  
  this->SC_GPURayCastICPEkt->GetWidget()->SetRange(0, 20);
  this->SC_GPURayCastICPEkt->GetWidget()->SetResolution(0.01);
  this->SC_GPURayCastICPEkt->SetValue(vspNode->GetICPEScale());

  this->SC_GPURayCastICPEks->GetWidget()->SetRange(0, 1);
  this->SC_GPURayCastICPEks->GetWidget()->SetResolution(0.01);
  this->SC_GPURayCastICPEks->SetValue(vspNode->GetICPESmoothness());

  //get scalar range
  double scalarRange[2];
  vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData()->GetPointData()->GetScalars()->GetRange(scalarRange, 0);
  this->SC_GPURayCastDepthPeelingThreshold->GetWidget()->SetRange(scalarRange[0],scalarRange[1]);
  this->SC_GPURayCastDepthPeelingThreshold->GetWidget()->SetResolution((scalarRange[1] - scalarRange[0])*0.01);
  this->SC_GPURayCastDepthPeelingThreshold->SetValue(vspNode->GetDepthPeelingThreshold());

  this->SC_GPURayCastIIFgBgRatio->SetRange(0, 1);
  this->SC_GPURayCastIIFgBgRatio->SetResolution(0.01);
  this->SC_GPURayCastIIFgBgRatio->SetValue(vspNode->GetGPURaycastIIBgFgRatio());

  this->CB_FollowVolumeDisplayNode->GetWidget()->SetSelectedState(vspNode->GetFollowVolumeDisplayNode());
  
  //-------------------------bg threshold--------------------------
  this->CB_UseThreshold->GetWidget()->SetSelectedState(vspNode->GetUseThreshold());
  this->RA_Threshold->SetWholeRange(scalarRange);
  this->RA_Threshold->SetRange(vspNode->GetThreshold());
  this->RA_Threshold->SetResolution((scalarRange[1] - scalarRange[0])*0.01);

  //-------------------------bg volume property--------------------
  this->SVP_VolumePropertyWidget->SetDataSet(vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData());

  this->SVP_VolumePropertyWidget->SetHistogramSet(this->Gui->GetLogic()->GetHistogramSet());
  this->SVP_VolumePropertyWidget->SetVolumeProperty(vspNode->GetVolumePropertyNode()->GetVolumeProperty());
  this->SVP_VolumePropertyWidget->Update();

  this->SC_ThresholdOpacity->GetWidget()->SetRange(0, 1);
  this->SC_ThresholdOpacity->GetWidget()->SetResolution(.001);
  this->SC_ThresholdOpacity->SetValue(1.0);

  if (vspNode->GetFollowVolumeDisplayNode())
  {
    this->CB_UseThreshold->EnabledOff();
    this->SVP_VolumePropertyWidget->GetEditorFrame()->CollapseFrame();
    this->FrameThresholding->CollapseFrame();
  }
  
  if (vspNode->GetFgVolumeNode())
  {
    vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetFgVolumeNode())->GetImageData()->GetPointData()->GetScalars()->GetRange(scalarRange, 0);

    //-------------------------fg threshold--------------------------
    this->CB_UseThresholdFg->GetWidget()->SetSelectedState(vspNode->GetUseFgThreshold());
    this->RA_ThresholdFg->SetWholeRange(scalarRange);
    this->RA_ThresholdFg->SetRange(vspNode->GetThresholdFg());
    this->RA_ThresholdFg->SetResolution((scalarRange[1] - scalarRange[0])*0.01);

    //-------------------------fg volume property--------------------
    this->SVP_VolumePropertyWidgetFg->SetDataSet(vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetFgVolumeNode())->GetImageData());

    this->SVP_VolumePropertyWidgetFg->SetHistogramSet(this->Gui->GetLogic()->GetHistogramSetFg());
    this->SVP_VolumePropertyWidgetFg->SetVolumeProperty(vspNode->GetFgVolumePropertyNode()->GetVolumeProperty());
    this->SVP_VolumePropertyWidgetFg->Update();

    this->SC_ThresholdOpacityFg->GetWidget()->SetRange(0, 1);
    this->SC_ThresholdOpacityFg->GetWidget()->SetResolution(.001);
    this->SC_ThresholdOpacityFg->SetValue(0.95);
  }

  //------------------------gpu memory size------------------------
  //default 256M
  int id = 1;

  if(this->Gui->GetApplication()->HasRegistryValue(2,"VolumeRendering","MB_GPUMemorySize"))
    id = this->Gui->GetApplication()->GetIntRegistryValue(2,"VolumeRendering","MB_GPUMemorySize");

  vspNode->SetGPUMemorySize(id);//save it so logic knows, need a way to detect hardware property instead of user selection
  this->MB_GPUMemorySize->GetWidget()->GetMenu()->SelectItem(id);

  //------------------------hook up mapper for rendering
  int mapper = this->Gui->GetCurrentParametersNode()->GetCurrentVolumeMapper();

  if (mapper == -1)
  {
    // load registry
    if(this->Gui->GetApplication()->HasRegistryValue(2,"VolumeRendering","MB_Mapper"))
      mapper = this->Gui->GetApplication()->GetIntRegistryValue(2,"VolumeRendering","MB_Mapper");
    else
      mapper = 0;

    vspNode->SetCurrentVolumeMapper(mapper);
  }

  this->MB_Mapper->GetWidget()->GetMenu()->SelectItem(mapper);

  this->ProcessRenderingMethodEvents(vspNode->GetCurrentVolumeMapper());

  this->SetupGUIFromParametersNodeFlag = 0;
}

void vtkSlicerVolumeRenderingHelper::ProcessGPURayCastTechnique(int id)
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();

  vspNode->SetGPURaycastTechnique(id);

  this->Gui->GetLogic()->SetGPURaycastParameters(vspNode);
  this->Gui->RequestRender();
}

void vtkSlicerVolumeRenderingHelper::ProcessGPURayCastTechniqueII(int id)
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();

  vspNode->SetGPURaycastTechniqueII(id);

  this->Gui->GetLogic()->SetGPURaycastIIParameters(vspNode);
  this->Gui->RequestRender();
}

void vtkSlicerVolumeRenderingHelper::ProcessGPURayCastTechniqueIIFg(int id)
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();

  vspNode->SetGPURaycastTechniqueIIFg(id);

  this->Gui->GetLogic()->SetGPURaycastIIParameters(vspNode);
  this->Gui->RequestRender();
}

void vtkSlicerVolumeRenderingHelper::ProcessGPURayCastTechnique3(int id)
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();

  vspNode->SetGPURaycastTechnique3(id);

  this->Gui->GetLogic()->SetGPURaycast3Parameters(vspNode);
  this->Gui->RequestRender();
}

void vtkSlicerVolumeRenderingHelper::ProcessGPURayCastColorOpacityFusion(int id)
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();

  vspNode->SetGPURaycastIIFusion(id);

  this->Gui->GetLogic()->SetGPURaycastIIParameters(vspNode);
  this->Gui->RequestRender();
}

void vtkSlicerVolumeRenderingHelper::ProcessGPUMemorySize(int id)
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();

  vspNode->SetGPUMemorySize(id);

  this->Gui->GetLogic()->SetGPUMemorySize(vspNode);
  this->Gui->RequestRender();
}

void vtkSlicerVolumeRenderingHelper::ProcessRenderingMethodEvents(int id)
{
  //abort current (cpu ray cast) rendering
  int numViewer = this->Gui->GetApplicationGUI()->GetNumberOfViewerWidgets();
  
  for (int i = 0; i < numViewer; i++)
  {
    vtkSlicerViewerWidget *slicer_viewer_widget = this->Gui->GetApplicationGUI()->GetNthViewerWidget(i);
    if (slicer_viewer_widget)
    {
      slicer_viewer_widget->GetMainViewer()->GetRenderWindow()->SetAbortRender(1);
    }
  }

  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();

  vspNode->SetCurrentVolumeMapper(id);

  int success = this->Gui->GetLogic()->SetupMapperFromParametersNode(vspNode);

  this->FrameGPURayCasting->CollapseFrame();
  this->FrameGPURayCastingII->CollapseFrame();
  this->FrameGPURayCasting3->CollapseFrame();
  this->FramePolygonBlending->CollapseFrame();
  this->FrameCPURayCasting->CollapseFrame();

  switch(id)
  {
  case 0://softwrae ray casting
    this->FrameCPURayCasting->ExpandFrame();
    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using VTK CPU Raycasting");
    break;
  case 3://gpu ray casting
    if (success)
    {
      this->FrameGPURayCasting->ExpandFrame();
      this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using NCI GPU Raycasting");
    }
    else
      this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("NCI GPU ray casting is not supported by your computer.");
    break;
  case 4://gpu ray casting II
    if (success)
    {
      this->FrameGPURayCastingII->ExpandFrame();
      this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using NCI GPU Raycasting (Multi-Volume). Experimental.");
    }
    else
      this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("NCI GPU ray casting (Multi-Volume) is not supported by your computer.");
    break;
  case 2://old school opengl 2D Polygon Texture 3D
    if (success)
    {
      this->FramePolygonBlending->ExpandFrame();
      this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using VTK OpenGL 3D Texture Mapping");
    }
    else//seldom should we see this error message unless really low end graphics card...
      this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("VTK OpenGL 3D Texture Mapping is not supported by your computer.");
    break;
  case 1://vtk edge gpu ray casting
    if (success)
      {
      this->FrameGPURayCasting3->ExpandFrame();
      this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using VTK GPU Raycasting");
      }
    else
      this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("VTK GPU ray casting is not supported by your computer.");
    break;
  }

  //turn off possible hanging progress message from cpu ray casting
  char buf[4] = "";
  this->Gui->GetApplicationGUI()->SetExternalProgress(buf, 0.0);
  
  this->Gui->RequestRender();
}

void vtkSlicerVolumeRenderingHelper::ProcessPauseResume(void)
{
  //Resume Rendering
  if(this->RenderingPaused == 1)
  {
    this->RenderingPaused = 0;
    this->Gui->GetLogic()->SetVolumeVisibility(1);
    
    this->PB_PauseResume->GetWidget()->SetImageToIcon(this->VI_PauseResume->GetVisibleIcon());;
  }
  else//Pause Rendering
  {
    this->RenderingPaused = 1;
    this->Gui->GetLogic()->SetVolumeVisibility(0);
    
    this->PB_PauseResume->GetWidget()->SetImageToIcon(this->VI_PauseResume->GetInvisibleIcon());
  }

  this->Gui->RequestRender();
}
void vtkSlicerVolumeRenderingHelper::ProcessThreshold(double, double)
{
  if (this->SetupGUIFromParametersNodeFlag)
    return;

  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();
  if (!vspNode->GetUseThreshold())
    return;

  vtkImageData *iData = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData();

  //Delete all old Mapping Points
  vtkPiecewiseFunction *opacity = vspNode->GetVolumePropertyNode()->GetVolumeProperty()->GetScalarOpacity();
  opacity->RemoveAllPoints();

  double step = (iData->GetScalarRange()[1] - iData->GetScalarRange()[0]) * 0.001;

  opacity->AddPoint(iData->GetScalarRange()[0], 0.0);
  opacity->AddPoint(iData->GetScalarRange()[1], 0.0);

  opacity->AddPoint(this->RA_Threshold->GetRange()[0], 0.0);
  opacity->AddPoint(this->RA_Threshold->GetRange()[0] + step, this->SC_ThresholdOpacity->GetWidget()->GetValue());
  opacity->AddPoint(this->RA_Threshold->GetRange()[1] - step, this->SC_ThresholdOpacity->GetWidget()->GetValue());
  opacity->AddPoint(this->RA_Threshold->GetRange()[1], 0.0);

  vspNode->SetThreshold(this->RA_Threshold->GetRange());

  this->Gui->RequestRender();
}

void vtkSlicerVolumeRenderingHelper::ProcessThresholdFg(double, double)
{
  if (this->SetupGUIFromParametersNodeFlag)
    return;

  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();
  if (!vspNode->GetUseFgThreshold())
    return;

  vtkImageData *iData = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetFgVolumeNode())->GetImageData();

  //Delete all old Mapping Points
  vtkPiecewiseFunction *opacity = vspNode->GetFgVolumePropertyNode()->GetVolumeProperty()->GetScalarOpacity();
  opacity->RemoveAllPoints();

  double step = (iData->GetScalarRange()[1] - iData->GetScalarRange()[0]) * 0.001;

  opacity->AddPoint(iData->GetScalarRange()[0], 0.0);
  opacity->AddPoint(iData->GetScalarRange()[1], 0.0);

  opacity->AddPoint(this->RA_ThresholdFg->GetRange()[0], 0.0);
  opacity->AddPoint(this->RA_ThresholdFg->GetRange()[0] + step, this->SC_ThresholdOpacityFg->GetWidget()->GetValue());
  opacity->AddPoint(this->RA_ThresholdFg->GetRange()[1] - step, this->SC_ThresholdOpacityFg->GetWidget()->GetValue());
  opacity->AddPoint(this->RA_ThresholdFg->GetRange()[1], 0.0);

  vspNode->SetThreshold(this->RA_ThresholdFg->GetRange());

  this->Gui->RequestRender();
}

void vtkSlicerVolumeRenderingHelper::ProcessPerformanceControl(int id)
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();

  vspNode->SetPerformanceControl(id);

  this->ProcessExpectedFPS();
}

void vtkSlicerVolumeRenderingHelper::ProcessExpectedFPS(void)
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();
  
  int fps = this->SC_ExpectedFPS->GetValue();

  switch(vspNode->GetPerformanceControl())
  {
    case 0:
      break;
    case 1:
      fps = 0;
      break;
    case 2:
      break;
  }
  
  vspNode->SetExpectedFPS(fps);

  this->Gui->GetLogic()->SetExpectedFPS(vspNode);

  int numViewer = this->Gui->GetApplicationGUI()->GetNumberOfViewerWidgets();
  for (int i = 0; i < numViewer; i++)
    {
    vtkSlicerViewerWidget *slicer_viewer_widget = this->Gui->GetApplicationGUI()->GetNthViewerWidget(i);
    if (slicer_viewer_widget)
      {
      vtkRenderWindow* renderWindow = slicer_viewer_widget->GetMainViewer()->GetRenderWindow();
      renderWindow->SetDesiredUpdateRate(fps);
      renderWindow->GetInteractor()->SetDesiredUpdateRate(fps);
      }
    }

  this->Gui->RequestRender();
}

void vtkSlicerVolumeRenderingHelper::UpdateROI()
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();

  vtkMRMLROINode *roiNode = vspNode->GetROINode();

  this->ROIWidget->SetROINode(roiNode);

  this->CB_CroppingButton->GetWidget()->SetSelectedState(vspNode->GetCroppingEnabled());
}

void vtkSlicerVolumeRenderingHelper::UpdateVolumeProperty()
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();

  this->SVP_VolumePropertyWidget->SetHistogramSet(this->Gui->GetLogic()->GetHistogramSet());
  this->SVP_VolumePropertyWidget->SetVolumeProperty(vspNode->GetVolumePropertyNode()->GetVolumeProperty());
  this->SVP_VolumePropertyWidget->Update();
}

void vtkSlicerVolumeRenderingHelper::UpdateVolumePropertyFg()
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();

  this->SVP_VolumePropertyWidgetFg->SetHistogramSet(this->Gui->GetLogic()->GetHistogramSetFg());
  this->SVP_VolumePropertyWidgetFg->SetVolumeProperty(vspNode->GetFgVolumePropertyNode()->GetVolumeProperty());
  this->SVP_VolumePropertyWidgetFg->Update();
}
