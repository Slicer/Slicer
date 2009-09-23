#include "vtkVolumeRenderingGUI.h"

#include <ostream>

#include "vtkBMPReader.h"
#include "vtkBMPWriter.h"
#include "vtkCellArray.h"
#include "vtkCylinderSource.h"
#include "vtkFloatArray.h"
#include "vtkImageData.h"
#include "vtkImageGradientMagnitude.h"
#include "vtkImageMapper.h"
#include "vtkIndent.h"
#include "vtkKWCheckButton.h"
#include "vtkKWColorTransferFunctionEditor.h"
#include "vtkKWEntry.h"
#include "vtkKWEvent.h"
#include "vtkKWHistogramSet.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWPiecewiseFunctionEditor.h"
#include "vtkKWProgressGauge.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkMRMLVolumePropertyNode.h"
#include "vtkMRMLVolumePropertyNode.h"
#include "vtkPiecewiseFunction.h"
#include "vtkPlaneSource.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkRendererCollection.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerMRMLTreeWidget.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerVolumeRenderingHelper.h"
#include "vtkSlicerVolumePropertyWidget.h"
#include "vtkTexture.h"
#include "vtkTimerLog.h"
#include "vtkVolume.h"

extern "C" int Volumerenderingreplacements_Init(Tcl_Interp *interp);

vtkVolumeRenderingGUI::vtkVolumeRenderingGUI(void)
{
  //In Debug Mode
  this->DebugOff();

  this->UpdatingGUI = 0;
  this->ProcessingGUIEvents = 0;
  this->ProcessingMRMLEvents = 0;

  this->ParametersNode = NULL;
  this->ParametersNode = NULL;

  this->Presets=NULL;

  this->PB_HideSurfaceModels=NULL;
  this->PB_CreateNewVolumeRenderingNode=NULL;
  this->PB_CreateNewVolumeRenderingNodeFg=NULL;

  this->NS_ImageData=NULL;
  this->NS_ImageDataFg=NULL;
  this->NS_ImageDataLabelmap=NULL;

  this->NS_VolumeRenderingDataPresets=NULL;
  this->NS_VolumeRenderingDataScene=NULL;
  this->EWL_CreateNewVolumePropertyNode=NULL;

  this->NS_VolumeRenderingDataPresetsFg=NULL;
  this->NS_VolumeRenderingDataSceneFg=NULL;
  this->EWL_CreateNewVolumePropertyNodeFg=NULL;

  //Frame Details
  this->RenderingFrame=NULL;

  //Other members
  this->Presets=NULL;
  this->Helper=NULL;

  // :NOTE: 20080515 tgl: To use as a loadable module, initialize
  // the volume rendering replacements TCL wrappers.
  Tcl_Interp *interp = NULL;
  interp = vtkKWApplication::GetMainInterp();
  if (NULL != interp)
  {
    Volumerenderingreplacements_Init(interp);
  }

}

vtkVolumeRenderingGUI::~vtkVolumeRenderingGUI(void)
{
  vtkSetAndObserveMRMLNodeMacro(this->ParametersNode, NULL);

  //Not Delete?!
  //vtkVolumeRenderingModuleLogic *Logic;
  //vtkSlicerViewerWidget *ViewerWidget;
  //vtkSlicerViewerInteractorStyle *InteractorStyle;
  //vtkMRMLVolumePropertyNode  *CurrentNode;//really delete this

  if(this->PB_HideSurfaceModels)
  {
    this->PB_HideSurfaceModels->SetParent(NULL);
    this->PB_HideSurfaceModels->Delete();
    this->PB_HideSurfaceModels=NULL;
  }

  if (this->PB_CreateNewVolumeRenderingNode)
  {
    this->PB_CreateNewVolumeRenderingNode->SetParent(NULL);
    this->PB_CreateNewVolumeRenderingNode->Delete();
    this->PB_CreateNewVolumeRenderingNode=NULL;
  }

  if (this->PB_CreateNewVolumeRenderingNodeFg)
  {
    this->PB_CreateNewVolumeRenderingNodeFg->SetParent(NULL);
    this->PB_CreateNewVolumeRenderingNodeFg->Delete();
    this->PB_CreateNewVolumeRenderingNodeFg=NULL;
  }

  if (this->NS_ImageData)
  {
    this->NS_ImageData->SetParent(NULL);
    this->NS_ImageData->Delete();
    this->NS_ImageData=NULL;
  }
  if (this->NS_ImageDataFg)
  {
    this->NS_ImageDataFg->SetParent(NULL);
    this->NS_ImageDataFg->Delete();
    this->NS_ImageDataFg=NULL;
  }
  if (this->NS_ImageDataLabelmap)
  {
    this->NS_ImageDataLabelmap->SetParent(NULL);
    this->NS_ImageDataLabelmap->Delete();
    this->NS_ImageDataLabelmap=NULL;
  }
  if(this->NS_VolumeRenderingDataScene)
  {
    this->NS_VolumeRenderingDataScene->SetParent(NULL);
    this->NS_VolumeRenderingDataScene->Delete();
    this->NS_VolumeRenderingDataScene=NULL;
  }

  if(this->NS_VolumeRenderingDataPresets)
  {
    this->NS_VolumeRenderingDataPresets->SetParent(NULL);
    this->NS_VolumeRenderingDataPresets->Delete();
    this->NS_VolumeRenderingDataPresets=NULL;
  }

  if(this->EWL_CreateNewVolumePropertyNode)
  {
    this->EWL_CreateNewVolumePropertyNode->SetParent(NULL);
    this->EWL_CreateNewVolumePropertyNode->Delete();
    this->EWL_CreateNewVolumePropertyNode=NULL;
  }
  if(this->NS_VolumeRenderingDataSceneFg)
  {
    this->NS_VolumeRenderingDataSceneFg->SetParent(NULL);
    this->NS_VolumeRenderingDataSceneFg->Delete();
    this->NS_VolumeRenderingDataSceneFg=NULL;
  }

  if(this->NS_VolumeRenderingDataPresetsFg)
  {
    this->NS_VolumeRenderingDataPresetsFg->SetParent(NULL);
    this->NS_VolumeRenderingDataPresetsFg->Delete();
    this->NS_VolumeRenderingDataPresetsFg=NULL;
  }

  if(this->EWL_CreateNewVolumePropertyNodeFg)
  {
    this->EWL_CreateNewVolumePropertyNodeFg->SetParent(NULL);
    this->EWL_CreateNewVolumePropertyNodeFg->Delete();
    this->EWL_CreateNewVolumePropertyNodeFg=NULL;
  }

  if(this->Presets)
  {
    this->Presets->Delete();
    this->Presets=NULL;
  }

  if(this->RenderingFrame)
  {
    this->RenderingFrame->Delete();
    this->RenderingFrame=NULL;
  }

  if(this->Helper)
  {
    this->Helper->Delete();
    this->Helper=NULL;
  }

  //Remove the MRML observer
  if ( this->GetApplicationGUI() )
  {
    this->GetApplicationGUI()->GetMRMLScene()->RemoveObservers(vtkMRMLScene::SceneCloseEvent, this->MRMLCallbackCommand);
    this->GetApplicationGUI()->GetMRMLScene()->RemoveObservers(vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand);
    this->GetApplicationGUI()->GetMRMLScene()->RemoveObservers(vtkMRMLScene::NodeRemovedEvent, this->MRMLCallbackCommand);
  }
  this->SetViewerWidget(NULL);
  this->SetInteractorStyle(NULL);
}

