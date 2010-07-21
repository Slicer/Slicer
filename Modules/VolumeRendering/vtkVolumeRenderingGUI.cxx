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
#include "vtkMRMLROINode.h"
#include "vtkMRMLVolumePropertyNode.h"
#include "vtkMRMLViewNode.h"
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
#include "vtkKWCheckButtonWithLabel.h"

extern "C" int Volumerenderingreplacements_Init(Tcl_Interp *interp);

vtkVolumeRenderingGUI::vtkVolumeRenderingGUI(void)
{
  //In Debug Mode
  this->DebugOff();

  this->UpdatingGUI = 0;
  this->ProcessingGUIEvents = 0;
  this->ProcessingMRMLEvents = 0;

  this->NewVolumePropertyAddedFlag = 0;
  this->NewFgVolumePropertyAddedFlag = 0;
  this->NewParametersNodeForNewInputFlag = 0;
  this->NewParametersNodeFromSceneLoadingFlag = 0;
  
  this->ScenarioNode = NULL;

  this->Presets = NULL;

  this->CB_VolumeRenderingOnOff = NULL;
  this->NS_ParametersSet = NULL;

  this->NS_ImageData=NULL;
  this->NS_ImageDataFg=NULL;
  this->NS_ImageDataLabelmap=NULL;

  this->NS_VolumePropertyPresets=NULL;
  this->NS_VolumeProperty=NULL;

  this->NS_VolumePropertyPresetsFg=NULL;
  this->NS_VolumePropertyFg=NULL;

  this->NS_ROI = NULL;

  //Frame Details
  this->RenderingFrame=NULL;

  //Other members
  this->Presets=NULL;
  this->Helper=NULL;

  this->VolumeRenderingInteractionFlag = 0;
    
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
  this->ListenToTclMouseEvent(0);
   
  this->RemoveMRMLObservers();

  if(this->CB_VolumeRenderingOnOff != NULL)
  {
    this->CB_VolumeRenderingOnOff->GetWidget()->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent,(vtkCommand*)this->GUICallbackCommand);
    this->CB_VolumeRenderingOnOff->SetParent(NULL);
    this->CB_VolumeRenderingOnOff->Delete();
    this->CB_VolumeRenderingOnOff=NULL;
  }
  
  if (this->NS_ParametersSet)
  {
    this->NS_ParametersSet->SetParent(NULL);
    this->NS_ParametersSet->Delete();
    this->NS_ParametersSet=NULL;
  }
  if (this->NS_ROI)
  {
    this->NS_ROI->SetParent(NULL);
    this->NS_ROI->Delete();
    this->NS_ROI = NULL;
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
  if(this->NS_VolumeProperty)
  {
    this->NS_VolumeProperty->SetParent(NULL);
    this->NS_VolumeProperty->Delete();
    this->NS_VolumeProperty=NULL;
  }

  if(this->NS_VolumePropertyPresets)
  {
    this->NS_VolumePropertyPresets->SetParent(NULL);
    this->NS_VolumePropertyPresets->Delete();
    this->NS_VolumePropertyPresets=NULL;
  }

  if(this->NS_VolumePropertyFg)
  {
    this->NS_VolumePropertyFg->SetParent(NULL);
    this->NS_VolumePropertyFg->Delete();
    this->NS_VolumePropertyFg=NULL;
  }

  if(this->NS_VolumePropertyPresetsFg)
  {
    this->NS_VolumePropertyPresetsFg->SetParent(NULL);
    this->NS_VolumePropertyPresetsFg->Delete();
    this->NS_VolumePropertyPresetsFg=NULL;
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

void vtkVolumeRenderingGUI::ListenToTclMouseEvent(int OnOff)
{
  const char* tclName = this->GetTclName();
  
  if (OnOff)
  {
    //hookup mouse button interaction
    this->Script("bind all <Any-ButtonPress> {%s SetButtonDown 1}", tclName);
    this->Script("bind all <Any-ButtonRelease> {%s SetButtonDown 0}", tclName);
  }
  else
  {
    this->Script("bind all <Any-ButtonPress> {}", tclName);
    this->Script("bind all <Any-ButtonRelease> {}", tclName);
  }
}

void vtkVolumeRenderingGUI::BuildGUI(void)
{
  int labelWidth = 10;
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  this->GetUIPanel()->AddPage("VolumeRendering", "VolumeRendering", NULL);

  // Define your help text and build the help frame here.
  const char *help = "Volume Rendering allows the rendering of volumes in 3D space and not only as 2D surfaces defined in 3D space. \n<a>http://wiki.slicer.org/slicerWiki/index.php/Modules:VolumeRendering-Documentation-3.6</a>\n Tutorials are available at <a>http://www.slicer.org/slicerWiki/index.php/Slicer3:Volume_Rendering_Tutorials</a>";
  const char *about = "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. The VolumeRendering module was contributed by Yanling Liu, NCI-Frederick/SAIC-Frederick, Andreas Freudling, Student Intern at SPL, BWH (Ron Kikinis) and Alex Yarmarkovich, Isomics Inc. (Steve Pieper). Some of the transfer functions were contributed by Kitware, Inc. (VolView)";

  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "VolumeRendering" );
  this->BuildHelpAndAboutFrame ( page, help, about );

  //
  //Load and save
  //
  vtkSlicerModuleCollapsibleFrame *loadSaveDataFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  loadSaveDataFrame->SetParent (this->UIPanel->GetPageWidget("VolumeRendering"));
  loadSaveDataFrame->Create();
  loadSaveDataFrame->ExpandFrame();
  loadSaveDataFrame->SetLabelText("Inputs");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
      loadSaveDataFrame->GetWidgetName(), this->UIPanel->GetPageWidget("VolumeRendering")->GetWidgetName());

  //enable/disable cpu ray casting MIP rendering
  this->CB_VolumeRenderingOnOff = vtkKWCheckButtonWithLabel::New();
  this->CB_VolumeRenderingOnOff->SetParent(loadSaveDataFrame->GetFrame());
  this->CB_VolumeRenderingOnOff->Create();
  this->CB_VolumeRenderingOnOff->SetBalloonHelpString("If unchecked the VolumeRendering module will not monitor scene loading event.");
  this->CB_VolumeRenderingOnOff->SetLabelText("Monitor Scene Loading Event");
  this->CB_VolumeRenderingOnOff->SetLabelWidth(30);
  this->CB_VolumeRenderingOnOff->GetWidget()->SetSelectedState(1);
  this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", this->CB_VolumeRenderingOnOff->GetWidgetName() );
  this->CB_VolumeRenderingOnOff->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand*) this->GUICallbackCommand);
    
  this->NS_ParametersSet = vtkSlicerNodeSelectorWidget::New();
  this->NS_ParametersSet->SetNodeClass("vtkMRMLVolumeRenderingParametersNode", "", "", "");
  this->NS_ParametersSet->NoneEnabledOff();
  this->NS_ParametersSet->SetShowHidden(1);
  this->NS_ParametersSet->SetParent(loadSaveDataFrame->GetFrame());
  this->NS_ParametersSet->Create();
  this->NS_ParametersSet->SetMRMLScene(this->GetMRMLScene());
  this->NS_ParametersSet->UpdateMenu();

  this->NS_ParametersSet->SetBorderWidth(2);
  this->NS_ParametersSet->SetLabelText("Scenario:");
  this->NS_ParametersSet->SetLabelWidth(8);
  this->NS_ParametersSet->SetBalloonHelpString("select a volume rendering scenario.");

  app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_ParametersSet->GetWidgetName());

  this->NS_ROI = vtkSlicerNodeSelectorWidget::New();
  this->NS_ROI->SetNodeClass("vtkMRMLROINode", "", "", "");
  this->NS_ROI->SetNewNodeEnabled(1);
  this->NS_ROI->NoneEnabledOff();
  this->NS_ROI->SetShowHidden(1);
  this->NS_ROI->SetParent(loadSaveDataFrame->GetFrame());
  this->NS_ROI->Create();
  this->NS_ROI->SetMRMLScene(this->GetMRMLScene());
  this->NS_ROI->UpdateMenu();
  this->NS_ROI->SetBorderWidth(2);
  this->NS_ROI->SetLabelText( "ROI:");
  this->NS_ROI->SetLabelWidth(8);
  this->NS_ROI->SetBalloonHelpString("ROI node for current volume rendering scenario.");

  app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_ROI->GetWidgetName());

  {//background input volume
    vtkKWFrameWithLabel *inputVolumeFrame = vtkKWFrameWithLabel::New();
    inputVolumeFrame->SetParent(loadSaveDataFrame->GetFrame());
    inputVolumeFrame->Create();
    inputVolumeFrame->SetLabelText("Background Volume");
    app->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", inputVolumeFrame->GetWidgetName() );

    //NodeSelector for Node from MRML Scene
    this->NS_ImageData = vtkSlicerNodeSelectorWidget::New();
    this->NS_ImageData->SetParent(inputVolumeFrame->GetFrame());
    this->NS_ImageData->Create();
    this->NS_ImageData->NoneEnabledOn();
    this->NS_ImageData->SetLabelText("Source: ");
    this->NS_ImageData->SetBalloonHelpString("Background (default) volume input.");
    this->NS_ImageData->SetLabelWidth(labelWidth);
    this->NS_ImageData->SetNodeClass("vtkMRMLScalarVolumeNode","","","");
    this->NS_ImageData->SetChildClassesEnabled(0);
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_ImageData->GetWidgetName());

    //NodeSelector for VolumePropertyNode
    this->NS_VolumeProperty = vtkSlicerNodeSelectorWidget::New();
    this->NS_VolumeProperty->SetParent(inputVolumeFrame->GetFrame());
    this->NS_VolumeProperty->Create();
    this->NS_VolumeProperty->SetNewNodeEnabled(1);
    this->NS_VolumeProperty->NoneEnabledOff();
    this->NS_VolumeProperty->SetLabelText("Property:");
    this->NS_VolumeProperty->SetBalloonHelpString("Volume rendering property for current volume.");
    this->NS_VolumeProperty->SetLabelWidth(labelWidth);
    this->NS_VolumeProperty->EnabledOff();//By default off
    this->NS_VolumeProperty->SetShowHidden(1);
    this->NS_VolumeProperty->SetNodeClass("vtkMRMLVolumePropertyNode","","","VolumeProperty");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_VolumeProperty->GetWidgetName());

    //NodeSelector for VolumePropertyNode Preset
    this->NS_VolumePropertyPresets = vtkSlicerNodeSelectorWidget::New();
    this->NS_VolumePropertyPresets->SetParent(inputVolumeFrame->GetFrame());
    this->NS_VolumePropertyPresets->Create();
    this->NS_VolumePropertyPresets->SetLabelText("Presets:");
    this->NS_VolumePropertyPresets->SetBalloonHelpString("Use pre-defined opacity/color mapping functions.");
    this->NS_VolumePropertyPresets->SetLabelWidth(labelWidth);
    this->NS_VolumePropertyPresets->EnabledOff();//By default off
    this->NS_VolumePropertyPresets->NoneEnabledOn();
    this->NS_VolumePropertyPresets->SetShowHidden(1);
    this->NS_VolumePropertyPresets->SetNodeClass("vtkMRMLVolumePropertyNode","","","");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_VolumePropertyPresets->GetWidgetName());

    inputVolumeFrame->Delete();
  }

  {//foreground volume
    vtkKWFrameWithLabel *inputVolumeFrame = vtkKWFrameWithLabel::New();
    inputVolumeFrame->SetParent(loadSaveDataFrame->GetFrame());
    inputVolumeFrame->Create();
    inputVolumeFrame->SetLabelText("Foreground Volume");
    app->Script( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2", inputVolumeFrame->GetWidgetName() );

    //NodeSelector for Node from MRML Scene
    this->NS_ImageDataFg = vtkSlicerNodeSelectorWidget::New();
    this->NS_ImageDataFg->SetParent(inputVolumeFrame->GetFrame());
    this->NS_ImageDataFg->Create();
    this->NS_ImageDataFg->NoneEnabledOn();
    this->NS_ImageDataFg->EnabledOff();
    this->NS_ImageDataFg->SetLabelText("Source:");
    this->NS_ImageDataFg->SetBalloonHelpString("Foreground volume input.");
    this->NS_ImageDataFg->SetLabelWidth(labelWidth);
    this->NS_ImageDataFg->SetNodeClass("vtkMRMLScalarVolumeNode","","","");
    this->NS_ImageDataFg->SetChildClassesEnabled(0);
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_ImageDataFg->GetWidgetName());

    //NodeSelector for VolumePropertyNode
    this->NS_VolumePropertyFg = vtkSlicerNodeSelectorWidget::New();
    this->NS_VolumePropertyFg->SetParent(inputVolumeFrame->GetFrame());
    this->NS_VolumePropertyFg->Create();
    this->NS_VolumePropertyFg->SetNewNodeEnabled(1);
    this->NS_VolumePropertyFg->NoneEnabledOff();
    this->NS_VolumePropertyFg->SetLabelText("Property:");
    this->NS_VolumePropertyFg->SetBalloonHelpString("Volume rendering property for current volume.");
    this->NS_VolumePropertyFg->SetLabelWidth(labelWidth);
    this->NS_VolumePropertyFg->EnabledOff();//By default off
    this->NS_VolumePropertyFg->SetShowHidden(1);
    this->NS_VolumePropertyFg->SetNodeClass("vtkMRMLVolumePropertyNode","","","");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_VolumePropertyFg->GetWidgetName());

    //NodeSelector for VolumePropertyNode Preset
    this->NS_VolumePropertyPresetsFg = vtkSlicerNodeSelectorWidget::New();
    this->NS_VolumePropertyPresetsFg->SetParent(inputVolumeFrame->GetFrame());
    this->NS_VolumePropertyPresetsFg->Create();
    this->NS_VolumePropertyPresetsFg->SetLabelText("Presets:");
    this->NS_VolumePropertyPresetsFg->SetBalloonHelpString("Use pre-defined opacity/color mapping functions.");
    this->NS_VolumePropertyPresetsFg->SetLabelWidth(labelWidth);
    this->NS_VolumePropertyPresetsFg->EnabledOff();//By default off
    this->NS_VolumePropertyPresetsFg->NoneEnabledOn();
    this->NS_VolumePropertyPresetsFg->SetShowHidden(1);
    this->NS_VolumePropertyPresetsFg->SetNodeClass("vtkMRMLVolumePropertyNode","","","");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_VolumePropertyPresetsFg->GetWidgetName());

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
    this->NS_ImageDataLabelmap = vtkSlicerNodeSelectorWidget::New();
    this->NS_ImageDataLabelmap->SetParent(inputVolumeFrame->GetFrame());
    this->NS_ImageDataLabelmap->Create();
    this->NS_ImageDataLabelmap->NoneEnabledOn();
    this->NS_ImageDataLabelmap->EnabledOff();
    this->NS_ImageDataLabelmap->SetLabelText("Source:");
    this->NS_ImageDataLabelmap->SetBalloonHelpString("Labelmap volume input.");
    this->NS_ImageDataLabelmap->SetLabelWidth(labelWidth);
    this->NS_ImageDataLabelmap->SetNodeClass("vtkMRMLScalarVolumeNode","","","");
    this->NS_ImageDataLabelmap->SetChildClassesEnabled(0);
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_ImageDataLabelmap->GetWidgetName());

    inputVolumeFrame->EnabledOff();
    inputVolumeFrame->CollapseFrame();
    inputVolumeFrame->Delete();
  }

  loadSaveDataFrame->Delete();

  //Rendering (parameters) frame
  this->RenderingFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  this->RenderingFrame->SetParent (this->UIPanel->GetPageWidget("VolumeRendering"));
  this->RenderingFrame->Create();
  this->RenderingFrame->ExpandFrame();
  this->RenderingFrame->SetLabelText("Rendering");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
      this->RenderingFrame->GetWidgetName(), this->UIPanel->GetPageWidget("VolumeRendering")->GetWidgetName());

  //create a empty scenario node
  vtkMRMLVolumeRenderingScenarioNode* scenarioNode = this->GetLogic()->CreateScenarioNode();
  this->ScenarioNode = scenarioNode;

  this->LoadPresets();

  this->GetLogic()->SetGUICallbackCommand(this->GUICallbackCommand);

  this->AddMRMLObservers();

  this->Built = true;

  this->ListenToTclMouseEvent(1);
}

