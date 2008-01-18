#ifndef __vtkSlicerVRGrayscaleHelper_h
#define __vtkSlicerVRGrayscaleHelper_h

#include "vtkVolumeRenderingModule.h"
#include "vtkSlicerVRHelper.h"
#include <string>
#include "vtkKWRange.h"
                                                                       

class vtkKWHistogramSet;
class vtkSlicerVolumePropertyWidget;
class vtkRenderer;
class vtkSlicerVolumeTextureMapper3D;
class vtkTimerLog;
class vtkSlicerFixedPointVolumeRayCastMapper;
class vtkKWMenuButtonWithSpinButtonsWithLabel;
class vtkKWScaleWithLabel;
class vtkKWCheckButtonWithLabel;
class vtkKWFrameWithLabel;
class vtkKWNotebook;
class vtkKWMenuButtonWithLabel;
class vtkKWPushButton;
class vtkBoxWidget;
class vtkSlicerColorDisplayWidget;

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
    void ScheduleStageZero(void);
    void SetQuality(int qual)
    {
        this->Quality=qual;
    }
    vtkGetMacro(Quality,int);
    void SetButtonDown(int _arg)
    {
        vtkSlicerVRHelperDebug("setbutton %d",_arg);
        this->ButtonDown=(_arg);
    }
    //vtkSetMacro(ButtonDown,int);

    
    void Cropping(int index, double min,double max);
    void ProcessThresholdModeEvents(int id);
    void ProcessColorModeEvents(int id);
    void ProcessThresholdRange(double notUsed,double notUsedA);
    void ProcessThresholdZoomIn(void);
    void ProcessThresholdReset(void);
    void ProcessEnableDisableCropping(int cbSelectedState);
    void ProcessEnableDisableClippingPlanes(int clippingEnabled);
    void ProcessSelection(void);
    void ProcessPauseResume(void);

protected:
    vtkSlicerVRGrayscaleHelper(void);
    ~vtkSlicerVRGrayscaleHelper(void);
    vtkSlicerVRGrayscaleHelper(const vtkSlicerVRGrayscaleHelper&);//not implemented
    void operator=(const vtkSlicerVRGrayscaleHelper&);//not implemented

    //GUI
    vtkKWHistogramSet *Histograms;
    vtkSlicerVolumePropertyWidget *SVP_VolumeProperty;

    //GUI1
    //vtkKWMenuButtonWithSpinButtonsWithLabel *MB_Quality;
    vtkKWFrameWithLabel *MappersFrame;
    vtkKWNotebook *NB_Details;
    vtkKWCheckButtonWithLabel *CB_TextureLow;
    vtkKWCheckButtonWithLabel *CB_TextureHigh;
    vtkKWCheckButtonWithLabel *CB_RayCast;
    vtkKWCheckButtonWithLabel *CB_InteractiveFrameRate;
    vtkKWScaleWithLabel *SC_Framerate;
    // Cropping GUI
    vtkKWCheckButtonWithLabel *CB_Cropping;
    vtkKWRange *RA_Cropping[3];

    vtkBoxWidget *BW_Clipping;
    vtkKWCheckButtonWithLabel *CB_Clipping;

    //ColorDisplay Widget
    vtkSlicerColorDisplayWidget *ColorDisplay;


    //ThresholdGUI
    vtkKWMenuButtonWithLabel *MB_ThresholdMode;
    vtkKWMenuButtonWithLabel *MB_ColorMode;
    vtkKWRange *RA_RampRectangleHorizontal;
    vtkKWRange *RA_RampRectangleVertical;

    vtkKWPushButton *PB_ThresholdZoomIn;
    vtkKWPushButton *PB_Reset;
    int ColorMode;
    int ThresholdMode;

    //Pause Resume
    vtkKWPushButton *PB_PauseResume;


    
    
    int ScheduleMask[3];


    //Render logic
    vtkRenderer *renViewport;
    vtkRenderer *renPlane;
    int RenderPlane;
    vtkSlicerVolumeTextureMapper3D *MapperTexture;
    vtkSlicerFixedPointVolumeRayCastMapper *MapperRaycast;

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
    double PercentageNoChange;
    //How long to wait, before Rendering in High Resolution
    double TimeToWaitForHigherStage;
    //0 interactive, 1 High Resolution Texture VR, 2 SW Ray Cast
    int currentStage;

    int Quality;
    int ButtonDown;

    int scheduled;
    //Flag if next Render is a High Resolution Render
    int NextRenderHighResolution;
    int IgnoreStepZero;
    //BTX
    std::string EventHandlerID;
    std::string StageZeroEventHandlerID;
    //ETX
    
    double SavedStillRate;
    void UpdateSVP(void);

    void CheckAbort(void);

    void AdjustMapping(void);

    void UpdateQualityCheckBoxes(void);
    void CreateCropping(void);

    void CreateThreshold(void);
    void ResetRenderingAlgorithm(void);


    double ColorsClippingHandles[6][3];

    int NoSetRangeNeeded;

    //Description;
    //Indicates if the VolumeRendering is Paused or not
    int RenderingPaused;



};
#endif