vtkVolumeRenderingGUI* vtkVolumeRenderingGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkVolumeRenderingGUI");
  if(ret)
  {
    return (vtkVolumeRenderingGUI*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkVolumeRenderingGUI;
}

void vtkVolumeRenderingGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  os<<indent<<"vtkVolumeRenderingGUI"<<endl;
  os<<indent<<"vtkVolumeRenderingLogic"<<endl;
  if(this->GetLogic())
  {
    this->GetLogic()->PrintSelf(os,indent.GetNextIndent());
  }
}

void vtkVolumeRenderingGUI::BuildGUI(void)
{
  int labelWidth = 20;
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  this->GetUIPanel()->AddPage("VolumeRendering", "VolumeRendering", NULL);

  // Define your help text and build the help frame here.
  const char *help = "Volume Rendering allows the rendering of volumes in 3D space and not only as 2D surfaces defined in 3D space. \n<a>http://wiki.slicer.org/slicerWiki/index.php/Modules:VolumeRendering-Documentation-3.4</a>\n Tutorials are available at <a>http://www.na-mic.org/Wiki/index.php/Slicer3:Volume_Rendering_Tutorials</a>";
  const char *about = "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. The VolumeRendering module was contributed by Yanling Liu, SAIC-Frederick, Andreas Freudling, Student Intern at SPL, BWH (Ron Kikinis) and Alex Yarmarkovich, Isomics Inc. (Steve Pieper)";
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "VolumeRendering" );
  this->BuildHelpAndAboutFrame ( page, help, about );

  //
  //Load and save
  //
  vtkSlicerModuleCollapsibleFrame *loadSaveDataFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  loadSaveDataFrame->SetParent (this->UIPanel->GetPageWidget("VolumeRendering"));
  loadSaveDataFrame->Create();
  loadSaveDataFrame->ExpandFrame();
  loadSaveDataFrame->SetLabelText("Input Volumes");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
      loadSaveDataFrame->GetWidgetName(), this->UIPanel->GetPageWidget("VolumeRendering")->GetWidgetName());

  //Hide surface models pushbutton
  this->PB_HideSurfaceModels= vtkKWPushButton::New();
  this->PB_HideSurfaceModels->SetParent(loadSaveDataFrame->GetFrame());
  this->PB_HideSurfaceModels->Create();
  this->PB_HideSurfaceModels->SetText("Hide Surface Models");
  this->PB_HideSurfaceModels->SetBalloonHelpString("Make all surface models invisible. Go to models module to enable, disable only some of them.");
  this->PB_HideSurfaceModels->SetWidth(labelWidth);
  app->Script("pack %s -side top -anchor ne -padx 2 -pady 2",this->PB_HideSurfaceModels->GetWidgetName());

  {//background input volume
    vtkKWFrameWithLabel *inputVolumeFrame = vtkKWFrameWithLabel::New();
    inputVolumeFrame->SetParent(loadSaveDataFrame->GetFrame());
    inputVolumeFrame->Create();
    inputVolumeFrame->SetLabelText("Background Volume");
    app->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", inputVolumeFrame->GetWidgetName() );

    //NodeSelector for Node from MRML Scene
    this->NS_ImageData=vtkSlicerNodeSelectorWidget::New();
    this->NS_ImageData->SetParent(inputVolumeFrame->GetFrame());
    this->NS_ImageData->Create();
    this->NS_ImageData->NoneEnabledOn();
    this->NS_ImageData->SetLabelText("Source: ");
    this->NS_ImageData->SetBalloonHelpString("Select volume to render. Only one volume at the some time is possible.");
    this->NS_ImageData->SetLabelWidth(labelWidth);
    this->NS_ImageData->SetNodeClass("vtkMRMLScalarVolumeNode","","","");
    this->NS_ImageData->SetChildClassesEnabled(0);
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_ImageData->GetWidgetName());

    //NodeSelector for VolumePropertyNode Preset
    this->NS_VolumeRenderingDataPresets=vtkSlicerNodeSelectorWidget::New();
    this->NS_VolumeRenderingDataPresets->SetParent(inputVolumeFrame->GetFrame());
    this->NS_VolumeRenderingDataPresets->Create();
    this->NS_VolumeRenderingDataPresets->SetLabelText("Presets:");
    this->NS_VolumeRenderingDataPresets->SetBalloonHelpString("Select one of the existing parameter sets or presets.");
    this->NS_VolumeRenderingDataPresets->SetLabelWidth(labelWidth);
    this->NS_VolumeRenderingDataPresets->EnabledOff();//By default off
    this->NS_VolumeRenderingDataPresets->NoneEnabledOn();
    this->NS_VolumeRenderingDataPresets->SetShowHidden(1);
    this->NS_VolumeRenderingDataPresets->SetNodeClass("vtkMRMLVolumePropertyNode","","","");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_VolumeRenderingDataPresets->GetWidgetName());

    //NodeSelector for VolumePropertyNode Scene
    this->NS_VolumeRenderingDataScene=vtkSlicerNodeSelectorWidget::New();
    this->NS_VolumeRenderingDataScene->SetParent(inputVolumeFrame->GetFrame());
    this->NS_VolumeRenderingDataScene->Create();
    this->NS_VolumeRenderingDataScene->NoneEnabledOn();
    this->NS_VolumeRenderingDataScene->SetLabelText("Property:");
    this->NS_VolumeRenderingDataScene->SetBalloonHelpString("Select how the volume should be displayed. Multi-property per volume are possible");
    this->NS_VolumeRenderingDataScene->SetLabelWidth(labelWidth);
    this->NS_VolumeRenderingDataScene->EnabledOff();//By default off
    this->NS_VolumeRenderingDataScene->SetShowHidden(1);
    this->NS_VolumeRenderingDataScene->SetNodeClass("vtkMRMLVolumePropertyNode","","","");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_VolumeRenderingDataScene->GetWidgetName());
    //Missing: Load from file

    //Create New Volume Rendering Node
    //Entry With Label
    this->EWL_CreateNewVolumePropertyNode=vtkKWEntryWithLabel::New();
    this->EWL_CreateNewVolumePropertyNode->SetParent(inputVolumeFrame->GetFrame());
    this->EWL_CreateNewVolumePropertyNode->Create();
    this->EWL_CreateNewVolumePropertyNode->SetBalloonHelpString("Specify a name for a new volume property node.");
    this->EWL_CreateNewVolumePropertyNode->SetLabelText("New Property:");
    this->EWL_CreateNewVolumePropertyNode->SetLabelWidth(labelWidth);
    this->EWL_CreateNewVolumePropertyNode->EnabledOff();
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2", this->EWL_CreateNewVolumePropertyNode->GetWidgetName());

    this->PB_CreateNewVolumeRenderingNode=vtkKWPushButton::New();
    this->PB_CreateNewVolumeRenderingNode->SetParent(inputVolumeFrame->GetFrame());
    this->PB_CreateNewVolumeRenderingNode->Create();
    this->PB_CreateNewVolumeRenderingNode->SetBalloonHelpString("Create a new volume property node for the current volume. This way you can switch between different visualization settings for the same volume.");
    this->PB_CreateNewVolumeRenderingNode->SetText("Create New Property");
    app->Script("pack %s -side top -anchor ne -padx 2 -pady 2",this->PB_CreateNewVolumeRenderingNode->GetWidgetName());
    inputVolumeFrame->Delete();
  }

  {//foreground volume
    vtkKWFrameWithLabel *inputVolumeFrame = vtkKWFrameWithLabel::New();
    inputVolumeFrame->SetParent(loadSaveDataFrame->GetFrame());
    inputVolumeFrame->Create();
    inputVolumeFrame->SetLabelText("Foreground Volume");
    app->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", inputVolumeFrame->GetWidgetName() );

    //NodeSelector for Node from MRML Scene
    this->NS_ImageDataFg=vtkSlicerNodeSelectorWidget::New();
    this->NS_ImageDataFg->SetParent(inputVolumeFrame->GetFrame());
    this->NS_ImageDataFg->Create();
    this->NS_ImageDataFg->NoneEnabledOn();
    this->NS_ImageDataFg->EnabledOff();
    this->NS_ImageDataFg->SetLabelText("Source:");
    this->NS_ImageDataFg->SetBalloonHelpString("Select background volume to render");
    this->NS_ImageDataFg->SetLabelWidth(labelWidth);
    this->NS_ImageDataFg->SetNodeClass("vtkMRMLScalarVolumeNode","","","");
    this->NS_ImageDataFg->SetChildClassesEnabled(0);
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_ImageDataFg->GetWidgetName());

    //NodeSelector for VolumeRenderingNode Preset
    this->NS_VolumeRenderingDataPresetsFg=vtkSlicerNodeSelectorWidget::New();
    this->NS_VolumeRenderingDataPresetsFg->SetParent(inputVolumeFrame->GetFrame());
    this->NS_VolumeRenderingDataPresetsFg->Create();
    this->NS_VolumeRenderingDataPresetsFg->SetLabelText("Presets:");
    this->NS_VolumeRenderingDataPresetsFg->SetBalloonHelpString("Select one of the existing parameter sets or presets.");
    this->NS_VolumeRenderingDataPresetsFg->SetLabelWidth(labelWidth);
    this->NS_VolumeRenderingDataPresetsFg->EnabledOff();//By default off
    this->NS_VolumeRenderingDataPresetsFg->NoneEnabledOn();
    this->NS_VolumeRenderingDataPresetsFg->SetShowHidden(1);
    this->NS_VolumeRenderingDataPresetsFg->SetNodeClass("vtkMRMLVolumePropertyNode","","","");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_VolumeRenderingDataPresetsFg->GetWidgetName());

    //NodeSelector for VolumeRenderingNode Scene
    this->NS_VolumeRenderingDataSceneFg=vtkSlicerNodeSelectorWidget::New();
    this->NS_VolumeRenderingDataSceneFg->SetParent(inputVolumeFrame->GetFrame());
    this->NS_VolumeRenderingDataSceneFg->Create();
    this->NS_VolumeRenderingDataSceneFg->NoneEnabledOn();
    this->NS_VolumeRenderingDataSceneFg->SetLabelText("Property:");
    this->NS_VolumeRenderingDataSceneFg->SetBalloonHelpString("Select how the volume should be displayed. Several parameter sets per volume are possible");
    this->NS_VolumeRenderingDataSceneFg->SetLabelWidth(labelWidth);
    this->NS_VolumeRenderingDataSceneFg->EnabledOff();//By default off
    this->NS_VolumeRenderingDataSceneFg->SetShowHidden(1);
    this->NS_VolumeRenderingDataSceneFg->SetNodeClass("vtkMRMLVolumePropertyNode","","","");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_VolumeRenderingDataSceneFg->GetWidgetName());
    //Missing: Load from file

    //Create New Volume Rendering Node
    //Entry With Label
    this->EWL_CreateNewVolumePropertyNodeFg=vtkKWEntryWithLabel::New();
    this->EWL_CreateNewVolumePropertyNodeFg->SetParent(inputVolumeFrame->GetFrame());
    this->EWL_CreateNewVolumePropertyNodeFg->Create();
    this->EWL_CreateNewVolumePropertyNodeFg->SetBalloonHelpString("Specify a name for a new volume property.");
    this->EWL_CreateNewVolumePropertyNodeFg->SetLabelText("New Property:");
    this->EWL_CreateNewVolumePropertyNodeFg->SetLabelWidth(labelWidth);
    this->EWL_CreateNewVolumePropertyNodeFg->EnabledOff();
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2", this->EWL_CreateNewVolumePropertyNodeFg->GetWidgetName());

    this->PB_CreateNewVolumeRenderingNodeFg=vtkKWPushButton::New();
    this->PB_CreateNewVolumeRenderingNodeFg->SetParent(inputVolumeFrame->GetFrame());
    this->PB_CreateNewVolumeRenderingNodeFg->Create();
    this->PB_CreateNewVolumeRenderingNodeFg->SetBalloonHelpString("Create a new property for the current volume. This way you can switch between different visualization settings for the same volume.");
    this->PB_CreateNewVolumeRenderingNodeFg->SetText("Create New Property");
    app->Script("pack %s -side top -anchor ne -padx 2 -pady 2",this->PB_CreateNewVolumeRenderingNodeFg->GetWidgetName());

    inputVolumeFrame->CollapseFrame();
    inputVolumeFrame->Delete();
  }

  {//labelmap volume
    vtkKWFrameWithLabel *inputVolumeFrame = vtkKWFrameWithLabel::New();
    inputVolumeFrame->SetParent(loadSaveDataFrame->GetFrame());
    inputVolumeFrame->Create();
    inputVolumeFrame->SetLabelText("Labelmap Volume");
    app->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", inputVolumeFrame->GetWidgetName() );

    //NodeSelector for Node from MRML Scene
    this->NS_ImageDataLabelmap=vtkSlicerNodeSelectorWidget::New();
    this->NS_ImageDataLabelmap->SetParent(inputVolumeFrame->GetFrame());
    this->NS_ImageDataLabelmap->Create();
    this->NS_ImageDataLabelmap->NoneEnabledOn();
    this->NS_ImageDataLabelmap->EnabledOff();
    this->NS_ImageDataLabelmap->SetLabelText("Source:");
    this->NS_ImageDataLabelmap->SetBalloonHelpString("Select labelmap volume to render.");
    this->NS_ImageDataLabelmap->SetLabelWidth(labelWidth);
    this->NS_ImageDataLabelmap->SetNodeClass("vtkMRMLScalarVolumeNode","","","");
    this->NS_ImageDataLabelmap->SetChildClassesEnabled(0);
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_ImageDataLabelmap->GetWidgetName());

    inputVolumeFrame->CollapseFrame();
    inputVolumeFrame->Delete();
  }

  //Rendering (parameters) frame
  this->RenderingFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  this->RenderingFrame->SetParent (this->UIPanel->GetPageWidget("VolumeRendering"));
  this->RenderingFrame->Create();
  this->RenderingFrame->ExpandFrame();
  this->RenderingFrame->SetLabelText("Rendering");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
      this->RenderingFrame->GetWidgetName(), this->UIPanel->GetPageWidget("VolumeRendering")->GetWidgetName());

  //set subnodes
  //Delete frames
  if ( this->GetApplicationGUI() &&  this->GetApplicationGUI()->GetMRMLScene())
  {
    this->GetApplicationGUI()->GetMRMLScene()->AddObserver( vtkMRMLScene::SceneCloseEvent, this->MRMLCallbackCommand );
    this->MRMLScene->AddObserver(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    this->MRMLScene->AddObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  }

  vtkMRMLVolumeRenderingParametersNode* parametersNode = this->GetLogic()->GetParametersNode();
  vtkSetAndObserveMRMLNodeMacro(this->ParametersNode, parametersNode);

  loadSaveDataFrame->Delete();
  this->Built=true;
}

