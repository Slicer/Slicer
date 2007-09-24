#include "vtkVolumeRenderingModuleGUI.h"

#include <ostream>
#include "vtkMRMLVolumeRenderingNode.h"
#include "vtkVolumeTextureMapper3D.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkIndent.h"
#include "vtkPointData.h"
#include "vtkPiecewisefunction.h"


vtkVolumeRenderingModuleGUI::vtkVolumeRenderingModuleGUI(void)
{
    //In Debug Mode
    this->DebugOff();
   
}

vtkVolumeRenderingModuleGUI::~vtkVolumeRenderingModuleGUI(void)
{
}
vtkVolumeRenderingModuleGUI* vtkVolumeRenderingModuleGUI::New() {
    // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkVolumeRenderingModuleGUI");
  if(ret)
    {
      return (vtkVolumeRenderingModuleGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkVolumeRenderingModuleGUI;


}
void vtkVolumeRenderingModuleGUI::PrintSelf(ostream& os, vtkIndent indent)
{
    os<<indent<<"vtkVolumeRenderingModuleGUI"<<endl;
    os<<indent<<"vtkVolumeRenderingModuleLogic"<<endl;
    if(this->GetLogic())
    {
        this->GetLogic()->PrintSelf(os,indent.GetNextIndent());
    }
}
void vtkVolumeRenderingModuleGUI::BuildGUI(void)
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    this->GetUIPanel()->AddPage("VolumeRendering","VolumeRendering",NULL);

    // Define your help text and build the help frame here.
    const char *help = "VolumeRendering. 3D Segmentation This module is currently a prototype and will be under active development throughout 3DSlicer's Beta release.";
    const char *about = "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See http://www.slicer.org for details.";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "VolumeRendering" );
    this->BuildHelpAndAboutFrame ( page, help, about );
    //
    //Load and save
    //
    vtkSlicerModuleCollapsibleFrame *loadSaveDataFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    loadSaveDataFrame->SetParent (this->UIPanel->GetPageWidget("VolumeRendering"));
    loadSaveDataFrame->Create();
    loadSaveDataFrame->ExpandFrame();
    loadSaveDataFrame->SetLabelText("Load and save");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
        loadSaveDataFrame->GetWidgetName(), this->UIPanel->GetPageWidget("VolumeRendering")->GetWidgetName());
    
    //Testing Pushbutton
    this->PB_Testing= vtkKWPushButton::New();
    this->PB_Testing->SetParent(loadSaveDataFrame->GetFrame());
    this->PB_Testing->Create();
    this->PB_Testing->SetText("Testing");
    app->Script("pack %s -side top -anchor e -padx 2 -pady 2",this->PB_Testing->GetWidgetName());
    
    //NodeSelector  for Node from MRML Scene And Button
    this->NS_ImageData=vtkSlicerNodeSelectorWidget::New();
    this->NS_ImageData->SetParent(loadSaveDataFrame->GetFrame());
    this->NS_ImageData->Create();
    this->NS_ImageData->SetLabelText("Source Volume");
    app->Script("pack %s -side top -anchor e -padx 2 -pady 2",this->NS_ImageData->GetWidgetName());

    this->PB_LoadImageData=vtkKWPushButton::New();
    this->PB_LoadImageData->SetParent(loadSaveDataFrame->GetFrame());
    this->PB_LoadImageData->Create();
    this->PB_LoadImageData->SetText("Load Node");
    app->Script("pack %s -side top -anchor e -padx 2 -pady 2", this->PB_LoadImageData->GetWidgetName());


    //NodeSelector for VolumeRenderingNode
    this->NS_VolumeRenderingDataScene=vtkSlicerNodeSelectorWidget::New();
    this->NS_VolumeRenderingDataScene->SetParent(loadSaveDataFrame->GetFrame());
    this->NS_VolumeRenderingDataScene->Create();
    this->NS_VolumeRenderingDataScene->SetLabelText("VolumeRenderingNode from Scene");
    app->Script("pack %s -side top -anchor e -padx 2 -pady 2",this->NS_VolumeRenderingDataScene->GetWidgetName());
    //Missing: Load from file

    this->PB_LoadVolumeRenderingDataSlicer=vtkKWPushButton::New();
    this->PB_LoadVolumeRenderingDataSlicer->SetParent(loadSaveDataFrame->GetFrame());
    this->PB_LoadVolumeRenderingDataSlicer->Create();
    this->PB_LoadVolumeRenderingDataSlicer->SetText("Load VolumeRenderingNode");
    app->Script("pack %s -side top -anchor e -padx 2 -pady 2",this->PB_LoadVolumeRenderingDataSlicer->GetWidgetName());

    //Status
    this->L_Status=vtkKWLabel::New();
    this->L_Status->SetParent(loadSaveDataFrame->GetFrame());
    this->L_Status->Create();
    this->L_Status->SetText("Volume: NOT LOADED,Rendering: NOT LOADED");
    app->Script("pack %s -side top -anchor e -padx 2 -pady 2",this->L_Status->GetWidgetName());

    //Details frame
     vtkSlicerModuleCollapsibleFrame *detailsFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    detailsFrame->SetParent (this->UIPanel->GetPageWidget("VolumeRendering"));
    detailsFrame->Create();
    detailsFrame->ExpandFrame();
    detailsFrame->SetLabelText("Details");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
        detailsFrame->GetWidgetName(), this->UIPanel->GetPageWidget("VolumeRendering")->GetWidgetName());
    
    this->SVP_VolumeProperty=vtkSlicerVolumePropertyWidget::New();
    this->SVP_VolumeProperty->SetParent(detailsFrame->GetFrame());
    this->SVP_VolumeProperty->Create();
    app->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",this->SVP_VolumeProperty->GetWidgetName());

      
        
        /*
    #Button to make all Models invisible
    set ::VR($this,buttonAllModelsInvisible) [vtkKWPushButton New]
    puts "buttonLoadNode: $::VR($this,buttonAllModelsInvisible)"
    $::VR($this,buttonAllModelsInvisible) SetParent [$::VR($this,cFrameVolumes) GetFrame]
    $::VR($this,buttonAllModelsInvisible) Create
    $::VR($this,buttonAllModelsInvisible) SetText "AllModelsInvisible"
    $::VR($this,buttonAllModelsInvisible) SetBalloonHelpString "Make all models invisible"
    pack [$::VR($this,buttonAllModelsInvisible) GetWidgetName] -side top -anchor e -padx 0 -pady 2*/
    //Delete frames
    loadSaveDataFrame->Delete();
    detailsFrame->Delete();
    this->Built=true;
}

