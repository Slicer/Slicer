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
#include "vtkSlicerVolumePropertyWidget.h"
#include "vtkSlicerVolumeTextureMapper3D.h"
#include "vtkTexture.h"
#include "vtkTimerLog.h"
#include "vtkVolume.h"

extern "C" int Volumerenderingreplacements1_Init(Tcl_Interp *interp);

vtkVolumeRenderingGUI::vtkVolumeRenderingGUI(void)
{
    
  //In Debug Mode
  this->DebugOff();
    
  this->UpdatingGUI = 0;
  this->ProcessingGUIEvents = 0;
  this->ProcessingMRMLEvents = 0;

  this->Logic = NULL;
  this->ParametersNode = NULL;
  this->ViewerWidget = NULL;
  this->InteractorStyle = NULL;

  //Frame Details
  this->DetailsFrame=NULL;
  this->HideSurfaceModelsButton = NULL;
  this->VolumeNodeSelector = NULL;
  this->VolumeRenderingParameterSelector = NULL;

   // :NOTE: 20080515 tgl: To use as a loadable module, initialize
  // the volume rendering replacements TCL wrappers.
  Tcl_Interp *interp = NULL;
  interp = vtkKWApplication::GetMainInterp();
  if (NULL != interp)
    {
    Volumerenderingreplacements1_Init(interp);
    }

}

