//Slicer
#include "vtkSlicerVolumeTextureMapper3D.h"
#include "vtkSlicerGPUVolumeTextureMapper3D.h"
#include "vtkSlicerGPURayCastVolumeTextureMapper3D.h"
#include "vtkSlicerGPURayCastVolumeMapper.h"
#include "vtkSlicerFixedPointVolumeRayCastMapper.h"
#include "vtkSlicerVolumeRenderingHelper.h"
#include "vtkVolumeRenderingGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerColorDisplayWidget.h"
#include "vtkSlicerVolumePropertyWidget.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkSlicerVisibilityIcons.h"
#include "vtkSlicerVRMenuButtonColorMode.h"

//CUDA
#include "vtkCudaVolumeMapper.h"

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

  //GUI:
  this->CB_CUDARayCastShading = NULL;

  this->MB_GPURayCastTechnique = NULL;
  
  this->MB_GPURayCastTechniqueII = NULL;
  this->MB_GPURayCastTechniqueIIFg = NULL;
  
  this->SC_GPURayCastDepthPeelingThreshold = NULL;
  this->SC_GPURayCastICPEkt = NULL;
  this->SC_GPURayCastICPEks = NULL;
  this->MB_GPUMemorySize = NULL;
  this->MB_GPURayCastColorOpacityFusion = NULL;

  this->SC_ExpectedFPS = NULL;
  this->SC_GPURayCastIIFgBgRatio = NULL;
  this->MB_Mapper = NULL;

  this->FrameFPS = NULL;
  this->FrameGPURayCasting = NULL;
  this->FramePolygonBlending = NULL;
  this->FrameCPURayCasting = NULL;
  this->FrameTechniques = NULL;

  this->PB_HideSurfaceModels = NULL;

  this->SVP_VolumePropertyWidget = NULL;
  this->SVP_VolumePropertyWidgetFg = NULL;

  this->ROIWidget = NULL;
  this->CroppingButton = NULL;
  this->FitROIButton = NULL;
  
  this->CB_UseThreshold = NULL;
  this->FrameThresholding = NULL;
  this->RA_Threshold = NULL;
  
  this->CB_UseThresholdFg = NULL;
  this->FrameThresholdingFg = NULL;
  this->RA_ThresholdFg = NULL;

  //PauseResume
  this->PB_PauseResume = NULL;
  this->VI_PauseResume = NULL;

  this->CPURayCastingInteractionFlag = 0;

  this->RenderingPaused = 0;
}

