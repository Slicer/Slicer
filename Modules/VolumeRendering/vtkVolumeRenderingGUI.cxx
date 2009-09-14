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
#include "vtkMRMLVolumeRenderingNode.h"
#include "vtkMRMLVolumeRenderingNode.h"
#include "vtkPiecewiseFunction.h"
#include "vtkPlaneSource.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkRendererCollection.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerFixedPointVolumeRayCastMapper.h"
#include "vtkSlicerMRMLTreeWidget.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerVRGrayscaleHelper.h"
#include "vtkSlicerVRHelper.h"
#include "vtkSlicerVolumePropertyWidget.h"
#include "vtkSlicerVolumeTextureMapper3D.h"
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

  this->SelectionNode = NULL;
  
  this->Presets=NULL;
  
  this->PreviousNS_ImageData="";
  this->PreviousNS_ImageDataFg="";
  this->PreviousNS_ImageDataLabelmap="";
  
  this->PreviousNS_VolumeRenderingDataScene="";
  this->PreviousNS_VolumeRenderingSlicer="";
  
  this->PreviousNS_VolumeRenderingDataSceneFg="";
  this->PreviousNS_VolumeRenderingSlicerFg="";
  
  this->PB_HideSurfaceModels=NULL;
  this->PB_CreateNewVolumeRenderingNode=NULL;
  this->PB_CreateNewVolumeRenderingNodeFg=NULL;
  
  this->NS_ImageData=NULL;
  this->NS_ImageDataFg=NULL;
  this->NS_ImageDataLabelmap=NULL;
  
  this->NS_VolumeRenderingDataPresets=NULL;
  this->NS_VolumeRenderingDataScene=NULL;
  this->EWL_CreateNewVolumeRenderingNode=NULL;

  this->NS_VolumeRenderingDataPresetsFg=NULL;
  this->NS_VolumeRenderingDataSceneFg=NULL;
  this->EWL_CreateNewVolumeRenderingNodeFg=NULL;
  
  //Frame Details
  this->RenderingFrame=NULL;

  //Other members
  this->CurrentNode=NULL;
  this->CurrentNodeFg=NULL;
  
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

  vtkSetAndObserveMRMLNodeMacro(this->SelectionNode, NULL);

  vtkSetAndObserveMRMLNodeMacro(this->CurrentNode, NULL);
  vtkSetAndObserveMRMLNodeMacro(this->CurrentNodeFg, NULL);

  //Not Delete?!
  //vtkVolumeRenderingModuleLogic *Logic;
  //vtkSlicerViewerWidget *ViewerWidget;
  //vtkSlicerViewerInteractorStyle *InteractorStyle;
  //vtkMRMLVolumeRenderingNode  *CurrentNode;//really delete this

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

  if(this->EWL_CreateNewVolumeRenderingNode)
    {
    this->EWL_CreateNewVolumeRenderingNode->SetParent(NULL);
    this->EWL_CreateNewVolumeRenderingNode->Delete();
    this->EWL_CreateNewVolumeRenderingNode=NULL;
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

  if(this->EWL_CreateNewVolumeRenderingNodeFg)
    {
    this->EWL_CreateNewVolumeRenderingNodeFg->SetParent(NULL);
    this->EWL_CreateNewVolumeRenderingNodeFg->Delete();
    this->EWL_CreateNewVolumeRenderingNodeFg=NULL;
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
  if(this->CurrentNode)
    {
    //this->CurrentNode->Delete();
    this->CurrentNode=NULL;
    }
  if(this->CurrentNodeFg)
    {
    //this->CurrentNode->Delete();
    this->CurrentNodeFg=NULL;
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
  int labelWidth=20;
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  this->GetUIPanel()->AddPage("VolumeRendering","VolumeRendering",NULL);

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
  
    //NodeSelector for VolumeRenderingNode Preset
    this->NS_VolumeRenderingDataPresets=vtkSlicerNodeSelectorVolumeRenderingWidget::New();
    this->NS_VolumeRenderingDataPresets->SetParent(inputVolumeFrame->GetFrame());
    this->NS_VolumeRenderingDataPresets->Create();
    this->NS_VolumeRenderingDataPresets->SetLabelText("Presets:");
    this->NS_VolumeRenderingDataPresets->SetBalloonHelpString("Select one of the existing parameter sets or presets.");
    this->NS_VolumeRenderingDataPresets->SetLabelWidth(labelWidth);
    this->NS_VolumeRenderingDataPresets->EnabledOff();//By default off
    this->NS_VolumeRenderingDataPresets->NoneEnabledOn();
    this->NS_VolumeRenderingDataPresets->SetShowHidden(1);
    this->NS_VolumeRenderingDataPresets->SetNodeClass("vtkMRMLVolumeRenderingNode","","","");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_VolumeRenderingDataPresets->GetWidgetName());

    //NodeSelector for VolumeRenderingNode Scene
    this->NS_VolumeRenderingDataScene=vtkSlicerNodeSelectorVolumeRenderingWidget::New();
    this->NS_VolumeRenderingDataScene->SetParent(inputVolumeFrame->GetFrame());
    this->NS_VolumeRenderingDataScene->Create();
    this->NS_VolumeRenderingDataScene->NoneEnabledOn();
    this->NS_VolumeRenderingDataScene->SetLabelText("Current Parameter Set:");
    this->NS_VolumeRenderingDataScene->SetBalloonHelpString("Select how the volume should be displayed. Several parameter sets per volume are possible");
    this->NS_VolumeRenderingDataScene->SetLabelWidth(labelWidth);
    this->NS_VolumeRenderingDataScene->EnabledOff();//By default off
    this->NS_VolumeRenderingDataScene->SetShowHidden(1);
    this->NS_VolumeRenderingDataScene->SetNodeClass("vtkMRMLVolumeRenderingNode","","","");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_VolumeRenderingDataScene->GetWidgetName());
    //Missing: Load from file
  
    //Create New Volume Rendering Node
    //Entry With Label
    this->EWL_CreateNewVolumeRenderingNode=vtkKWEntryWithLabel::New();
    this->EWL_CreateNewVolumeRenderingNode->SetParent(inputVolumeFrame->GetFrame());
    this->EWL_CreateNewVolumeRenderingNode->Create();
    this->EWL_CreateNewVolumeRenderingNode->SetBalloonHelpString("Specify a name for a new parameter set.");
    this->EWL_CreateNewVolumeRenderingNode->SetLabelText("New Parameter Set:");
    this->EWL_CreateNewVolumeRenderingNode->SetLabelWidth(labelWidth);
    this->EWL_CreateNewVolumeRenderingNode->EnabledOff();
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2", this->EWL_CreateNewVolumeRenderingNode->GetWidgetName());
  
    this->PB_CreateNewVolumeRenderingNode=vtkKWPushButton::New();
    this->PB_CreateNewVolumeRenderingNode->SetParent(inputVolumeFrame->GetFrame());
    this->PB_CreateNewVolumeRenderingNode->Create();
    this->PB_CreateNewVolumeRenderingNode->SetBalloonHelpString("Create a new parameter set for the current volume. This way you can switch between different visualization settings for the same volume.");
    this->PB_CreateNewVolumeRenderingNode->SetText("Create New Visualization Parameter Set");
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
    this->NS_VolumeRenderingDataPresetsFg=vtkSlicerNodeSelectorVolumeRenderingWidget::New();
    this->NS_VolumeRenderingDataPresetsFg->SetParent(inputVolumeFrame->GetFrame());
    this->NS_VolumeRenderingDataPresetsFg->Create();
    this->NS_VolumeRenderingDataPresetsFg->SetLabelText("Presets:");
    this->NS_VolumeRenderingDataPresetsFg->SetBalloonHelpString("Select one of the existing parameter sets or presets.");
    this->NS_VolumeRenderingDataPresetsFg->SetLabelWidth(labelWidth);
    this->NS_VolumeRenderingDataPresetsFg->EnabledOff();//By default off
    this->NS_VolumeRenderingDataPresetsFg->NoneEnabledOn();
    this->NS_VolumeRenderingDataPresetsFg->SetShowHidden(1);
    this->NS_VolumeRenderingDataPresetsFg->SetNodeClass("vtkMRMLVolumeRenderingNode","","","");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_VolumeRenderingDataPresetsFg->GetWidgetName());

    //NodeSelector for VolumeRenderingNode Scene
    this->NS_VolumeRenderingDataSceneFg=vtkSlicerNodeSelectorVolumeRenderingWidget::New();
    this->NS_VolumeRenderingDataSceneFg->SetParent(inputVolumeFrame->GetFrame());
    this->NS_VolumeRenderingDataSceneFg->Create();
    this->NS_VolumeRenderingDataSceneFg->NoneEnabledOn();
    this->NS_VolumeRenderingDataSceneFg->SetLabelText("Current Parameter Set:");
    this->NS_VolumeRenderingDataSceneFg->SetBalloonHelpString("Select how the volume should be displayed. Several parameter sets per volume are possible");
    this->NS_VolumeRenderingDataSceneFg->SetLabelWidth(labelWidth);
    this->NS_VolumeRenderingDataSceneFg->EnabledOff();//By default off
    this->NS_VolumeRenderingDataSceneFg->SetShowHidden(1);
    this->NS_VolumeRenderingDataSceneFg->SetNodeClass("vtkMRMLVolumeRenderingNode","","","");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_VolumeRenderingDataSceneFg->GetWidgetName());
    //Missing: Load from file
  
    //Create New Volume Rendering Node
    //Entry With Label
    this->EWL_CreateNewVolumeRenderingNodeFg=vtkKWEntryWithLabel::New();
    this->EWL_CreateNewVolumeRenderingNodeFg->SetParent(inputVolumeFrame->GetFrame());
    this->EWL_CreateNewVolumeRenderingNodeFg->Create();
    this->EWL_CreateNewVolumeRenderingNodeFg->SetBalloonHelpString("Specify a name for a new parameter set.");
    this->EWL_CreateNewVolumeRenderingNodeFg->SetLabelText("New Parameter Set:");
    this->EWL_CreateNewVolumeRenderingNodeFg->SetLabelWidth(labelWidth);
    this->EWL_CreateNewVolumeRenderingNodeFg->EnabledOff();
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2", this->EWL_CreateNewVolumeRenderingNodeFg->GetWidgetName());
  
    this->PB_CreateNewVolumeRenderingNodeFg=vtkKWPushButton::New();
    this->PB_CreateNewVolumeRenderingNodeFg->SetParent(inputVolumeFrame->GetFrame());
    this->PB_CreateNewVolumeRenderingNodeFg->Create();
    this->PB_CreateNewVolumeRenderingNodeFg->SetBalloonHelpString("Create a new parameter set for the current volume. This way you can switch between different visualization settings for the same volume.");
    this->PB_CreateNewVolumeRenderingNodeFg->SetText("Create New Visualization Parameter Set");
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
    
  vtkMRMLVolumeRenderingSelectionNode* selectionNode = this->GetLogic()->GetSelectionNode();
  vtkSetAndObserveMRMLNodeMacro(this->SelectionNode, selectionNode);
  
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
//  if (this->ProcessingGUIEvents || this->ProcessingMRMLEvents)
  if (this->ProcessingGUIEvents )
    {
    return;
    }
  this->ProcessingGUIEvents = 1;

  vtkDebugMacro("vtkVolumeRenderingGUI::ProcessGUIEvents: event = " << event);

  //
  //Check PushButtons
  //
  vtkKWPushButton *callerObject=vtkKWPushButton::SafeDownCast(caller);
  // hide surface models to reveal volume rendering
  if(callerObject==this->PB_HideSurfaceModels&&event==vtkKWPushButton::InvokedEvent)
    {
    int count=this->GetLogic()->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLModelNode");
    for(int i=0;i<count;i++)
      {
      (vtkMRMLModelNode::SafeDownCast(this->GetLogic()->GetMRMLScene()->GetNthNodeByClass(i,"vtkMRMLModelNode")))->GetModelDisplayNode()->VisibilityOff();
      }
    }
  //Create New VolumeRenderingNode
  else if (callerObject==this->PB_CreateNewVolumeRenderingNode&&event==vtkKWPushButton::InvokedEvent)
    {
    //Get a new auto CurrentNode
    this->InitializePipelineNewCurrentNode();
    //this->CurrentNode->HideFromEditorsOff();

    //Set the right name
    const char *name=this->EWL_CreateNewVolumeRenderingNode->GetWidget()->GetValue();
    if(!name || strlen(name) == 0)
      {
      this->CurrentNode->SetName("No Name");
      }
    else 
      {
      this->CurrentNode->SetName(name);
      }
    //Remove Text from Entry
    this->EWL_CreateNewVolumeRenderingNode->GetWidget()->SetValue("");
    this->NS_VolumeRenderingDataScene->UpdateMenu();
    }
  else if (callerObject==this->PB_CreateNewVolumeRenderingNodeFg&&event==vtkKWPushButton::InvokedEvent)
    {
    //Get a new auto CurrentNode
    this->InitializePipelineNewCurrentNodeFg();
    //this->CurrentNode->HideFromEditorsOff();

    //Set the right name
    const char *name=this->EWL_CreateNewVolumeRenderingNodeFg->GetWidget()->GetValue();
    if(!name || strlen(name) == 0)
      {
      this->CurrentNodeFg->SetName("No Name");
      }
    else 
      {
      this->CurrentNodeFg->SetName(name);
      }
    //Remove Text from Entry
    this->EWL_CreateNewVolumeRenderingNodeFg->GetWidget()->SetValue("");
    this->NS_VolumeRenderingDataSceneFg->UpdateMenu();
    }
  //
  // End Check PushButtons
  // 

  //
  //Check Node Selectors
  //
  vtkSlicerNodeSelectorWidget *callerObjectNS=vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  //Load Volume
  if(callerObjectNS==this->NS_ImageDataFg&&event==vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    if(this->NS_ImageDataFg->GetSelected() == NULL)
    {
      //Remove the old observer for the imagedata
      if(strcmp("",this->PreviousNS_ImageDataFg.c_str())==0)
        {
        vtkMRMLNode *oldNode=this->GetLogic()->GetMRMLScene()->GetNodeByID(this->PreviousNS_ImageDataFg.c_str());
        if(oldNode!=NULL)
          {
          oldNode->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent,(vtkCommand *) this->MRMLCallbackCommand);
          //NH
          oldNode->RemoveObservers(vtkMRMLScalarVolumeNode::ImageDataModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );
          }
            
        }
           
      this->SelectionNode->SetActiveVolumeIDFg(NULL);

      //Try to keep navigation render online
      this->GetApplicationGUI()->GetViewControlGUI()->GetEnableDisableNavButton()->SelectedStateOn();
      this->PreviousNS_ImageDataFg="";
    }
    else if(strcmp(this->NS_ImageDataFg->GetSelected()->GetID(),this->PreviousNS_ImageDataFg.c_str())!=0)//Only proceed event,if new Node
      {
      this->SelectionNode->SetActiveVolumeIDFg(this->NS_ImageDataFg->GetSelected()->GetID());

      //Try to keep navigation Render online
      //this->GetApplicationGUI()->GetViewControlGUI()->GetEnableDisableNavButton()->SelectedStateOff();
      this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->Disable();

      vtkMRMLScalarVolumeNode *selectedImageData=vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageDataFg->GetSelected());
      //Add observer to trigger update of transform
      selectedImageData->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent,(vtkCommand *) this->MRMLCallbackCommand);
      //NH
      selectedImageData->AddObserver(vtkMRMLScalarVolumeNode::ImageDataModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );

      this->Helper->SetupHistogramFg();
      
      //Initialize the Pipeline
      this->InitializePipelineFromImageDataFg();

      //update previous:
      this->PreviousNS_ImageDataFg=this->NS_ImageData->GetSelected()->GetID();//only when not "None"
      this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->Enable();
      if (this->Helper)
        {
        this->Helper->WithdrawProgressDialog();
        }
      }//else if
  }
  else if(callerObjectNS==this->NS_ImageData&&event==vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
    {
    if(this->NS_ImageData->GetSelected()==NULL)
      {
      //Remove the old observer for the imagedata
      if(strcmp("",this->PreviousNS_ImageData.c_str())==0)
        {
        vtkMRMLNode *oldNode=this->GetLogic()->GetMRMLScene()->GetNodeByID(this->PreviousNS_ImageData.c_str());
        if(oldNode!=NULL)
          {
          oldNode->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent,(vtkCommand *) this->MRMLCallbackCommand);
          //NH
          oldNode->RemoveObservers(vtkMRMLScalarVolumeNode::ImageDataModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );
          }
            
        }
           
      this->SelectionNode->SetActiveVolumeID(NULL);

      //Unpack the details frame
      this->UnpackSvpGUI();
      //Try to keep navigation render online
      this->GetApplicationGUI()->GetViewControlGUI()->GetEnableDisableNavButton()->SelectedStateOn();
      this->PreviousNS_ImageData="";
      }
    //Only proceed event,if new Node
    else if(strcmp(this->NS_ImageData->GetSelected()->GetID(),this->PreviousNS_ImageData.c_str())!=0)
      {
      this->SelectionNode->SetActiveVolumeID(this->NS_ImageData->GetSelected()->GetID());

      //Try to keep navigation Render online
      //this->GetApplicationGUI()->GetViewControlGUI()->GetEnableDisableNavButton()->SelectedStateOff();
      this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->Disable();

      vtkMRMLScalarVolumeNode *selectedImageData=vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected());
      //Add observer to trigger update of transform
      selectedImageData->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent,(vtkCommand *) this->MRMLCallbackCommand);
      //NH
      selectedImageData->AddObserver(vtkMRMLScalarVolumeNode::ImageDataModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );
      this->UnpackSvpGUI();
      this->PackSvpGUI();

      //Initialize the Pipeline
      this->InitializePipelineFromImageData();

      //update previous:
      this->PreviousNS_ImageData=this->NS_ImageData->GetSelected()->GetID();//only when not "None"
      this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->GetRenderWindowInteractor()->Enable();
      if (this->Helper)
        {
        this->Helper->WithdrawProgressDialog();
        }


      }//else if
    }//if
  //Volume RenderingDataScene
  else if(callerObjectNS==this->NS_VolumeRenderingDataScene&&event==vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
    {
    //Check for None selected //Just to be safe
    if(this->NS_VolumeRenderingDataScene->GetSelected()==NULL)
      {
      this->PreviousNS_VolumeRenderingDataScene="";
      }
    //Only proceed event,if new Node
    else if(strcmp(this->NS_VolumeRenderingDataScene->GetSelected()->GetID(),this->PreviousNS_VolumeRenderingDataScene.c_str())!=0)
      {
      vtkSetAndObserveMRMLNodeMacro(this->CurrentNode, vtkMRMLVolumeRenderingNode::SafeDownCast(this->NS_VolumeRenderingDataScene->GetSelected()));
      this->SelectionNode->SetActiveVolumeRenderingID(this->CurrentNode->GetID());
      this->InitializePipelineFromMRMLScene();
      this->PreviousNS_VolumeRenderingDataScene=this->NS_VolumeRenderingDataScene->GetSelected()->GetID();
      }
    }
  else if(callerObjectNS==this->NS_VolumeRenderingDataSceneFg&&event==vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
    {
    //Check for None selected //Just to be safe
    if(this->NS_VolumeRenderingDataSceneFg->GetSelected()==NULL)
      {
      this->PreviousNS_VolumeRenderingDataSceneFg="";
      }
    //Only proceed event,if new Node
    else if(strcmp(this->NS_VolumeRenderingDataSceneFg->GetSelected()->GetID(),this->PreviousNS_VolumeRenderingDataSceneFg.c_str())!=0)
      {
      vtkSetAndObserveMRMLNodeMacro(this->CurrentNodeFg, vtkMRMLVolumeRenderingNode::SafeDownCast(this->NS_VolumeRenderingDataSceneFg->GetSelected()));
      this->SelectionNode->SetActiveVolumeRenderingIDFg(this->CurrentNodeFg->GetID());
      this->InitializePipelineFromMRMLScene();
      this->PreviousNS_VolumeRenderingDataSceneFg = this->NS_VolumeRenderingDataSceneFg->GetSelected()->GetID();
      }
    }
  //VolumeRenderingDataSlicer
  else if(callerObjectNS==this->NS_VolumeRenderingDataPresets&&event==vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
    {
    //Check for None selected
    if(this->NS_VolumeRenderingDataPresets->GetSelected()==NULL)
      {
      this->PreviousNS_VolumeRenderingSlicer="";
      }
    //Only proceed event,if other Node
    else if(strcmp(this->NS_VolumeRenderingDataPresets->GetSelected()->GetID(),this->PreviousNS_VolumeRenderingSlicer.c_str())!=0)
      {
      //check if we have a preset or a "normal VolumeRenderingNode

      //We have a preset, we can find id in our Presets
      if(this->Presets->GetNodeByID(this->NS_VolumeRenderingDataPresets->GetSelected()->GetID())!=NULL)
        {
        //Copy Preset Information in current Node
        this->CurrentNode->CopyParameterSet(this->NS_VolumeRenderingDataPresets->GetSelected());
        this->Helper->UpdateGUIElements();
        }
      //It's not a preset so just update references, and select the new Added Node
      else 
        {
        vtkMRMLVolumeRenderingNode *rnode = vtkMRMLVolumeRenderingNode::SafeDownCast(this->NS_VolumeRenderingDataPresets->GetSelected());
        rnode->AddReference(this->NS_ImageData->GetSelected()->GetID());
        vtkSetAndObserveMRMLNodeMacro(this->CurrentNode, rnode);
        this->SelectionNode->SetActiveVolumeRenderingID(this->CurrentNode->GetID());
        this->NS_VolumeRenderingDataScene->UpdateMenu();
        this->NS_VolumeRenderingDataScene->SetSelected(this->NS_VolumeRenderingDataPresets->GetSelected());
        }
      }
    }
  else if(callerObjectNS==this->NS_VolumeRenderingDataPresetsFg&&event==vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
    {
    //Check for None selected
    if(this->NS_VolumeRenderingDataPresetsFg->GetSelected()==NULL)
      {
      this->PreviousNS_VolumeRenderingSlicerFg="";
      }
    //Only proceed event,if other Node
    else if(strcmp(this->NS_VolumeRenderingDataPresetsFg->GetSelected()->GetID(),this->PreviousNS_VolumeRenderingSlicerFg.c_str())!=0)
      {
      //check if we have a preset or a "normal VolumeRenderingNode

      //We have a preset, we can find id in our Presets
      if(this->Presets->GetNodeByID(this->NS_VolumeRenderingDataPresetsFg->GetSelected()->GetID())!=NULL)
        {
        //Copy Preset Information in current Node
        this->CurrentNodeFg->CopyParameterSet(this->NS_VolumeRenderingDataPresetsFg->GetSelected());
        this->Helper->UpdateGUIElements();
        }
      //It's not a preset so just update references, and select the new Added Node
      else 
        {
        vtkMRMLVolumeRenderingNode *rnode = vtkMRMLVolumeRenderingNode::SafeDownCast(this->NS_VolumeRenderingDataPresetsFg->GetSelected());
        rnode->AddReference(this->NS_ImageDataFg->GetSelected()->GetID());
        vtkSetAndObserveMRMLNodeMacro(this->CurrentNodeFg, rnode);
        this->SelectionNode->SetActiveVolumeRenderingIDFg(this->CurrentNodeFg->GetID());
        this->NS_VolumeRenderingDataSceneFg->UpdateMenu();
        this->NS_VolumeRenderingDataSceneFg->SetSelected(this->NS_VolumeRenderingDataPresetsFg->GetSelected());
        }
      }
    }
  //
  //End Check NodeSelectors
  //
  //Update GUI
  this->UpdateGUI();
  this->ProcessingGUIEvents = 0;

}
void vtkVolumeRenderingGUI::ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
  if (this->ProcessingGUIEvents || this->ProcessingMRMLEvents)
    {
    return;
    }
  this->ProcessingMRMLEvents = 1;

  vtkMRMLNode *addedNode = NULL;
    
  if (event == vtkMRMLScene::NodeAddedEvent && this->MRMLScene)
    {
    addedNode = reinterpret_cast<vtkMRMLNode *>(callData);
    }
        
  if (event == vtkMRMLScene::NodeAddedEvent && addedNode &&
      addedNode->IsA("vtkMRMLVolumeRenderingSelectionNode"))
    {
    vtkMRMLVolumeRenderingSelectionNode* selectionNode = this->GetLogic()->GetSelectionNode();
    vtkSetAndObserveMRMLNodeMacro(this->SelectionNode, selectionNode);
    //this->UpdateGUI();
    }
  if (event == vtkMRMLScene::NodeAddedEvent && addedNode &&
      addedNode->IsA("vtkMRMLVolumeNode"))
    {
    this->UpdateGUI();
    }   
         
  if (vtkMRMLVolumeRenderingSelectionNode::SafeDownCast(caller)&&
      this->SelectionNode == vtkMRMLVolumeRenderingSelectionNode::SafeDownCast(caller) &&
      event == vtkCommand::ModifiedEvent && this->MRMLScene)
    {
    this->UpdateGUI();
    }
  if (vtkMRMLVolumeRenderingNode::SafeDownCast(caller)&&
      (this->CurrentNode == vtkMRMLVolumeRenderingNode::SafeDownCast(caller) || this->CurrentNodeFg == vtkMRMLVolumeRenderingNode::SafeDownCast(caller)) &&
      event == vtkCommand::ModifiedEvent && this->MRMLScene)
    {
    this->UpdateGUI();
    }
      
  if (event == vtkMRMLScene::SceneCloseEvent)
    {
    if(this->Helper!=NULL)
      {
      this->Helper->Delete();
      this->Helper=NULL;
      }
    //Reset every Node related stuff
    this->PreviousNS_ImageData="";
    this->PreviousNS_ImageDataFg="";
    this->PreviousNS_ImageDataLabelmap="";
    this->PreviousNS_VolumeRenderingDataScene="";
    this->PreviousNS_VolumeRenderingSlicer="";
    this->CurrentNode=NULL;
    this->PreviousNS_VolumeRenderingDataSceneFg="";
    this->PreviousNS_VolumeRenderingSlicerFg="";
    this->CurrentNodeFg=NULL;
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

  if(event == vtkMRMLScalarVolumeNode::ImageDataModifiedEvent){
    this->GetApplicationGUI()->GetViewerWidget()->RequestRender();
  }

}

void vtkVolumeRenderingGUI::Enter(void)
{
  vtkDebugMacro("Enter Volume Rendering Module");
  //Load Presets

  if(!this->Presets && this->GetLogic())
    {
    vtkMRMLVolumeRenderingNode *vrNode=vtkMRMLVolumeRenderingNode::New();
    //Instance internal MRMLScene for Presets
    this->Presets=vtkMRMLScene::New();
    //Register node class
    this->Presets->RegisterNodeClass(vrNode);
    vrNode->Delete();

    vtksys_stl::string presetFileName(
      this->GetLogic()->GetModuleShareDirectory());
    presetFileName += "/presets.xml";

    this->Presets->SetURL(presetFileName.c_str());
    this->Presets->Connect();
    this->NS_VolumeRenderingDataPresets->SetAdditionalMRMLScene(this->Presets);
    }
  //End Load Presets

  if ( this->Built == false )
    {
    this->BuildGUI();
    this->AddGUIObservers();
    }
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
  if (!this->Built ||
      !this->GetLogic()->GetMRMLScene() || 
      this->UpdatingGUI)
    {
    //if not return
    return;
    }
    
  this->UpdatingGUI = 1;
   
  if (this->SelectionNode)
    {
    vtkMRMLVolumeRenderingNode *rnode = vtkMRMLVolumeRenderingNode::SafeDownCast(
      this->MRMLScene->GetNodeByID(this->SelectionNode->GetActiveVolumeRenderingID()) );
    vtkSetAndObserveMRMLNodeMacro(this->CurrentNode, rnode);
    this->NS_VolumeRenderingDataScene->SetSelected( this->CurrentNode) ;
                
    this->NS_ImageData->SetSelected( vtkMRMLVolumeNode::SafeDownCast(
                                       this->MRMLScene->GetNodeByID(this->SelectionNode->GetActiveVolumeID()) ) );
    }
    
  if (this->SelectionNode)
    {
    vtkMRMLVolumeRenderingNode *rnode = vtkMRMLVolumeRenderingNode::SafeDownCast(
      this->MRMLScene->GetNodeByID(this->SelectionNode->GetActiveVolumeRenderingIDFg()) );
    vtkSetAndObserveMRMLNodeMacro(this->CurrentNodeFg, rnode);
    this->NS_VolumeRenderingDataSceneFg->SetSelected( this->CurrentNodeFg) ;
                
    this->NS_ImageDataFg->SetSelected( vtkMRMLVolumeNode::SafeDownCast(
                                       this->MRMLScene->GetNodeByID(this->SelectionNode->GetActiveVolumeIDFg()) ) );
    }
      
  if(this->NS_ImageData->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
    {
    //Update the NodeSelector for Volumes
    this->NS_ImageData->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_ImageData->UpdateMenu();
    }
  if(this->NS_ImageDataFg->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
    {
    //Update the NodeSelector for Volumes
    this->NS_ImageDataFg->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_ImageDataFg->UpdateMenu();
    }
  if(this->NS_ImageDataLabelmap->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
    {
    //Update the NodeSelector for Volumes
    this->NS_ImageDataLabelmap->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_ImageDataLabelmap->UpdateMenu();
    }
    
  if(this->NS_VolumeRenderingDataScene->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
    {
    //Update NodeSelector for VolumeRendering Node
    this->NS_VolumeRenderingDataScene->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_VolumeRenderingDataScene->UpdateMenu();
    }
  if(this->NS_VolumeRenderingDataSceneFg->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
    {
    //Update NodeSelector for VolumeRendering Node
    this->NS_VolumeRenderingDataSceneFg->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_VolumeRenderingDataSceneFg->UpdateMenu();
    }  


  //Set the new condition
  if(this->NS_ImageData->GetSelected()!=NULL&&(this->NS_VolumeRenderingDataScene->GetCondition()!=this->NS_ImageData->GetSelected()->GetID()))
    {
    this->NS_VolumeRenderingDataScene->SetCondition(this->NS_ImageData->GetSelected()->GetID(),vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetLabelMap(),true);
    }
  this->NS_VolumeRenderingDataScene->UpdateMenu();
  
  if(this->NS_ImageDataFg->GetSelected()!=NULL&&(this->NS_VolumeRenderingDataSceneFg->GetCondition()!=this->NS_ImageDataFg->GetSelected()->GetID()))
    {
    this->NS_VolumeRenderingDataSceneFg->SetCondition(this->NS_ImageDataFg->GetSelected()->GetID(),vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageDataFg->GetSelected())->GetLabelMap(),true);
    }
  this->NS_VolumeRenderingDataSceneFg->UpdateMenu();

  //Take care about Presets...
  //We need None for this
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
    
  if(this->NS_ImageData->GetSelected()!=NULL)
    {
    this->NS_VolumeRenderingDataPresets->SetCondition(this->NS_ImageData->GetSelected()->GetID(),vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetLabelMap(),false);
    this->NS_VolumeRenderingDataPresets->UpdateMenu();
    }
  if(this->NS_ImageDataFg->GetSelected()!=NULL)
    {
    this->NS_VolumeRenderingDataPresetsFg->SetCondition(this->NS_ImageDataFg->GetSelected()->GetID(),vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageDataFg->GetSelected())->GetLabelMap(),false);
    this->NS_VolumeRenderingDataPresetsFg->UpdateMenu();
    }
    
  //Disable/Enable after Volume is selected
  if(this->NS_ImageData->GetSelected()!=NULL)
    {
    this->PB_CreateNewVolumeRenderingNode->EnabledOn();
    this->PB_HideSurfaceModels->EnabledOn();
    this->NS_VolumeRenderingDataScene->EnabledOn();
    this->NS_VolumeRenderingDataScene->NoneEnabledOff();
    this->EWL_CreateNewVolumeRenderingNode->EnabledOn();
    this->NS_VolumeRenderingDataPresets->EnabledOn();
    
    this->NS_ImageDataFg->EnabledOn();
    }
  else
    {
    this->EWL_CreateNewVolumeRenderingNode->EnabledOff();
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
    this->EWL_CreateNewVolumeRenderingNodeFg->EnabledOn();
    this->NS_VolumeRenderingDataPresetsFg->EnabledOn();
    }
  else
    {
    this->EWL_CreateNewVolumeRenderingNodeFg->EnabledOff();
    this->PB_CreateNewVolumeRenderingNodeFg->EnabledOff();
    this->NS_VolumeRenderingDataSceneFg->NoneEnabledOn();
    this->NS_VolumeRenderingDataSceneFg->SetSelected(NULL);
    this->NS_VolumeRenderingDataSceneFg->EnabledOff();
    this->NS_VolumeRenderingDataPresetsFg->EnabledOff();
    }
    
  //In Presets always "None" is selected
  this->NS_VolumeRenderingDataPresets->SetSelected(NULL);
  this->NS_VolumeRenderingDataPresetsFg->SetSelected(NULL);
    
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

void vtkVolumeRenderingGUI::PackSvpGUI()
{
  if (!this->Helper)
    {
    this->Helper=vtkSlicerVRGrayscaleHelper::New();
    }
  this->Helper->Init(this);
}

void vtkVolumeRenderingGUI::UnpackSvpGUI()
{
  if(this->Helper!=NULL)
    {
    this->Helper->Delete();
    this->Helper=NULL;
    }
}

void vtkVolumeRenderingGUI::InitializePipelineNewCurrentNodeFg()
{
  //TODO move this part
  vtkMRMLVolumeRenderingNode *rnode =vtkMRMLVolumeRenderingNode::New();
  //rnode->HideFromEditorsOff();
  rnode->SetSelectable(1);
  //Add Node to Scene
  this->GetLogic()->GetMRMLScene()->AddNode(rnode);
  rnode->AddReference(this->NS_ImageDataFg->GetSelected()->GetID());
  rnode->Delete();
    
  vtkSetAndObserveMRMLNodeMacro(this->CurrentNodeFg, rnode);

  this->SelectionNode->SetActiveVolumeRenderingIDFg(this->CurrentNodeFg->GetID());
  //Update the menu
  this->PreviousNS_VolumeRenderingDataSceneFg=this->CurrentNodeFg->GetID();
  this->NS_VolumeRenderingDataSceneFg->SetSelected(this->CurrentNodeFg);
  this->NS_VolumeRenderingDataSceneFg->UpdateMenu(); 
  //this->PreviousNS_VolumeRenderingDataScene=this->Gui->GetCurrentNode()->GetID();
  //The Helper has to do something too
  this->Helper->InitializePipelineNewCurrentNodeFg();
  this->Helper->UpdateGUIElements();
  this->Helper->UpdateRendering();
  //take care about references
}

void vtkVolumeRenderingGUI::InitializePipelineNewCurrentNode()
{
  //TODO move this part
  vtkMRMLVolumeRenderingNode *rnode =vtkMRMLVolumeRenderingNode::New();
  //rnode->HideFromEditorsOff();
  rnode->SetSelectable(1);
  //Add Node to Scene
  this->GetLogic()->GetMRMLScene()->AddNode(rnode);
  rnode->AddReference(this->NS_ImageData->GetSelected()->GetID());
  rnode->Delete();
    
  vtkSetAndObserveMRMLNodeMacro(this->CurrentNode, rnode);
  
  this->SelectionNode->SetActiveVolumeRenderingID(this->CurrentNode->GetID());
  //Update the menu
  this->PreviousNS_VolumeRenderingDataScene=this->CurrentNode->GetID();
  this->NS_VolumeRenderingDataScene->SetSelected(this->CurrentNode);
  this->NS_VolumeRenderingDataScene->UpdateMenu(); 
  //this->PreviousNS_VolumeRenderingDataScene=this->Gui->GetCurrentNode()->GetID();
  //The Helper has to do something too
  this->Helper->InitializePipelineNewCurrentNode();
  this->Helper->UpdateGUIElements();
  this->Helper->UpdateRendering();
  //take care about references
}

void vtkVolumeRenderingGUI::InitializePipelineFromImageData()
{
  //First check if we already have a reference
  const char *id=this->NS_ImageData->GetSelected()->GetID();
  vtkMRMLVolumeRenderingNode *tmp = NULL;
    
  if (this->SelectionNode && this->SelectionNode->GetActiveVolumeRenderingID())
    {
    tmp=vtkMRMLVolumeRenderingNode::SafeDownCast(
      this->GetLogic()->GetMRMLScene()->GetNodeByID(this->SelectionNode->GetActiveVolumeRenderingID()) );
    }
  else 
    {
    //loop over existing Nodes in scene
    for( int i=0;i<this->GetLogic()->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLVolumeRenderingNode");i++)
      {
      tmp=vtkMRMLVolumeRenderingNode::SafeDownCast(this->GetLogic()->GetMRMLScene()->GetNthNodeByClass(i,"vtkMRMLVolumeRenderingNode"));
      if(tmp->HasReference(id))
        {
        break;
        }//if
      }//for
      
      tmp = NULL; //didn't find any suitable node
    }
    
  if (tmp)
    {
    //So everyting will be treated when InitializeFromMRMLScene
    this->PreviousNS_VolumeRenderingDataScene=tmp->GetID();
    vtkSetAndObserveMRMLNodeMacro(this->CurrentNode, tmp);
    this->SelectionNode->SetActiveVolumeRenderingID(this->CurrentNode->GetID());
    this->NS_VolumeRenderingDataScene->SetSelected(tmp);
    //We will call the initialize on our own.
    this->InitializePipelineFromMRMLScene();
    }
      
  //If not initialize a new auto generated Volume Rendering Node
  if(!tmp)
    {
    this->InitializePipelineNewCurrentNode();
    }
  //Ensure that none is not visible
  this->NS_VolumeRenderingDataScene->NoneEnabledOff();
  this->NS_VolumeRenderingDataScene->UpdateMenu();
  //Render it
  this->PipelineInitializedOn();
}

void vtkVolumeRenderingGUI::InitializePipelineFromImageDataFg()
{
  //First check if we already have a reference
  const char *id=this->NS_ImageDataFg->GetSelected()->GetID();
  vtkMRMLVolumeRenderingNode *tmp = NULL;
    
  if (this->SelectionNode && this->SelectionNode->GetActiveVolumeRenderingIDFg())
    {
    tmp=vtkMRMLVolumeRenderingNode::SafeDownCast(
      this->GetLogic()->GetMRMLScene()->GetNodeByID(this->SelectionNode->GetActiveVolumeRenderingIDFg()) );
    }
  else 
    {
    //loop over existing Nodes in scene
    for( int i=0;i<this->GetLogic()->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLVolumeRenderingNode");i++)
      {
      tmp=vtkMRMLVolumeRenderingNode::SafeDownCast(this->GetLogic()->GetMRMLScene()->GetNthNodeByClass(i,"vtkMRMLVolumeRenderingNode"));
      if(tmp->HasReference(id))
        {
        break;
        }//if
      }//for
      
      tmp = NULL;//didn't find any suitable node
    }
    
  if (tmp)
    {
    //So everyting will be treated when InitializeFromMRMLScene
    this->PreviousNS_VolumeRenderingDataSceneFg = tmp->GetID();
    vtkSetAndObserveMRMLNodeMacro(this->CurrentNodeFg, tmp);
    this->SelectionNode->SetActiveVolumeRenderingIDFg(this->CurrentNodeFg->GetID());
    this->NS_VolumeRenderingDataSceneFg->SetSelected(tmp);
    //We will call the initialize on our own.
    this->InitializePipelineFromMRMLScene();
    }
      
  //If not initialize a new auto generated Volume Rendering Node
  if(!tmp)
    {
    this->InitializePipelineNewCurrentNodeFg();
    }
  //Ensure that none is not visible
  this->NS_VolumeRenderingDataSceneFg->NoneEnabledOff();
  this->NS_VolumeRenderingDataSceneFg->UpdateMenu();
  //Render it
  this->PipelineInitializedFgOn();
}

