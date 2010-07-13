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
class vtkKWMenuButtonWithSpinButtonsWithLabel;
class vtkKWScaleWithLabel;
class vtkKWScaleWithEntry;
class vtkKWScale;
class vtkKWCheckButtonWithLabel;
class vtkKWFrameWithLabel;
class vtkKWNotebook;
class vtkKWMenuButtonWithLabel;
class vtkKWPushButton;
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
class vtkSlicerROIDisplayWidget;
class vtkKWLoadSaveButton;
class vtkMRMLVolumeRenderingParametersNode;
class vtkKWCheckButton;

class VTK_SLICERVOLUMERENDERING_EXPORT vtkSlicerVolumeRenderingHelper :public vtkKWObject
{
public:
    // Description:
    // Usual vtk/KWWidgets methods: go to www.vtk.org / www.kwwidgets.org for more details
    static vtkSlicerVolumeRenderingHelper *New();
    vtkTypeRevisionMacro(vtkSlicerVolumeRenderingHelper,vtkKWObject);

    // Description:
    // This method has to be called directly after the widget is created and before any other method is called.
    virtual void Init(vtkVolumeRenderingGUI *gui);

    // Description:
    // Own callback command for volume rendering related events
    vtkCallbackCommand* GUICallbackCommand;

    //------------------------------------------------------
    // GUI processing functions
    //------------------------------------------------------

    // Description:
    // Callback, processed when the visibility button is pressed
    void ProcessPauseResume(void);

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
    void ProcessGPURayCastTechniqueIIFg(int id);
    void ProcessGPURayCastTechnique3(int id);

    // Description:
    // Callback, that is processed when the multi-volume rendering fusion method is changed
    void ProcessGPURayCastColorOpacityFusion(int id);

    // Description:
    // Callback, that is processed when the GPU ray cast mapper internal volume size changed
    void ProcessGPUMemorySize(int id);

    // Description:
    // process when user change the expected FPS
    void ProcessExpectedFPS(void);

    // Description:
    // process when user change the performance control method
    void ProcessPerformanceControl(int id);

    //--------------------------------------------------------
    // Property
    //--------------------------------------------------------
    void ProcessThreshold(double, double);
    void ProcessThresholdFg(double, double);

    //--------------------------------------------------------
    // Utility functions
    //--------------------------------------------------------
    void UpdateVolumeProperty();
    void UpdateVolumePropertyFg();

    void UpdateROI();
    
protected:
    // Description:
    // Use ::New() to get a new instance.
    vtkSlicerVolumeRenderingHelper(void);

    // Description:
    // Use ->Delete() to delete object
    ~vtkSlicerVolumeRenderingHelper(void);

    // Description:
    // Reference to the VolumeRenderingGUI. No delete!
    vtkVolumeRenderingGUI *Gui;

    // Description:
    static void GUIEventsCallback( vtkObject *__caller, unsigned long eid, void *__clientData, void *callData );

    // Description:
    virtual void ProcessGUIEvents(vtkObject *caller, unsigned long eid, void *callData);

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
    // Adjust the frame for interactive rendering methods
    vtkKWScaleWithEntry *SC_ExpectedFPS;

    // Description:
    // Define performance control method
    vtkKWMenuButtonWithLabel *MB_PerformanceControl;
    
    vtkKWPushButton *PB_HideSurfaceModels;

    //--------------------------------------------------------------------------
    // Performance/Quality
    //--------------------------------------------------------------------------
    // Description:
    // Frame to configure the mapping options
    // we save pointers here for dynamically collapse/expend these frames
    vtkKWFrameWithLabel *FrameTechniques;
    vtkKWFrameWithLabel *FrameGPURayCasting;
    vtkKWFrameWithLabel *FrameGPURayCastingII;
    vtkKWFrameWithLabel *FrameGPURayCasting3;
    vtkKWFrameWithLabel *FramePolygonBlending;
    vtkKWFrameWithLabel *FrameCPURayCasting;

    // Description:
    // Menu button to select which mapper to use
    vtkKWMenuButtonWithLabel *MB_Mapper;

    // Description:
    // Menu button to select internal volume size
    vtkKWMenuButtonWithLabel *MB_GPUMemorySize;