vtkSlicerVolumeRenderingHelper::~vtkSlicerVolumeRenderingHelper(void)
{
  this->Gui->Script("bind all <Any-ButtonPress> {}",this->GetTclName());
  this->Gui->Script("bind all <Any-ButtonRelease> {}",this->GetTclName());

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

  //hookup mouse button interaction (needed by CPU ray cast)
  this->Gui->Script("bind all <Any-ButtonPress> {%s SetButtonDown 1}", this->GetTclName());
  this->Gui->Script("bind all <Any-ButtonRelease> {%s SetButtonDown 0}", this->GetTclName());
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
    this->MB_Mapper->GetWidget()->GetMenu()->AddRadioButton("Software Ray Casting");
    this->MB_Mapper->GetWidget()->GetMenu()->SetItemCommand(0, this,"ProcessRenderingMethodEvents 0");
    this->MB_Mapper->GetWidget()->GetMenu()->AddRadioButton("GPU Ray Casting (GLSL)");
    this->MB_Mapper->GetWidget()->GetMenu()->SetItemCommand(1, this,"ProcessRenderingMethodEvents 1");
    this->MB_Mapper->GetWidget()->GetMenu()->AddRadioButton("GPU Ray Casting II (GLSL)");
    this->MB_Mapper->GetWidget()->GetMenu()->SetItemCommand(2, this,"ProcessRenderingMethodEvents 2");
    this->MB_Mapper->GetWidget()->GetMenu()->AddRadioButton("OpenGL Polygon Texture 3D");
    this->MB_Mapper->GetWidget()->GetMenu()->SetItemCommand(3, this,"ProcessRenderingMethodEvents 3");
    this->MB_Mapper->GetWidget()->GetMenu()->AddRadioButton("CUDA Ray Casting");
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
    this->MB_GPUMemorySize->SetBalloonHelpString("Specify size of your GPU memory. Generally the larger GPU memory the better rendering quality. Do not select memory size larger than physical GPU memory size.");
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
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->MB_GPUMemorySize->GetWidgetName() );
  }

  //Framerate
  {
    this->FrameFPS = vtkKWFrameWithLabel::New();
    this->FrameFPS->SetParent(this->FrameTechniques->GetFrame());
    this->FrameFPS->Create();
    this->FrameFPS->AllowFrameToCollapseOff();
    this->FrameFPS->SetLabelText("Expected Framerate (FPS)");
    this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FrameFPS->GetWidgetName() );

    this->SC_ExpectedFPS=vtkKWScale::New();
    this->SC_ExpectedFPS->SetParent(this->FrameFPS->GetFrame());
    this->SC_ExpectedFPS->Create();
    this->SC_ExpectedFPS->SetBalloonHelpString("Adjust performance/quality. 1 fps: low performance/high quality. 20 fps: high performance/low quality.");
    this->SC_ExpectedFPS->SetRange(1,20);
    this->SC_ExpectedFPS->SetResolution(1);
    this->SC_ExpectedFPS->SetValue(5.0);
    this->SC_ExpectedFPS->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *) this->GUICallbackCommand);
    this->SC_ExpectedFPS->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *) this->GUICallbackCommand);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->SC_ExpectedFPS->GetWidgetName() );

  }

  labelWidth = 24;
  //software ray casting
  {
    this->FrameCPURayCasting = vtkKWFrameWithLabel::New();
    this->FrameCPURayCasting->SetParent(this->FrameTechniques->GetFrame());
    this->FrameCPURayCasting->Create();
    this->FrameCPURayCasting->AllowFrameToCollapseOff();
    this->FrameCPURayCasting->SetLabelText("Software Ray Casting");
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
  //GPU ray casting
  {
    this->FrameGPURayCasting = vtkKWFrameWithLabel::New();
    this->FrameGPURayCasting->SetParent(this->FrameTechniques->GetFrame());
    this->FrameGPURayCasting->Create();
    this->FrameGPURayCasting->AllowFrameToCollapseOff();
    this->FrameGPURayCasting->SetLabelText("GPU Ray Casting");
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

    this->SC_GPURayCastICPEkt = vtkKWScaleWithEntry::New();
    this->SC_GPURayCastICPEkt->SetParent(this->FrameGPURayCasting->GetFrame());
    this->SC_GPURayCastICPEkt->Create();
    this->SC_GPURayCastICPEkt->SetLabelText("ICPE Scale:");
    this->SC_GPURayCastICPEkt->SetLabelWidth(labelWidth);
    this->SC_GPURayCastICPEkt->SetBalloonHelpString("Parameter kt for Illustrative Context Preserving Exploration.");
    this->SC_GPURayCastICPEkt->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangingEvent, (vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastICPEkt->GetWidget()->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *) this->GUICallbackCommand);
    this->SC_GPURayCastICPEkt->SetEntryWidth(5);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->SC_GPURayCastICPEkt->GetWidgetName() );

    this->SC_GPURayCastICPEks = vtkKWScaleWithEntry::New();
    this->SC_GPURayCastICPEks->SetParent(this->FrameGPURayCasting->GetFrame());
    this->SC_GPURayCastICPEks->Create();
    this->SC_GPURayCastICPEks->SetLabelText("ICPE Smoothness:");
    this->SC_GPURayCastICPEks->SetLabelWidth(labelWidth);
    this->SC_GPURayCastICPEks->SetBalloonHelpString("Parameter ks for Illustrative Context Preserving Exploration.");
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
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->SC_GPURayCastDepthPeelingThreshold->GetWidgetName() );
  }

  //GPU ray casting II
  {
    this->FrameGPURayCastingII = vtkKWFrameWithLabel::New();
    this->FrameGPURayCastingII->SetParent(this->FrameTechniques->GetFrame());
    this->FrameGPURayCastingII->Create();
    this->FrameGPURayCastingII->AllowFrameToCollapseOff();
    this->FrameGPURayCastingII->SetLabelText("GPU Ray Casting II");
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

    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->MB_GPURayCastTechniqueII->GetWidgetName() );

    //set technique
    this->MB_GPURayCastTechniqueIIFg = vtkKWMenuButtonWithLabel::New();
    this->MB_GPURayCastTechniqueIIFg->SetParent(this->FrameGPURayCastingII->GetFrame());
    this->MB_GPURayCastTechniqueIIFg->SetLabelText("Technique (Fg):");
    this->MB_GPURayCastTechniqueIIFg->Create();
    this->MB_GPURayCastTechniqueIIFg->SetLabelWidth(labelWidth);
    this->MB_GPURayCastTechniqueIIFg->SetBalloonHelpString("Select GPU ray casting technique for fg volume");
    this->MB_GPURayCastTechniqueIIFg->GetWidget()->GetMenu()->AddRadioButton("Composite With Shading");
    this->MB_GPURayCastTechniqueIIFg->GetWidget()->GetMenu()->SetItemCommand(0, this,"ProcessGPURayCastTechniqueII 0");
    this->MB_GPURayCastTechniqueIIFg->GetWidget()->GetMenu()->AddRadioButton("Composite Psuedo Shading");
    this->MB_GPURayCastTechniqueIIFg->GetWidget()->GetMenu()->SetItemCommand(1, this,"ProcessGPURayCastTechniqueII 1");
    this->MB_GPURayCastTechniqueIIFg->GetWidget()->GetMenu()->AddRadioButton("Maximum Intensity Projection");
    this->MB_GPURayCastTechniqueIIFg->GetWidget()->GetMenu()->SetItemCommand(2, this,"ProcessGPURayCastTechniqueII 2");
    this->MB_GPURayCastTechniqueIIFg->GetWidget()->GetMenu()->AddRadioButton("Minimum Intensity Projection");
    this->MB_GPURayCastTechniqueIIFg->GetWidget()->GetMenu()->SetItemCommand(3, this,"ProcessGPURayCastTechniqueII 3");

    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->MB_GPURayCastTechniqueIIFg->GetWidgetName() );
    
    this->MB_GPURayCastColorOpacityFusion = vtkKWMenuButtonWithLabel::New();
    this->MB_GPURayCastColorOpacityFusion->SetParent(this->FrameGPURayCastingII->GetFrame());
    this->MB_GPURayCastColorOpacityFusion->SetLabelText("Fusion:");
    this->MB_GPURayCastColorOpacityFusion->Create();
    this->MB_GPURayCastColorOpacityFusion->SetLabelWidth(labelWidth);
    this->MB_GPURayCastColorOpacityFusion->SetBalloonHelpString("Select color fusion method in multi-volume rendering");
    this->MB_GPURayCastColorOpacityFusion->GetWidget()->GetMenu()->AddRadioButton("Composite");
    this->MB_GPURayCastColorOpacityFusion->GetWidget()->GetMenu()->SetItemCommand(0, this,"ProcessGPURayCastColorOpacityFusion 0");

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

  //opengl 2D Polygon Texture 3D
  {
    this->FramePolygonBlending = vtkKWFrameWithLabel::New();
    this->FramePolygonBlending->SetParent(this->FrameTechniques->GetFrame());
    this->FramePolygonBlending->Create();
    this->FramePolygonBlending->AllowFrameToCollapseOff();
    this->FramePolygonBlending->SetLabelText("OpenGL Polygon Texture 3D");
    this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FramePolygonBlending->GetWidgetName() );

    //currently no parameters
  }

  //CUDA ray casting
  {
    this->FrameCUDARayCasting = vtkKWFrameWithLabel::New();
    this->FrameCUDARayCasting->SetParent(this->FrameTechniques->GetFrame());
    this->FrameCUDARayCasting->Create();
    this->FrameCUDARayCasting->AllowFrameToCollapseOff();
    this->FrameCUDARayCasting->SetLabelText("CUDA Ray Casting");
    this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->FrameCUDARayCasting->GetWidgetName() );

    //enable/disable CUDA ray casting shading
    this->CB_CUDARayCastShading=vtkKWCheckButtonWithLabel::New();
    this->CB_CUDARayCastShading->SetParent(this->FrameCUDARayCasting->GetFrame());
    this->CB_CUDARayCastShading->Create();
    this->CB_CUDARayCastShading->SetBalloonHelpString("Enable lighting/shading in CUDA ray cast.");
    this->CB_CUDARayCastShading->SetLabelText("Enable Lighting");
    this->CB_CUDARayCastShading->SetLabelWidth(labelWidth);
    this->CB_CUDARayCastShading->GetWidget()->SetSelectedState(0);
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->CB_CUDARayCastShading->GetWidgetName() );
    this->CB_CUDARayCastShading->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand*) this->GUICallbackCommand);
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
  if(this->CB_CUDARayCastShading != NULL)
  {
    this->CB_CUDARayCastShading->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*)this->GUICallbackCommand);
    this->CB_CUDARayCastShading->SetParent(NULL);
    this->CB_CUDARayCastShading->Delete();
    this->CB_CUDARayCastShading=NULL;
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

  if (this->FrameCUDARayCasting != NULL)
  {
    this->FrameCUDARayCasting->SetParent(NULL);
    this->FrameCUDARayCasting->Delete();
    this->FrameCUDARayCasting = NULL;
  }

  if (this->FrameGPURayCasting != NULL)
  {
    this->FrameGPURayCasting->SetParent(NULL);
    this->FrameGPURayCasting->Delete();
    this->FrameGPURayCasting = NULL;
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

  if (this->FrameFPS != NULL)
  {
    this->FrameFPS->SetParent(NULL);
    this->FrameFPS->Delete();
    this->FrameFPS = NULL;
  }
}