void vtkVolumeRenderingGUI::TearDownGUI(void)
{
  this->Exit();
  if ( this->Built )
  {
    this->RemoveGUIObservers();
  }
}

void vtkVolumeRenderingGUI::CreateModuleEventBindings(void)
{
  vtkDebugMacro("VolumeRendering: CreateModuleEventBindings: No ModuleEventBindings yet");
}

void vtkVolumeRenderingGUI::ReleaseModuleEventBindings(void)
{
  vtkDebugMacro("VolumeRendering: ReleaseModuleEventBindings: No ModuleEventBindings to remove yet");
}

void vtkVolumeRenderingGUI::AddGUIObservers(void)
{
  this->NS_ImageData->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NS_ImageDataFg->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  //  this->NS_ImageDataLabelmap->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->NS_VolumeRenderingDataScene->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NS_VolumeRenderingDataPresets->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->PB_CreateNewVolumeRenderingNode->AddObserver(vtkKWPushButton::InvokedEvent,(vtkCommand*)this->GUICallbackCommand);

  this->NS_VolumeRenderingDataSceneFg->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NS_VolumeRenderingDataPresetsFg->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->PB_CreateNewVolumeRenderingNodeFg->AddObserver(vtkKWPushButton::InvokedEvent,(vtkCommand*)this->GUICallbackCommand);

  this->PB_HideSurfaceModels->AddObserver(vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallbackCommand );
}

