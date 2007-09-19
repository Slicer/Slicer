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
    this->currentNode->Setmapper(vtkVolumeTextureMapper3D::New());
    this->currentNode->SetvolumeProperty(vtkVolumeProperty::New());
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
    os<<indent<<"print volumeRendering"<<endl;
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
    vtkSlicerModuleCollapsibleFrame *loadSaveDataFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    loadSaveDataFrame->SetParent (this->UIPanel->GetPageWidget("VolumeRendering"));
    loadSaveDataFrame->Create();
    loadSaveDataFrame->ExpandFrame();
    loadSaveDataFrame->SetLabelText("Testing");
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
        loadSaveDataFrame->GetWidgetName(), this->UIPanel->GetPageWidget("VolumeRendering")->GetWidgetName());
    this->testingPB= vtkKWPushButton::New();
    this->testingPB->SetParent(loadSaveDataFrame->GetFrame());
    this->testingPB->Create();
    this->testingPB->SetText("Testing");
    app->Script("pack %s -side top -anchor nw -padx 2 -pady 2",this->testingPB->GetWidgetName());
    
    //Delete frames
    loadSaveDataFrame->Delete();
}

void vtkVolumeRenderingModuleGUI::TearDownGUI(void)
{

}

void vtkVolumeRenderingModuleGUI::CreateModuleEventBindings(void)
{
}

void vtkVolumeRenderingModuleGUI::ReleaseModuleEventBindings(void)
{

}

void vtkVolumeRenderingModuleGUI::AddGUIObservers(void)
{
    this->testingPB->AddObserver(vtkKWPushButton::InvokedEvent,(vtkCommand *)this->GUICallbackCommand );
}
void vtkVolumeRenderingModuleGUI::RemoveGUIObservers(void)
{

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
     vtkKWPushButton *callerObject=vtkKWPushButton::SafeDownCast(caller);
     if(callerObject==this->testingPB&&event==vtkKWPushButton::InvokedEvent)
     {
         char* result=new char();
         int size= vtkMRMLVolumeRenderingDisplayNode::getPiecewiseFunctionString(this->currentNode->GetvolumeProperty()->GetScalarOpacity(),result);
        vtkDebugMacro("size"<<size);
         
         //vtkMRMLVolumeRenderingDisplayNode::
     }
}
void vtkVolumeRenderingModuleGUI::ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData)
{
}

void vtkVolumeRenderingModuleGUI::Enter(void)
{
}

void vtkVolumeRenderingModuleGUI::Exit(void)
{
}

void vtkVolumeRenderingModuleGUI::SetViewerWidget(vtkSlicerViewerWidget *viewerWidget)
{
}
void vtkVolumeRenderingModuleGUI::SetInteractorStyle(vtkSlicerViewerInteractorStyle *interactorStyle)
{
}
