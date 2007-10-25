#include "vtkSlicerVRHelper.h"
#include "vtkVolumeRenderingModuleGUI.h"
#include "vtkFixedPointVolumeRayCastMapper.h"
#include "vtkVolume.h"
#include "vtkVolumeTextureMapper3D.h"
#include "vtkCallbackCommand.h"


vtkCxxRevisionMacro(vtkSlicerVRHelper, "$Revision: 1.46 $");
vtkStandardNewMacro(vtkSlicerVRHelper);

vtkSlicerVRHelper::vtkSlicerVRHelper(void)
{
    this->VolumeRenderingCallbackCommand=vtkCallbackCommand::New();
    this->VolumeRenderingCallbackCommand->SetClientData(reinterpret_cast<void *>(this));
    this->VolumeRenderingCallbackCommand->SetCallback(vtkSlicerVRHelper::VolumeRenderingCallback);
    this->Volume=NULL;
}

vtkSlicerVRHelper::~vtkSlicerVRHelper(void)
{
    if(this->Volume!=NULL)
    {
        vtkKWRenderWidget *renderWidget= this->Gui->GetApplicationGUI()->GetViewerWidget()->GetMainViewer();
        renderWidget->RemoveViewProp(this->Volume);
        renderWidget->Render();
        this->Volume->Delete();
        this->Volume=NULL;
    }
}

void vtkSlicerVRHelper::InitializePipelineNewCurrentNode()
{
    
   
}
void vtkSlicerVRHelper::Rendering()
{
   
}

void vtkSlicerVRHelper::UpdateRendering()
{
}

void vtkSlicerVRHelper::ShutdownPipeline()
{
   
}


void vtkSlicerVRHelper::VolumeRenderingCallback( vtkObject *caller, unsigned long eid, void *clientData, void *callData )
{
    vtkSlicerVRHelper *self = reinterpret_cast<vtkSlicerVRHelper *>(clientData);


    if (self->GetInVolumeRenderingCallbackFlag())
    {
#ifdef _DEBUG
        vtkDebugWithObjectMacro(self, "In vtkVOlumeRendering *********GUICallback called recursively?");
#endif
        //return;
    }

    vtkDebugWithObjectMacro(self, "In vtkVolumeREndering GUICallback");

    self->SetInVolumeRenderingCallbackFlag(1);
    self->ProcessVolumeRenderingEvents(caller, eid, callData);
    self->SetInVolumeRenderingCallbackFlag(0);

}

void vtkSlicerVRHelper::UpdateGUIElements(void)
{
}
void vtkSlicerVRHelper::ProcessVolumeRenderingEvents(vtkObject *caller, unsigned long eid, void *callData)
{
}
void vtkSlicerVRHelper::Init(vtkVolumeRenderingModuleGUI *gui)
{
    this->Gui=gui;
    this->SetApplication(this->Gui->GetApplication());
}