void vtkVolumeRenderingGUI::RemoveGUIObservers(void)
{
  this->NS_ImageData->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->NS_ImageDataFg->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
  //  this->NS_ImageDataLabelmap->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->NS_VolumeRenderingDataScene->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->NS_VolumeRenderingDataPresets->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->PB_CreateNewVolumeRenderingNode->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->NS_VolumeRenderingDataSceneFg->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->NS_VolumeRenderingDataPresetsFg->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->PB_CreateNewVolumeRenderingNodeFg->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->PB_HideSurfaceModels->RemoveObservers (vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallbackCommand);
}

void vtkVolumeRenderingGUI::RemoveMRMLNodeObservers(void)
{
}

void vtkVolumeRenderingGUI::RemoveLogicObservers(void)
{
}

void vtkVolumeRenderingGUI::ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData)
{
  if (this->ProcessingGUIEvents )
    return;

  this->ProcessingGUIEvents = 1;

  //Check PushButtons
  vtkKWPushButton *callerObject=vtkKWPushButton::SafeDownCast(caller);
  // hide surface models to reveal volume rendering
  if(callerObject == this->PB_HideSurfaceModels && event == vtkKWPushButton::InvokedEvent)
  {
    int count=this->GetLogic()->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLModelNode");
    for(int i = 0; i < count; i++)
      (vtkMRMLModelNode::SafeDownCast(this->GetLogic()->GetMRMLScene()->GetNthNodeByClass(i,"vtkMRMLModelNode")))->GetModelDisplayNode()->VisibilityOff();
  }
  else if(callerObject == this->PB_CreateNewVolumeRenderingNode && event == vtkKWPushButton::InvokedEvent)//Create New volume property node
  {
    //Get a new auto node
    this->InitializePipelineNewVolumeProperty();

    //Set the right name
    const char *name=this->EWL_CreateNewVolumePropertyNode->GetWidget()->GetValue();
    if(!name || strlen(name) == 0)
      this->ParametersNode->GetVolumePropertyNode()->SetName("No Name");
    else
      this->ParametersNode->GetVolumePropertyNode()->SetName(name);

    //Remove Text from Entry
    this->EWL_CreateNewVolumePropertyNode->GetWidget()->SetValue("");
    this->NS_VolumeRenderingDataScene->UpdateMenu();
  }
  else if (callerObject == this->PB_CreateNewVolumeRenderingNodeFg && event == vtkKWPushButton::InvokedEvent)//create new fg volume property node
  {
    //Get a new auto node
    this->InitializePipelineNewVolumePropertyFg();

    //Set the right name
    const char *name=this->EWL_CreateNewVolumePropertyNodeFg->GetWidget()->GetValue();
    if(!name || strlen(name) == 0)
      this->ParametersNode->GetFgVolumePropertyNode()->SetName("No Name");
    else
      this->ParametersNode->GetVolumePropertyNode()->SetName(name);

    //Remove Text from Entry
    this->EWL_CreateNewVolumePropertyNodeFg->GetWidget()->SetValue("");
    this->NS_VolumeRenderingDataSceneFg->UpdateMenu();
  }// End Check PushButtons

  //Check Node Selectors
  vtkSlicerNodeSelectorWidget *callerObjectNS=vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  //Load Volume
  //-----------------------------------fg volume----------------------------------------------------
  if(callerObjectNS == this->NS_ImageDataFg && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    if(this->NS_ImageDataFg->GetSelected() == NULL)//user selected none
    {
      //Remove the old observer for the imagedata
      if(this->ParametersNode->GetFgVolumeNodeID() != NULL)
      {
        vtkMRMLNode *oldNode = this->GetLogic()->GetMRMLScene()->GetNodeByID(this->ParametersNode->GetFgVolumeNodeID());
        if(oldNode != NULL)
        {
          oldNode->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *) this->MRMLCallbackCommand);
          oldNode->RemoveObservers(vtkMRMLScalarVolumeNode::ImageDataModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );
        }
      }

      this->ParametersNode->SetAndObserveFgVolumeNodeID(NULL);

      //Try to keep navigation render online
      this->GetApplicationGUI()->GetViewControlGUI()->GetEnableDisableNavButton()->SelectedStateOn();
    }
    else //user selected a node
    {
      if(this->ParametersNode->GetFgVolumeNodeID() != NULL &&
          strcmp(this->NS_ImageDataFg->GetSelected()->GetID(), this->ParametersNode->GetFgVolumeNodeID()) == 0 )
      {
        return;// return if not new Node
      }

      this->ParametersNode->SetAndObserveFgVolumeNodeID(this->NS_ImageDataFg->GetSelected()->GetID());

      this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->Disable();

      vtkMRMLScalarVolumeNode *selectedImageData = vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageDataFg->GetSelected());
      //Add observer to trigger update of transform
      selectedImageData->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *) this->MRMLCallbackCommand);
      selectedImageData->AddObserver(vtkMRMLScalarVolumeNode::ImageDataModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );

      this->Helper->SetupHistogramFg();

      //Initialize the Pipeline
      this->InitializePipelineFromImageDataFg();

      //update previous:
      this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->Enable();
    }//else
  }//----------------------------------bg volume---------------------------------------------------------
  else if(callerObjectNS == this->NS_ImageData && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    if(this->NS_ImageData->GetSelected() == NULL)//user selected "none"
    {
      //Remove the old observer for the imagedata
      if(this->ParametersNode->GetVolumeNodeID() != NULL)
      {
        vtkMRMLNode *oldNode = this->GetLogic()->GetMRMLScene()->GetNodeByID(this->ParametersNode->GetVolumeNodeID());
        if(oldNode != NULL)
        {
          oldNode->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent, (vtkCommand *) this->MRMLCallbackCommand);
          oldNode->RemoveObservers(vtkMRMLScalarVolumeNode::ImageDataModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );
        }
      }

      this->ParametersNode->SetAndObserveVolumeNodeID(NULL);

      //Unpack the rendering frame
      this->DeleteRenderingFrame();

      //Try to keep navigation render online
      this->GetApplicationGUI()->GetViewControlGUI()->GetEnableDisableNavButton()->SelectedStateOn();
    }
    else//Only proceed event,if new Node
    {
      if(this->ParametersNode->GetVolumeNodeID() != NULL &&
          strcmp(this->NS_ImageData->GetSelected()->GetID(), this->ParametersNode->GetVolumeNodeID()) == 0)
      {
        return;// return if not new Node
      }

      this->ParametersNode->SetAndObserveVolumeNodeID(this->NS_ImageData->GetSelected()->GetID());

      //Try to keep navigation Render online
      this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->Disable();

      vtkMRMLScalarVolumeNode *selectedImageData = vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected());
      //Add observer to trigger update of transform
      selectedImageData->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent,(vtkCommand *) this->MRMLCallbackCommand);
      selectedImageData->AddObserver(vtkMRMLScalarVolumeNode::ImageDataModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );

      this->DeleteRenderingFrame();
      this->CreateRenderingFrame();

      //Initialize the Pipeline
      this->InitializePipelineFromImageData();

      //update previous:
      this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->Enable();
    }//else
  }
  else if(callerObjectNS == this->NS_VolumeRenderingDataScene && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    //Check for None selected //Just to be safe
    if(this->NS_VolumeRenderingDataScene->GetSelected() == NULL)
    {
      this->ParametersNode->SetAndObserveVolumePropertyNodeID(NULL);
    }
    else//Only proceed event,if new Node
    {
       if(this->ParametersNode->GetVolumePropertyNodeID() != NULL &&
          strcmp(this->NS_VolumeRenderingDataScene->GetSelected()->GetID(), this->ParametersNode->GetVolumePropertyNodeID()) == 0)
      {
      return;//return if same node
      }

      this->ParametersNode->SetAndObserveVolumePropertyNodeID(this->NS_VolumeRenderingDataScene->GetSelected()->GetID());
      this->InitializePipelineFromMRMLScene();
    }
  }
  else if(callerObjectNS == this->NS_VolumeRenderingDataSceneFg && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    //Check for None selected //Just to be safe
    if(this->NS_VolumeRenderingDataSceneFg->GetSelected() == NULL)
    {
      this->ParametersNode->SetAndObserveFgVolumePropertyNodeID(NULL);
    }
    else
    {
      if(this->ParametersNode->GetFgVolumePropertyNodeID() != NULL &&
          strcmp(this->NS_VolumeRenderingDataSceneFg->GetSelected()->GetID(), this->ParametersNode->GetFgVolumePropertyNode()->GetID()) == 0)
      {
        return;//return if same node
      }

      this->ParametersNode->SetAndObserveFgVolumePropertyNodeID(this->NS_VolumeRenderingDataSceneFg->GetSelected()->GetID());
      this->InitializePipelineFromMRMLScene();
    }
  }
  else if(callerObjectNS == this->NS_VolumeRenderingDataPresets && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    //Check for None selected
    if(this->NS_VolumeRenderingDataPresets->GetSelected() == NULL)
    {
      this->ParametersNode->SetPresetsNodeID(NULL);
    }
    else//Only proceed event,if other Node
    {
      if(this->ParametersNode->GetPresetsNodeID() != NULL &&
        strcmp(this->NS_VolumeRenderingDataPresets->GetSelected()->GetID(), this->ParametersNode->GetPresetsNodeID()) ==0 )
      {
        return;
      }

      if(this->Presets->GetNodeByID(this->NS_VolumeRenderingDataPresets->GetSelected()->GetID()) != NULL)
      {
        //Copy Preset Information in current Node
        this->ParametersNode->GetVolumePropertyNode()->CopyParameterSet(this->NS_VolumeRenderingDataPresets->GetSelected());
        this->Helper->UpdateGUIElements();
        this->ParametersNode->SetPresetsNodeID(this->NS_VolumeRenderingDataPresets->GetSelected()->GetID());
      }
    }
  }
  else if(callerObjectNS == this->NS_VolumeRenderingDataPresetsFg && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    //Check for None selected
    if(this->NS_VolumeRenderingDataPresetsFg->GetSelected() == NULL)
    {
      this->ParametersNode->SetFgPresetsNodeID(NULL);
    }
    else//Only proceed event,if other Node
    {
      if(this->ParametersNode->GetFgPresetsNodeID() != NULL &&
        strcmp(this->NS_VolumeRenderingDataPresetsFg->GetSelected()->GetID(), this->ParametersNode->GetFgPresetsNodeID()) ==0 )
      {
        return;
      }

      if(this->Presets->GetNodeByID(this->NS_VolumeRenderingDataPresetsFg->GetSelected()->GetID()) != NULL)
      {
        //Copy Preset Information in current Node
        this->ParametersNode->GetFgVolumePropertyNode()->CopyParameterSet(this->NS_VolumeRenderingDataPresetsFg->GetSelected());
        this->Helper->UpdateGUIElements();
        this->ParametersNode->SetFgPresetsNodeID(this->NS_VolumeRenderingDataPresetsFg->GetSelected()->GetID());
      }
    }
  }
  //End Check NodeSelectors

  //Update GUI
  this->UpdateGUI();
  this->ProcessingGUIEvents = 0;
}

