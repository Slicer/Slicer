#ifndef __vtkSlicerVRGrayscaleHelper_h
#define __vtkSlicerVRGrayscaleHelper_h

#include "vtkVolumeRenderingModule.h"
#include "vtkSlicerVRHelper.h"
#include <string>

class vtkKWHistogramSet;
class vtkSlicerVolumePropertyWidget;
class vtkRenderer;
class vtkVolumeTextureMapper3D;
class vtkTimerLog;
class vtkFixedPointVolumeRayCastMapper;
class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerVRGrayscaleHelper :public vtkSlicerVRHelper
{
public:
    static vtkSlicerVRGrayscaleHelper *New();
    vtkTypeRevisionMacro(vtkSlicerVRGrayscaleHelper,vtkSlicerVRHelper);
    virtual void Init(vtkVolumeRenderingModuleGUI *gui);
    virtual void UpdateGUIElements(void);
    virtual void InitializePipelineNewCurrentNode(void);
    virtual void Rendering(void);
    virtual void UpdateRendering(void);
    virtual void ShutdownPipeline(void);
    virtual void ProcessVolumeRenderingEvents(vtkObject *caller,unsigned long eid,void *callData);
    void ScheduleRender(void);

protected:
    vtkSlicerVRGrayscaleHelper(void);
    ~vtkSlicerVRGrayscaleHelper(void);
    vtkSlicerVRGrayscaleHelper(const vtkSlicerVRGrayscaleHelper&);//not implemented
    void operator=(const vtkSlicerVRGrayscaleHelper&);//not implemented

    //GUI
    vtkKWHistogramSet *Histograms;
    vtkSlicerVolumePropertyWidget *SVP_VolumeProperty;


    //Render logic
    vtkRenderer *renViewport;
    vtkRenderer *renPlane;
    int RenderPlane;
    vtkVolumeTextureMapper3D *MapperTexture;
    vtkFixedPointVolumeRayCastMapper *MapperRaycast;

        //Initial Factor for Interactive Rendering
    double InitialDropLowRes;
    //Factor during last low Resolution Rendering
    double FactorLastLowRes;
    //Time for the last High Resolution Rendering
    double LastTimeHighRes;
    //Time for the last Low Resolution Rendering
    double LastTimeLowRes;
    //Timer
    vtkTimerLog *timer;
    //Which time would we like to achieve
    double GoalLowResTime;
    //Area in which no change in Factor will be made.
    int PercentageNoChange;
    //How long to wait, before Rendering in High Resolution
    double TimeToWaitForHigherStage;
    //0 interactive, 1 High Resolution Texture VR, 2 SW Ray Cast
    int currentStage;

    int scheduled;
    //Flag if next Render is a High Resolution Render
    int NextRenderHighResolution;
    //BTX
    std::string EventHandlerID;
    //ETX

    void UpdateSVP(void);

    void CheckAbort(void);
    int Scheduled;

};
#endif
