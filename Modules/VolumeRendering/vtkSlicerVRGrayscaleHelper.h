// .NAME vtkSlicerVRGrayscaleHelper - Central logic and GUI behind volume rendering
// .SECTION Description
// vtkSlicerVRGrayscaleHelper is the core class for volume rendering. It creates the pages Treshold, Performance, Cropping and Advanced.
// It also includes the whole rendering algorithm including a special alogrithm for a low resolution rendering
// This class should be split into several different new classes.
#ifndef __vtkSlicerVRGrayscaleHelper_h
#define __vtkSlicerVRGrayscaleHelper_h

#include "vtkVolumeRendering.h"
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


class VTK_VOLUMERENDERING_EXPORT vtkSlicerVRGrayscaleHelper :public vtkSlicerVRHelper
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
    virtual void Init(vtkVolumeRenderingGUI *gui);

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

    // Description:
    // Workaround to get knowledge about resize of the renderwidget. TODO: Should get into KWWidgets.
    void ProcessConfigureCallback(void);



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
    // Description:
    // Set /Get methods for sample distance. Meant to influence the sample distance during runtime, in case artefacts occur.
    vtkSetMacro(SampleDistanceHighRes, double);
    vtkGetMacro(SampleDistanceHighRes, double);

    vtkSetMacro(SampleDistanceHighResImage, double);
    vtkGetMacro(SampleDistanceHighResImage, double);

    vtkSetMacro(SampleDistanceLowRes, double);
    vtkGetMacro(SampleDistanceLowRes, double);

    vtkSetMacro(SampleDistanceFactor, double);
    vtkGetMacro(SampleDistanceFactor, double);

    // Description:
    // Calculate and set sample distances (SampleDistanceHighRes,SampleDistanceHighResImage,SampleDistanceLowRes).
    // The current image spacing is the basis for the calculation
    void CalculateAndSetSampleDistances();



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
    // Main notebook in the details frame
    vtkKWNotebook *NB_Details;

    // Description:
    // Toggle the visibility of volume rendering
    vtkKWPushButtonWithLabel *PB_PauseResume;

    // Description:
    // Icon for the visibility of volume rendering
    vtkSlicerVisibilityIcons *VI_PauseResume;

    //--------------------------------------------------------------------------
    // Treshold
    //--------------------------------------------------------------------------

    // Description:
    // Selection of the Treshold  mode
    vtkKWMenuButtonWithLabel *MB_ThresholdMode;

    // Description:
    // Selection of the color mode
    vtkSlicerVRMenuButtonColorMode *VRMB_ColorMode;

    // Description:
    // Threshold for gray value
    vtkKWRange *RA_RampRectangleHorizontal;

    // Description:
    // Threshold settings for opacity
    vtkKWRange *RA_RampRectangleVertical;

    // Description:
    // Zoom into the threshold sliders
    vtkKWPushButton *PB_ThresholdZoomIn;

    // Description:
    // Reset the "Zoom" of the threshold sliders
    vtkKWPushButton *PB_Reset;

    // Description:
    // internal saving of the threshold mode
    int ThresholdMode;

    // Description:
    // Create the treshold page
    void CreateThreshold(void);

    // Description:
    // Destroy the treshold page
    void DestroyTreshold(void);

    //--------------------------------------------------------------------------
    // Mapper
    //--------------------------------------------------------------------------

    // Description:
    // Frame to configure the mapping options
    vtkKWFrameWithLabel *MappersFrame;

    // Description:
    // Enable/Disable low quality texture rendering
    vtkKWCheckButtonWithLabel *CB_TextureLow;

    // Description:
    // Enable/Disable high quality texture rendering
    vtkKWCheckButtonWithLabel *CB_TextureHigh;

    // Description:
    // Enable/Disable Ray Cast Mapping
    vtkKWCheckButtonWithLabel *CB_RayCast;

    // Description:
    // Add interactive frame rates to ray cast mapping
    vtkKWCheckButtonWithLabel *CB_InteractiveFrameRate;

    // Description:
    // Adjust the frame for interactive rendering methods
    vtkKWScaleWithLabel *SC_Framerate;

    // Description:
    // Update the quality check boxes ( e.g. ensure that one cb is always enabled etc.)
    void UpdateQualityCheckBoxes(void);

    // Description:
    // Create the performance page
    void CreatePerformance(void);

    // Description:
    // Destroy the performance page
    void DestroyPerformance(void);

    //--------------------------------------------------------------------------
    // Cropping
    //--------------------------------------------------------------------------

    // Description:
    // Node selector to select a transform, that will be added to the clipping sliders
    vtkSlicerNodeSelectorWidget *NS_TransformNode;

    // Description:
    // Enable/Disable cropping in general
    vtkKWCheckButtonWithLabel *CB_Cropping;

    // Description:
    // Change the range of cropping in "BOX coordinates"
    vtkKWRange *RA_Cropping[3];

    // Description:
    // The interactive clipping box.
    vtkSlicerBoxWidget *BW_Clipping;

    // Description:
    // Enable/Disable the interactive clipping box
    vtkKWCheckButtonWithLabel *CB_Clipping;

    // Description:
    //Transform node that can be used in addition to clipping sliders and box widget
    vtkMRMLLinearTransformNode *CurrentTransformNodeCropping;

    // Description:
    // transform representation of the additionl transform node
    vtkTransform *AdditionalClippingTransform;

    // Description:
    // inverse transform representation of the additional transform node
    vtkTransform *InverseAdditionalClippingTransform;

    // Description:
    // Colors of the six clipping handles
    double ColorsClippingHandles[6][3];

    // Description:
    // Flag to ensure that the clipping sliders are not adjusted, prevent endless loops
    int NoSetRangeNeeded;

    // Description:
    // The volume boundaries in box coordinates(two points).
    double VolumeBoundariesBoxCoordinates[2][3];

    // Description:
    // Create the cropping page
    void CreateCropping(void);

    // Description:
    // Delete the cropping page
    void DestroyCropping(void);

    // Description:
    // Convert a point in world coordinates to box coordinates. input=output
    void ConvertWorldToBoxCoordinates(double* inputOutput);

    // Description:
    // Convert a point in box coordinates to world coordinates
    void ConvertBoxCoordinatesToWorld(double* inputOutput);

    // Description:
    // Calculate and set the boundaries of the volume in box coordinates
    void CalculateBoxCoordinatesBoundaries(void);





    //--------------------------------------------------------------------------
    // Advanced
    //--------------------------------------------------------------------------

    // Description:
    // A set of histograms used in the volume property widget
    vtkKWHistogramSet *Histograms;

    // Description:
    // The volume property widget in the advanced page.
    vtkSlicerVolumePropertyWidget *SVP_VolumeProperty;

    // Description:
    // Update the volume property widget
    void UpdateSVP(void);

    // Description:
    // Adjust the transfer functions to the current setting of vtkVolumeProperty
    void AdjustMapping(void);

    //--------------------------------------------------------------------------
    // Rendering Logic
    //--------------------------------------------------------------------------

    // Description:
    // Mask which rendering algorithms to use
    int ScheduleMask[3];

    // Description:
    // Main renderer of the renderwidget. Used to reduce the viewport size
    vtkRenderer *RenViewport;

    // Description:
    // Renderer used to render the plane
    vtkRenderer *RenPlane;

    // Description:
    // Flag to signalize if the next rendering is to render the plane
    int RenderPlane;

    // Description:
    // The hardware accelerated texture mapper.
    vtkSlicerVolumeTextureMapper3D *MapperTexture;

    // Description:
    // The software accelerated software mapper
    vtkSlicerFixedPointVolumeRayCastMapper *MapperRaycast;

    // Description:
    //Initial Factor for Interactive Rendering
    double InitialDropLowRes;

    // Description:
    //Factor during last low Resolution Rendering
    double FactorLastLowRes;

    // Description:
    //Time for the last Low Resolution Rendering
    double LastTimeLowRes;

    // Description:
    // Timer to measure the duration of the low resolution rendering
    vtkTimerLog *Timer;

    // Description:
    // Which time would we like to achieve
    double GoalLowResTime;

    // Description:
    // Area in which no change in Factor will be made.
    double PercentageNoChange;

    // Description:
    //H ow long to wait, before Rendering in High Resolution
    double TimeToWaitForHigherStage;

    // Description:
    // 0 interactive, 1 High Resolution Texture VR, 2 SW Ray Cast
    int CurrentStage;

    // Description:
    // Similiar to current stage, but describing the lowest stage
    int Quality;

    // Description:
    // 1 mouse button is down at the moment
    int ButtonDown;

    // Description:
    // Indicates if the next rendering comes from a scheduled rendering
    int Scheduled;

    //BTX

    // Description:
    // Event id of the next scheduled rendering (not necessary lowest quality)
    std::string EventHandlerID;

    // Description:
    // Event id of the lowest quality scheduled rendering
    std::string StageZeroEventHandlerID;
    //ETX

    //Description:
    //Indicates if the VolumeRendering is Paused or not
    int RenderingPaused;
    // Description:
    // Sample distance for high resolution rendering
    double SampleDistanceHighRes;

    // Description:
    // Image sample distance for high resolution rendering
    double SampleDistanceHighResImage;

    // Description:
    // Sample distance for low resolution rendering
    double SampleDistanceLowRes;

    // Description:
    // Sample distance factor to determine the step size based on the 
    // spacing of the input volume.  The minimum size of the volume is divided by this
    // factor to calculate the sample distance.  See:
    // void vtkSlicerVRGrayscaleHelper::CalculateAndSetSampleDistances
    double SampleDistanceFactor;

    // Description:
    // Reset the rendering algorithm back to normal
    void ResetRenderingAlgorithm(void);


    // Description:
    // Check if we have to abort the rendering because of interaction
    void CheckAbort(void);







private:
    // Description:
    // Caution: Not implemented
    vtkSlicerVRGrayscaleHelper(const vtkSlicerVRGrayscaleHelper&);//not implemented
    void operator=(const vtkSlicerVRGrayscaleHelper&);//not implemented

};
#endif