void vtkVolumeRenderingGUI::ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
  if (this->ProcessingGUIEvents || this->ProcessingMRMLEvents)
    return;

  this->ProcessingMRMLEvents = 1;

  vtkMRMLNode *addedNode = NULL;

  if (event == vtkMRMLScene::NodeAddedEvent && this->MRMLScene)
  {
    addedNode = reinterpret_cast<vtkMRMLNode *>(callData);

    if (addedNode != NULL && addedNode->IsA("vtkMRMLVolumeRenderingParametersNode") )
    {
      vtkSetAndObserveMRMLNodeMacro(this->ParametersNode, this->GetLogic()->GetParametersNode());

      if (this->ParametersNode->GetVolumeNodeID() != NULL)
      {
        this->UpdateGUI();

        DeleteRenderingFrame();
        CreateRenderingFrame();

        this->InitializePipelineFromMRMLScene();
      }
    }

    if (addedNode != NULL && addedNode->IsA("vtkMRMLVolumeNode") )
      this->UpdateGUI();
  }

  if (vtkMRMLVolumeRenderingParametersNode::SafeDownCast(caller)&&
      this->ParametersNode == vtkMRMLVolumeRenderingParametersNode::SafeDownCast(caller) &&
      event == vtkCommand::ModifiedEvent && this->MRMLScene)
  {
    this->UpdateGUI();
  }

  if (vtkMRMLVolumePropertyNode::SafeDownCast(caller) && event == vtkCommand::ModifiedEvent && this->MRMLScene)
  {
    if (this->ParametersNode->GetVolumePropertyNode() == vtkMRMLVolumePropertyNode::SafeDownCast(caller) ||
        this->ParametersNode->GetFgVolumePropertyNode() == vtkMRMLVolumePropertyNode::SafeDownCast(caller) )
      this->UpdateGUI();
  }

  if (event == vtkMRMLScene::SceneCloseEvent)
  {
    if(this->Helper != NULL)
    {
      this->Helper->Delete();
      this->Helper = NULL;
    }

    //Reset every Node related stuff
    this->ParametersNode->SetAndObserveVolumeNodeID(NULL);
    this->ParametersNode->SetAndObserveVolumePropertyNodeID(NULL);
    this->ParametersNode->SetPresetsNodeID(NULL);

    this->ParametersNode->SetAndObserveROINodeID(NULL);

    this->ParametersNode->SetAndObserveFgVolumeNodeID(NULL);
    this->ParametersNode->SetAndObserveFgVolumePropertyNodeID(NULL);
    this->ParametersNode->SetFgPresetsNodeID(NULL);

    this->UpdateGUI();

  }

  if(event == vtkMRMLTransformableNode::TransformModifiedEvent)
  {
    if (this->Helper!=NULL)
    {
      this->Helper->UpdateRendering();
    }
    //TODO when can we remove the op
  }

  this->ProcessingMRMLEvents = 0;

  if(event == vtkMRMLScalarVolumeNode::ImageDataModifiedEvent)
  {
    this->GetApplicationGUI()->GetViewerWidget()->RequestRender();
  }

}