void vtkVolumeRenderingGUI::TearDownGUI(void)
{
  this->Exit();
  
  if ( this->Built )
  {
    this->RemoveGUIObservers();

    this->DeleteRenderingFrame();

    int numViewer = this->GetApplicationGUI()->GetNumberOfViewerWidgets();
  
    for (int i = 0; i < numViewer; i++)
    {
      vtkSlicerViewerWidget *slicer_viewer_widget = this->GetApplicationGUI()->GetNthViewerWidget(i);
      if (slicer_viewer_widget)
      {
        slicer_viewer_widget->GetMainViewer()->RemoveViewProp(this->GetLogic()->GetVolumeActor() );
      }
    }
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

void vtkVolumeRenderingGUI::AddMRMLObservers(void)
{
  //Remove the MRML observer
  if ( this->GetApplicationGUI() )
  {
    this->GetApplicationGUI()->GetMRMLScene()->AddObserver(vtkMRMLScene::SceneClosedEvent, this->MRMLCallbackCommand);
    this->GetApplicationGUI()->GetMRMLScene()->AddObserver(vtkMRMLScene::SceneAboutToBeImportedEvent, this->MRMLCallbackCommand);
    this->GetApplicationGUI()->GetMRMLScene()->AddObserver(vtkMRMLScene::SceneImportedEvent, this->MRMLCallbackCommand);
    this->GetApplicationGUI()->GetMRMLScene()->AddObserver(vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand);
    this->GetApplicationGUI()->GetMRMLScene()->AddObserver(vtkMRMLScene::NodeRemovedEvent, this->MRMLCallbackCommand);
  }
}

void vtkVolumeRenderingGUI::RemoveMRMLObservers(void)
{
  //Remove the MRML observer
  if ( this->GetApplicationGUI() )
  {
    this->GetApplicationGUI()->GetMRMLScene()->RemoveObservers(vtkMRMLScene::SceneClosedEvent, this->MRMLCallbackCommand);
    this->GetApplicationGUI()->GetMRMLScene()->RemoveObservers(vtkMRMLScene::SceneAboutToBeImportedEvent, this->MRMLCallbackCommand);
    this->GetApplicationGUI()->GetMRMLScene()->RemoveObservers(vtkMRMLScene::SceneImportedEvent, this->MRMLCallbackCommand);
    this->GetApplicationGUI()->GetMRMLScene()->RemoveObservers(vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand);
    this->GetApplicationGUI()->GetMRMLScene()->RemoveObservers(vtkMRMLScene::NodeRemovedEvent, this->MRMLCallbackCommand);
  }
  vtkMRMLVolumeRenderingParametersNode *vspNode = this->GetCurrentParametersNode();
  if (vspNode)
  {
    //remove existing observers
    vtkMRMLScalarVolumeNode *selectedImageData = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode());

    //remove observer to trigger update of transform
    selectedImageData->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand);
    selectedImageData->RemoveObservers(vtkMRMLScalarVolumeNode::ImageDataModifiedEvent, this->MRMLCallbackCommand);
    selectedImageData->RemoveObservers(vtkMRMLVolumeNode::DisplayModifiedEvent, this->MRMLCallbackCommand);
  }
}

