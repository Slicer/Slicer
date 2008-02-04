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
class vtkSlicerBoxWidget;
class vtkSlicerColorDisplayWidget;
class vtkSlicerNodeSelectorWidget;
class vtkMRMLLinearTransformNode;
class vtkTransform;
class vtkSlicerVisibilityIcons;
class vtkKWPushButtonWithLabel;
class vtkSlicerVRMenuButtonColorMode;


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

    
    void Cropping(int index, double min,double max);
    void ProcessThresholdModeEvents(int id);
    void ProcessThresholdRange(double notUsed,double notUsedA);
    void ProcessThresholdZoomIn(void);
    void ProcessThresholdReset(void);
    void ProcessEnableDisableCropping(int cbSelectedState);
    void ProcessEnableDisableClippingPlanes(int clippingEnabled);
    void ProcessSelection(void);
    void ProcessPauseResume(void);
    void ProcessClippingModified(void);

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
    vtkSlicerNodeSelectorWidget *NS_TransformNode;
    vtkKWCheckButtonWithLabel *CB_Cropping;
    vtkKWRange *RA_Cropping[3];
    vtkSlicerBoxWidget *BW_Clipping;
    vtkKWCheckButtonWithLabel *CB_Clipping;

    vtkMRMLLinearTransformNode *CurrentTransformNodeCropping;
     vtkTransform *AdditionalClippingTransform;
     vtkTransform *InverseAdditionalClippingTransform;

    //ColorDisplay Widget
    vtkKWCheckButtonWithLabel *CB_LabelmapMode;
    vtkSlicerColorDisplayWidget *ColorDisplay;
    vtkKWPushButton *PB_LabelsSelectAll;
    vtkKWPushButton *PB_LabelsDeselectAll;


    //ThresholdGUI
    vtkKWMenuButtonWithLabel *MB_ThresholdMode;
    vtkSlicerVRMenuButtonColorMode *VRMB_ColorMode;
    vtkKWRange *RA_RampRectangleHorizontal;
    vtkKWRange *RA_RampRectangleVertical;

    vtkKWPushButton *PB_ThresholdZoomIn;
    vtkKWPushButton *PB_Reset;
    int ColorMode;
    int ThresholdMode;

    //Pause Resume
    vtkKWPushButtonWithLabel *PB_PauseResume;
    vtkSlicerVisibilityIcons *VI_PauseResume;


    
    
    int ScheduleMask[3];


    //Render logic
    vtkRenderer *RenViewport;
    vtkRenderer *RenPlane;
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
    vtkTimerLog *Timer;
    //Which time would we like to achieve
    double GoalLowResTime;
    //Area in which no change in Factor will be made.
    double PercentageNoChange;
    //How long to wait, before Rendering in High Resolution
    double TimeToWaitForHigherStage;
    //0 interactive, 1 High Resolution Texture VR, 2 SW Ray Cast
    int CurrentStage;

    int Quality;
    int ButtonDown;

    int Scheduled;
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

    void CreateThreshold(void);
    void DestroyTreshold(void);

    void CreatePerformance(void);
    void DestroyPerformance(void);
    
    void CreateCropping(void);
    void DestroyCropping(void);
    void UpdateQualityCheckBoxes(void);

    void CreateLabelmap(void);
    void DestroyLabelmap(void);


    double ColorsClippingHandles[6][3];

    int NoSetRangeNeeded;
    void ResetRenderingAlgorithm(void);

    //Description;
    //Indicates if the VolumeRendering is Paused or not
    int RenderingPaused;

    void CalculateAndSetSampleDistances();
    double SampleDistanceHighRes;
    double SampleDistanceHighResImage;
    double SampleDistanceLowRes;

    void ConvertWorldToBoxCoordinates(double* inputOutput);
    void ConvertBoxCoordinatesToWorld(double* inputOutput);


};
#endif
