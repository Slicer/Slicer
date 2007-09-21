#include "vtkVolumeRenderingModuleGUI.h"

#include <ostream>
#include "vtkMRMLVolumeRenderingDisplayNode.h"
#include "vtkVolumeTextureMapper3D.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkIndent.h"


vtkVolumeRenderingModuleGUI::vtkVolumeRenderingModuleGUI(void)
{
    //In Debug Mode
    this->DebugOn();
    this->currentNode=vtkMRMLVolumeRenderingDisplayNode::New();
   
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
    //Testing Frame
    //
    vtkSlicerModuleCollapsibleFrame *loadSaveDataFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    loadSaveDataFrame->SetParent (this->UIPanel->GetPageWidget("VolumeRendering"));
    loadSaveDataFrame->Create();
    loadSaveDataFrame->ExpandFrame();
    loadSaveDataFrame->SetLabelText("Testing");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
        loadSaveDataFrame->GetWidgetName(), this->UIPanel->GetPageWidget("VolumeRendering")->GetWidgetName());
    
    //Testing Pushbutton
    this->PB_Testing= vtkKWPushButton::New();
    this->PB_Testing->SetParent(loadSaveDataFrame->GetFrame());
    this->PB_Testing->Create();
    this->PB_Testing->SetText("Testing");
    app->Script("pack %s -side top -anchor e -padx 2 -pady 2",this->PB_Testing->GetWidgetName());
    
    //NodeSelector And Button
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
         //char* result=this->currentNode->getPiecewiseFunctionString(this->currentNode->GetVolumeProperty()->GetScalarOpacity());
           
   char* pass="vtkPiecewiseFunction#6#0#0.2#1#0.1#2#0.3\0";
   this->currentNode->GetPiecewiseFunctionFromString(pass,this->currentNode->GetVolumeProperty()->GetScalarOpacity());
   char* result=this->currentNode->getPiecewiseFunctionString(this->currentNode->GetVolumeProperty()->GetScalarOpacity());
   if(strcmp(pass,result)!=0)
   {
       vtkErrorMacro("Error in writing reading");
   }
        vtkDebugMacro("size"<<&result);
        return;
         
         //vtkMRMLVolumeRenderingDisplayNode::
     }
     if(callerObject==this->PB_LoadImageData&&event==vtkKWPushButton::InvokedEvent)
     {

         this->currentNode->InitializePipeline(vtkMRMLScalarVolumeNode::SafeDownCast(this->NS_ImageData->GetSelected())->GetDisplayNode());

     }
     //if(callerObject==this->

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
    //Update the NodeSelector
    this->NS_ImageData->SetMRMLScene(this->GetLogic()->GetMRMLScene());
    this->NS_ImageData->SetNodeClass("vtkMRMLScalarVolumeNode","","","");
    this->NS_ImageData->UpdateMenu();
}
void vtkVolumeRenderingModuleGUI::SetViewerWidget(vtkSlicerViewerWidget *viewerWidget)
{
}
void vtkVolumeRenderingModuleGUI::SetInteractorStyle(vtkSlicerViewerInteractorStyle *interactorStyle)
{
}