void vtkSlicerVolumeRenderingHelper::CreateROITab()
{
  vtkKWFrameWithLabel *mainFrame = vtkKWFrameWithLabel::New();
  mainFrame->SetParent(this->NB_Details->GetFrame("ROI"));
  mainFrame->SetLabelText("ROI (Clipping)");
  mainFrame->Create();
  this->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", mainFrame->GetWidgetName() );

  this->CroppingButton = vtkKWCheckButtonWithLabel::New();
  this->CroppingButton->SetParent ( mainFrame->GetFrame() );
  this->CroppingButton->Create ( );
  this->CroppingButton->SetLabelText("Cropping Enabled");
  this->CroppingButton->SetBalloonHelpString("Enable cropping.");
  this->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->CroppingButton->GetWidgetName());

  this->CroppingButton->GetWidget()->AddObserver (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->FitROIButton = vtkKWPushButton::New();
  this->FitROIButton->SetParent( mainFrame->GetFrame() );
  this->FitROIButton->Create();
  this->FitROIButton->SetText("Fit ROI To Volume");
  this->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->FitROIButton->GetWidgetName());

  this->FitROIButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

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
  
  roiFrame->Delete();
  
  this->UpdateROI();
}

void vtkSlicerVolumeRenderingHelper::DestroyROITab()
{
  this->CroppingButton->GetWidget()->RemoveObservers (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->FitROIButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

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
    this->Script("pack %s -side left -anchor nw -expand yes -fill x -padx 2 -pady 2", this->RA_Threshold->GetWidgetName());
        
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
    this->Script("pack %s -side left -anchor nw -expand yes -fill x -padx 2 -pady 2", this->RA_ThresholdFg->GetWidgetName());
        
    this->FrameThresholdingFg->CollapseFrame();
  }
  
  this->SVP_VolumePropertyWidgetFg = vtkSlicerVolumePropertyWidget::New();
  this->SVP_VolumePropertyWidgetFg->SetParent(mainFrameFg->GetFrame());
  this->SVP_VolumePropertyWidgetFg->Create();
  this->SVP_VolumePropertyWidgetFg->ScalarOpacityUnitDistanceVisibilityOff ();
  this->SVP_VolumePropertyWidgetFg->InteractiveApplyModeOn ();

  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->SVP_VolumePropertyWidgetFg->GetWidgetName());

  this->SVP_VolumePropertyWidgetFg->AddObserver(vtkKWEvent::VolumePropertyChangingEvent, (vtkCommand*)this->GUICallbackCommand);
}