void vtkVolumeRenderingModuleGUI::TearDownGUI(void)
{
    this->Exit();
    if ( this->Built )
    {
        this->RemoveGUIObservers();
    }
}

void vtkVolumeRenderingModuleGUI::CreateModuleEventBindings(void)
{
    vtkDebugMacro("VolumeRenderingModule: CreateModuleEventBindings: No ModuleEventBindings yet");
}

void vtkVolumeRenderingModuleGUI::ReleaseModuleEventBindings(void)
{
    vtkDebugMacro("VolumeRenderingModule: ReleaseModuleEventBindings: No ModuleEventBindings to remove yet");
}

void vtkVolumeRenderingModuleGUI::AddGUIObservers(void)
{
    this->PB_Testing->AddObserver(vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallbackCommand );
    this->PB_LoadImageData->AddObserver(vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallbackCommand);
}
void vtkVolumeRenderingModuleGUI::RemoveGUIObservers(void)
{
    this->PB_Testing->RemoveObservers (vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallbackCommand);
    this->PB_LoadImageData->RemoveObservers(vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallback);
}
void vtkVolumeRenderingModuleGUI::RemoveMRMLNodeObservers(void)
{

}
void vtkVolumeRenderingModuleGUI::RemoveLogicObservers(void)
{
}

void vtkVolumeRenderingModuleGUI::ProcessGUIEvents(vtkObject *caller, unsigned long event, void *callData)
{
     vtkDebugMacro("vtkVolumeRenderingModuleGUI::ProcessGUIEvents: event = " << event);
     //
     //Check PushButtons
     //
     vtkKWPushButton *callerObject=vtkKWPushButton::SafeDownCast(caller);
     //Testing Button ?
     if(callerObject==this->PB_Testing&&event==vtkKWPushButton::InvokedEvent)
     {
         this->InitializePipeline();
         this->currentNode->HideFromEditorsOff();
         this->GetLogic()->GetMRMLScene()->AddNode(this->currentNode);
         return;
         //vtkMRMLVolumeRenderingNode::
     }
     if(callerObject==this->PB_LoadImageData&&event==vtkKWPushButton::InvokedEvent)
     {
         this->InitializePipeline();
         //this->currentNode->InitializePipeline(vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetDisplayNode());

     }
    
     this->UpdateGUI();

}
void vtkVolumeRenderingModuleGUI::ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
}

