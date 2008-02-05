// .NAME vtkSlicerVRGrayscaleHelper - Central logic and GUI behind volume rendering
// .SECTION Description
// vtkSlicerVRGrayscaleHelper is the core class for volume rendering. It creates the pages Treshold, Performance, Cropping and Advanced.
// It also includes the whole rendering algorithm including a special alogrithm for a low resolution rendering
// This class should be split into several different new classes.
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
    //--------------------------------------------------------------------------
    // General
    //--------------------------------------------------------------------------
    // Description:
    // Usual vtk/KWWidgets methods: go to www.vtk.org / www.kwwidgets.org for more details
    static vtkSlicerVRGrayscaleHelper *New();
    vtkTypeRevisionMacro(vtkSlicerVRGrayscaleHelper,vtkSlicerVRHelper);

    // Description:
    // Init the rendering process and the GUI.
    // Call this method before any other method.
    virtual void Init(vtkVolumeRenderingModuleGUI *gui);

    // Description:
    // Update all GUI elements.
    virtual void UpdateGUIElements(void);

    // Description:
    // Process all callbacks beside of the callbacks marked with "Process"
    virtual void ProcessVolumeRenderingEvents(vtkObject *caller,unsigned long eid,void *callData);

    // Description:
    // Callback, processed when the visibility button is pressed
    void ProcessPauseResume(void);



    //--------------------------------------------------------------------------
    // Treshold
    //--------------------------------------------------------------------------
    // Description:
    //Callback, that is processed when the treshold mode is changed
    void ProcessThresholdModeEvents(int id);

    // Description:
    // Callback, that is processed when the range of the treshold is changed
    void ProcessThresholdRange(double notUsed,double notUsedA);

    // Description:
    // Callback, that is processed when one zooms into the treshold range sliders
    void ProcessThresholdZoomIn(void);

    // Description:
    // Callback, that is processed when one resets the treshold range sliders
    void ProcessThresholdReset(void);

    //--------------------------------------------------------------------------
    // Mapper
    //--------------------------------------------------------------------------



    //--------------------------------------------------------------------------
    // Cropping
    //--------------------------------------------------------------------------

    // Description:
    // Callback, that is processed when on of the cropping sliders is moved
    void ProcessCropping(int index, double min,double max);

    // Description:
    // Callback, that is processed when cropping gets enabled or disabled in general
    void ProcessEnableDisableCropping(int cbSelectedState);

    // Description:
    // Callback, that is processed when the visibility of the clipping box is toggled
    void ProcessDisplayClippingBox(int clippingEnabled);

    // Description:
    // Callback, that is processed when the clipping was modified e. g. because of transform node 
    void ProcessClippingModified(void);



    //--------------------------------------------------------------------------
    // Advanced
    //--------------------------------------------------------------------------


    //--------------------------------------------------------------------------
    // Rendering Logic
    //--------------------------------------------------------------------------
    // Description:
    // Create a new volume rendering node using default values
    virtual void InitializePipelineNewCurrentNode(void);

    // Description:
    // Is called at the first renderings.
    virtual void Rendering(void);

    // Description:
    // Is called for all next renderings
    virtual void UpdateRendering(void);

    // Description:
    // Schedule a rendering (called from "after ms")
    void ScheduleRender(void);

    // Description:
    // Schedule a rendering of the lowest stage
    void ScheduleStageZero(void);

    // Description:
    // Set/Get the quality of the rendering to the specific stage
    void SetQuality(int qual)
    {
        this->Quality=qual;
    }
    vtkGetMacro(Quality,int);

    // Description:
    // Called everytime a mouse button is pressed
    void SetButtonDown(int _arg)
    {
        vtkSlicerVRHelperDebug("setbutton %d",_arg);
        this->ButtonDown=(_arg);
    }