void vtkSlicerVolumeRenderingHelper::DestroyPropertyTab()
{
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
}

void vtkSlicerVolumeRenderingHelper::ProcessGUIEvents(vtkObject *caller,unsigned long eid,void *callData)
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();

  {//SVP
    vtkSlicerVolumePropertyWidget *callerObjectSVP = vtkSlicerVolumePropertyWidget::SafeDownCast(caller);
    if(callerObjectSVP == this->SVP_VolumePropertyWidget && eid == vtkKWEvent::VolumePropertyChangingEvent)
    {
      this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
      return;
    }
    if(callerObjectSVP == this->SVP_VolumePropertyWidgetFg && eid == vtkKWEvent::VolumePropertyChangingEvent)
    {
      this->Gui->GetLogic()->UpdateVolumePropertyGPURaycastII(vspNode);
      this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
      return;
    }
  }

  //scales
  {
    vtkKWScale *callerObjectSC = vtkKWScale::SafeDownCast(caller);
    if(callerObjectSC == this->SC_ExpectedFPS)
    {
      this->ProcessExpectedFPS();
      return;
    }
    if(callerObjectSC == this->SC_GPURayCastDepthPeelingThreshold->GetWidget())
    {
      vspNode->SetDepthPeelingThreshold(this->SC_GPURayCastDepthPeelingThreshold->GetWidget()->GetValue());

      this->Gui->GetLogic()->SetGPURaycastParameters(vspNode);

      this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
      return;
    }

    if(callerObjectSC == this->SC_GPURayCastICPEkt->GetWidget())
    {
      vspNode->SetICPEScale(this->SC_GPURayCastICPEkt->GetWidget()->GetValue());

      this->Gui->GetLogic()->SetGPURaycastParameters(vspNode);

      this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
      return;
    }

    if(callerObjectSC == this->SC_GPURayCastICPEks->GetWidget())
    {
      vspNode->SetICPESmoothness(this->SC_GPURayCastICPEks->GetWidget()->GetValue());

      this->Gui->GetLogic()->SetGPURaycastParameters(vspNode);

      this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
      return;
    }

  }

  //Check the checkbuttons
  {
    vtkKWCheckButton *callerObjectCheckButton = vtkKWCheckButton::SafeDownCast(caller);

    if(callerObjectCheckButton == this->CB_CPURayCastMIP->GetWidget())
    {
      vspNode->SetCPURaycastMode(this->CB_CPURayCastMIP->GetWidget()->GetSelectedState());

      this->Gui->GetLogic()->SetCPURaycastParameters(vspNode);

      this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
      return;
    }
    else if (eid == vtkKWCheckButton::SelectedStateChangedEvent && this->CroppingButton->GetWidget() == vtkKWCheckButton::SafeDownCast(caller) )
    {
      vspNode->SetCroppingEnabled(this->CroppingButton->GetWidget()->GetSelectedState());
      this->Gui->GetLogic()->SetROI(vspNode);

      this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
      return;
    }
    else if(callerObjectCheckButton == this->CB_UseThreshold->GetWidget())
    {
      vspNode->SetUseThreshold(this->CB_UseThreshold->GetWidget()->GetSelectedState());
      
      if (this->CB_UseThreshold->GetWidget()->GetSelectedState())
      {
        this->SVP_VolumePropertyWidget->GetEditorFrame()->CollapseFrame();
        this->FrameThresholding->ExpandFrame();
      }
      else
      {
        this->SVP_VolumePropertyWidget->GetEditorFrame()->ExpandFrame();
        this->SVP_VolumePropertyWidget->Update();
        this->FrameThresholding->CollapseFrame();
      }
      return;
    }
  }

  {//push button
    if (vtkKWPushButton::SafeDownCast(caller) == this->FitROIButton && eid == vtkKWPushButton::InvokedEvent )
    {
      this->Gui->GetLogic()->FitROIToVolume(vspNode);
      this->Gui->GetLogic()->SetROI(vspNode);

      this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
      return;
    }
  }
}