void vtkVolumeRenderingGUI::AddGUIObservers(void)
{
  this->NS_ParametersSet->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->NS_ImageData->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NS_ImageDataFg->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  //  this->NS_ImageDataLabelmap->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->NS_ROI->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->NS_VolumeProperty->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NS_VolumeProperty->AddObserver(vtkSlicerNodeSelectorWidget::NewNodeEvent, (vtkCommand *)this->GUICallbackCommand );

  this->NS_VolumePropertyPresets->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->NS_VolumePropertyFg->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NS_VolumePropertyPresetsFg->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  int numViewer = this->GetApplicationGUI()->GetNumberOfViewerWidgets();
  
  for (int i = 0; i < numViewer; i++)
  {
    vtkSlicerViewerWidget *slicer_viewer_widget = this->GetApplicationGUI()->GetNthViewerWidget(i);
    if (slicer_viewer_widget)
    {
      slicer_viewer_widget->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::AbortCheckEvent, (vtkCommand*)this->GUICallbackCommand);
      slicer_viewer_widget->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::EndEvent, (vtkCommand*)this->GUICallbackCommand);
    }
  }
}

void vtkVolumeRenderingGUI::RemoveGUIObservers(void)
{
  this->NS_ParametersSet->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->NS_ImageData->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->NS_ImageDataFg->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
  //  this->NS_ImageDataLabelmap->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->NS_ROI->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->NS_VolumeProperty->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->NS_VolumeProperty->RemoveObservers(vtkSlicerNodeSelectorWidget::NewNodeEvent, (vtkCommand *)this->GUICallbackCommand);

  this->NS_VolumePropertyPresets->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->NS_VolumePropertyFg->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->NS_VolumePropertyPresetsFg->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);

  int numViewer = this->GetApplicationGUI()->GetNumberOfViewerWidgets();
  
  for (int i = 0; i < numViewer; i++)
  {
    vtkSlicerViewerWidget *slicer_viewer_widget = this->GetApplicationGUI()->GetNthViewerWidget(i);
    if (slicer_viewer_widget)
    {
      slicer_viewer_widget->GetMainViewer()->GetRenderWindow()->RemoveObservers(vtkCommand::AbortCheckEvent, (vtkCommand *)this->GUICallbackCommand);
      slicer_viewer_widget->GetMainViewer()->GetRenderWindow()->RemoveObservers(vtkCommand::EndEvent, (vtkCommand*)this->GUICallbackCommand);
    }
  }
}

void vtkVolumeRenderingGUI::RemoveLogicObservers(void)
{
}

void vtkVolumeRenderingGUI::CheckAbort(void)
{
  // return if nothing is selected to prevent 
  // aborting rendering in other modulde
  // must be a better way of doing this
  // since something may be selectes while other module is running
  if (this->NS_ImageData->GetSelected() == NULL &&
      this->NS_ImageDataFg->GetSelected() == NULL &&
      this->NS_ImageDataLabelmap->GetSelected() == NULL)
    {
    return;
    }
  int numViewer = this->GetApplicationGUI()->GetNumberOfViewerWidgets();
  
  for (int i = 0; i < numViewer; i++)
  {
    vtkSlicerViewerWidget *slicer_viewer_widget = this->GetApplicationGUI()->GetNthViewerWidget(i);
    if (slicer_viewer_widget)
    {
      int pending = slicer_viewer_widget->GetMainViewer()->GetRenderWindow()->GetEventPending();
      if(pending != 0)
      {
        slicer_viewer_widget->GetMainViewer()->GetRenderWindow()->SetAbortRender(1);
        break;
      }

      int pendingGUI = vtkKWTkUtilities::CheckForPendingInteractionEvents(slicer_viewer_widget->GetMainViewer()->GetRenderWindow());
      if(pendingGUI != 0)
      {
        slicer_viewer_widget->GetMainViewer()->GetRenderWindow()->SetAbortRender(1);
        break;
      }
    }
  }
}

void vtkVolumeRenderingGUI::DisplayMessageDialog(const char *message)
{
    vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
    dialog->SetParent (  this->GetApplicationGUI()->GetMainSlicerWindow() );
    dialog->SetStyleToMessage();
    dialog->SetText(message);
    dialog->Create ( );
    dialog->SetMasterWindow( this->GetApplicationGUI()->GetMainSlicerWindow() );
    dialog->ModalOn();
    dialog->Invoke();
    dialog->Delete();
}

