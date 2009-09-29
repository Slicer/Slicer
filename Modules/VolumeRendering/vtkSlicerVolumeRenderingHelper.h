// .NAME vtkSlicerVolumeRenderingHelper
// .SECTION Description
// Takes care about issues both derived classes have in Common: Callback, Progress for Gradients, initialization...


#ifndef __vtkSlicerVolumeRenderingHelper_h
#define __vtkSlicerVolumeRenderingHelper_h

#include "vtkVolumeRendering.h"
#include "vtkKWObject.h"

class vtkVolumeRenderingGUI;
class vtkCallbackCommand;
class vtkVolume;
class vtkMatrix4x4;
class vtkKWHistogramSet;
class vtkSlicerVolumePropertyWidget;
class vtkRenderer;
class vtkSlicerVolumeTextureMapper3D;
class vtkTimerLog;
class vtkSlicerFixedPointVolumeRayCastMapper;
class vtkSlicerGPURayCastVolumeTextureMapper3D;
class vtkSlicerGPURayCastVolumeMapper;
class vtkCudaVolumeMapper;
class vtkKWMenuButtonWithSpinButtonsWithLabel;
class vtkKWScaleWithLabel;
class vtkKWScaleWithEntry;
class vtkKWScale;
class vtkKWCheckButtonWithLabel;
class vtkKWFrameWithLabel;
class vtkKWNotebook;
class vtkKWMenuButtonWithLabel;
class vtkKWPushButton;
class vtkSlicerBoxWidget2;
class vtkSlicerBoxRepresentation;
class vtkSlicerColorDisplayWidget;
class vtkSlicerNodeSelectorWidget;
class vtkMRMLLinearTransformNode;
class vtkTransform;
class vtkSlicerVisibilityIcons;
class vtkKWPushButtonWithLabel;
class vtkSlicerVRMenuButtonColorMode;
class vtkKWComboBoxWithLabel;
class vtkKWComboBox;
class vtkKWEntry;
class vtkVolumeProperty;
class vtkKWRange;

class VTK_SLICERVOLUMERENDERING_EXPORT vtkSlicerVolumeRenderingHelper :public vtkKWObject
{
public:
    // Description:
    // Usual vtk/KWWidgets methods: go to www.vtk.org / www.kwwidgets.org for more details
    static vtkSlicerVolumeRenderingHelper *New();
    vtkTypeRevisionMacro(vtkSlicerVolumeRenderingHelper,vtkKWObject);

    // Description:
    // Update all GUI Elements
    virtual void UpdateGUIElements(void);

    // Description:
    // This method hast to be called directly after the widget is created and before any other method is called.
    virtual void Init(vtkVolumeRenderingGUI *gui);

    // Description:
    // Initialize the Rendering pipeline by creating an new vtkMRMLVolumeRenderingNode
    virtual void InitializePipelineNewVolumeProperty(void);
    virtual void InitializePipelineNewVolumePropertyFg(void);

    virtual void SetupHistogramFg();

    // Description:
    // Update the Rendering, takes care, that the volumeproperty etc. is up to date
    virtual void UpdateRendering(void);

    // Description:
    // Called everytime a mouse button is pressed
    void SetButtonDown(int _arg)
    {
      this->ButtonDown=(_arg);
      SetupCPURayCastInteractive();
    }

    //------------------------------------------------------
    // GUI processing functions
    //------------------------------------------------------

    // Description:
    // Callback, processed when the visibility button is pressed
    void ProcessPauseResume(void);

    //--------------------------------------------------------------------------
    // Treshold
    //--------------------------------------------------------------------------
    // Description:
    //Callback, that is processed when the threshold mode is changed
    void ProcessThresholdModeEvents(int id);

    // Description:
    // Callback, that is processed when the range of the threshold is changed
    void ProcessThresholdRange(double notUsed,double notUsedA);

    // Description:
    // Callback, that is processed when one zooms into the threshold range sliders
    void ProcessThresholdZoomIn(void);

    // Description:
    // Callback, that is processed when one resets the threshold range sliders
    void ProcessThresholdReset(void);

    //--------------------------------------------------------------------------
    // Performance/Quality
    //--------------------------------------------------------------------------

    // Description:
    // Callback, that is processed when the rendering method (mapper) is changed
    void ProcessRenderingMethodEvents(int id);

    // Description:
    // Callback, that is processed when the GPU ray casting rendering technique is changed
    void ProcessGPURayCastTechnique(int id);
    void ProcessGPURayCastTechniqueII(int id);

    // Description:
    // Callback, that is processed when the multi-volume rendering fusion method is changed
    void ProcessGPURayCastColorOpacityFusion(int id);

    // Description:
    // Callback, that is processed when the GPU ray cast mapper internal volume size changed
    void ProcessGPUMemorySize(int id);

