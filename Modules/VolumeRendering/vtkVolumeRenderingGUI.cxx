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

  this->NS_ImageData=NULL;
  this->NS_ImageDataFg=NULL;
  this->NS_ImageDataLabelmap=NULL;

  this->NS_VolumePropertyPresets=NULL;
  this->NS_VolumeProperty=NULL;

  this->NS_VolumePropertyPresetsFg=NULL;
  this->NS_VolumePropertyFg=NULL;

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
  int labelWidth = 12;
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
  loadSaveDataFrame->SetLabelText("Inputs");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
      loadSaveDataFrame->GetWidgetName(), this->UIPanel->GetPageWidget("VolumeRendering")->GetWidgetName());

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

    //NodeSelector for VolumePropertyNode
    this->NS_VolumeProperty = vtkSlicerNodeSelectorWidget::New();
    this->NS_VolumeProperty->SetParent(inputVolumeFrame->GetFrame());
    this->NS_VolumeProperty->Create();
    this->NS_VolumeProperty->SetNewNodeEnabled(1);
    this->NS_VolumeProperty->NoneEnabledOn();
    this->NS_VolumeProperty->SetLabelText("Property:");
    this->NS_VolumeProperty->SetBalloonHelpString("Select how the volume should be displayed. Multi-property per volume are possible");
    this->NS_VolumeProperty->SetLabelWidth(labelWidth);
    this->NS_VolumeProperty->EnabledOff();//By default off
    this->NS_VolumeProperty->SetShowHidden(1);
    this->NS_VolumeProperty->SetNodeClass("vtkMRMLVolumePropertyNode","","","");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_VolumeProperty->GetWidgetName());

    //NodeSelector for VolumePropertyNode Preset
    this->NS_VolumePropertyPresets = vtkSlicerNodeSelectorWidget::New();
    this->NS_VolumePropertyPresets->SetParent(inputVolumeFrame->GetFrame());
    this->NS_VolumePropertyPresets->Create();
    this->NS_VolumePropertyPresets->SetLabelText("Presets:");
    this->NS_VolumePropertyPresets->SetBalloonHelpString("Select one of the existing parameter sets or presets.");
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

    //NodeSelector for VolumePropertyNode
    this->NS_VolumePropertyFg=vtkSlicerNodeSelectorWidget::New();
    this->NS_VolumePropertyFg->SetParent(inputVolumeFrame->GetFrame());
    this->NS_VolumePropertyFg->Create();
    this->NS_VolumePropertyFg->SetNewNodeEnabled(1);
    this->NS_VolumePropertyFg->NoneEnabledOn();
    this->NS_VolumePropertyFg->SetLabelText("Property:");
    this->NS_VolumePropertyFg->SetBalloonHelpString("Select how the volume should be displayed. Several parameter sets per volume are possible");
    this->NS_VolumePropertyFg->SetLabelWidth(labelWidth);
    this->NS_VolumePropertyFg->EnabledOff();//By default off
    this->NS_VolumePropertyFg->SetShowHidden(1);
    this->NS_VolumePropertyFg->SetNodeClass("vtkMRMLVolumePropertyNode","","","");
    app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->NS_VolumePropertyFg->GetWidgetName());

    //NodeSelector for VolumePropertyNode Preset
    this->NS_VolumePropertyPresetsFg=vtkSlicerNodeSelectorWidget::New();
    this->NS_VolumePropertyPresetsFg->SetParent(inputVolumeFrame->GetFrame());
    this->NS_VolumePropertyPresetsFg->Create();
    this->NS_VolumePropertyPresetsFg->SetLabelText("Presets:");
    this->NS_VolumePropertyPresetsFg->SetBalloonHelpString("Select one of the existing parameter sets or presets.");
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

  this->NS_VolumeProperty->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NS_VolumePropertyPresets->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->NS_VolumePropertyFg->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NS_VolumePropertyPresetsFg->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
}