void vtkVolumeRenderingGUI::ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData)
{
  if (this->ProcessingGUIEvents || this->ProcessingMRMLEvents)
    return;

  this->ProcessingGUIEvents = 1;
  
  //check abort
  if(event == vtkCommand::AbortCheckEvent)
  {
    this->CheckAbort();
  }
  else if(event == vtkCommand::VolumeMapperComputeGradientsProgressEvent)
  {
    float *progress = (float*)callData;
    char buf[32] = "Gradient...";

    //turn off external progress bar when complete
    //would it be better or more expensive to send ending event?
    if (*progress > 0.99f)
      this->GetApplicationGUI()->SetExternalProgress(buf, 0.0);
    else
      this->GetApplicationGUI()->SetExternalProgress(buf, *progress);
  }
  else if (event == vtkCommand::ProgressEvent)
  {
    float *progress = (float*)callData;
    char buf[32] = "Rendering...";

    if (*progress > 0.99f)
      this->GetApplicationGUI()->SetExternalProgress(buf, 0.0);
    else
      this->GetApplicationGUI()->SetExternalProgress(buf, *progress);
  }
  else if (event == vtkCommand::EndEvent)
  {
    char buf[32] = "Done.";
    this->GetApplicationGUI()->SetExternalProgress(buf, 0.0);
  }

  //Check Node Selectors
  vtkSlicerNodeSelectorWidget *callerObjectNS = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  //-----------------------------------parameters set-----------------------------------------------
  if(callerObjectNS == this->NS_ParametersSet && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    if (this->NS_ParametersSet->GetSelected() && this->ScenarioNode)
    {
      if (this->ScenarioNode->GetParametersNodeID() &&
          strcmp(this->NS_ParametersSet->GetSelected()->GetID(), this->ScenarioNode->GetParametersNodeID()) != 0)
      {
        this->ScenarioNode->SetParametersNodeID(this->NS_ParametersSet->GetSelected()->GetID());
        //init pipeline from the newly selected parameters node
        if (this->GetCurrentParametersNode())
        {
          this->InitializePipelineFromParametersNode();
        }
      }
    }
  }//----------------------------------ROI--------------------------------------------------------------
  else if(callerObjectNS == this->NS_ROI && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    if (this->NS_ROI->GetSelected())
    {
      this->UpdatePipelineByROI();
    }
  }//----------------------------------bg volume---------------------------------------------------------
  else if(callerObjectNS == this->NS_ImageData && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    if (this->NS_ImageData->GetSelected())
    {
      int dim[3];
      vtkMRMLVolumeNode *vnode = vtkMRMLVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected());
      vnode->GetImageData()->GetDimensions(dim);
      for (int ii=0; ii<3; ii++)
        {
        if (dim[ii] < 2) 
          {
          this->DisplayMessageDialog("Invalid volume dimension, at least 2 is required");
          this->NS_ImageData->SetSelected(NULL);
          return;
          }
        }
      //user selected one node, init pipeline based on it
      this->InitializePipelineFromImageData();
    }
  }//-----------------------------------fg volume----------------------------------------------------
  else if(callerObjectNS == this->NS_ImageDataFg && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    if (this->NS_ImageDataFg->GetSelected())
    {
      int dim[3];
      vtkMRMLVolumeNode *vnode = vtkMRMLVolumeNode::SafeDownCast(this->NS_ImageDataFg->GetSelected());
      vnode->GetImageData()->GetDimensions(dim);
      for (int ii=0; ii<3; ii++)
        {
        if (dim[ii] < 2) 
          {
          this->DisplayMessageDialog("Invalid volume dimension, at least 2 is required");
          this->NS_ImageDataFg->SetSelected(NULL);
          return;
          }
        }

      //user selected one node, init pipeline based on it
      this->InitializePipelineFromImageDataFg();
    }
  }//----------------------------------------bg volume property--------------------------------
  else if(callerObjectNS == this->NS_VolumeProperty && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    vtkMRMLVolumeRenderingParametersNode* vspNode = this->GetCurrentParametersNode();

    if(vspNode->GetVolumePropertyNodeID() != NULL &&
          strcmp(this->NS_VolumeProperty->GetSelected()->GetID(), vspNode->GetVolumePropertyNodeID()) == 0)
    {
      this->ProcessingGUIEvents = 0;
      return;//return if same node
    }

    vspNode->SetAndObserveVolumePropertyNodeID(this->NS_VolumeProperty->GetSelected()->GetID());

    if (this->NewVolumePropertyAddedFlag)
    {
      this->GetLogic()->SetupVolumePropertyFromImageData(vspNode);
      this->NewVolumePropertyAddedFlag = 0;
    }

    this->UpdatePipelineByVolumeProperty();
  }//-----------------------------------------new bg volume property--------------------------
  else if(callerObjectNS == this->NS_VolumeProperty && event == vtkSlicerNodeSelectorWidget::NewNodeEvent)
  {
    //remember new volume property node added here
    //at this moment the node selector has not been updated yet
    //new volume property node not in the node selector menu yet!!
    this->NewVolumePropertyAddedFlag = 1;

  }//----------------------------------------fg volume property---------------------------
  else if(callerObjectNS == this->NS_VolumePropertyFg && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    vtkMRMLVolumeRenderingParametersNode* vspNode = this->GetCurrentParametersNode();

    if(vspNode->GetFgVolumePropertyNodeID() != NULL &&
          strcmp(this->NS_VolumePropertyFg->GetSelected()->GetID(), vspNode->GetFgVolumePropertyNodeID()) == 0)
    {
      this->ProcessingGUIEvents = 0;
      return;//return if same node
    }

    vspNode->SetAndObserveFgVolumePropertyNodeID(this->NS_VolumePropertyFg->GetSelected()->GetID());

    if (this->NewFgVolumePropertyAddedFlag)
    {
      this->GetLogic()->SetupFgVolumePropertyFromImageData(vspNode);
      this->NewFgVolumePropertyAddedFlag = 0;
    }

    this->UpdatePipelineByFgVolumeProperty();
  }//-----------------------------------------new fg volume property--------------------------
  else if(callerObjectNS == this->NS_VolumeProperty && event == vtkSlicerNodeSelectorWidget::NewNodeEvent)
  {
    //remember new volume property node added here
    //at this moment the node selector has not been updated yet
    //new volume property node not in the node selector menu yet!!
    this->NewFgVolumePropertyAddedFlag = 1;

  }//---------------------------------------bg presets-----------------------------------
  else if(callerObjectNS == this->NS_VolumePropertyPresets && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    if (this->NS_VolumePropertyPresets->GetSelected() == NULL)
    {
      this->ProcessingGUIEvents = 0;
      return;
    }

    vtkMRMLVolumeRenderingParametersNode* vspNode = this->GetCurrentParametersNode();

    if(this->Presets->GetNodeByID(this->NS_VolumePropertyPresets->GetSelected()->GetID()) != NULL)
    {
      //Copy Preset Information in current Node
      vspNode->GetVolumePropertyNode()->CopyParameterSet(this->NS_VolumePropertyPresets->GetSelected());

      this->UpdatePipelineByVolumeProperty();
    }

    //this->NS_VolumePropertyPresets->SetSelected(NULL);
  }//-----------------------------------fg presets-------------------------------------
  else if(callerObjectNS == this->NS_VolumePropertyPresetsFg && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    if (this->NS_VolumePropertyPresetsFg->GetSelected() == NULL)
    {
      this->ProcessingGUIEvents = 0;
      return;
    }

    vtkMRMLVolumeRenderingParametersNode* vspNode = this->GetCurrentParametersNode();

    if(this->Presets->GetNodeByID(this->NS_VolumePropertyPresetsFg->GetSelected()->GetID()) != NULL)
    {
      //Copy Preset Information in current Node
      vspNode->GetFgVolumePropertyNode()->CopyParameterSet(this->NS_VolumePropertyPresetsFg->GetSelected());

      this->UpdatePipelineByFgVolumeProperty();
    }

    //this->NS_VolumePropertyPresetsFg->SetSelected(NULL);
  }
  //End Check NodeSelectors

  vtkKWCheckButton *callerObjectCheckButton = vtkKWCheckButton::SafeDownCast(caller);
  if (callerObjectCheckButton == this->CB_VolumeRenderingOnOff->GetWidget())
  {
    if (this->CB_VolumeRenderingOnOff->GetWidget()->GetSelectedState() && this->ScenarioNode != NULL)
      this->InitializePipelineFromParametersNode();
  }
  
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

    if (addedNode != NULL)
    {
      if (addedNode->IsA("vtkMRMLVolumeRenderingScenarioNode") )//first time empty node or loaded from scene file
      {
        //remember the newly added scenarioNode
        vtkMRMLVolumeRenderingScenarioNode *sNode = vtkMRMLVolumeRenderingScenarioNode::SafeDownCast(addedNode);
        this->ScenarioNode = sNode;
      }
      else if (addedNode->IsA("vtkMRMLVolumeRenderingParametersNode") )
      {
        //make sure we only process this for new parameters node from other module
        if (!this->NewParametersNodeForNewInputFlag && !this->NewParametersNodeFromSceneLoadingFlag)
        {
          vtkMRMLVolumeRenderingParametersNode *addedVspNode = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(addedNode);

          if (this->ScenarioNode == NULL)
            this->ScenarioNode = this->GetLogic()->CreateScenarioNode();
          
          this->ScenarioNode->SetParametersNodeID(addedVspNode->GetID());
          
          if (this->GetCurrentParametersNode() != NULL && this->ValidateParametersNode(this->GetCurrentParametersNode()) )
          {
            this->InitializePipelineFromParametersNode();
          }
        }
        else if (this->NewParametersNodeForNewInputFlag)
        {
          this->NewParametersNodeForNewInputFlag = 0;
        }        
      }
      else if (addedNode->IsA("vtkMRMLViewNode"))
      {
        vtkMRMLViewNode *viewNode = vtkMRMLViewNode::SafeDownCast(addedNode);
        viewNode->AddObserver(vtkMRMLViewNode::GraphicalResourcesCreatedEvent, (vtkCommand *) this->MRMLCallbackCommand);
      }
    }
  }
  else if (event == vtkMRMLScene::SceneAboutToBeImportedEvent)
  {
    this->NewParametersNodeFromSceneLoadingFlag = 1;
  }
  else if (event == vtkMRMLScene::SceneImportedEvent)
  {
    this->NewParametersNodeFromSceneLoadingFlag = 0;
    
    if (this->GetCurrentParametersNode() != NULL && this->CB_VolumeRenderingOnOff->GetWidget()->GetSelectedState())
    {
      this->InitializePipelineFromParametersNode();
    }
  }
  else if(event == vtkMRMLScalarVolumeNode::ImageDataModifiedEvent)
  {
    this->RequestRender();
  }
  else if (event == vtkMRMLScene::SceneClosedEvent)
  {
    this->DeleteRenderingFrame();

    int numViewer = this->GetApplicationGUI()->GetNumberOfViewerWidgets();
  
    for (int i = 0; i < numViewer; i++)
    {
      vtkSlicerViewerWidget *slicer_viewer_widget = this->GetApplicationGUI()->GetNthViewerWidget(i);
      if (slicer_viewer_widget)
      {
        slicer_viewer_widget->GetMainViewer()->RemoveViewProp(this->GetLogic()->GetVolumeActor() );
        slicer_viewer_widget->RequestRender();
      }
    }

    this->GetLogic()->Reset();

    this->ScenarioNode = NULL;

    this->UpdateGUI();
  }
  else if(event == vtkMRMLTransformableNode::TransformModifiedEvent)
  {
    vtkMRMLVolumeRenderingParametersNode* vspNode = this->GetCurrentParametersNode();
    this->GetLogic()->TransformModified(vspNode);

    this->RequestRender();
  }
  else if(event == vtkMRMLVolumeNode::DisplayModifiedEvent)
  {
    this->UpdatePipelineByDisplayNode();
  }
  else if(event == vtkCommand::ModifiedEvent && vtkMRMLROINode::SafeDownCast(caller))
  {
    vtkMRMLROINode *roiNode = vtkMRMLROINode::SafeDownCast(caller);
    vtkMRMLVolumeRenderingParametersNode* vspNode = this->GetCurrentParametersNode();

    if (roiNode == vspNode->GetROINode())
    {
      this->GetLogic()->SetROI(vspNode);

      this->RequestRender();
    }
  }
  else if (event == vtkMRMLViewNode::GraphicalResourcesCreatedEvent)
  {
    int numViewer = this->GetApplicationGUI()->GetNumberOfViewerWidgets();
    
    for (int i = 0; i < numViewer; i++)
    {
      vtkSlicerViewerWidget *slicer_viewer_widget = this->GetApplicationGUI()->GetNthViewerWidget(i);
    
      if (slicer_viewer_widget)
      {
        slicer_viewer_widget->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::AbortCheckEvent, (vtkCommand*)this->GUICallbackCommand);
        slicer_viewer_widget->GetMainViewer()->GetRenderWindow()->AddObserver(vtkCommand::EndEvent, (vtkCommand*)this->GUICallbackCommand);
        slicer_viewer_widget->GetMainViewer()->AddViewProp(this->GetLogic()->GetVolumeActor() );
        slicer_viewer_widget->RequestRender();
      }
    }
  }

  this->ProcessingMRMLEvents = 0;
}