void vtkSlicerVolumeRenderingHelper::SetupGUIFromParametersNode(vtkMRMLVolumeRenderingParametersNode* vspNode)
{
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
  
  this->SC_GPURayCastICPEkt->GetWidget()->SetRange(0, 10);
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
  this->SC_GPURayCastIIFgBgRatio->SetValue(0.0);
    
  //-------------------------bg volume property--------------------
  this->SVP_VolumePropertyWidget->SetDataSet(vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData());

  this->SVP_VolumePropertyWidget->SetHistogramSet(this->Gui->GetLogic()->GetHistogramSet());
  this->SVP_VolumePropertyWidget->SetVolumeProperty(vspNode->GetVolumePropertyNode()->GetVolumeProperty());
  this->SVP_VolumePropertyWidget->Update();
  
  //-------------------------bg threshold--------------------------
  this->CB_UseThreshold->GetWidget()->SetSelectedState(vspNode->GetUseThreshold());
  this->RA_Threshold->SetWholeRange(scalarRange);
  this->RA_Threshold->SetRange(vspNode->GetThreshold());
  this->RA_Threshold->SetResolution((scalarRange[1] - scalarRange[0])*0.01);
  
  if (vspNode->GetFgVolumeNode())
  {
    //-------------------------fg volume property--------------------
    this->SVP_VolumePropertyWidgetFg->SetDataSet(vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetFgVolumeNode())->GetImageData());

    this->SVP_VolumePropertyWidget->SetHistogramSet(this->Gui->GetLogic()->GetHistogramSetFg());
    this->SVP_VolumePropertyWidget->SetVolumeProperty(vspNode->GetFgVolumePropertyNode()->GetVolumeProperty());
    this->SVP_VolumePropertyWidget->Update();
    
    vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetFgVolumeNode())->GetImageData()->GetPointData()->GetScalars()->GetRange(scalarRange, 0);
    
    //-------------------------fg threshold--------------------------
    this->CB_UseThresholdFg->GetWidget()->SetSelectedState(vspNode->GetUseThreshold());
    this->RA_ThresholdFg->SetWholeRange(scalarRange);
    this->RA_ThresholdFg->SetRange(vspNode->GetThresholdFg());
    this->RA_ThresholdFg->SetResolution((scalarRange[1] - scalarRange[0])*0.01);
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
}