void vtkVolumeRenderingGUI::RemoveGUIObservers(void)
{
  this->NS_ImageData->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->NS_ImageDataFg->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
  //  this->NS_ImageDataLabelmap->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->NS_VolumeProperty->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->NS_VolumePropertyPresets->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->NS_VolumePropertyFg->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->NS_VolumePropertyPresetsFg->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
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
  }//----------------------------------------bg volume property--------------------------------
  else if(callerObjectNS == this->NS_VolumeProperty && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    //Check for None selected //Just to be safe
    if(this->NS_VolumeProperty->GetSelected() == NULL)
    {
      this->ParametersNode->SetAndObserveVolumePropertyNodeID(NULL);
    }
    else//Only proceed event,if new Node
    {
       if(this->ParametersNode->GetVolumePropertyNodeID() != NULL &&
          strcmp(this->NS_VolumeProperty->GetSelected()->GetID(), this->ParametersNode->GetVolumePropertyNodeID()) == 0)
      {
      return;//return if same node
      }

      this->ParametersNode->SetAndObserveVolumePropertyNodeID(this->NS_VolumeProperty->GetSelected()->GetID());
      this->InitializePipelineFromMRMLScene();
    }
  }
  else if(callerObjectNS == this->NS_VolumePropertyFg && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    //Check for None selected //Just to be safe
    if(this->NS_VolumePropertyFg->GetSelected() == NULL)
    {
      this->ParametersNode->SetAndObserveFgVolumePropertyNodeID(NULL);
    }
    else
    {
      if(this->ParametersNode->GetFgVolumePropertyNodeID() != NULL &&
          strcmp(this->NS_VolumePropertyFg->GetSelected()->GetID(), this->ParametersNode->GetFgVolumePropertyNode()->GetID()) == 0)
      {
        return;//return if same node
      }

      this->ParametersNode->SetAndObserveFgVolumePropertyNodeID(this->NS_VolumePropertyFg->GetSelected()->GetID());
      this->InitializePipelineFromMRMLScene();
    }
  }
  else if(callerObjectNS == this->NS_VolumePropertyPresets && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    //Check for None selected
    if(this->NS_VolumePropertyPresets->GetSelected() == NULL)
    {
      this->ParametersNode->SetPresetsNodeID(NULL);
    }
    else//Only proceed event,if other Node
    {
      if(this->ParametersNode->GetPresetsNodeID() != NULL &&
        strcmp(this->NS_VolumePropertyPresets->GetSelected()->GetID(), this->ParametersNode->GetPresetsNodeID()) ==0 )
      {
        return;
      }

      if(this->Presets->GetNodeByID(this->NS_VolumePropertyPresets->GetSelected()->GetID()) != NULL)
      {
        //Copy Preset Information in current Node
        this->ParametersNode->GetVolumePropertyNode()->CopyParameterSet(this->NS_VolumePropertyPresets->GetSelected());
        this->Helper->UpdateGUIElements();
        this->ParametersNode->SetPresetsNodeID(this->NS_VolumePropertyPresets->GetSelected()->GetID());
      }
    }
  }
  else if(callerObjectNS == this->NS_VolumePropertyPresetsFg && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
  {
    //Check for None selected
    if(this->NS_VolumePropertyPresetsFg->GetSelected() == NULL)
    {
      this->ParametersNode->SetFgPresetsNodeID(NULL);
    }
    else//Only proceed event,if other Node
    {
      if(this->ParametersNode->GetFgPresetsNodeID() != NULL &&
        strcmp(this->NS_VolumePropertyPresetsFg->GetSelected()->GetID(), this->ParametersNode->GetFgPresetsNodeID()) ==0 )
      {
        return;
      }

      if(this->Presets->GetNodeByID(this->NS_VolumePropertyPresetsFg->GetSelected()->GetID()) != NULL)
      {
        //Copy Preset Information in current Node
        this->ParametersNode->GetFgVolumePropertyNode()->CopyParameterSet(this->NS_VolumePropertyPresetsFg->GetSelected());
        this->Helper->UpdateGUIElements();
        this->ParametersNode->SetFgPresetsNodeID(this->NS_VolumePropertyPresetsFg->GetSelected()->GetID());
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

    if (addedNode != NULL)
    {
      if (addedNode->IsA("vtkMRMLVolumeRenderingParametersNode") )
      {
        vtkSetAndObserveMRMLNodeMacro(this->ParametersNode, this->GetLogic()->GetParametersNode());

        //when a new parameters node created, GetVolumeNodeID returns NULL
        if (this->ParametersNode->GetVolumeNodeID() != NULL)
        {
          this->UpdateGUI();

          DeleteRenderingFrame();
          CreateRenderingFrame();

          this->InitializePipelineFromMRMLScene();
        }
      }
      else if (addedNode->IsA("vtkMRMLVolumeNode") )
        this->UpdateGUI();
      else if (addedNode->IsA("vtkMRMLVolumePropertyNode") )//user selected "create new node" from the menu
      {
        if (vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->NS_VolumeProperty)
        {
          this->ParametersNode->SetAndObserveVolumePropertyNodeID(vtkMRMLVolumePropertyNode::SafeDownCast(addedNode)->GetID());

          this->Helper->InitializePipelineNewVolumeProperty();
          this->Helper->UpdateRendering();
        }
        else if (vtkSlicerNodeSelectorWidget::SafeDownCast(caller) == this->NS_VolumePropertyFg)
        {
          this->ParametersNode->SetAndObserveFgVolumePropertyNodeID(vtkMRMLVolumePropertyNode::SafeDownCast(addedNode)->GetID());

          this->Helper->InitializePipelineNewVolumePropertyFg();
          this->Helper->UpdateRendering();
        }
      }
    }
  }
  else if (vtkMRMLVolumeRenderingParametersNode::SafeDownCast(caller)&&
      this->ParametersNode == vtkMRMLVolumeRenderingParametersNode::SafeDownCast(caller) &&
      event == vtkCommand::ModifiedEvent && this->MRMLScene)
  {
    this->UpdateGUI();
  }
  else if (vtkMRMLVolumePropertyNode::SafeDownCast(caller) && event == vtkCommand::ModifiedEvent && this->MRMLScene)
  {vtkErrorMacro("volume property node modified");
    if (this->ParametersNode->GetVolumePropertyNode() == vtkMRMLVolumePropertyNode::SafeDownCast(caller) ||
        this->ParametersNode->GetFgVolumePropertyNode() == vtkMRMLVolumePropertyNode::SafeDownCast(caller) )
      this->UpdateGUI();
  }
  else if(event == vtkMRMLScalarVolumeNode::ImageDataModifiedEvent)
  {vtkErrorMacro("image data modified");
    this->GetApplicationGUI()->GetViewerWidget()->RequestRender();
  }
  else if (event == vtkMRMLScene::SceneCloseEvent)
  {
    if(this->Helper != NULL)
    {
      this->Helper->Delete();
      this->Helper = NULL;
    }
    this->ParametersNode->Reset();
    this->UpdateGUI();vtkErrorMacro("scene close");
  }
  else if(event == vtkMRMLTransformableNode::TransformModifiedEvent)
  {vtkErrorMacro("transform");
    if (this->Helper != NULL)
    {
      this->Helper->UpdateRendering();
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
    this->NS_VolumePropertyPresets->SetMRMLScene(this->Presets);
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
  if(this->NS_VolumeProperty->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
  {
    this->NS_VolumeProperty->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_VolumeProperty->UpdateMenu();
  }

  if(this->NS_VolumePropertyFg->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
  {
    this->NS_VolumePropertyFg->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_VolumePropertyFg->UpdateMenu();
  }

  //  presets
  if(this->NS_VolumePropertyPresets->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
  {
    this->NS_VolumePropertyPresets->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_VolumePropertyPresets->UpdateMenu();
  }

  if(this->NS_VolumePropertyPresetsFg->GetMRMLScene()!=this->GetLogic()->GetMRMLScene())
  {
    this->NS_VolumePropertyPresetsFg->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_VolumePropertyPresetsFg->UpdateMenu();
  }

  //then set menu selected node
  if (this->ParametersNode)
  {
    this->NS_ImageData->SetSelected( this->ParametersNode->GetVolumeNode() );
    this->NS_ImageDataFg->SetSelected( this->ParametersNode->GetFgVolumeNode() );

    this->NS_VolumeProperty->SetSelected( this->ParametersNode->GetVolumePropertyNode()) ;
    this->NS_VolumePropertyFg->SetSelected( this->ParametersNode->GetFgVolumePropertyNode()) ;

    /*presets is a little bit complicated here*/
  }

  //Disable/Enable after Volume is selected
  if(this->NS_ImageData->GetSelected()!=NULL)
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

  if(this->NS_ImageDataFg->GetSelected()!=NULL)
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
  this->NS_VolumePropertyFg->SetSelected(rnode);
  this->NS_VolumePropertyFg->UpdateMenu();

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
  this->NS_VolumeProperty->SetSelected(rnode);
  this->NS_VolumeProperty->UpdateMenu();

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
    this->NS_VolumeProperty->SetSelected(tmp);

    this->InitializePipelineFromMRMLScene();
  }
  else
  {
    this->InitializePipelineNewVolumeProperty();
  }

  //Ensure that none is not visible
  this->NS_VolumeProperty->NoneEnabledOff();
  this->NS_VolumeProperty->UpdateMenu();

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
    this->NS_VolumePropertyFg->SetSelected(tmp);

    this->InitializePipelineFromMRMLScene();
  }
  else
  {
    this->InitializePipelineNewVolumePropertyFg();
  }

  //Ensure that none is not visible
  this->NS_VolumePropertyFg->NoneEnabledOff();
  this->NS_VolumePropertyFg->UpdateMenu();

  this->PipelineInitializedFgOn();
}