void vtkVolumeRenderingGUI::Enter(void)
{
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
  this->ReleaseModuleEventBindings();
}

void vtkVolumeRenderingGUI::SetButtonDown(int isDown)
{
  if (this->Helper == NULL)
    return;
    
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->GetCurrentParametersNode();
  if (vspNode->GetPerformanceControl() != 0)
    return;
    
  int val = this->GetLogic()->SetupVolumeRenderingInteractive(this->GetCurrentParametersNode(), isDown);

  if (val == 0)
    return;

  if (isDown == 1)
    this->VolumeRenderingInteractionFlag = 1;
  else
  {
    if (this->VolumeRenderingInteractionFlag == 1)//avoid endless loop
    {
      this->RequestRender();
      this->VolumeRenderingInteractionFlag = 0;
    }
  }
}

void vtkVolumeRenderingGUI::UpdateGUI()
{
  //First of all check if we have a MRML Scene
  if (!this->Built || !this->GetLogic()->GetMRMLScene() || this->UpdatingGUI)
  {
    //if not return
    return;
  }

  this->UpdatingGUI = 1;

  //update menu first
  this->NS_ParametersSet->SetMRMLScene(this->GetLogic()->GetMRMLScene());
  this->NS_ParametersSet->UpdateMenu();

  this->NS_ROI->SetMRMLScene(this->GetLogic()->GetMRMLScene());
  this->NS_ROI->UpdateMenu();

  this->NS_ImageData->SetMRMLScene(this->GetLogic()->GetMRMLScene());
  this->NS_ImageData->UpdateMenu();

  this->NS_ImageDataFg->SetMRMLScene(this->GetLogic()->GetMRMLScene());
  this->NS_ImageDataFg->UpdateMenu();

  this->NS_ImageDataLabelmap->SetMRMLScene(this->GetLogic()->GetMRMLScene());
  this->NS_ImageDataLabelmap->UpdateMenu();

  //  volume rendering nodes (lighting/material, transfer functions)
  this->NS_VolumeProperty->SetMRMLScene(this->GetLogic()->GetMRMLScene());
  this->NS_VolumeProperty->UpdateMenu();

  this->NS_VolumePropertyFg->SetMRMLScene(this->GetLogic()->GetMRMLScene());
  this->NS_VolumePropertyFg->UpdateMenu();

  //  presets
  this->NS_VolumePropertyPresets->SetMRMLScene(this->Presets);
  this->NS_VolumePropertyPresets->UpdateMenu();

  this->NS_VolumePropertyPresetsFg->SetMRMLScene(this->Presets);
  this->NS_VolumePropertyPresetsFg->UpdateMenu();

  //then set menu selected node
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->GetCurrentParametersNode();

  if (vspNode)
  {
    this->NS_ParametersSet->SetSelected( vspNode );

    if (vspNode->GetVolumeNode())
      this->NS_ImageData->SetSelected( vspNode->GetVolumeNode() );
    if (vspNode->GetFgVolumeNode())
      this->NS_ImageDataFg->SetSelected( vspNode->GetFgVolumeNode() );

    if (vspNode->GetVolumePropertyNode())
      this->NS_VolumeProperty->SetSelected( vspNode->GetVolumePropertyNode()) ;
    if (vspNode->GetFgVolumePropertyNode())
      this->NS_VolumePropertyFg->SetSelected( vspNode->GetFgVolumePropertyNode()) ;

    if (vspNode->GetROINode())
      this->NS_ROI->SetSelected(vspNode->GetROINode());

    /*presets is a little bit complicated here*/

    //---------remove none from menu
    if (this->NS_ImageData->GetSelected())
    {
      this->NS_ImageData->NoneEnabledOff();
      this->NS_ImageData->UpdateMenu();
    }
  }
  else
  {
    this->NS_ImageData->NoneEnabledOn();
    this->NS_ImageData->UpdateMenu();
    this->NS_ImageData->SetSelected(NULL);

    this->NS_ImageDataFg->NoneEnabledOn();
    this->NS_ImageDataFg->UpdateMenu();
    this->NS_ImageDataFg->SetSelected(NULL);
  }

  //Disable/Enable after Volume is selected
  if(this->NS_ImageData->GetSelected() != NULL)
  {
    this->NS_VolumeProperty->EnabledOn();
    this->NS_VolumeProperty->NoneEnabledOff();
    this->NS_VolumePropertyPresets->EnabledOn();

    this->NS_ImageDataFg->EnabledOn();
  }
  else
  {
    this->NS_VolumeProperty->NoneEnabledOn();
    this->NS_VolumeProperty->SetSelected(NULL);
    this->NS_VolumeProperty->EnabledOff();
    this->NS_VolumePropertyPresets->EnabledOff();

    this->NS_ImageDataFg->EnabledOff();//if bg is not selected then turn off fg and labelmap
  }

  if(this->NS_ImageDataFg->GetSelected() != NULL)
  {
    this->NS_VolumePropertyFg->EnabledOn();
    this->NS_VolumePropertyFg->NoneEnabledOff();
    this->NS_VolumePropertyPresetsFg->EnabledOn();
  }
  else
  {
    this->NS_VolumePropertyFg->NoneEnabledOn();
    this->NS_VolumePropertyFg->SetSelected(NULL);
    this->NS_VolumePropertyFg->EnabledOff();
    this->NS_VolumePropertyPresetsFg->EnabledOff();
  }

  if (this->NS_ParametersSet->GetSelected() == NULL)
    this->NS_ROI->EnabledOff();
  else
    this->NS_ROI->EnabledOn();

  this->UpdatingGUI = 0;
}