void vtkVolumeRenderingModuleGUI::Enter(void)
{
     vtkDebugMacro("Enter Volume Rendering Module");
  if ( this->Built == false )
    {
    this->BuildGUI();
    this->AddGUIObservers();
    }
  this->CreateModuleEventBindings();
  this->UpdateGUI();
}

void vtkVolumeRenderingModuleGUI::Exit(void)
{
    vtkDebugMacro("Exit: removeObservers for VolumeRenderingModule");
  this->ReleaseModuleEventBindings();
}

void vtkVolumeRenderingModuleGUI::UpdateGUI(void)
{
    //First of all check if we have a MRML Scene
    if (!this->GetLogic()->GetMRMLScene())
    {
        //if not return
        return;
    }
    //Update the NodeSelector for Volumes
    this->NS_ImageData->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_ImageData->SetNodeClass("vtkMRMLScalarVolumeNode","","","");
    this->NS_ImageData->UpdateMenu();

    //Update NodeSelector for VolumeRendering Nodes

    this->NS_VolumeRenderingDataScene->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_VolumeRenderingDataScene->SetNodeClass("vtkMRMLVolumeRenderingNode","","","");
    this->NS_VolumeRenderingDataScene->UpdateMenu();


    //Update Label
    std::stringstream label;
    label<<"Volume: ";
    if(this->VolumeSelected==1){
        label<<this->NS_ImageData->GetSelected()->GetName();
    }
    else 
    {
        label<<"NOT loaded";
    }
    label<<"   Rendering";
        if(this->VolumeRenderingNodeSelected==1)
    {
        label<<this->NS_VolumeRenderingDataScene->GetSelected()->GetName();
    }
    else
    {
        label<<"NOT loaded";
    }
    this->L_Status->SetText(label.str().c_str());
    //this->L_Status->UpdateText();

}
void vtkVolumeRenderingModuleGUI::SetViewerWidget(vtkSlicerViewerWidget *viewerWidget)
{
}
void vtkVolumeRenderingModuleGUI::SetInteractorStyle(vtkSlicerViewerInteractorStyle *interactorStyle)
{
}

