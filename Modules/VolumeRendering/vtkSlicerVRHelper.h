#ifndef __vtkSlicerVRHelper_h
#define __vtkSlicerVRHelper_h

#include "vtkVolumeRenderingModule.h"
#include "vtkKWObject.h"

# define vtkSlicerVRHelperDebug(message,format)                                        \
  {                                                                             \
  if (this->GetTCLDebug())                                                         \
    {                                                                           \
        this->Script("puts \""message"\"",format);                              \
    }                                                                           \
  }
class vtkVolumeRenderingModuleGUI;
class vtkCallbackCommand;
class vtkVolume;

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerVRHelper :public vtkKWObject
{
public:
    static vtkSlicerVRHelper *New();
    vtkTypeRevisionMacro(vtkSlicerVRHelper,vtkKWObject);
    virtual void UpdateGUIElements(void);
    virtual void Init(vtkVolumeRenderingModuleGUI *gui);
    virtual void InitializePipelineNewCurrentNode(void);
    virtual void ShutdownPipeline(void);
    virtual void UpdateRendering(void);
    vtkSetMacro(TCLDebug,int);
    vtkGetMacro(TCLDebug, int);


protected:
    vtkSlicerVRHelper(void);
    ~vtkSlicerVRHelper(void);
    vtkSlicerVRHelper(const vtkSlicerVRHelper&);//not implemented
    void operator=(const vtkSlicerVRHelper&);//not implemented

    //Rendering pipeline
    vtkVolume *Volume;
    int TCLDebug;

    vtkVolumeRenderingModuleGUI *Gui;
    //Callbacks
    void SetInVolumeRenderingCallbackFlag (int flag) {
        this->InVolumeRenderingCallbackFlag = flag;
    }
    vtkGetMacro(InVolumeRenderingCallbackFlag, int);
    vtkCallbackCommand* VolumeRenderingCallbackCommand;
    int InVolumeRenderingCallbackFlag;
    static void VolumeRenderingCallback( vtkObject *__caller,unsigned long eid, void *__clientData, void *callData );
    virtual void ProcessVolumeRenderingEvents(vtkObject *caller,unsigned long eid,void *callData);

    virtual void Rendering(void);

};
#endif