void vtkVolumeRenderingGUI::SetViewerWidget(vtkSlicerViewerWidget *vtkNotUsed(viewerWidget))
{
}

void vtkVolumeRenderingGUI::SetInteractorStyle(vtkSlicerViewerInteractorStyle *vtkNotUsed(interactorStyle))
{
}

void vtkVolumeRenderingGUI::UpdatePipelineByROI()
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->GetCurrentParametersNode();

  if (vspNode == NULL)
    return;

  if (vspNode->GetROINodeID() != NULL &&
      strcmp(this->NS_ROI->GetSelected()->GetID(), vspNode->GetROINodeID()) == 0)
  {
      return;// return if the node already selected
  }

  //remove existing observers
  vtkMRMLROINode *roi = vtkMRMLROINode::SafeDownCast(vspNode->GetROINode());
  if (roi)
  {
  roi->RemoveObservers(vtkCommand::ModifiedEvent, this->MRMLCallbackCommand);
  }

  vspNode->SetAndObserveROINodeID(this->NS_ROI->GetSelected()->GetID());

  if (!vspNode->GetROINode())
    return;

  vspNode->GetROINode()->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *) this->MRMLCallbackCommand);
  vspNode->GetROINode()->InsideOutOn();

  if (this->Helper)
    this->Helper->UpdateROI();

  this->GetLogic()->SetROI(vspNode);

  vtkSlicerViewerWidget *slicer_viewer_widget = 
    this->GetApplicationGUI()->GetActiveViewerWidget();
  if (slicer_viewer_widget)
    {
    slicer_viewer_widget->RequestRender();
    }
}

void vtkVolumeRenderingGUI::UpdatePipelineByDisplayNode()
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->GetCurrentParametersNode();
  if (!vspNode->GetFollowVolumeDisplayNode())
    return;
    
  vtkSlicerViewerWidget *slicer_viewer_widget = this->GetApplicationGUI()->GetActiveViewerWidget();
  if (slicer_viewer_widget)
  {
    slicer_viewer_widget->GetMainViewer()->GetRenderWindowInteractor()->Disable();
  }
  
  this->GetLogic()->UpdateVolumePropertyByDisplayNode(vspNode);
  this->Helper->UpdateVolumeProperty();

  if (slicer_viewer_widget)
  {
    slicer_viewer_widget->GetMainViewer()->GetRenderWindowInteractor()->Enable();
    slicer_viewer_widget->RequestRender();
  }
}

void vtkVolumeRenderingGUI::UpdatePipelineByVolumeProperty()
{
  vtkSlicerViewerWidget *slicer_viewer_widget = this->GetApplicationGUI()->GetActiveViewerWidget();
  if (slicer_viewer_widget)
  {
    slicer_viewer_widget->GetMainViewer()->GetRenderWindowInteractor()->Disable();
  }

  vtkMRMLVolumeRenderingParametersNode* vspNode = this->GetCurrentParametersNode();
  this->GetLogic()->SetupHistograms(vspNode);
  this->GetLogic()->UpdateVolumePropertyScalarRange(vspNode);
  this->GetLogic()->SetupMapperFromParametersNode(vspNode);

  //update rendering frame
  // at this place this->Helper should not be NULL
  this->Helper->UpdateVolumeProperty();

  if (slicer_viewer_widget)
  {
    slicer_viewer_widget->GetMainViewer()->GetRenderWindowInteractor()->Enable();
    slicer_viewer_widget->RequestRender();
  }
}