vtkVolumeRenderingGUI::~vtkVolumeRenderingGUI(void)
{

  vtkSetAndObserveMRMLNodeMacro(this->ParametersNode, NULL);

  this->RemoveMRMLObservers();

  if(this->HideSurfaceModelsButton)
    {
    this->HideSurfaceModelsButton->SetParent(NULL);
    this->HideSurfaceModelsButton->Delete();
    this->HideSurfaceModelsButton=NULL;
    }

  if (this->VolumeNodeSelector)
    {
    this->VolumeNodeSelector->SetParent(NULL);
    this->VolumeNodeSelector->Delete();
    this->VolumeNodeSelector=NULL;
    }
  if (this->VolumeRenderingParameterSelector)
    {
    this->VolumeRenderingParameterSelector->SetParent(NULL);
    this->VolumeRenderingParameterSelector->Delete();
    this->VolumeRenderingParameterSelector=NULL;
    }


  if(this->DetailsFrame)
    {
    this->DetailsFrame->Delete();
    this->DetailsFrame=NULL;
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
  if (this->GetApplicationGUI() && 
      this->GetApplicationGUI()->GetViewerWidget() && 
      this->GetLogic())
    {
    this->GetApplicationGUI()->GetViewerWidget()->GetMainViewer()->AddViewProp(this->GetLogic()->GetVolume());
    }

  this->AddMRMLObservers();


  int labelWidth=20;
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  this->GetUIPanel()->AddPage("VolumeRendering","VolumeRendering",NULL);

  // Define your help text and build the help frame here.
  const char *help = "Volume Rendering allows the rendering of volumes in 3D space and not only as 2D surfaces defined in 3D space. \n<a>http://wiki.slicer.org/slicerWiki/index.php/Modules:VolumeRendering-Documentation-3.4</a>\n Tutorials are available at <a>http://www.na-mic.org/Wiki/index.php/Slicer3:Volume_Rendering_Tutorials</a>";
  const char *about = "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. The Volumes module was contributed by Andreas Freudling, Student Intern at SPL, BWH (Ron Kikinis) and Alex Yarmarkovich, Isomics Inc. (Steve Pieper)";
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "VolumeRendering" );
  this->BuildHelpAndAboutFrame ( page, help, about );
  //
  //Load and save
  //
  vtkSlicerModuleCollapsibleFrame *loadSaveDataFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  loadSaveDataFrame->SetParent (this->UIPanel->GetPageWidget("VolumeRendering"));
  loadSaveDataFrame->Create();
  loadSaveDataFrame->ExpandFrame();
  loadSaveDataFrame->SetLabelText("Load and Save");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                loadSaveDataFrame->GetWidgetName(), this->UIPanel->GetPageWidget("VolumeRendering")->GetWidgetName());

  //Hide surface models pushbutton
  this->HideSurfaceModelsButton= vtkKWPushButton::New();
  this->HideSurfaceModelsButton->SetParent(loadSaveDataFrame->GetFrame());
  this->HideSurfaceModelsButton->Create();
  this->HideSurfaceModelsButton->SetText("Hide Surface Models");
  this->HideSurfaceModelsButton->SetBalloonHelpString("Make all surface models invisible. Go to models module to enable, disable only some of them.");
  this->HideSurfaceModelsButton->SetWidth(labelWidth);
  app->Script("pack %s -side top -anchor ne -padx 2 -pady 2",this->HideSurfaceModelsButton->GetWidgetName());

  //VolumeRenderingParameterSelector for Node from MRML Scene
  this->VolumeRenderingParameterSelector=vtkSlicerNodeSelectorWidget::New();
  this->VolumeRenderingParameterSelector->SetNodeClass("vtkMRMLVolumeRenderingParametersNode", NULL, NULL, "Parameters");
  this->VolumeRenderingParameterSelector->SetNewNodeEnabled(1);
  this->VolumeRenderingParameterSelector->NoneEnabledOff();
  this->VolumeRenderingParameterSelector->SetShowHidden(1);
  this->VolumeRenderingParameterSelector->SetParent(loadSaveDataFrame->GetFrame());
  this->VolumeRenderingParameterSelector->Create();
  this->VolumeRenderingParameterSelector->SetMRMLScene(this->GetMRMLScene());
  this->VolumeRenderingParameterSelector->UpdateMenu();

  this->VolumeRenderingParameterSelector->SetBorderWidth(2);
  this->VolumeRenderingParameterSelector->SetLabelText( "Parameters");
  this->VolumeRenderingParameterSelector->SetBalloonHelpString("select a parameter node from the current mrml scene.");

  app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->VolumeRenderingParameterSelector->GetWidgetName());

    //NodeSelector for Node from MRML Scene
  this->VolumeNodeSelector=vtkSlicerNodeSelectorWidget::New();
  this->VolumeNodeSelector->SetNodeClass("vtkMRMLScalarVolumeNode",NULL,NULL,NULL);
  this->VolumeNodeSelector->SetParent(loadSaveDataFrame->GetFrame());
  this->VolumeNodeSelector->NoneEnabledOn();
  this->VolumeNodeSelector->Create();
  this->VolumeNodeSelector->SetMRMLScene(this->GetApplicationLogic()->GetMRMLScene());
  this->VolumeNodeSelector->UpdateMenu();

  this->VolumeNodeSelector->SetLabelText("Source Volume: ");
  this->VolumeNodeSelector->SetBalloonHelpString("Select volume to render. Only one volume at the some time is possible.");
  this->VolumeNodeSelector->SetLabelWidth(labelWidth);
  app->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->VolumeNodeSelector->GetWidgetName());


  //Details frame
  this->DetailsFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  this->DetailsFrame->SetParent (this->UIPanel->GetPageWidget("VolumeRendering"));
  this->DetailsFrame->Create();
  this->DetailsFrame->ExpandFrame();
  this->DetailsFrame->SetLabelText("Details");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->DetailsFrame->GetWidgetName(), this->UIPanel->GetPageWidget("VolumeRendering")->GetWidgetName());


  //set subnodes
  //Delete frames
  if ( this->GetApplicationGUI() &&  this->GetApplicationGUI()->GetMRMLScene())
    {
    this->GetApplicationGUI()->GetMRMLScene()->AddObserver( vtkMRMLScene::SceneCloseEvent, this->MRMLCallbackCommand );
    this->MRMLScene->AddObserver(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    this->MRMLScene->AddObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
    
  //vtkMRMLVolumeRenderingParametersNode* parametersNode = this->GetLogic()->GetParametersNode();
  //vtkSetAndObserveMRMLNodeMacro(this->ParametersNode, parametersNode);

  
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

  this->VolumeNodeSelector->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->VolumeRenderingParameterSelector->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HideSurfaceModelsButton->AddObserver(vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallbackCommand );

}
void vtkVolumeRenderingGUI::RemoveGUIObservers(void)
{
  this->VolumeNodeSelector->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->VolumeRenderingParameterSelector->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HideSurfaceModelsButton->RemoveObservers (vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallbackCommand);
}
void vtkVolumeRenderingGUI::AddMRMLObservers(void)
{
  //Remove the MRML observer
  if ( this->GetApplicationGUI() )
    {
    this->GetApplicationGUI()->GetMRMLScene()->AddObserver(vtkMRMLScene::SceneCloseEvent, this->MRMLCallbackCommand);
    this->GetApplicationGUI()->GetMRMLScene()->AddObserver(vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand);
    this->GetApplicationGUI()->GetMRMLScene()->AddObserver(vtkMRMLScene::NodeRemovedEvent, this->MRMLCallbackCommand);
    }

}
void vtkVolumeRenderingGUI::RemoveMRMLObservers(void)
{
  //Remove the MRML observer
  if ( this->GetApplicationGUI() )
    {
    this->GetApplicationGUI()->GetMRMLScene()->RemoveObservers(vtkMRMLScene::SceneCloseEvent, this->MRMLCallbackCommand);
    this->GetApplicationGUI()->GetMRMLScene()->RemoveObservers(vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand);
    this->GetApplicationGUI()->GetMRMLScene()->RemoveObservers(vtkMRMLScene::NodeRemovedEvent, this->MRMLCallbackCommand);
    }

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
  if(callerObject==this->HideSurfaceModelsButton&&event==vtkKWPushButton::InvokedEvent)
    {
    int count=this->GetLogic()->GetMRMLScene()->GetNumberOfNodesByClass("vtkMRMLModelNode");
    for(int i=0;i<count;i++)
      {
      (vtkMRMLModelNode::SafeDownCast(this->GetLogic()->GetMRMLScene()->GetNthNodeByClass(i,"vtkMRMLModelNode")))->GetModelDisplayNode()->VisibilityOff();
      }
    this->ProcessingGUIEvents = 0;
    return;
    }


  // check paramaters node first

  if (this->GetParametersNode() == NULL)
    {
    this->CreateParametersNode();
    }

  //
  //Check Node Selectors
  //
  vtkSlicerNodeSelectorWidget *callerObjectNS=vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

  if(callerObjectNS == this->VolumeNodeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
    {
    vtkMRMLScalarVolumeNode *volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(this->VolumeNodeSelector->GetSelected());
    if (this->ParametersNode)
      {
      vtkMRMLScalarVolumeNode *volumeNodePrev = this->ParametersNode->GetVolumeNode();
      if (volumeNodePrev)
        {
        volumeNodePrev->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent,(vtkCommand *) this->MRMLCallbackCommand);
        }
      if (volumeNode)
        {
        this->ParametersNode->SetAndObserveVolumeNodeID(volumeNode->GetID());
        }
      else
        {
        this->ParametersNode->SetAndObserveVolumeNodeID(NULL);
        }
      }
    if (volumeNode)
      {
      volumeNode->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent,(vtkCommand *) this->MRMLCallbackCommand);
      this->Logic->UpdateTransform(volumeNode);
      }
    this->Logic->SetParametersNode(this->ParametersNode);
    this->GetApplicationGUI()->GetViewerWidget()->RequestRender();
   
    }

  if(callerObjectNS == this->VolumeRenderingParameterSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
    {
    vtkMRMLVolumeRenderingParametersNode *paramNode = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(this->VolumeRenderingParameterSelector->GetSelected());
    vtkSetAndObserveMRMLNodeMacro(this->ParametersNode, paramNode);
    this->UpdateParametersNode();
    this->Logic->SetParametersNode(this->ParametersNode);
    this->UpdateGUI();
    this->GetApplicationGUI()->GetViewerWidget()->RequestRender();
    }


  //Update GUI
  this->UpdateMRML();
  this->ProcessingGUIEvents = 0;

}

void vtkVolumeRenderingGUI::UpdateMRML(void)
{
  vtkMRMLVolumeRenderingParametersNode* n = this->GetParametersNode();
  if (n == NULL)
    {
    this->CreateParametersNode();
    }
  if(this->VolumeNodeSelector->GetSelected()!=NULL)
    {
    this->ParametersNode->SetAndObserveVolumeNodeID(this->VolumeNodeSelector->GetSelected()->GetID());
    }

  this->Logic->SetParametersNode(this->ParametersNode);


}

void vtkVolumeRenderingGUI::CreateParametersNode(void)
{
  vtkMRMLVolumeRenderingParametersNode* n = this->GetParametersNode();
  if (n == NULL)
    {
    // no parameter node selected yet, create new
    this->VolumeRenderingParameterSelector->SetSelectedNew("vtkMRMLVolumeRenderingParametersNode");
    this->VolumeRenderingParameterSelector->ProcessNewNodeCommand("vtkMRMLVolumeRenderingParametersNode", "Parameters");
    n = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(this->VolumeRenderingParameterSelector->GetSelected());

    vtkSetAndObserveMRMLNodeMacro(this->ParametersNode, n);

    this->UpdateParametersNode();

    this->Logic->SetParametersNode(this->ParametersNode);

    }
}

void vtkVolumeRenderingGUI::UpdateParametersNode(void)
{
  if (this->ParametersNode)
    {
    // set an observe new node
    if(this->ParametersNode->GetVolumeNode() == NULL && this->VolumeNodeSelector->GetSelected() != NULL)
    {
      this->ParametersNode->SetAndObserveVolumeNodeID(this->VolumeNodeSelector->GetSelected()->GetID());
    }

    if (this->ParametersNode->GetVolumePropertyNode() == NULL)
      {
      vtkMRMLVolumePropertyNode *vpNode = vtkMRMLVolumePropertyNode::New();
      this->Logic->GetMRMLScene()->AddNode(vpNode);
      vpNode->Delete();
      //vpNode->CreateDefaultStorageNode();
      this->ParametersNode->SetAndObserveVolumePropertyNodeID(vpNode->GetID());
      }
    /**
    if (this->ParametersNode->GetROINode() == NULL)
      {
      vtkMRMLROINode *roiNode = vtkMRMLROINode::New();
      this->Logic->GetMRMLScene()->AddNode(roiNode);
      roiNode->Delete();
      this->ParametersNode->SetAndObserveROINodeID(roiNode->GetID());
      }
      **/
    }
}

void vtkVolumeRenderingGUI::ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
  if (this->ProcessingGUIEvents || this->ProcessingMRMLEvents)
    {
    return;
    }
  this->ProcessingMRMLEvents = 1;

  vtkMRMLNode *addedNode = NULL;
    
  if (event == vtkMRMLScene::NodeAddedEvent && 
      this->MRMLScene)
    {
    addedNode = reinterpret_cast<vtkMRMLNode *>(callData);
    }
        
  if (event == vtkMRMLScene::NodeAddedEvent && addedNode &&
      addedNode->IsA("vtkMRMLVolumeRenderingParametersNode"))
    {
    if (this->GetParametersNode() == NULL)
      {
      vtkSetAndObserveMRMLNodeMacro(this->ParametersNode, addedNode);
      this->Logic->SetParametersNode(this->ParametersNode);
      this->UpdateGUI();
      }
    }
         
      
  if (event == vtkMRMLScene::SceneCloseEvent)
    {
    vtkSetAndObserveMRMLNodeMacro(this->ParametersNode, NULL);
    this->Logic->SetParametersNode(NULL);
    this->UpdateGUI();
    }

  if(event == vtkMRMLScalarVolumeNode::ImageDataModifiedEvent)
    {
    this->GetApplicationGUI()->GetViewerWidget()->RequestRender();
    }

  if(event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {
    // update transform
    vtkMRMLScalarVolumeNode *volumeNode = this->ParametersNode->GetVolumeNode();
    this->Logic->UpdateTransform(volumeNode);
    
    this->GetApplicationGUI()->GetViewerWidget()->RequestRender();
    } 

  if(event == vtkCommand::ModifiedEvent)
    {
    vtkMRMLVolumeRenderingParametersNode *node = vtkMRMLVolumeRenderingParametersNode::SafeDownCast(caller);
    this->Logic->SetParametersNode(node);
    this->GetApplicationGUI()->GetViewerWidget()->RequestRender();
    }

  this->ProcessingMRMLEvents = 0;

}

void vtkVolumeRenderingGUI::Enter(void)
{


  vtkDebugMacro("Enter Volume Rendering Module");
  //Load Presets

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
   
  if (this->ParametersNode)
    {                
    this->VolumeNodeSelector->SetSelected( vtkMRMLVolumeNode::SafeDownCast(
                                              this->MRMLScene->GetNodeByID(this->ParametersNode->GetVolumeNodeID()) ) );
    this->VolumeRenderingParameterSelector->SetSelected( this->ParametersNode);
    
    }
  else
    {
    this->VolumeNodeSelector->SetSelected(NULL);
    this->VolumeRenderingParameterSelector->SetSelected(NULL);
    }

  this->UpdatingGUI = 0;

}

void vtkVolumeRenderingGUI::SetViewerWidget(vtkSlicerViewerWidget *viewerWidget)
{
}
void vtkVolumeRenderingGUI::SetInteractorStyle(vtkSlicerViewerInteractorStyle *interactorStyle)
{
}




