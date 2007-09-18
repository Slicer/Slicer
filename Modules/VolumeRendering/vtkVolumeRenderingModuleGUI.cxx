#include "vtkVolumeRenderingModuleGUI.h"
#include "vtkIndent.h"
#include <ostream>

vtkVolumeRenderingModuleGUI::vtkVolumeRenderingModuleGUI(void)
{
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