void vtkVolumeRenderingGUI::UpdatePipelineByFgVolumeProperty()
{
  vtkSlicerViewerWidget *slicer_viewer_widget = 
    this->GetApplicationGUI()->GetActiveViewerWidget();
  if (slicer_viewer_widget)
    {
    slicer_viewer_widget->GetMainViewer()->GetRenderWindowInteractor()->Disable();
    }

  vtkMRMLVolumeRenderingParametersNode* vspNode = this->GetCurrentParametersNode();
  this->GetLogic()->SetupHistogramsFg(vspNode);
  this->GetLogic()->UpdateFgVolumePropertyScalarRange(vspNode);
  this->GetLogic()->CreateVolumePropertyGPURaycastII(vspNode);
  this->GetLogic()->SetupMapperFromParametersNode(vspNode);

  //update rendering frame
  // at this place this->Helper should not be NULL
  this->Helper->UpdateVolumePropertyFg();

  if (slicer_viewer_widget)
    {
    slicer_viewer_widget->GetMainViewer()->GetRenderWindowInteractor()->Enable();

    slicer_viewer_widget->RequestRender();
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
  rnode->SetModifiedSinceRead(1);
  rnode->SetSelectable(1);
  //Add Node to Scene
  this->GetLogic()->GetMRMLScene()->AddNode(rnode);
//  rnode->AddReference(this->NS_ImageDataFg->GetSelected()->GetID());
  rnode->Delete();

  vtkMRMLVolumeRenderingParametersNode* vspNode = this->GetCurrentParametersNode();

  vspNode->SetAndObserveFgVolumePropertyNodeID(rnode->GetID());

  std::stringstream autoname;
  autoname << "autoViz_";
  autoname << this->NS_ImageDataFg->GetSelected()->GetName();
  vspNode->GetFgVolumePropertyNode()->SetName(autoname.str().c_str());

  //Update the menu
  this->NS_VolumePropertyFg->SetSelected(rnode);
  this->NS_VolumePropertyFg->UpdateMenu();

}

void vtkVolumeRenderingGUI::InitializePipelineNewVolumeProperty()
{
  vtkMRMLVolumePropertyNode *rnode = vtkMRMLVolumePropertyNode::New();
  //rnode->HideFromEditorsOff();
  rnode->SetModifiedSinceRead(1);
  rnode->SetSelectable(1);

  //Add Node to Scene
  this->GetLogic()->GetMRMLScene()->AddNode(rnode);
//  rnode->AddReference(this->NS_ImageData->GetSelected()->GetID());
  rnode->Delete();

  vtkMRMLVolumeRenderingParametersNode* vspNode = this->GetCurrentParametersNode();

  vspNode->SetAndObserveVolumePropertyNodeID(rnode->GetID());

  std::stringstream autoname;
  autoname << "autoViz_";
  autoname << this->NS_ImageData->GetSelected()->GetName();
  vspNode->GetVolumePropertyNode()->SetName(autoname.str().c_str());

  //Update the menu
  this->NS_VolumeProperty->SetSelected(rnode);
  this->NS_VolumeProperty->UpdateMenu();
}

void vtkVolumeRenderingGUI::InitializePipelineFromImageData()
{
  //when scene closed this->ScenarioNode would be NULL
  if (this->ScenarioNode == NULL)
    this->ScenarioNode = this->GetLogic()->CreateScenarioNode();

  vtkMRMLVolumeRenderingParametersNode *vspNode = this->GetCurrentParametersNode();

  if (vspNode != NULL && vspNode->GetVolumeNodeID() != NULL &&
      strcmp(this->NS_ImageData->GetSelected()->GetID(), vspNode->GetVolumeNodeID()) == 0)
  {
    return;// return if the node already selected
  }

  char buf[32] = "Initializing...";
  this->GetApplicationGUI()->SetExternalProgress(buf, 0.1);

  if (vspNode)
  {
    //remove existing observers
    vtkMRMLScalarVolumeNode *selectedImageData = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode());

    //remove observer to trigger update of transform
    selectedImageData->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand);
    selectedImageData->RemoveObservers(vtkMRMLScalarVolumeNode::ImageDataModifiedEvent, this->MRMLCallbackCommand);
    selectedImageData->RemoveObservers(vtkMRMLVolumeNode::DisplayModifiedEvent, this->MRMLCallbackCommand);

    if (vspNode->GetROINode())
      vspNode->GetROINode()->RemoveObservers(vtkCommand::ModifiedEvent, (vtkCommand *) this->MRMLCallbackCommand);
  }

  this->GetApplicationGUI()->SetExternalProgress(buf, 0.2);

  vspNode = NULL;//reset vspNode for remaining process

  //first loop though MRML to see if we have a parameters node matching input volumes
  for( int i = 0; i < this->GetLogic()->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLVolumeRenderingParametersNode"); i++)
  {
    vtkMRMLVolumeRenderingParametersNode *tmp = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(
                  this->GetLogic()->GetMRMLScene()->GetNthNodeByClass(i,"vtkMRMLVolumeRenderingParametersNode"));

    bool match = this->NS_ImageData->GetSelected() && tmp->GetVolumeNodeID() &&
                  (strcmp(tmp->GetVolumeNodeID(), this->NS_ImageData->GetSelected()->GetID()) == 0);
    bool matchFg = this->NS_ImageDataFg->GetSelected() && tmp->GetFgVolumeNodeID() &&
                  (strcmp(tmp->GetFgVolumeNodeID(), this->NS_ImageDataFg->GetSelected()->GetID()) == 0);

    if ( this->NS_ImageDataFg->GetSelected() == NULL)
    {
      if (tmp->GetFgVolumeNodeID() == NULL)
        matchFg = true;
      else if (strcmp(tmp->GetFgVolumeNodeID(), "NULL") == 0)
        matchFg = true;
    }

    if (match && matchFg)
    {
      vspNode = tmp;
      break;
    }
  }

  this->GetApplicationGUI()->SetExternalProgress(buf, 0.3);

  if ( !vspNode )//no match
  {
    this->NewParametersNodeForNewInputFlag = 1;//distiguish from new parameters node created from other modules
    
    vspNode = this->GetLogic()->CreateParametersNode();
    
    vspNode->SetAndObserveVolumeNodeID(this->NS_ImageData->GetSelected()->GetID());

    vtkMRMLVolumePropertyNode *vpNode = vtkMRMLVolumePropertyNode::New();
    vpNode->SetModifiedSinceRead(1);
    this->GetLogic()->GetMRMLScene()->AddNode(vpNode);
    vpNode->Delete();
    vspNode->SetAndObserveVolumePropertyNodeID(vpNode->GetID());

    vtkMRMLROINode *roiNode = vtkMRMLROINode::New();
    this->GetLogic()->GetMRMLScene()->AddNode(roiNode);
    roiNode->InsideOutOn();
    roiNode->VisibilityOff();//by default not show new ROI node
    vspNode->SetAndObserveROINodeID(roiNode->GetID());
    roiNode->Delete();

    this->GetLogic()->FitROIToVolume(vspNode);
    this->GetLogic()->SetupVolumePropertyFromImageData(vspNode);

    double scalarRange[2];
    vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetVolumeNode())->GetImageData()->GetPointData()->GetScalars()->GetRange(scalarRange, 0);
    vspNode->SetDepthPeelingThreshold(scalarRange[0]);

    vspNode->SetThreshold(scalarRange);
  }

  this->GetApplicationGUI()->SetExternalProgress(buf, 0.4);

  this->ScenarioNode->SetParametersNodeID(vspNode->GetID());
  this->UpdateGUI();

  this->GetApplicationGUI()->SetExternalProgress(buf, 0.5);

  int numViewer = this->GetApplicationGUI()->GetNumberOfViewerWidgets();
  
  for (int i = 0; i < numViewer; i++)
  {
    vtkSlicerViewerWidget *slicer_viewer_widget = this->GetApplicationGUI()->GetNthViewerWidget(i);
    if (slicer_viewer_widget)
    {
      slicer_viewer_widget->GetMainViewer()->GetRenderWindowInteractor()->Disable();
    }
  }
  
  vtkMRMLScalarVolumeNode *selectedImageData = vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected());
  //Add observer to trigger update of transform
  selectedImageData->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent,(vtkCommand *) this->MRMLCallbackCommand);
  selectedImageData->AddObserver(vtkMRMLScalarVolumeNode::ImageDataModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  selectedImageData->AddObserver(vtkMRMLVolumeNode::DisplayModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand);
  
  if (vspNode->GetROINode())
  {
    vspNode->GetROINode()->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *) this->MRMLCallbackCommand);

    this->GetLogic()->SetROI(vspNode);
  }

  this->GetApplicationGUI()->SetExternalProgress(buf, 0.6);

  this->DeleteRenderingFrame();
  this->CreateRenderingFrame();

  this->GetApplicationGUI()->SetExternalProgress(buf, 0.9);

  for (int i = 0; i < numViewer; i++)
  {
    vtkSlicerViewerWidget *slicer_viewer_widget = this->GetApplicationGUI()->GetNthViewerWidget(i);
    
    if (slicer_viewer_widget)
    {
      slicer_viewer_widget->GetMainViewer()->AddViewProp(this->GetLogic()->GetVolumeActor() );
      slicer_viewer_widget->GetMainViewer()->GetRenderWindowInteractor()->Enable();
      slicer_viewer_widget->RequestRender();
    }
  }

  this->GetApplicationGUI()->SetExternalProgress(buf, 1.0);
}

/*
 * Fg volume is disabled till a bg volume has been selected
 *
 * */
void vtkVolumeRenderingGUI::InitializePipelineFromImageDataFg()
{
  vtkMRMLVolumeRenderingParametersNode *vspNode = this->GetCurrentParametersNode();

  if (vspNode != NULL && vspNode->GetFgVolumeNodeID() != NULL &&
      strcmp(this->NS_ImageDataFg->GetSelected()->GetID(), vspNode->GetFgVolumeNodeID()) == 0)
  {
    return;// return if the node already selected
  }

  char buf[32] = "Initializing...";
  this->GetApplicationGUI()->SetExternalProgress(buf, 0.1);

  if (vspNode)
  {
    //remove existing observers
    vtkMRMLScalarVolumeNode *selectedImageData = vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetFgVolumeNode());

    if (selectedImageData)
    {
      //remove observer to trigger update of transform
      selectedImageData->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent, this->MRMLCallbackCommand);
      selectedImageData->RemoveObservers(vtkMRMLScalarVolumeNode::ImageDataModifiedEvent, this->MRMLCallbackCommand);
    }
  }

  this->GetApplicationGUI()->SetExternalProgress(buf, 0.2);

  vtkMRMLVolumeRenderingParametersNode* matchedVspNode = NULL;

  //first loop though MRML to see if we have a parameters node matching input volumes
  for( int i = 0; i < this->GetLogic()->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLVolumeRenderingParametersNode"); i++)
  {
    vtkMRMLVolumeRenderingParametersNode *tmp = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(
                  this->GetLogic()->GetMRMLScene()->GetNthNodeByClass(i,"vtkMRMLVolumeRenderingParametersNode"));

    bool match = this->NS_ImageData->GetSelected() && tmp->GetVolumeNodeID() &&
                  (strcmp(tmp->GetVolumeNodeID(), this->NS_ImageData->GetSelected()->GetID()) == 0);
    bool matchFg = this->NS_ImageDataFg->GetSelected() && tmp->GetFgVolumeNodeID() &&
                  (strcmp(tmp->GetFgVolumeNodeID(), this->NS_ImageDataFg->GetSelected()->GetID()) == 0);

    if (match && matchFg)
    {
      matchedVspNode = tmp;
      break;
    }
  }

  this->GetApplicationGUI()->SetExternalProgress(buf, 0.3);

  if ( !matchedVspNode )//no match, fit in fg volume parameters into current vspNode
  {
    vspNode->SetAndObserveFgVolumeNodeID(this->NS_ImageDataFg->GetSelected()->GetID());

    vtkMRMLVolumePropertyNode *vpNode = vtkMRMLVolumePropertyNode::New();
    vpNode->SetModifiedSinceRead(1);
    this->GetLogic()->GetMRMLScene()->AddNode(vpNode);
    vpNode->Delete();
    vspNode->SetAndObserveFgVolumePropertyNodeID(vpNode->GetID());

    this->GetLogic()->SetupFgVolumePropertyFromImageData(vspNode);

    double scalarRange[2];
    vtkMRMLScalarVolumeNode::SafeDownCast(vspNode->GetFgVolumeNode())->GetImageData()->GetPointData()->GetScalars()->GetRange(scalarRange, 0);

    vspNode->SetThresholdFg(scalarRange);
  }
  else//there is a match, then use matched vsp node
  {
    this->ScenarioNode->SetParametersNodeID(matchedVspNode->GetID());
  }

  this->GetApplicationGUI()->SetExternalProgress(buf, 0.4);

  this->UpdateGUI();

  this->GetApplicationGUI()->SetExternalProgress(buf, 0.5);

  int numViewer = this->GetApplicationGUI()->GetNumberOfViewerWidgets();
  
  for (int i = 0; i < numViewer; i++)
  {
    vtkSlicerViewerWidget *slicer_viewer_widget = this->GetApplicationGUI()->GetNthViewerWidget(i);
    if (slicer_viewer_widget)
    {
      slicer_viewer_widget->GetMainViewer()->GetRenderWindowInteractor()->Disable();
    }
  }

  vtkMRMLScalarVolumeNode *selectedImageData = vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageDataFg->GetSelected());
  //Add observer to trigger update of transform
  selectedImageData->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent,(vtkCommand *) this->MRMLCallbackCommand);
  selectedImageData->AddObserver(vtkMRMLScalarVolumeNode::ImageDataModifiedEvent, (vtkCommand *)this->MRMLCallbackCommand );

  this->GetApplicationGUI()->SetExternalProgress(buf, 0.6);

  this->DeleteRenderingFrame();
  this->CreateRenderingFrame();

  this->GetApplicationGUI()->SetExternalProgress(buf, 0.9);

  for (int i = 0; i < numViewer; i++)
  {
    vtkSlicerViewerWidget *slicer_viewer_widget = this->GetApplicationGUI()->GetNthViewerWidget(i);
    
    if (slicer_viewer_widget)
    {
      slicer_viewer_widget->GetMainViewer()->AddViewProp(this->GetLogic()->GetVolumeActor() );
      slicer_viewer_widget->GetMainViewer()->GetRenderWindowInteractor()->Enable();
      slicer_viewer_widget->RequestRender();
    }
  }

  this->GetApplicationGUI()->SetExternalProgress(buf, 1.0);

}