    // Description:
    // process when user change the expected FPS
    void ProcessExpectedFPS(void);

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

protected:
    // Description:
    // Use ::New() to get a new instance.
    vtkSlicerVolumeRenderingHelper(void);

    // Description:
    // Use ->Delete() to delete object
    ~vtkSlicerVolumeRenderingHelper(void);

    // Description:
    // Actor used for Volume Rendering
    vtkVolume *Volume;

    // Description:
    // Reference to the VolumeRenderingGUI. No delete!
    vtkVolumeRenderingGUI *Gui;

    // Description:
    // Flag to avoid recursive callbacks
    int InVolumeRenderingCallbackFlag;

    // Description:
    // Own callback command for volume rendering related events
    vtkCallbackCommand* VolumeRenderingCallbackCommand;

    // Description:
    // Set/Get flag to avoid recursive rendering
    void SetInVolumeRenderingCallbackFlag (int flag) {
        this->InVolumeRenderingCallbackFlag = flag;
    }
    vtkGetMacro(InVolumeRenderingCallbackFlag, int);

    // Description:
    // Callback function for volume rendering callbacks
    static void VolumeRenderingCallback( vtkObject *__caller, unsigned long eid, void *__clientData, void *callData );

    // Description:
    // Called when volume rendering callbacks are invoked
    virtual void ProcessVolumeRenderingEvents(vtkObject *caller, unsigned long eid, void *callData);

    // Description:
    // called to initialize the rendering (only the first time).
    virtual void Rendering(void);

    // Description:
    // Calculate the matrix that will be used for the rendering (includes the consideration of possible transformnodes and the IJK to RAS transform)
    void CalculateMatrix(vtkMatrix4x4 *output);

    //-------------------------------------------------------
    // GUI variables
    //-------------------------------------------------------

    // Description:
    // Main notebook in the rendering frame
    vtkKWNotebook *NB_Details;

    // Description:
    // Toggle the visibility of volume rendering
    vtkKWPushButtonWithLabel *PB_PauseResume;

    // Description:
    // Icon for the visibility of volume rendering
    vtkSlicerVisibilityIcons *VI_PauseResume;

    // Description:
    // Frame to configure the mapping options
    // we save pointers here for dynamically collapse/expend these frames
    vtkKWFrameWithLabel *FramePerformance;
    vtkKWFrameWithLabel *FrameCUDARayCasting;
    vtkKWFrameWithLabel *FrameGPURayCasting;
    vtkKWFrameWithLabel *FrameGPURayCastingII;
    vtkKWFrameWithLabel *FramePolygonBlending;
    vtkKWFrameWithLabel *FrameCPURayCasting;
    vtkKWFrameWithLabel *FrameFPS;

    // Description:
    // Adjust the frame for interactive rendering methods
    vtkKWScale *SC_ExpectedFPS;

    vtkKWPushButton *PB_HideSurfaceModels;

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
    vtkKWRange *RA_RampRectangleScalar;

    // Description:
    // Threshold settings for opacity
    vtkKWRange *RA_RampRectangleOpacity;

    // Description:
    // Zoom into the threshold sliders
    vtkKWPushButton *PB_ThresholdZoomIn;

    // Description:
    // Reset the "Zoom" of the threshold sliders
    vtkKWPushButton *PB_Reset;

    // Description:
    // internal saving of the threshold mode
    int ThresholdMode;

    //--------------------------------------------------------------------------
    // Performance/Quality
    //--------------------------------------------------------------------------

    // Description:
    // Menu button to select which mapper to use
    vtkKWMenuButtonWithLabel *MB_Mapper;

    // Description:
    // Enable/Disable shading in CUDA Ray Cast Mapping
    vtkKWCheckButtonWithLabel *CB_CUDARayCastShading;

    // Description:
    // Menu button to select internal volume size
    vtkKWMenuButtonWithLabel *MB_GPUMemorySize;

    // Description:
    // Menu button to select rendering technique
    vtkKWMenuButtonWithLabel *MB_GPURayCastTechnique;
    vtkKWMenuButtonWithLabel *MB_GPURayCastTechniqueII;

    // Description:
    // Menu button to select color/opacity fusion method in multi-volume rendering
    vtkKWMenuButtonWithLabel *MB_GPURayCastColorOpacityFusion;

    // Description:
    // Enable/Disable CPU ray cast MIP rendering
    vtkKWCheckButtonWithLabel *CB_CPURayCastMIP;

    // Description:
    // Adjust the frame for interactive rendering methods
    vtkKWScale *SC_GPURayCastIIFgBgRatio;

    // Description:
    // Depth peeling threshold for GPU ray casting
    vtkKWScaleWithEntry *SC_GPURayCastDepthPeelingThreshold;

    // Description:
    // Depth/smoothness for GPU ray casting (ICPE)
    vtkKWScaleWithEntry *SC_GPURayCastICPEkt;//depth
    vtkKWScaleWithEntry *SC_GPURayCastICPEks;//smoothness

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
    // The interactive clipping box widget
    vtkSlicerBoxWidget2 *BW_Clipping_Widget;