void vtkVolumeRenderingModuleGUI::InitializePipeline()
{
        this->currentNode=vtkMRMLVolumeRenderingNode::New();
        vtkImageData* imageData=vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetImageData();
        
        this->currentNode->GetMapper()->SetInput(imageData);
    this->HIST_Opacity=vtkKWHistogram::New();
    this->HIST_Opacity->BuildHistogram(imageData->GetPointData()->GetScalars(),0);
    //automatic Mode
    if(vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetLabelMap()==1)
    {
        vtkWarningMacro("Not yet implemented");
    }
    //automatic Mode for LabelMaps
    else
    {
        double totalOccurance=this->HIST_Opacity->GetTotalOccurence();
        double tresholdLow=totalOccurance*0.1;
        double tresholdHigh=totalOccurance*0.9;
        double *range=new double[2];
        this->HIST_Opacity->GetRange(range);
        double tresholdLowIndex=range[0];
        double sumLowIndex=0;
        double tresholdHighIndex=range[0];
        double sumHighIndex=0;
        while (sumLowIndex<tresholdLow)
        {
            sumLowIndex+=this->HIST_Opacity->GetOccurenceAtValue(tresholdLowIndex);
            tresholdLowIndex+=0.1;
        }
        while(sumHighIndex<tresholdHigh)
        {
            sumHighIndex*=this->HIST_Opacity->GetOccurenceAtValue(tresholdHighIndex);
            tresholdHighIndex+=0.1;
        }
        vtkPiecewiseFunction *opacity=this->currentNode->GetVolumeProperty()->GetScalarOpacity();
        vtkColorTransferFunction *colorTransfer=this->currentNode->GetVolumeProperty()->GetRGBTransferFunction();
        colorTransfer->AddRGBPoint(range[0],.3,.3,1.);
        colorTransfer->AddRGBPoint(sumLowIndex,.3,.3,1.);
        colorTransfer->AddRGBPoint(sumLowIndex+.5*(sumHighIndex-sumLowIndex),.3,1.,.3);
        colorTransfer->AddRGBPoint(sumHighIndex,1.,.3,.3);
        colorTransfer->AddRGBPoint(range[1],1,.3,.3);
        
    }
    
        /*
            $::VR($this,colorTransferFunction) AddRGBPoint      [lindex [$::VR($this,pfed_hist) GetRange] 0] 0.3 0.3 1.0
            $::VR($this,colorTransferFunction) AddRGBPoint      [expr [lindex [$::VR($this,pfed_hist) GetRange] 1] * 0.25] 0.3 0.3 1.0
            $::VR($this,colorTransferFunction) AddRGBPoint      [expr [lindex [$::VR($this,pfed_hist) GetRange] 1] * 0.5] 0.3 1.0 0.3
            $::VR($this,colorTransferFunction) AddRGBPoint    [expr [lindex [$::VR($this,pfed_hist) GetRange] 1] * 0.75] 1.0 0.3 0.3
            $::VR($this,colorTransferFunction) AddRGBPoint     [lindex [$::VR($this,pfed_hist) GetRange] 1] 1    .3    .3
        } ;#else
            

        set ::VR($this,volumeProperty) [vtkVolumeProperty New]
        puts "volumeProperty: $::VR($this,volumeProperty)"
            $::VR($this,volumeProperty) SetScalarOpacity $::VR($this,opacityTransferFunction)
            $::VR($this,volumeProperty) SetColor $::VR($this,colorTransferFunction)
            #$::VR($this,volumeProperty) SetGradientOpacity $gradientOpacityTransferFunction
            $::VR($this,volumeProperty) SetInterpolationTypeToNearest
            $::VR($this,volumeProperty) ShadeOff
        $::VR($this,presetSelector) SetPresetVolumeProperty 0 $::VR($this,volumeProperty)

        puts "334"
        #Switch here for MIP and Normal Mode
        if 0 {
            set ::VR($this,volumeMapper) [vtkFixedPointVolumeRayCastMapper New]
            $::VR($this,volumeMapper) SetInput $::VR($this,aImageData)
            }
            
        if 1 {
      set ::VR($this,converter) [vtkImageShiftScale New]
            $::VR($this,converter) SetOutputScalarTypeToUnsignedChar 
            $::VR($this,converter) SetInput $::VR($this,aImageData)
            set ::VR($this,volumeMapper) [vtkVolumeTextureMapper3D New]
            $::VR($this,volumeMapper) SetInput [$::VR($this,converter) GetOutput]
            $::VR($this,volumeMapper) SetSampleDistance 0.1
        
        }
            puts "348"
    $::VR($this,volumeMapper) AddObserver ProgressEvent {puts "progress:"}
        set ::VR($this,volume) [vtkVolume New]
        $::VR($this,volume) SetMapper $::VR($this,volumeMapper)
        $::VR($this,volume) SetProperty $::VR($this,volumeProperty)
        set ::VR($this,matrix) [vtkMatrix4x4 New]
        puts "after setMatrix"
        puts "after cast"
        $::VR($this,aMRMLNODE) GetIJKToRASMatrix $::VR($this,matrix)
        puts "after get"
        $::VR($this,volume) PokeMatrix $::VR($this,matrix)
        puts "PokeMatrix"
        #Add data to vtkKWVolumePropertyWidget
        $::VR($this,vpw) SetVolumeProperty $::VR($this,volumeProperty)
        $::VR($this,vpw) ComponentWeightsVisibilityOff
        #$::VR($this,vpw) Update

        #Add property to RenderWidget
        $::VR($this,renderWidget) AddViewProp $::VR($this,volume)
    $::VR($this,renderWidget) AddObserver ProgressEvent {puts "progress: widget"}
        #Add Histograms to Widget
        [$::VR($this,vpw) GetScalarOpacityFunctionEditor]  SetHistogram $::VR($this,pfed_hist)
        [$::VR($this,vpw) GetScalarColorFunctionEditor] SetHistogram $::VR($this,pfed_hist)
        #No Gradient
        #Update vtkKWVolumePropertyWidget
        $::VR($this,renderWidget) Create
            puts "Render ENDE"
    } ;#else
    puts "proc Load Node Processed"*/
    
    
    
    this->PipelineInitializedOn();
    this->SVP_VolumeProperty->SetVolumeProperty(this->currentNode->GetVolumeProperty());

    /*set ::VR($this,pfed_hist) [vtkKWHistogram New]
        puts "pfed_hist: $$::VR($this,pfed_hist)"
        $::VR($this,pfed_hist) BuildHistogram [[$::VR($this,aImageData) GetPointData] GetScalars] 0*/
}
void vtkVolumeRenderingModuleGUI::InitializePipelineFromMRMLScene()
{
}
void vtkVolumeRenderingModuleGUI::InitializePipelineFromSlicer()
{
}
