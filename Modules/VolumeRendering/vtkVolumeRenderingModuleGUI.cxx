#include "vtkVolumeRenderingModuleGUI.h"

#include <ostream>
#include "vtkMRMLVolumeRenderingNode.h"
#include "vtkVolumeTextureMapper3D.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkIndent.h"
#include "vtkPointData.h"
#include "vtkPiecewisefunction.h"
#include "vtkKWPiecewiseFunctionEditor.h"
#include "vtkKWColorTransferFunctionEditor.h"
#include "vtkImageGradientMagnitude.h"


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
    this->NS_ImageData->NoneEnabledOn();
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
    this->NS_VolumeRenderingDataScene->NoneEnabledOn();
    this->NS_VolumeRenderingDataScene->SetLabelText("VolumeRenderingNode from Scene");
    app->Script("pack %s -side top -anchor e -padx 2 -pady 2",this->NS_VolumeRenderingDataScene->GetWidgetName());
    //Missing: Load from file

    this->PB_LoadVolumeRenderingDataScene=vtkKWPushButton::New();
    this->PB_LoadVolumeRenderingDataScene->SetParent(loadSaveDataFrame->GetFrame());
    this->PB_LoadVolumeRenderingDataScene->Create();
    this->PB_LoadVolumeRenderingDataScene->SetText("Load VolumeRenderingNode");
    app->Script("pack %s -side top -anchor e -padx 2 -pady 2",this->PB_LoadVolumeRenderingDataScene->GetWidgetName());
    
    this->PB_SaveCurrentAsNew=vtkKWPushButton::New();
    this->PB_SaveCurrentAsNew->SetParent(loadSaveDataFrame->GetFrame());
    this->PB_SaveCurrentAsNew->Create();
    this->PB_SaveCurrentAsNew->SetText("SaveCurrentSettingAsNew");
    app->Script("pack %s -side top -anchor e -padx 2 -pady 2",this->PB_SaveCurrentAsNew->GetWidgetName());

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

    //Histogram
    this->HIST_Opacity=vtkKWHistogram::New();
    this->HIST_Gradient=vtkKWHistogram::New();
      
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
    this->PB_SaveCurrentAsNew->AddObserver(vtkKWPushButton::InvokedEvent,(vtkCommand*)this->GUICallbackCommand);
    this->PB_LoadVolumeRenderingDataScene->AddObserver(vtkKWPushButton::InvokedEvent,(vtkCommand*)this->GUICallbackCommand);
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
     else if(callerObject==this->PB_LoadImageData&&event==vtkKWPushButton::InvokedEvent)
     {
         this->InitializePipeline();
         //this->currentNode->InitializePipeline(vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetDisplayNode());

     }
     else if (callerObject==this->PB_SaveCurrentAsNew&&event==vtkKWPushButton::InvokedEvent)
     {
         this->currentNode->HideFromEditorsOff();
         this->GetLogic()->GetMRMLScene()->AddNode(this->currentNode);


     }
     else if (callerObject==this->PB_LoadVolumeRenderingDataScene&&event==vtkKWPushButton::InvokedEvent)
     {
         //No Node selected
         if(this->PipelineInitialized==0)
         {
             vtkDebugMacro("VolumeRenderingNode selected before Input selected");
            return;
         }
         //Change everthing for new Pointer
         //Delete ol one
         this->currentNode->Delete();
        this->currentNode=vtkMRMLVolumeRenderingNode::SafeDownCast(this->NS_VolumeRenderingDataScene->GetSelected());
        this->SVP_VolumeProperty->SetVolumeProperty(currentNode->GetVolumeProperty());
         vtkImageData* imageData=vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetImageData();
        //Update Input of Mapper
        this->currentNode->GetMapper()->SetInput(imageData);

    //Take care about histogram and automatic mapping
    //Add Histogram to GUI
    this->HIST_Opacity->BuildHistogram(imageData->GetPointData()->GetScalars(),0);
    this->SVP_VolumeProperty->GetScalarOpacityFunctionEditor()->SetHistogram(this->HIST_Opacity);
    this->SVP_VolumeProperty->GetScalarColorFunctionEditor()->SetHistogram(this->HIST_Opacity);


    vtkImageGradientMagnitude *grad=vtkImageGradientMagnitude::New();
    grad->SetDimensionality(3);
    grad->SetInput(imageData);
    grad->Update();
    vtkImageData *test=grad->GetOutput();
    this->HIST_Gradient->BuildHistogram(test->GetPointData()->GetScalars(),0);
    this->SVP_VolumeProperty->GetGradientOpacityFunctionEditor()->SetHistogram(this->HIST_Gradient);
    vtkPiecewiseFunction *gradFunction=this->currentNode->GetVolumeProperty()->GetGradientOpacity();
    gradFunction->AdjustRange(test->GetPointData()->GetScalars()->GetRange());

     this->SVP_VolumeProperty->SetVolumeProperty(this->currentNode->GetVolumeProperty());
     this->SVP_VolumeProperty->Modified();
            
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
    this->L_Status->Modified();
    this->Modified();

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
    //Update Input of Mapper
    this->currentNode->GetMapper()->SetInput(imageData);

    //Take care about histogram and automatic mapping
    //Add Histogram to GUI
    this->HIST_Opacity->BuildHistogram(imageData->GetPointData()->GetScalars(),0);
    this->SVP_VolumeProperty->GetScalarOpacityFunctionEditor()->SetHistogram(this->HIST_Opacity);
    this->SVP_VolumeProperty->GetScalarColorFunctionEditor()->SetHistogram(this->HIST_Opacity);
    //automatic Mode
    if(vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetLabelMap()==1)
    {
        vtkWarningMacro("Not yet implemented");
    }
    //automatic Mode for LabelMaps
    else
    {
        double totalOccurance=this->HIST_Opacity->GetTotalOccurence();
        double tresholdLow=totalOccurance*0.2;
        double tresholdHigh=totalOccurance*0.8;
        double *range=new double[2];
        this->HIST_Opacity->GetRange(range);
        double tresholdLowIndex=range[0];
        double sumLowIndex=0;
        double tresholdHighIndex=range[0];
        double sumHighIndex=0;
        //calculate distance
        double bin_width = (range[1] == range[0] ? 1 :(double)this->HIST_Opacity->GetNumberOfBins() / (range[1] - range[0]));
        while (sumLowIndex<tresholdLow)
        {
            sumLowIndex+=this->HIST_Opacity->GetOccurenceAtValue(tresholdLowIndex);
            tresholdLowIndex+=bin_width;
        }
        while(sumHighIndex<tresholdHigh)
        {
            sumHighIndex+=this->HIST_Opacity->GetOccurenceAtValue(tresholdHighIndex);
            tresholdHighIndex+=bin_width;

        }
        vtkPiecewiseFunction *opacity=this->currentNode->GetVolumeProperty()->GetScalarOpacity();
        opacity->RemoveAllPoints();
        opacity->AddPoint(range[0],0.);
        opacity->AddPoint(tresholdLowIndex,0.0);
        opacity->AddPoint(tresholdHighIndex,0.2);
        opacity->AddPoint(range[1],0.2);
        vtkColorTransferFunction *colorTransfer=this->currentNode->GetVolumeProperty()->GetRGBTransferFunction();
        colorTransfer->RemoveAllPoints();
        colorTransfer->AddRGBPoint(range[0],.3,.3,1.);
        colorTransfer->AddRGBPoint(tresholdLowIndex,.3,.3,1.);
        colorTransfer->AddRGBPoint(tresholdLowIndex+.5*(tresholdHighIndex-tresholdLowIndex),.3,1.,.3);
        colorTransfer->AddRGBPoint(tresholdHighIndex,1.,.3,.3);
        colorTransfer->AddRGBPoint(range[1],1,.3,.3);
        vtkImageGradientMagnitude *grad=vtkImageGradientMagnitude::New();
        grad->SetDimensionality(3);
        grad->SetInput(imageData);
        grad->Update();

        vtkImageData *test=grad->GetOutput();
        this->HIST_Gradient->BuildHistogram(test->GetPointData()->GetScalars(),0);
        this->SVP_VolumeProperty->GetGradientOpacityFunctionEditor()->SetHistogram(this->HIST_Gradient);
        vtkPiecewiseFunction *gradFunction=this->currentNode->GetVolumeProperty()->GetGradientOpacity();
        gradFunction->AdjustRange(test->GetPointData()->GetScalars()->GetRange());


        //Add VolumeProperty To Gui
        this->SVP_VolumeProperty->SetVolumeProperty(this->currentNode->GetVolumeProperty());
    
    this->PipelineInitializedOn();
    }//else
}//method
void vtkVolumeRenderingModuleGUI::InitializePipelineFromMRMLScene()
{
}
void vtkVolumeRenderingModuleGUI::InitializePipelineFromSlicer()
{
}
