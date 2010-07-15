#include "vtkVolumeRenderingGUI.h"

#include <ostream>

#include "vtkBMPReader.h"
#include "vtkBMPWriter.h"
#include "vtkCellArray.h"
#include "vtkFloatArray.h"
#include "vtkImageData.h"
#include "vtkImageGradientMagnitude.h"
#include "vtkImageMapper.h"
#include "vtkIndent.h"
#include "vtkPiecewiseFunction.h"
#include "vtkPlaneSource.h"
#include "vtkPointData.h"
#include "vtkPoints.h"
#include "vtkPolyData.h"
#include "vtkTexture.h"
#include "vtkTimerLog.h"
#include "vtkVolume.h"

#include "vtkKWCheckButton.h"
#include "vtkKWColorTransferFunctionEditor.h"
#include "vtkKWEntry.h"
#include "vtkKWEvent.h"
#include "vtkKWHistogramSet.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWPiecewiseFunctionEditor.h"
#include "vtkKWProgressGauge.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWCheckButtonWithLabel.h"

#include "vtkRendererCollection.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerFixedPointVolumeRayCastMapper.h"
#include "vtkSlicerMRMLTreeWidget.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerVolumePropertyWidget.h"
#include "vtkSlicerVolumeTextureMapper3D.h"
#include "vtkSlicerROIDisplayWidget.h"
#include "vtkMRMLSliceLogic.h"

extern "C" int Volumerenderingreplacements1_Init(Tcl_Interp *interp);