    // Description:
    // Menu button to select rendering technique
    vtkKWMenuButtonWithLabel *MB_GPURayCastTechnique;

    vtkKWMenuButtonWithLabel *MB_GPURayCastTechniqueII;
    vtkKWMenuButtonWithLabel *MB_GPURayCastTechniqueIIFg;
    vtkKWMenuButtonWithLabel *MB_GPURayCastTechnique3;

    // Description:
    // Menu button to select color/opacity fusion method in multi-volume rendering
    vtkKWMenuButtonWithLabel *MB_GPURayCastColorOpacityFusion;

    // Description:
    // Enable/Disable CPU ray cast MIP rendering
    vtkKWCheckButtonWithLabel *CB_CPURayCastMIP;

    // Description:
    // Adjust the frame for interactive rendering methods
    vtkKWScaleWithEntry *SC_GPURayCastIIFgBgRatio;

    // Description:
    // Depth peeling threshold for GPU ray casting
    vtkKWScaleWithEntry *SC_GPURayCastDepthPeelingThreshold;

    // Description:
    // Depth/smoothness for GPU ray casting (ICPE)
    vtkKWScaleWithEntry *SC_GPURayCastICPEkt;//depth
    vtkKWScaleWithEntry *SC_GPURayCastICPEks;//smoothness

    // Description:
    // Distance Color Blending for GPU ray casting
    vtkKWScaleWithEntry *SC_GPURayCastDistanceColorBlending;

    //--------------------------------------------------------------------------
    // Volume Property
    //--------------------------------------------------------------------------
    
    // Description:
    // kwWidget volume property widget for default (bg) volume
    vtkSlicerVolumePropertyWidget *SVP_VolumePropertyWidget;

    // Description:
    // kwWidget volume property widget for fg volume
    vtkSlicerVolumePropertyWidget *SVP_VolumePropertyWidgetFg;

    vtkKWCheckButtonWithLabel *CB_FollowVolumeDisplayNode;
    vtkKWCheckButtonWithLabel *CB_FollowVolumeDisplayNodeFg;

    // Description:
    // Enable/Disable same volume property for both bg and fg volume
    vtkKWCheckButtonWithLabel *CB_UseSingleVolumeProperty;
    
    // Description:
    // Enable/Disable thresholding
    vtkKWCheckButtonWithLabel *CB_UseThreshold;

    vtkKWFrameWithLabel       *FrameThresholding;
    vtkKWRange                *RA_Threshold;
    vtkKWScaleWithEntry       *SC_ThresholdOpacity;

    vtkKWCheckButtonWithLabel *CB_UseThresholdFg;
    
    vtkKWFrameWithLabel       *FrameThresholdingFg;
    vtkKWRange                *RA_ThresholdFg;
    vtkKWScaleWithEntry       *SC_ThresholdOpacityFg;
    
    vtkKWLoadSaveButton       *LoadVolumePropertyButton;

    //-----------------------------------------------------------
    // ROI (clipping)
    //-----------------------------------------------------------
    vtkSlicerROIDisplayWidget  *ROIWidget;
    vtkKWCheckButtonWithLabel  *CB_CroppingButton;
    vtkKWPushButton            *PB_FitROIButton;
    //--------------------------------------------------------------------------
    // Rendering Logic
    //--------------------------------------------------------------------------

    // Description:
    // Indicates if the VolumeRendering is Paused or not
    int RenderingPaused;

    int SetupGUIFromParametersNodeFlag;

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
    // Create/Destroy rendering techniques page
    void CreateTechniquesTab(void);
    void DestroyTechniquesTab(void);

    // Description:
    // Create/Destroy volume property page
    void CreatePropertyTab(void);
    void DestroyPropertyTab(void);

    // Description:
    // Create/Destroy ROI page
    void CreateROITab(void);
    void DestroyROITab(void);

    // Description:
    // Create/Destroy Misc page
    void CreateMiscTab(void);
    void DestroyMiscTab(void);

    void SetupGUIFromParametersNode(vtkMRMLVolumeRenderingParametersNode* vspNode);
    void SetROIRange(vtkMRMLVolumeRenderingParametersNode* vspNode);

    // Description:
    // Setup volume rendering interactive mode parameters
    void SetupVolumeRenderingInteractive();
};
#endif