    // Description:
    // The geometrical representation of the interactive clipping box widget
    vtkSlicerBoxRepresentation *BW_Clipping_Representation;

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

    //--------------------------------------------------------------------------
    // Advanced
    //--------------------------------------------------------------------------

    // Description:
    // A set of histograms used in the volume property widget
    vtkKWHistogramSet *Histograms;

    // Description:
    // A set of histograms used in the volume property widget
    vtkKWHistogramSet *HistogramsFg;

    // Description:
    // The volume property widget in the advanced page.
    vtkSlicerVolumePropertyWidget *SVP_VolumeProperty;

    // Description:
    // The volume property widget in the advanced page.
    vtkSlicerVolumePropertyWidget *SVP_VolumePropertyFg;

    //--------------------------------------------------------------------------
    // Rendering Logic
    //--------------------------------------------------------------------------

    // Description:
    // The hardware accelerated texture mapper.
    vtkSlicerVolumeTextureMapper3D *MapperTexture;

    // Description:
    // The hardware accelerated gpu (CUDA) ray cast mapper.
    vtkCudaVolumeMapper *MapperCUDARaycast;

    // Description:
    // The hardware accelerated gpu ray cast mapper.
    vtkSlicerGPURayCastVolumeTextureMapper3D *MapperGPURaycast;

    // Description:
    // The hardware accelerated gpu ray cast II mapper (multi-volume rendering).
    vtkSlicerGPURayCastVolumeMapper *MapperGPURaycastII;

    // Description:
    // The software accelerated software mapper
    vtkSlicerFixedPointVolumeRayCastMapper *MapperRaycast;

    // Description:
    // Indicates if the VolumeRendering is Paused or not
    int RenderingPaused;

    // Description:
    // estimated sample distance for cpu ray casting
    double EstimatedSampleDistance;//move to parameters node

    // Description:
    // if updating GUI
    int UpdateingGUI;

    // Description:
    // if texture mapper supported
    int IsTextureMappingSupported;

    // Description:
    // if CUDA ray casting supported
    int IsCUDARayCastingSupported;

    // Description:
    // if GPU ray casting supported
    int IsGPURayCastingSupported;

    // Description:
    // 1 mouse button is down at the moment
    int ButtonDown;

    // Description:
    // if cpu ray casting is in interaction mode
    int CPURayCastingInteractionFlag;

    // Description:
    // internal volume property for GPU ray cast II (multi-volume rendering)
    vtkVolumeProperty*    VolumePropertyGPURaycastII;

private:
    // Description:
    // Caution: Not implemented
    vtkSlicerVolumeRenderingHelper(const vtkSlicerVolumeRenderingHelper&);//not implemented
    void operator=(const vtkSlicerVolumeRenderingHelper&);//not implemented

    // Description:
    // create rendering frame GUI
    void BuildRenderingFrameGUI();

    //-------------------------------
    // Functions create/Destroy tabs
    //-------------------------------

    // Description:
    // Create/Destroy perfromance page
    void CreatePerformanceTab(void);
    void DestroyPerformanceTab(void);

    // Description:
    // Create/Destroy thresholding page
    void CreateThresholdTab(void);
    void DestroyThresholdTab(void);

    // Description:
    // Create/Destroy cropping page
    void CreateCroppingTab(void);
    void DestroyCroppingTab(void);

    //------------------------------
    // utility functions (move to logic?)
    //------------------------------
    // Description:
    // Convert a point in world coordinates to box coordinates. input=output
    void ConvertWorldToBoxCoordinates(double* inputOutput);

    // Description:
    // Convert a point in box coordinates to world coordinates
    void ConvertBoxCoordinatesToWorld(double* inputOutput);

    // Description:
    // Calculate and set the boundaries of the volume in box coordinates
    void CalculateBoxCoordinatesBoundaries(void);

    // Description:
    // Update the volume property widget
    void UpdateSVP(void);

    // Description:
    // Update the volume property widget
    void UpdateSVPFg(void);

    // Description:
    // Adjust the transfer functions to the current setting of vtkVolumeProperty
    void AdjustMapping(void);

    // Description:
    // Adjust the transfer functions to the current setting of vtkVolumeProperty
    void AdjustMappingFg(void);

    // Description:
    // Setup CPU ray casing interactive mode parameters
    void SetupCPURayCastInteractive();

    // Description:
    // Prepare internal volume property for GPU ray casting II
    void CreateVolumePropertyGPURaycastII();

    // Description:
    // check abort event
    void CheckAbort(void);

    // Description:
    // default sample distances
    void EstimateSampleDistances(void);

    // Description:
    // determine intermal volume storage size
    void ComputeInternalVolumeSize(int index);

    // Description:
    // hook up mapper to volume actor and update frames
    void SetMapperFromSelection(int id);
};
#endif