vtkVolumeRenderingGUI::vtkVolumeRenderingGUI(void)
{
    
  //In Debug Mode
  this->DebugOff();
    
  this->UpdatingGUI = 0;
  this->ProcessingGUIEvents = 0;
  this->ProcessingMRMLEvents = 0;
  this->AddingVolumePropertyNode = 0;

  this->Logic = NULL;
  this->ParametersNode = NULL;
  this->ROINode = NULL;
  this->ViewerWidget = NULL;
  this->InteractorStyle = NULL;

  //Frame Details
  this->DetailsFrame=NULL;
  this->HideSurfaceModelsButton = NULL;
  this->VolumeNodeSelector = NULL;
  this->VolumePropertyNodeSelector = NULL;
  this->ROINodeSelector = NULL;
  this->VolumeRenderingParameterSelector = NULL;
  this->GradientMagnitude = NULL;
  this->VolumePropertyWidget =NULL;
  this->ROIWidget = NULL;
  this->CroppingButton = NULL;
  this->FitROIButton = NULL;
  this->GradientHistogram = NULL;
  this->Histograms = NULL;

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

  if (this->VolumeRenderingParameterSelector)
    {
    this->VolumeRenderingParameterSelector->SetParent(NULL);
    this->VolumeRenderingParameterSelector->Delete();
    this->VolumeRenderingParameterSelector=NULL;
    }
  if (this->VolumeNodeSelector)
    {
    this->VolumeNodeSelector->SetParent(NULL);
    this->VolumeNodeSelector->Delete();
    this->VolumeNodeSelector=NULL;
    }
  if (this->VolumePropertyNodeSelector)
    {
    this->VolumePropertyNodeSelector->SetParent(NULL);
    this->VolumePropertyNodeSelector->Delete();
    this->VolumePropertyNodeSelector=NULL;
    }
  if (this->ROINodeSelector)
    {
    this->ROINodeSelector->SetParent(NULL);
    this->ROINodeSelector->Delete();
    this->ROINodeSelector=NULL;
    }
  if (this->CroppingButton)
    {
    this->CroppingButton->SetParent(NULL);
    this->CroppingButton->Delete();
    this->CroppingButton=NULL;
    }
  //Remove Volume

  if (this->GradientHistogram)
    {
    this->GradientHistogram->Delete();
    }
  if (this->Histograms)
    {
    this->Histograms->Delete();
    }
  if (this->GradientMagnitude)
    {
    this->GradientMagnitude->Delete();
    }

  if(this->VolumePropertyWidget!=NULL)
    {
    this->VolumePropertyWidget->SetHistogramSet(NULL);
    this->VolumePropertyWidget->SetVolumeProperty(NULL);
    this->VolumePropertyWidget->SetDataSet(NULL);
    this->VolumePropertyWidget->SetParent(NULL);
    this->VolumePropertyWidget->Delete();
    this->VolumePropertyWidget=NULL;
    }
  if (this->ROIWidget)
    {
    this->ROIWidget->SetParent(NULL);
    this->ROIWidget->Delete();
    this->ROIWidget=NULL;
    }

  if ( this->FitROIButton ) {
      this->FitROIButton->SetParent(NULL);
      this->FitROIButton->Delete();
      this->FitROIButton = NULL;
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

void vtkVolumeRenderingGUI::UpdateVolumeActor()
{
  if (this->GetApplicationGUI() && 
      this->GetApplicationGUI()->GetActiveViewerWidget() && 
      this->GetLogic() &&
      this->GetLogic()->GetVolume()
      )
    {
    if (this->VolumeNodeSelector->GetSelected())
      {
      if (!this->GetApplicationGUI()->GetActiveViewerWidget()->GetMainViewer()->HasViewProp(this->GetLogic()->GetVolume())) 
        {
        this->GetApplicationGUI()->GetActiveViewerWidget()->GetMainViewer()->AddViewProp(this->GetLogic()->GetVolume());
        }
      }
    else
      {
      if (this->GetApplicationGUI()->GetActiveViewerWidget()->GetMainViewer()->HasViewProp(this->GetLogic()->GetVolume())) 
        {
        this->GetApplicationGUI()->GetActiveViewerWidget()->GetMainViewer()->RemoveViewProp(this->GetLogic()->GetVolume());
        }
      }

    }
}

void vtkVolumeRenderingGUI::UpdateHistogram()
{
  vtkMRMLScalarVolumeNode *volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(this->VolumeNodeSelector->GetSelected());
  if (volumeNode)
    {
    if (volumeNode->GetImageData() && volumeNode->GetImageData()->GetPointData())
    {
        this->Histograms->AddHistograms(volumeNode->GetImageData()->GetPointData()->GetScalars());
    }
    
    this->GradientMagnitude->SetInput(volumeNode->GetImageData());
    this->VolumePropertyWidget->SetDataSet(volumeNode->GetImageData());
    //Build the gradient histogram
    this->GradientMagnitude->Update();
    this->GradientHistogram->BuildHistogram(GradientMagnitude->GetOutput()->GetPointData()->GetScalars(),0);
    this->VolumePropertyWidget->SetHistogramSet(this->Histograms);
    if (this->ParametersNode &&  this->ParametersNode->GetVolumePropertyNode())
      {
      this->VolumePropertyWidget->SetVolumeProperty(this->ParametersNode->GetVolumePropertyNode()->GetVolumeProperty());
      }
    }
}

void vtkVolumeRenderingGUI::InitTransferFunction()
{
    if (this->ParametersNode &&  this->ParametersNode->GetVolumePropertyNode())
      {
      this->VolumePropertyWidget->SetVolumeProperty(this->ParametersNode->GetVolumePropertyNode()->GetVolumeProperty());
      }

    vtkKWHistogram *histogram=this->Histograms->GetHistogramWithName("0");
    if(histogram==NULL)
    {
        vtkErrorMacro("Problems with HistogramSet");
        return;
    }
    double totalOccurance=histogram->GetTotalOccurence();
    double thresholdLow=totalOccurance*0.2;
    double thresholdHigh=totalOccurance*0.8;
    double range[2];

    histogram->GetRange(range);
    double thresholdLowIndex=range[0];
    double sumLowIndex=0;
    double thresholdHighIndex=range[0];
    double sumHighIndex=0;
    //calculate distance
    double bin_width = (range[1] == range[0] ? 1 : (range[1] - range[0])/(double)histogram->GetNumberOfBins());
    while (sumLowIndex<thresholdLow)
    {
        sumLowIndex+=histogram->GetOccurenceAtValue(thresholdLowIndex);
        thresholdLowIndex+=bin_width;
    }
    while(sumHighIndex<thresholdHigh)
    {
        sumHighIndex+=histogram->GetOccurenceAtValue(thresholdHighIndex);
        thresholdHighIndex+=bin_width;

    }

    if (this->ParametersNode &&
        this->ParametersNode->GetVolumePropertyNode() &&
        this->ParametersNode->GetVolumePropertyNode()->GetVolumeProperty())
    {
      vtkVolumeProperty *vprop = this->ParametersNode->GetVolumePropertyNode()->GetVolumeProperty();
      vprop->SetInterpolationTypeToLinear();
      vtkPiecewiseFunction *opacity=vprop->GetScalarOpacity();
      opacity->RemoveAllPoints();
      opacity->AddPoint(range[0],0.);
      opacity->AddPoint(thresholdLowIndex,0.0);
      opacity->AddPoint(thresholdHighIndex,0.2);
      opacity->AddPoint(range[1],0.2);
      vtkColorTransferFunction *colorTransfer=vprop->GetRGBTransferFunction();
      colorTransfer->RemoveAllPoints();
      colorTransfer->AddRGBPoint(range[0],.3,.3,1.);
      colorTransfer->AddRGBPoint(thresholdLowIndex,.3,.3,1.);
      colorTransfer->AddRGBPoint(thresholdLowIndex+.5*(thresholdHighIndex-thresholdLowIndex),.3,1.,.3);
      colorTransfer->AddRGBPoint(thresholdHighIndex,1.,.3,.3);
      colorTransfer->AddRGBPoint(range[1],1,.3,.3);

      //Enable shading as default
      vprop->ShadeOn();
      vprop->SetAmbient(.20);
      vprop->SetDiffuse(.80);
      vprop->SetSpecular(.50);
      vprop->SetSpecularPower(40);
    }
    this->VolumePropertyWidget->SetHSVColorSelectorVisibility(1);
    this->VolumePropertyWidget->Update();

}

void vtkVolumeRenderingGUI::BuildGUI(void)
{

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
  loadSaveDataFrame->SetLabelText("Input");
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
  this->VolumeRenderingParameterSelector->SetNodeClass("vtkMRMLVolumeRendering1ParametersNode", NULL, NULL, "Parameters");
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

  //
  //Transfer function
  //
  vtkSlicerModuleCollapsibleFrame *volumePropertyFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  volumePropertyFrame->SetParent (this->UIPanel->GetPageWidget("VolumeRendering"));
  volumePropertyFrame->Create();
  //volumePropertyFrame->ExpandFrame();
  volumePropertyFrame->SetLabelText("Volume Properties");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                volumePropertyFrame->GetWidgetName(), this->UIPanel->GetPageWidget("VolumeRendering")->GetWidgetName());


  this->VolumePropertyNodeSelector=vtkSlicerNodeSelectorWidget::New();
  this->VolumePropertyNodeSelector->SetNodeClass("vtkMRMLVolumePropertyNode", NULL, NULL, "Volume Property");
  this->VolumePropertyNodeSelector->SetNewNodeEnabled(1);
  this->VolumePropertyNodeSelector->NoneEnabledOff();
  this->VolumePropertyNodeSelector->SetShowHidden(1);
  this->VolumePropertyNodeSelector->SetParent(volumePropertyFrame->GetFrame());
  this->VolumePropertyNodeSelector->Create();
  this->VolumePropertyNodeSelector->SetMRMLScene(this->GetMRMLScene());
  this->VolumePropertyNodeSelector->UpdateMenu();
  this->VolumePropertyNodeSelector->SetBorderWidth(2);
  this->VolumePropertyNodeSelector->SetLabelText( "Volume Property");
  this->VolumePropertyNodeSelector->SetBalloonHelpString("select a volume property node from the current mrml scene.");

  this->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->VolumePropertyNodeSelector->GetWidgetName());

  this->VolumePropertyWidget=vtkSlicerVolumePropertyWidget::New();
  this->VolumePropertyWidget->SetParent(volumePropertyFrame->GetFrame());
  this->VolumePropertyWidget->Create();
  this->VolumePropertyWidget->ScalarOpacityUnitDistanceVisibilityOff ();
  this->VolumePropertyWidget->InteractiveApplyModeOn ();

  this->Histograms=vtkKWHistogramSet::New();
  this->GradientMagnitude=vtkImageGradientMagnitude::New();
  this->GradientMagnitude->SetDimensionality(3);

  this->GradientHistogram=vtkKWHistogram::New();
  this->Histograms->AddHistogram(GradientHistogram, "0gradient");

  //this->VolumePropertyWidget->SetHistogramSet(this->Histograms);

  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->VolumePropertyWidget->GetWidgetName());
  
  
  //
  //ROI frame
  //
  vtkSlicerModuleCollapsibleFrame *ROIframe = vtkSlicerModuleCollapsibleFrame::New ( );
  ROIframe->SetParent (this->UIPanel->GetPageWidget("VolumeRendering"));
  ROIframe->Create();
  //ROIframe->ExpandFrame();
  ROIframe->SetLabelText("Volume ROI");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                ROIframe->GetWidgetName(), this->UIPanel->GetPageWidget("VolumeRendering")->GetWidgetName());

  this->CroppingButton = vtkKWCheckButtonWithLabel::New();
  this->CroppingButton->SetParent ( ROIframe->GetFrame() );
  this->CroppingButton->Create ( );
  this->CroppingButton->SetLabelText("Cropping Enabled");
  this->CroppingButton->SetBalloonHelpString("Enable cropping.");
  this->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->CroppingButton->GetWidgetName());


  this->ROINodeSelector=vtkSlicerNodeSelectorWidget::New();
  this->ROINodeSelector->SetNodeClass("vtkMRMLROINode", NULL, NULL, "VolumeRenderingROI");
  this->ROINodeSelector->SetNewNodeEnabled(1);
  this->ROINodeSelector->NoneEnabledOff();
  this->ROINodeSelector->SetShowHidden(1);
  this->ROINodeSelector->SetParent(ROIframe->GetFrame());
  this->ROINodeSelector->Create();
  this->ROINodeSelector->SetMRMLScene(this->GetMRMLScene());
  this->ROINodeSelector->UpdateMenu();
  this->ROINodeSelector->SetBorderWidth(2);
  this->ROINodeSelector->SetLabelText( "ROI");
  this->ROINodeSelector->SetBalloonHelpString("select a roi  node from the current mrml scene.");

  this->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->ROINodeSelector->GetWidgetName());

  this->FitROIButton = vtkKWPushButton::New();
  this->FitROIButton->SetParent( ROIframe->GetFrame() );
  this->FitROIButton->Create();
  this->FitROIButton->SetText("Fit ROI To Volume");
  this->Script("pack %s -side top -fill x -anchor nw -padx 2 -pady 2",this->FitROIButton->GetWidgetName());

  this->ROIWidget=vtkSlicerROIDisplayWidget::New();
  this->ROIWidget->SetParent(ROIframe->GetFrame());
  this->ROIWidget->Create();

  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->ROIWidget->GetWidgetName());
  
  
  
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
    this->GetApplicationGUI()->GetMRMLScene()->AddObserver( vtkMRMLScene::SceneClosedEvent, this->MRMLCallbackCommand );
    this->MRMLScene->AddObserver(vtkMRMLScene::NodeAddedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    this->MRMLScene->AddObserver(vtkMRMLScene::NodeRemovedEvent, (vtkCommand *)this->MRMLCallbackCommand);
    }
    
  //vtkMRMLVolumeRendering1ParametersNode* parametersNode = this->GetLogic()->GetParametersNode();
  //vtkSetAndObserveMRMLNodeMacro(this->ParametersNode, parametersNode);

  
  loadSaveDataFrame->Delete();
  volumePropertyFrame->Delete();
  ROIframe->Delete();
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
  this->VolumePropertyNodeSelector->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ROINodeSelector->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HideSurfaceModelsButton->AddObserver(vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallbackCommand );
  this->VolumePropertyWidget->AddObserver(vtkKWEvent::VolumePropertyChangingEvent,(vtkCommand*)this->GUICallbackCommand);
  this->CroppingButton->GetWidget()->AddObserver (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->FitROIButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

}
void vtkVolumeRenderingGUI::RemoveGUIObservers(void)
{
  this->VolumeNodeSelector->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->VolumeRenderingParameterSelector->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->VolumePropertyNodeSelector->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ROINodeSelector->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HideSurfaceModelsButton->RemoveObservers (vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallbackCommand);
  this->VolumePropertyWidget->RemoveObservers(vtkKWEvent::VolumePropertyChangingEvent,(vtkCommand*)this->GUICallbackCommand);
  this->CroppingButton->GetWidget()->RemoveObservers (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->FitROIButton->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

}
void vtkVolumeRenderingGUI::AddMRMLObservers(void)
{
  //Remove the MRML observer
  if ( this->GetApplicationGUI() )
    {
    this->GetApplicationGUI()->GetMRMLScene()->AddObserver(vtkMRMLScene::SceneClosedEvent, this->MRMLCallbackCommand);
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
    this->GetApplicationGUI()->GetMRMLScene()->RemoveObservers(vtkMRMLScene::NodeAddedEvent, this->MRMLCallbackCommand);
    this->GetApplicationGUI()->GetMRMLScene()->RemoveObservers(vtkMRMLScene::NodeRemovedEvent, this->MRMLCallbackCommand);
    }

}
void vtkVolumeRenderingGUI::RemoveLogicObservers(void)
{
}

void vtkVolumeRenderingGUI::ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData)
{
  //if (this->ProcessingGUIEvents || this->ProcessingMRMLEvents)
  if (this->ProcessingGUIEvents )
    {
    return;
    }
  this->ProcessingGUIEvents = 1;

  vtkDebugMacro("vtkVolumeRenderingGUI::ProcessGUIEvents: event = " << event);

  this->UpdateVolumeActor();

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
    vtkMRMLScalarVolumeNode *volumeNodePrev = this->ParametersNode->GetVolumeNode();
    if (volumeNodePrev)
      {
      volumeNodePrev->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent,(vtkCommand *) this->MRMLCallbackCommand);
      }
    if (volumeNode)
      {
      volumeNode->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent,(vtkCommand *) this->MRMLCallbackCommand);
      this->Logic->UpdateTransform(volumeNode);
      this->ParametersNode->SetAndObserveVolumeNodeID(volumeNode->GetID());

      vtkMRMLVolumePropertyNode *vpNode = this->CreateVolumePropertyNode();
      this->ParametersNode->SetAndObserveVolumePropertyNodeID(vpNode->GetID());

      this->UpdateHistogram();
      this->InitTransferFunction();
      }
    else
      {
      this->ParametersNode->SetAndObserveVolumeNodeID(NULL);
      }
    }

  if(callerObjectNS == this->VolumeRenderingParameterSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
    {
    vtkMRMLVolumeRendering1ParametersNode *paramNode = vtkMRMLVolumeRendering1ParametersNode::SafeDownCast(this->VolumeRenderingParameterSelector->GetSelected());
    if (paramNode != this->GetParametersNode())
      {
      this->UpdateParametersNode();
      }
    vtkSetAndObserveMRMLNodeMacro(this->ParametersNode, paramNode);
    this->UpdateGUIFromMRML();
    }

  if(callerObjectNS == this->VolumePropertyNodeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
    {
    vtkMRMLVolumeRendering1ParametersNode *paramNode = vtkMRMLVolumeRendering1ParametersNode::SafeDownCast(this->VolumeRenderingParameterSelector->GetSelected());
    vtkMRMLVolumePropertyNode *propertyNode = vtkMRMLVolumePropertyNode::SafeDownCast(this->VolumePropertyNodeSelector->GetSelected());
    if (paramNode )
      {
      paramNode->SetAndObserveVolumePropertyNodeID(propertyNode->GetID());
      this->VolumePropertyWidget->SetVolumeProperty(propertyNode->GetVolumeProperty());
      if ( this->AddingVolumePropertyNode  )
        {
        this->UpdateHistogram();
        this->AddingVolumePropertyNode = 0;
        }
      this->VolumePropertyWidget->Update();
      }
    }

  if(callerObjectNS == this->ROINodeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent)
    {
    vtkMRMLVolumeRendering1ParametersNode *paramNode = vtkMRMLVolumeRendering1ParametersNode::SafeDownCast(this->VolumeRenderingParameterSelector->GetSelected());
    vtkMRMLROINode *roiNode = vtkMRMLROINode::SafeDownCast(this->ROINodeSelector->GetSelected());
    if (paramNode )
      {
      paramNode->SetAndObserveROINodeID(roiNode->GetID());
      this->ROIWidget->SetAndObserveMRMLScene(this->GetMRMLScene());
      this->ROIWidget->SetROINode(roiNode);
      vtkSetAndObserveMRMLNodeMacro(this->ROINode, roiNode);
      }
   }

  vtkSlicerVolumePropertyWidget *callerObjectSVP=vtkSlicerVolumePropertyWidget::SafeDownCast(caller);
  if(callerObjectSVP == this->VolumePropertyWidget && event == vtkKWEvent::VolumePropertyChangingEvent)
    {
    if (this->GetApplicationGUI()->GetActiveViewerWidget())
      {
      this->GetApplicationGUI()->GetActiveViewerWidget()->RequestRender();
      }
    this->ProcessingGUIEvents = 0;
    return;
    }
  
  if (vtkKWPushButton::SafeDownCast(caller) == this->FitROIButton && event == vtkKWPushButton::InvokedEvent ) 
    {
    this->FitROIToVolume();
    }

  if (event == vtkKWCheckButton::SelectedStateChangedEvent && 
          this->CroppingButton->GetWidget() == vtkKWCheckButton::SafeDownCast(caller) )
    {
    vtkMRMLVolumeRendering1ParametersNode *paramNode = vtkMRMLVolumeRendering1ParametersNode::SafeDownCast(this->VolumeRenderingParameterSelector->GetSelected());
    if (paramNode)
      {
      paramNode->SetCroppingEnabled(this->CroppingButton->GetWidget()->GetSelectedState());
      }
    }

  //Update GUI
  this->UpdateMRMLFromGUI();
  this->Logic->SetParametersNode(this->ParametersNode);
  if (this->GetApplicationGUI()->GetActiveViewerWidget())
    {
    this->GetApplicationGUI()->GetActiveViewerWidget()->RequestRender();
    }

  this->ProcessingGUIEvents = 0;

}

void vtkVolumeRenderingGUI::UpdateMRMLFromGUI(void)
{
  vtkMRMLVolumeRendering1ParametersNode* node = this->GetParametersNode();
  if (node == NULL)
    {
    this->CreateParametersNode();
    }

  vtkMRMLVolumeRendering1ParametersNode *paramNode = vtkMRMLVolumeRendering1ParametersNode::SafeDownCast(this->VolumeRenderingParameterSelector->GetSelected());
  vtkSetAndObserveMRMLNodeMacro(this->ParametersNode, paramNode);

  if(this->VolumeNodeSelector->GetSelected() != NULL)
    {
    this->ParametersNode->SetAndObserveVolumeNodeID(this->VolumeNodeSelector->GetSelected()->GetID());
    }
  else
    {
    this->ParametersNode->SetAndObserveVolumeNodeID(NULL);
    }
  if (this->ParametersNode && this->ParametersNode->GetVolumePropertyNode())
    {
    this->VolumePropertyWidget->SetVolumeProperty(this->ParametersNode->GetVolumePropertyNode()->GetVolumeProperty());
    this->VolumePropertyWidget->SetHSVColorSelectorVisibility(1);
    this->VolumePropertyWidget->Update();
    }
  if (this->ParametersNode)
    {
    this->ParametersNode->SetCroppingEnabled(this->CroppingButton->GetWidget()->GetSelectedState());
    }

}

void vtkVolumeRenderingGUI::CreateParametersNode(void)
{
  vtkMRMLVolumeRendering1ParametersNode* n = this->GetParametersNode();
  if (n == NULL)
    {
    // no parameter node selected yet, create new
    this->VolumeRenderingParameterSelector->SetSelectedNew("vtkMRMLVolumeRendering1ParametersNode");
    this->VolumeRenderingParameterSelector->ProcessNewNodeCommand("vtkMRMLVolumeRendering1ParametersNode", "Parameters");
    n = vtkMRMLVolumeRendering1ParametersNode::SafeDownCast(this->VolumeRenderingParameterSelector->GetSelected());

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

    /**
    if (this->ParametersNode->GetVolumePropertyNode() == NULL)
      {
      vtkMRMLVolumePropertyNode *vpNode = this->CreateVolumePropertyNode();
      this->ParametersNode->SetAndObserveVolumePropertyNodeID(vpNode->GetID());
      }
      **/
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

vtkMRMLVolumePropertyNode* vtkVolumeRenderingGUI::CreateVolumePropertyNode(void)
{
  vtkMRMLVolumePropertyNode *vpNode = vtkMRMLVolumePropertyNode::New();
  this->Logic->GetMRMLScene()->AddNode(vpNode);
  vpNode->Delete();
  vpNode->CreateDefaultStorageNode();
  //this->ParametersNode->SetAndObserveVolumePropertyNodeID(vpNode->GetID());
  return vpNode;
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
      addedNode->IsA("vtkMRMLVolumeRendering1ParametersNode"))
    {
    if (this->ParametersNode && this->ParametersNode->GetVolumeNode() )
      {
      this->ParametersNode->GetVolumeNode()->RemoveObservers(vtkMRMLTransformableNode::TransformModifiedEvent,(vtkCommand *) this->MRMLCallbackCommand);
      }

    vtkSetAndObserveMRMLNodeMacro(this->ParametersNode, addedNode);
    
    vtkMRMLScalarVolumeNode *volumeNode = this->ParametersNode->GetVolumeNode();
    if (volumeNode)
      {
      volumeNode->AddObserver(vtkMRMLTransformableNode::TransformModifiedEvent,(vtkCommand *) this->MRMLCallbackCommand);
      }
    }


  else if (event == vtkMRMLScene::NodeAddedEvent && addedNode &&
      addedNode->IsA("vtkMRMLVolumePropertyNode"))
    {
    this->AddingVolumePropertyNode = 1;
    }
  else if (event == vtkMRMLScene::NodeAddedEvent)
    {
    this->ProcessingMRMLEvents = 0;
    return;
    }
         
  if (event == vtkMRMLScene::SceneClosedEvent)
    {
    vtkSetAndObserveMRMLNodeMacro(this->ParametersNode, NULL);
    }

  if(event == vtkMRMLScalarVolumeNode::ImageDataModifiedEvent)
    {
    }

  if(event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {
    // update transform
    vtkMRMLScalarVolumeNode *volumeNode = this->ParametersNode->GetVolumeNode();
    this->Logic->UpdateTransform(volumeNode);    
    } 

  if(event == vtkCommand::ModifiedEvent && vtkMRMLROINode::SafeDownCast(caller))
    {
    // this is from ROI node
    }

  this->UpdateVolumeActor();
  this->UpdateGUIFromMRML();
  this->Logic->SetParametersNode(this->ParametersNode);
  if (this->GetApplicationGUI()->GetActiveViewerWidget())
    {
    this->GetApplicationGUI()->GetActiveViewerWidget()->RequestRender();
    }

  this->ProcessingMRMLEvents = 0;

}

void vtkVolumeRenderingGUI::UpdateGUIFromMRML(void)
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
    this->VolumePropertyNodeSelector->SetSelected( vtkMRMLVolumePropertyNode::SafeDownCast(
                                              this->MRMLScene->GetNodeByID(this->ParametersNode->GetVolumePropertyNodeID()) ) );
    this->ROINodeSelector->SetSelected( vtkMRMLROINode::SafeDownCast(
                                              this->MRMLScene->GetNodeByID(this->ParametersNode->GetROINodeID() ) ) );
    this->VolumeRenderingParameterSelector->SetSelected( this->ParametersNode);

    if (this->ParametersNode->GetVolumePropertyNode()) 
      {
      this->VolumePropertyWidget->SetVolumeProperty(this->ParametersNode->GetVolumePropertyNode()->GetVolumeProperty());
      this->VolumePropertyWidget->SetHSVColorSelectorVisibility(1);
      this->UpdateHistogram();
      this->VolumePropertyWidget->Update();
      }
    this->CroppingButton->GetWidget()->SetSelectedState(this->ParametersNode->GetCroppingEnabled());
    this->ROIWidget->SetROINode(this->ParametersNode->GetROINode());
    }
  else
    {
    this->VolumeNodeSelector->SetSelected(NULL);
    this->VolumePropertyNodeSelector->SetSelected(NULL);
    this->ROINodeSelector->SetSelected(NULL);
    this->VolumeRenderingParameterSelector->SetSelected(NULL);
    }

  this->UpdatingGUI = 0;

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
  this->UpdateGUIFromMRML();
}

void vtkVolumeRenderingGUI::Exit(void)
{
  vtkDebugMacro("Exit: removeObservers for VolumeRendering");
  this->ReleaseModuleEventBindings();
}



void vtkVolumeRenderingGUI::SetViewerWidget(vtkSlicerViewerWidget *viewerWidget)
{
}
void vtkVolumeRenderingGUI::SetInteractorStyle(vtkSlicerViewerInteractorStyle *interactorStyle)
{
}

void vtkVolumeRenderingGUI::FitROIToVolume()
{
  // resize the ROI to fit the volume
  vtkMRMLROINode *roiNode = vtkMRMLROINode::SafeDownCast(this->ROINodeSelector->GetSelected());
  vtkMRMLScalarVolumeNode *volumeNode = vtkMRMLScalarVolumeNode::SafeDownCast(this->VolumeNodeSelector->GetSelected());
  if (volumeNode && roiNode)
    {
    double xyz[3];
    double center[3];

    vtkMRMLSliceLogic::GetVolumeRASBox(volumeNode, xyz,  center);
    for (int i=0; i<3; i++)
      {
      xyz[i] /= 2;
      }
    roiNode->SetXYZ(center);
    roiNode->SetRadiusXYZ(xyz);
    }
}