int vtkVolumeRenderingGUI::ValidateParametersNode(vtkMRMLVolumeRenderingParametersNode* vspNode)
{
  if (vspNode == NULL)
    return 0;
    
  //check all inputs
  if (vspNode->GetVolumeNodeID() && strcmp(vspNode->GetVolumeNodeID(), "NULL") != 0 && vspNode->GetVolumeNode() == NULL)
    return 0;

  if (vspNode->GetFgVolumeNodeID() && strcmp(vspNode->GetFgVolumeNodeID(), "NULL") != 0 && vspNode->GetFgVolumeNode() == NULL)
    return 0;
    
  return 1;
}

//initialize pipeline from a loaded or user selected parameters node
void vtkVolumeRenderingGUI::InitializePipelineFromParametersNode()
{
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->GetCurrentParametersNode();

  if (!ValidateParametersNode(vspNode))
    return;

  char buf[32] = "Initializing...";
  this->GetApplicationGUI()->SetExternalProgress(buf, 0.1);

  //update input frame (all node selectors)
  this->UpdateGUI();

  this->GetApplicationGUI()->SetExternalProgress(buf, 0.2);

  //init mappers, transfer functions, and so on    
  int numViewer = this->GetApplicationGUI()->GetNumberOfViewerWidgets();
  
  for (int i = 0; i < numViewer; i++)
  {
    vtkSlicerViewerWidget *slicer_viewer_widget = this->GetApplicationGUI()->GetNthViewerWidget(i);
    if (slicer_viewer_widget)
    {
      slicer_viewer_widget->GetMainViewer()->GetRenderWindowInteractor()->Disable();
    }
  }

  this->GetLogic()->SetupHistograms(vspNode);
  if (vspNode->GetFgVolumeNode())
    this->GetLogic()->SetupHistogramsFg(vspNode);

  this->GetApplicationGUI()->SetExternalProgress(buf, 0.3);

  if (vspNode->GetROINode())
  {
    vspNode->GetROINode()->AddObserver(vtkCommand::ModifiedEvent, (vtkCommand *) this->MRMLCallbackCommand);
    vspNode->GetROINode()->InsideOutOn();

    this->GetLogic()->SetROI(vspNode);
  }

  this->GetApplicationGUI()->SetExternalProgress(buf, 0.4);

  //prepare rendering frame
  this->DeleteRenderingFrame();
  this->CreateRenderingFrame();

  this->GetApplicationGUI()->SetExternalProgress(buf, 0.9);

  vtkMRMLScalarVolumeNode *selectedImageData = vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected());
  //Add observer to trigger update of transform
  selectedImageData->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent,(vtkCommand *) this->MRMLCallbackCommand);
  selectedImageData->AddObserver(vtkMRMLScalarVolumeNode::ImageDataModifiedEvent, (vtkCommand *) this->MRMLCallbackCommand );

  selectedImageData = vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageDataFg->GetSelected());
  if (this->NS_ImageDataFg->GetSelected())
  {
    //Add observer to trigger update of transform
    selectedImageData->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent,(vtkCommand *) this->MRMLCallbackCommand);
    selectedImageData->AddObserver(vtkMRMLScalarVolumeNode::ImageDataModifiedEvent, (vtkCommand *) this->MRMLCallbackCommand );
  }

  for (int i = 0; i < numViewer; i++)
  {
    vtkSlicerViewerWidget *slicer_viewer_widget = this->GetApplicationGUI()->GetNthViewerWidget(i);
    
    if (slicer_viewer_widget)
    {
      slicer_viewer_widget->GetMainViewer()->AddViewProp(this->GetLogic()->GetVolumeActor() );
      slicer_viewer_widget->GetMainViewer()->GetRenderWindowInteractor()->Enable();
      slicer_viewer_widget->RequestRender();
    }
  }

  this->GetApplicationGUI()->SetExternalProgress(buf, 1.0);
}

vtkMRMLVolumeRenderingParametersNode* vtkVolumeRenderingGUI::GetCurrentParametersNode()
{
  if (this->ScenarioNode)
    return vtkMRMLVolumeRenderingParametersNode::SafeDownCast(
      this->GetLogic()->GetMRMLScene()->GetNodeByID(this->ScenarioNode->GetParametersNodeID()));
  else
    return NULL;
}

void vtkVolumeRenderingGUI::LoadPresets()
{
  vtkMRMLVolumePropertyNode *vrNode = vtkMRMLVolumePropertyNode::New();
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
}

vtkMRMLVolumePropertyNode* vtkVolumeRenderingGUI::GetVolumePropertyNode()
{
  vtkMRMLVolumePropertyNode *vpNode = NULL;
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->GetCurrentParametersNode();
  if (vspNode) 
  {
    vpNode = vspNode->GetVolumePropertyNode();
  }
  return vpNode;
}

vtkMRMLVolumePropertyNode* vtkVolumeRenderingGUI::GetFgVolumePropertyNode()
{
  vtkMRMLVolumePropertyNode *vpNode = NULL;
  vtkMRMLVolumeRenderingParametersNode* vspNode = this->GetCurrentParametersNode();
  if (vspNode) 
  {
    vpNode = vspNode->GetFgVolumePropertyNode();
  }
  return vpNode;
}

void vtkVolumeRenderingGUI::RequestRender()
{
  int numViewer = this->GetApplicationGUI()->GetNumberOfViewerWidgets();
  
  for (int i = 0; i < numViewer; i++)
  {
    vtkSlicerViewerWidget *slicer_viewer_widget = this->GetApplicationGUI()->GetNthViewerWidget(i);
    if (slicer_viewer_widget)
    {
      slicer_viewer_widget->RequestRender();
    }
  }
}

void vtkVolumeRenderingGUI::PauseRenderInteraction()
{
  vtkSlicerViewerWidget *slicer_viewer_widget = this->GetApplicationGUI()->GetActiveViewerWidget();
  if (slicer_viewer_widget)
  {
    slicer_viewer_widget->GetMainViewer()->GetRenderWindowInteractor()->Disable();
  }
}

void vtkVolumeRenderingGUI::ResumeRenderInteraction()
{
  vtkSlicerViewerWidget *slicer_viewer_widget = this->GetApplicationGUI()->GetActiveViewerWidget();
  if (slicer_viewer_widget)
  {
    slicer_viewer_widget->GetMainViewer()->GetRenderWindowInteractor()->Enable();
  }
}
