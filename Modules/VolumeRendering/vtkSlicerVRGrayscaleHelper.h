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
class vtkSlicerGPURayCastVolumeTextureMapper3D;
class vtkKWMenuButtonWithSpinButtonsWithLabel;
class vtkKWScaleWithLabel;
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


class VTK_SLICERVOLUMERENDERING_EXPORT vtkSlicerVRGrayscaleHelper :public vtkSlicerVRHelper
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
    // Performance/Quality
    //--------------------------------------------------------------------------
    
    // Description:
    // Callback, that is processed when the rendering method (mapper) is changed
    void ProcessRenderingMethodEvents(int id);

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
    // Estimate sample distances for polygon blending and cpu ray casting
    // The current image spacing is the basis for the calculation
    void EstimateSampleDistances();


    // Description:
    // The hardware accelerated texture mapper.  
    vtkGetObjectMacro ( MapperTexture, vtkSlicerVolumeTextureMapper3D);

    // Description:
    // The software accelerated software mapper
    vtkGetObjectMacro ( MapperRaycast, vtkSlicerFixedPointVolumeRayCastMapper);

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
    // Performance/Quality
    //--------------------------------------------------------------------------

    // Description:
    // internal saving of the rendering method (mapper)
    int RenderingMethod;
    
    // Description:
    // Frame to configure the mapping options
    // we save pointers here for dynamically collapse/expend these frames
    vtkKWFrameWithLabel *FramePerformance;
    vtkKWFrameWithLabel *FrameGPURayCasting;
    vtkKWFrameWithLabel *FramePolygonBlending;
    vtkKWFrameWithLabel *FrameCPURayCasting;
    vtkKWFrameWithLabel *FrameFPS;
    
    // Description:
    // Menu button to select which mapper to use
    vtkKWMenuButtonWithLabel *MB_Mapper;
    
    // Description:
    // Enable/Disable MIP GPU Ray Cast Mapping
    vtkKWCheckButtonWithLabel *CB_GPURayCastMIP;

    // Description:
    // Enable/Disable shading in GPU Ray Cast Mapping
    // some graphics card may have problem to access more than one 3D texture 
    // in shader program so it would be safe to turn off shading by default
    vtkKWCheckButtonWithLabel *CB_GPURayCastShading;
    
    // Description:
    // Enable/Disable large volume size usage
    vtkKWCheckButtonWithLabel *CB_GPURayCastLargeVolume;
    
    // Description:
    // Enable/Disable adaptive framerate control
    vtkKWCheckButtonWithLabel *CB_GPURayCastForceHighQuality;
    
    // Description:
    // Enable/Disable adaptive framerate control
    vtkKWCheckButtonWithLabel *CB_TextureMapperForceHighQuality;
    
    // Description:
    // Enable/Disable adaptive framerate control
    vtkKWCheckButtonWithLabel *CB_CPURayCastForceHighQuality;

    // Description:
    // Enable/Disable CPU ray cast MIP rendering
    vtkKWCheckButtonWithLabel *CB_CPURayCastMIP;
    
    // Description:
    // Adjust the frame for interactive rendering methods
    vtkKWScale *SC_ExpectedFPS;

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
    // The hardware accelerated texture mapper.
    vtkSlicerVolumeTextureMapper3D *MapperTexture;    
    
    // Description:
    // The hardware accelerated gpu ray cast mapper.
    vtkSlicerGPURayCastVolumeTextureMapper3D *MapperGPURaycast;

    // Description:
    // The software accelerated software mapper
    vtkSlicerFixedPointVolumeRayCastMapper *MapperRaycast;

    // Description:
    // Indicates if the VolumeRendering is Paused or not
    int RenderingPaused;
    
    // Description:
    // estimated sample distance for polygon blending and cpu ray casting
    double EstimatedSampleDistance;
    double EstimatedInteractiveSampleDistance;

    
    // Description:
    // if updating GUI
    int UpdateingGUI;
    
    // Description:
    // if texture mapper supported
    int IsTextureMappingSupported;
    
    // Description:
    // if GPU ray casting supported
    int IsGPURayCastingSupported;
    
    // Description:
    // check abort event
    void CheckAbort(void);

private:
    // Description:
    // Caution: Not implemented
    vtkSlicerVRGrayscaleHelper(const vtkSlicerVRGrayscaleHelper&);//not implemented
    void operator=(const vtkSlicerVRGrayscaleHelper&);//not implemented

};
#endif