void vtkSlicerVolumeRenderingHelper::ProcessGPURayCastTechnique(int id)
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();

  vspNode->SetGPURaycastTechnique(id);

  this->Gui->GetLogic()->SetGPURaycastParameters(vspNode);

  this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
}

void vtkSlicerVolumeRenderingHelper::ProcessGPURayCastTechniqueII(int id)
{

}

void vtkSlicerVolumeRenderingHelper::ProcessGPURayCastColorOpacityFusion(int id)
{

}

void vtkSlicerVolumeRenderingHelper::ProcessGPUMemorySize(int id)
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();

  vspNode->SetGPUMemorySize(id);

  this->Gui->GetLogic()->SetGPUMemorySize(vspNode);

  this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
}

void vtkSlicerVolumeRenderingHelper::ProcessRenderingMethodEvents(int id)
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();

  vspNode->SetCurrentVolumeMapper(id);

  int success = this->Gui->GetLogic()->SetupMapperFromParametersNode(vspNode);

  this->FrameCUDARayCasting->CollapseFrame();
  this->FrameGPURayCasting->CollapseFrame();
  this->FrameGPURayCastingII->CollapseFrame();
  this->FramePolygonBlending->CollapseFrame();
  this->FrameCPURayCasting->CollapseFrame();

  switch(id)
  {
  case 0://softwrae ray casting
    this->FrameCPURayCasting->ExpandFrame();
    this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using CPU Raycasting");
    break;
  case 1://gpu ray casting
    if (success)
    {
      this->FrameGPURayCasting->ExpandFrame();
      this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using GPU Raycasting");
    }
    else
      this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("GPU ray casting is not supported by your computer.");
    break;
  case 2://gpu ray casting II
    if (success)
    {
      this->FrameGPURayCastingII->ExpandFrame();
      this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using GPU Raycasting II");
    }
    else
      this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("PU ray casting II is not supported by your computer.");
    break;
  case 3://old school opengl 2D Polygon Texture 3D
    if (success)
    {
      this->FramePolygonBlending->ExpandFrame();
      this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using OpenGL Polygon Texture 3D");
    }
    else//seldom should we see this error message unless really low end graphics card...
      this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("OpenGL Polygon Texture 3D is not supported by your computer.");
    break;
  case 4://CUDA ray casting
/*    if (success)
    {
      this->FrameCUDARayCasting->ExpandFrame();
      this->Volume->SetMapper(this->MapperCUDARaycast);
      vtkMatrix4x4 *matrix=vtkMatrix4x4::New();
      vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetIJKToRASMatrix(matrix);
      this->MapperCUDARaycast->SetOrientationMatrix(matrix);
      matrix->Delete();
      this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("Using CUDA");
    }
    else*/
      this->Gui->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText("CUDA is not supported by your computer.");
    break;
  }

  this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
}