protected:
    //--------------------------------------------------------------------------
    // General
    //--------------------------------------------------------------------------
    // Description:
    // Use ::New() to get a new instance.
    vtkSlicerVRGrayscaleHelper(void);

    // Description:
    // Use ->Delete() to delete object
    ~vtkSlicerVRGrayscaleHelper(void);

    // Description:
    //
    vtkKWNotebook *NB_Details;

    // Description:
    //
    vtkKWPushButtonWithLabel *PB_PauseResume;

    // Description:
    //
    vtkSlicerVisibilityIcons *VI_PauseResume;

    //--------------------------------------------------------------------------
    // Treshold
    //--------------------------------------------------------------------------

    // Description:
    //
    vtkKWMenuButtonWithLabel *MB_ThresholdMode;

    // Description:
    //
    vtkSlicerVRMenuButtonColorMode *VRMB_ColorMode;

    // Description:
    //
    vtkKWRange *RA_RampRectangleHorizontal;

    // Description:
    //
    vtkKWRange *RA_RampRectangleVertical;

    // Description:
    //
    vtkKWPushButton *PB_ThresholdZoomIn;

    // Description:
    //
    vtkKWPushButton *PB_Reset;

    // Description:
    //
    int ThresholdMode;

    // Description:
    //
    void CreateThreshold(void);

    // Description:
    //
    void DestroyTreshold(void);

    //--------------------------------------------------------------------------
    // Mapper
    //--------------------------------------------------------------------------

    // Description:
    //
    vtkKWFrameWithLabel *MappersFrame;

    // Description:
    //
    vtkKWCheckButtonWithLabel *CB_TextureLow;

    // Description:
    //
    vtkKWCheckButtonWithLabel *CB_TextureHigh;

    // Description:
    //
    vtkKWCheckButtonWithLabel *CB_RayCast;

    // Description:
    //
    vtkKWCheckButtonWithLabel *CB_InteractiveFrameRate;

    // Description:
    //
    vtkKWScaleWithLabel *SC_Framerate;

    // Description:
    //
    void CreatePerformance(void);

    // Description:
    //
    void DestroyPerformance(void);

    //--------------------------------------------------------------------------
    // Cropping
    //--------------------------------------------------------------------------

    // Description:
    //
    vtkSlicerNodeSelectorWidget *NS_TransformNode;

    // Description:
    //
    vtkKWCheckButtonWithLabel *CB_Cropping;

    // Description:
    //
    vtkKWRange *RA_Cropping[3];

    // Description:
    //
    vtkSlicerBoxWidget *BW_Clipping;

    // Description:
    //
    vtkKWCheckButtonWithLabel *CB_Clipping;

    // Description:
    //
    vtkMRMLLinearTransformNode *CurrentTransformNodeCropping;

    // Description:
    //
    vtkTransform *AdditionalClippingTransform;

    // Description:
    //
    vtkTransform *InverseAdditionalClippingTransform;

    // Description:
    //
    double ColorsClippingHandles[6][3];

    // Description:
    //
    int NoSetRangeNeeded;

    // Description:
    //
    void CreateCropping(void);

    // Description:
    //
    void DestroyCropping(void);

    // Description:
    //
    void UpdateQualityCheckBoxes(void);

    // Description:
    //
    void ConvertWorldToBoxCoordinates(double* inputOutput);

    // Description:
    //
    void ConvertBoxCoordinatesToWorld(double* inputOutput);


    //--------------------------------------------------------------------------
    // Advanced
    //--------------------------------------------------------------------------

    // Description:
    //
    vtkKWHistogramSet *Histograms;

    // Description:
    //
    vtkSlicerVolumePropertyWidget *SVP_VolumeProperty;

    // Description:
    //
    void UpdateSVP(void);

    //--------------------------------------------------------------------------
    // Rendering Logic
    //--------------------------------------------------------------------------

    // Description:
    //
    int ScheduleMask[3];

    // Description:
    //
    vtkRenderer *RenViewport;

    // Description:
    //
    vtkRenderer *RenPlane;

    // Description:
    //
    int RenderPlane;

    // Description:
    //
    vtkSlicerVolumeTextureMapper3D *MapperTexture;

    // Description:
    //
    vtkSlicerFixedPointVolumeRayCastMapper *MapperRaycast;

    // Description:
    //Initial Factor for Interactive Rendering
    double InitialDropLowRes;

    // Description:
    //Factor during last low Resolution Rendering

    double FactorLastLowRes;

    // Description:
    //Time for the last High Resolution Rendering
    double LastTimeHighRes;

    // Description:
    //Time for the last Low Resolution Rendering
    double LastTimeLowRes;

    // Description:
    //Timer
    vtkTimerLog *Timer;

    // Description:
    //Which time would we like to achieve
    double GoalLowResTime;

    // Description:
    //Area in which no change in Factor will be made.
    double PercentageNoChange;

    // Description:
    //How long to wait, before Rendering in High Resolution
    double TimeToWaitForHigherStage;

    // Description:
    //0 interactive, 1 High Resolution Texture VR, 2 SW Ray Cast
    int CurrentStage;

    // Description:
    //
    int Quality;

    // Description:
    //
    int ButtonDown;

    // Description:
    //
    int Scheduled;

    // Description:
    //Flag if next Render is a High Resolution Render
    int NextRenderHighResolution;

    // Description:
    //
    int IgnoreStepZero;
    //BTX

    // Description:
    //
    std::string EventHandlerID;

    // Description:
    //
    std::string StageZeroEventHandlerID;
    //ETX

    // Description:
    //
    double SavedStillRate;
    //Description:
    //Indicates if the VolumeRendering is Paused or not
    int RenderingPaused;
    // Description:
    //
    double SampleDistanceHighRes;

    // Description:
    //
    double SampleDistanceHighResImage;

    // Description:
    //
    double SampleDistanceLowRes;

    // Description:
    //
    void CalculateAndSetSampleDistances();

    // Description:
    //
    void ResetRenderingAlgorithm(void);


    // Description:
    //
    void CheckAbort(void);

    // Description:
    //
    void AdjustMapping(void);



private:
    // Description:
    // Caution: Not implemented
    vtkSlicerVRGrayscaleHelper(const vtkSlicerVRGrayscaleHelper&);//not implemented
    void operator=(const vtkSlicerVRGrayscaleHelper&);//not implemented

};
#endif