void vtkVolumeRenderingGUI::Enter(void)
{
  if ( this->Built == false )
  {
    this->BuildGUI();
    this->AddGUIObservers();
  }

  //Load Presets
  if(!this->Presets && this->GetLogic())
  {
    vtkMRMLVolumePropertyNode *vrNode=vtkMRMLVolumePropertyNode::New();
    //Instance internal MRMLScene for Presets
    this->Presets = vtkMRMLScene::New();
    //Register node class
    this->Presets->RegisterNodeClass(vrNode);
    vrNode->Delete();

    vtksys_stl::string presetFileName(
        this->GetLogic()->GetModuleShareDirectory());
    presetFileName += "/presets.xml";

    this->Presets->SetURL(presetFileName.c_str());
    this->Presets->Connect();
    this->NS_VolumeRenderingDataPresets->SetMRMLScene(this->Presets);
  }
  //End Load Presets

  this->CreateModuleEventBindings();
  this->UpdateGUI();
}

void vtkVolumeRenderingGUI::Exit(void)
{
  vtkDebugMacro("Exit: removeObservers for VolumeRendering");
  this->ReleaseModuleEventBindings();
}

void vtkVolumeRenderingGUI::UpdateGUI(void)
{
  //First of all check if we have a MRML Scene
  if (!this->Built || !this->GetLogic()->GetMRMLScene() || this->UpdatingGUI)
  {
    //if not return
    return;
  }

  this->UpdatingGUI = 1;

  //update menu first
  //  volume inputs
  if(this->NS_ImageData->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
  {
    this->NS_ImageData->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_ImageData->UpdateMenu();
  }

  if(this->NS_ImageDataFg->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
  {
    this->NS_ImageDataFg->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_ImageDataFg->UpdateMenu();
  }

  if(this->NS_ImageDataLabelmap->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
  {
    this->NS_ImageDataLabelmap->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_ImageDataLabelmap->UpdateMenu();
  }

  //  volume rendering nodes (lighting/material, transfer functions)
  if(this->NS_VolumeRenderingDataScene->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
  {
    this->NS_VolumeRenderingDataScene->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_VolumeRenderingDataScene->UpdateMenu();
  }

  if(this->NS_VolumeRenderingDataSceneFg->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
  {
    this->NS_VolumeRenderingDataSceneFg->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_VolumeRenderingDataSceneFg->UpdateMenu();
  }

  //  presets
  if(this->NS_VolumeRenderingDataPresets->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
  {
    this->NS_VolumeRenderingDataPresets->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_VolumeRenderingDataPresets->UpdateMenu();
  }

  if(this->NS_VolumeRenderingDataPresetsFg->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
  {
    this->NS_VolumeRenderingDataPresetsFg->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_VolumeRenderingDataPresetsFg->UpdateMenu();
  }

  //then set menu selected node
  if (this->ParametersNode)
  {
    this->NS_ImageData->SetSelected( this->ParametersNode->GetVolumeNode() );
    this->NS_ImageDataFg->SetSelected( this->ParametersNode->GetFgVolumeNode() );

    this->NS_VolumeRenderingDataScene->SetSelected( this->ParametersNode->GetVolumePropertyNode()) ;
    this->NS_VolumeRenderingDataSceneFg->SetSelected( this->ParametersNode->GetFgVolumePropertyNode()) ;

    /*presets is a little bit complicated here*/
  }

  //Disable/Enable after Volume is selected
  if(this->NS_ImageData->GetSelected()!=NULL)
  {
    this->PB_CreateNewVolumeRenderingNode->EnabledOn();
    this->PB_HideSurfaceModels->EnabledOn();
    this->NS_VolumeRenderingDataScene->EnabledOn();
    this->NS_VolumeRenderingDataScene->NoneEnabledOff();
    this->EWL_CreateNewVolumePropertyNode->EnabledOn();
    this->NS_VolumeRenderingDataPresets->EnabledOn();

    this->NS_ImageDataFg->EnabledOn();
  }
  else
  {
    this->EWL_CreateNewVolumePropertyNode->EnabledOff();
    this->PB_CreateNewVolumeRenderingNode->EnabledOff();
    this->PB_HideSurfaceModels->EnabledOff();
    this->NS_VolumeRenderingDataScene->NoneEnabledOn();
    this->NS_VolumeRenderingDataScene->SetSelected(NULL);
    this->NS_VolumeRenderingDataScene->EnabledOff();
    this->NS_VolumeRenderingDataPresets->EnabledOff();

    this->NS_ImageDataFg->EnabledOff();//if bg is not selected then turn off fg and labelmap
  }

  if(this->NS_ImageDataFg->GetSelected()!=NULL)
  {
    this->PB_CreateNewVolumeRenderingNodeFg->EnabledOn();
    this->NS_VolumeRenderingDataSceneFg->EnabledOn();
    this->NS_VolumeRenderingDataSceneFg->NoneEnabledOff();
    this->EWL_CreateNewVolumePropertyNodeFg->EnabledOn();
    this->NS_VolumeRenderingDataPresetsFg->EnabledOn();
  }
  else
  {
    this->EWL_CreateNewVolumePropertyNodeFg->EnabledOff();
    this->PB_CreateNewVolumeRenderingNodeFg->EnabledOff();
    this->NS_VolumeRenderingDataSceneFg->NoneEnabledOn();
    this->NS_VolumeRenderingDataSceneFg->SetSelected(NULL);
    this->NS_VolumeRenderingDataSceneFg->EnabledOff();
    this->NS_VolumeRenderingDataPresetsFg->EnabledOff();
  }

  this->InitializePipelineFromMRMLScene();

  this->UpdatingGUI = 0;
}

void vtkVolumeRenderingGUI::SetViewerWidget(vtkSlicerViewerWidget *viewerWidget)
{
}

void vtkVolumeRenderingGUI::SetInteractorStyle(vtkSlicerViewerInteractorStyle *interactorStyle)
{
}


void vtkVolumeRenderingGUI::InitializePipelineFromMRMLScene()
{
  if (this->Helper)
  {
    // vtkImageData* imageData=vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetImageData();
    this->Helper->UpdateGUIElements();
    this->Helper->UpdateRendering();
  }
}

void vtkVolumeRenderingGUI::CreateRenderingFrame()
{
  if (!this->Helper)
    this->Helper = vtkSlicerVolumeRenderingHelper::New();

  this->Helper->Init(this);
}

void vtkVolumeRenderingGUI::DeleteRenderingFrame()
{
  if(this->Helper != NULL)
  {
    this->Helper->Delete();
    this->Helper = NULL;
  }
}

void vtkVolumeRenderingGUI::InitializePipelineNewVolumePropertyFg()
{
  vtkMRMLVolumePropertyNode *rnode =vtkMRMLVolumePropertyNode::New();
  //rnode->HideFromEditorsOff();

  rnode->SetSelectable(1);
  //Add Node to Scene
  this->GetLogic()->GetMRMLScene()->AddNode(rnode);
//  rnode->AddReference(this->NS_ImageDataFg->GetSelected()->GetID());
  rnode->Delete();

  this->ParametersNode->SetAndObserveFgVolumePropertyNodeID(rnode->GetID());

  std::stringstream autoname;
  autoname << "autoViz_";
  autoname << this->NS_ImageDataFg->GetSelected()->GetName();
  this->ParametersNode->GetFgVolumePropertyNode()->SetName(autoname.str().c_str());

  //Update the menu
  this->NS_VolumeRenderingDataSceneFg->SetSelected(rnode);
  this->NS_VolumeRenderingDataSceneFg->UpdateMenu();

  //The Helper has to do something too
  this->Helper->InitializePipelineNewVolumePropertyFg();
  this->Helper->UpdateRendering();
  //take care about references
}

void vtkVolumeRenderingGUI::InitializePipelineNewVolumeProperty()
{
  vtkMRMLVolumePropertyNode *rnode = vtkMRMLVolumePropertyNode::New();
  //rnode->HideFromEditorsOff();
  rnode->SetSelectable(1);

  //Add Node to Scene
  this->GetLogic()->GetMRMLScene()->AddNode(rnode);
//  rnode->AddReference(this->NS_ImageData->GetSelected()->GetID());
  rnode->Delete();

  this->ParametersNode->SetAndObserveVolumePropertyNodeID(rnode->GetID());

  std::stringstream autoname;
  autoname << "autoViz_";
  autoname << this->NS_ImageData->GetSelected()->GetName();
  this->ParametersNode->GetVolumePropertyNode()->SetName(autoname.str().c_str());

  //Update the menu
  this->NS_VolumeRenderingDataScene->SetSelected(rnode);
  this->NS_VolumeRenderingDataScene->UpdateMenu();

  //The Helper has to do something too
  this->Helper->InitializePipelineNewVolumeProperty();
  this->Helper->UpdateRendering();
  //take care about references
}

void vtkVolumeRenderingGUI::InitializePipelineFromImageData()
{
  vtkMRMLVolumePropertyNode *tmp = NULL;

  if (this->ParametersNode && this->ParametersNode->GetVolumePropertyNodeID())
  {
    tmp = vtkMRMLVolumePropertyNode::SafeDownCast(
        this->GetLogic()->GetMRMLScene()->GetNodeByID(this->ParametersNode->GetVolumePropertyNodeID()) );
  }

  if (tmp)
  {
    this->ParametersNode->SetAndObserveVolumePropertyNodeID(tmp->GetID());
    this->NS_VolumeRenderingDataScene->SetSelected(tmp);

    this->InitializePipelineFromMRMLScene();
  }
  else
  {
    this->InitializePipelineNewVolumeProperty();
  }

  //Ensure that none is not visible
  this->NS_VolumeRenderingDataScene->NoneEnabledOff();
  this->NS_VolumeRenderingDataScene->UpdateMenu();

  this->PipelineInitializedOn();
}

void vtkVolumeRenderingGUI::InitializePipelineFromImageDataFg()
{
  vtkMRMLVolumePropertyNode *tmp = NULL;

  if (this->ParametersNode && this->ParametersNode->GetFgVolumePropertyNodeID())
  {
    tmp = vtkMRMLVolumePropertyNode::SafeDownCast(
        this->GetLogic()->GetMRMLScene()->GetNodeByID(this->ParametersNode->GetFgVolumePropertyNodeID()) );
  }

  if (tmp)
  {
    this->ParametersNode->SetAndObserveFgVolumePropertyNodeID(tmp->GetID());
    this->NS_VolumeRenderingDataSceneFg->SetSelected(tmp);

    this->InitializePipelineFromMRMLScene();
  }
  else
  {
    this->InitializePipelineNewVolumePropertyFg();
  }

  //Ensure that none is not visible
  this->NS_VolumeRenderingDataSceneFg->NoneEnabledOff();
  this->NS_VolumeRenderingDataSceneFg->UpdateMenu();

  this->PipelineInitializedFgOn();
}