void vtkSlicerVolumeRenderingHelper::ProcessPauseResume(void)
{
  //Resume Rendering
  if(this->RenderingPaused == 1)
  {
    this->RenderingPaused = 0;
    this->Gui->GetLogic()->SetVolumeVisibility(1);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
    this->PB_PauseResume->GetWidget()->SetImageToIcon(this->VI_PauseResume->GetVisibleIcon());;
  }
  //Pause Rendering
  else
  {
    this->RenderingPaused = 1;
    this->Gui->GetLogic()->SetVolumeVisibility(0);
    this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
    this->PB_PauseResume->GetWidget()->SetImageToIcon(this->VI_PauseResume->GetInvisibleIcon());
  }
}
void vtkSlicerVolumeRenderingHelper::ProcessThreshold(double, double)
{
  vtkImageData *iData = vtkMRMLScalarVolumeNode::SafeDownCast(this->Gui->GetNS_ImageData()->GetSelected())->GetImageData();
  
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();
  
  //Delete all old Mapping Points
  vtkPiecewiseFunction *opacity = vspNode->GetVolumePropertyNode()->GetVolumeProperty()->GetScalarOpacity();
  opacity->RemoveAllPoints();
  
  double step = (iData->GetScalarRange()[1] - iData->GetScalarRange()[0]) * 0.01;
  
  opacity->AddPoint(iData->GetScalarRange()[0], 0.0);
  opacity->AddPoint(iData->GetScalarRange()[1], 0.0);
  
  opacity->AddPoint(this->RA_Threshold->GetRange()[0], 0.0);
  opacity->AddPoint(this->RA_Threshold->GetRange()[0] + step, 1.0);
  opacity->AddPoint(this->RA_Threshold->GetRange()[1] - step, 1.0);
  opacity->AddPoint(this->RA_Threshold->GetRange()[1], 0.0);
  
  vspNode->SetThreshold(this->RA_Threshold->GetRange());
  
  this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
}

void vtkSlicerVolumeRenderingHelper::ProcessExpectedFPS(void)
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();

  vspNode->SetExpectedFPS(this->SC_ExpectedFPS->GetValue());

  this->Gui->GetLogic()->SetExpectedFPS(vspNode);

  this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
}

void vtkSlicerVolumeRenderingHelper::SetButtonDown(int isDown)
{
  if (this->Gui == NULL)
    return;

  int val = this->Gui->GetLogic()->SetupCPURayCastInteractive(this->Gui->GetCurrentParametersNode(), isDown);

  if (val == 0)
    return;

  if (isDown == 1)
    this->CPURayCastingInteractionFlag = 1;
  else
  {
    if (this->CPURayCastingInteractionFlag == 1)//avoid endless loop
    {
      this->Gui->GetApplicationGUI()->GetViewerWidget()->RequestRender();
      this->CPURayCastingInteractionFlag = 0;
    }
  }
}

void vtkSlicerVolumeRenderingHelper::UpdateROI()
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->Gui->GetCurrentParametersNode();

  vtkMRMLROINode *roiNode = vspNode->GetROINode();

  this->ROIWidget->SetROINode(roiNode);

  this->CroppingButton->GetWidget()->SetSelectedState(vspNode->GetCroppingEnabled());
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

  this->SVP_VolumePropertyWidget->SetHistogramSet(this->Gui->GetLogic()->GetHistogramSetFg());
  this->SVP_VolumePropertyWidget->SetVolumeProperty(vspNode->GetFgVolumePropertyNode()->GetVolumeProperty());
  this->SVP_VolumePropertyWidget->Update();
}
