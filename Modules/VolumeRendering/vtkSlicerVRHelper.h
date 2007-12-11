// .NAME vtkSlicerVRHelper - Superclass for vtkSlicerVRGrayscaleHelper and vtkSlicerVRLabelmapHelper
// .SECTION Description
// A superclass for vtkSlicerVRGrayscaleHelper and vtkSlicerVRLabelmapHelper. Allow different and GUI behaviours vor labelmaps
// and grayscale volumes. Takes care about issues both derived classes have in Common: Callback, Progress for Gradients, initialization...


#ifndef __vtkSlicerVRHelper_h
#define __vtkSlicerVRHelper_h

#include "vtkVolumeRenderingModule.h"
#include "vtkKWObject.h"


//show the debug message in the TCon not in the ErrorWindow
//Reason to deal this seperate: Performance issues in ErrorWindow
# define vtkSlicerVRHelperDebug(message,format)                                 \
  {                                                                             \
  if (this->GetTCLDebug())                                                      \
    {                                                                           \
    this->Script("puts \""message"\"",format);                                  \
    }                                                                           \
  }
class vtkVolumeRenderingModuleGUI;
class vtkCallbackCommand;
class vtkVolume;
class vtkMatrix4x4;
class vtkKWProgressDialog;

class VTK_VOLUMERENDERINGMODULE_EXPORT vtkSlicerVRHelper :public vtkKWObject
{
public:
    // Description:
    // Usual vtk/KWWidgets methods: go to www.vtk.org / www.kwwidgets.org for more details
    static vtkSlicerVRHelper *New();
    vtkTypeRevisionMacro(vtkSlicerVRHelper,vtkKWObject);

    // Description:
    // Update all GUI Elements
    virtual void UpdateGUIElements(void);

    // Description:
    // This method hast to be called directly after the widget is created and before any other method is called.
    virtual void Init(vtkVolumeRenderingModuleGUI *gui);

    // Description:
    // Initialize the Rendering pipeline by creating an new vtkMRMLVolumeRenderingNode
    virtual void InitializePipelineNewCurrentNode(void);

    // Description:
    // Update the Rendering, takes care, that the volumeproperty etc. is up to date
    virtual void UpdateRendering(void);

    // Description:
    // Enable/Disable the printing of debug messages in the TCon
    vtkSetMacro(TCLDebug,int);
    vtkGetMacro(TCLDebug, int);

    void DisplayProgressDialog(const char* message);

    void WithdrawProgressDialog(void);


protected:
    // Description:
    // Use ::New() to get a new instance.
    vtkSlicerVRHelper(void);

    // Description:
    // Use ->Delete() to delete object
    ~vtkSlicerVRHelper(void);

    // Description:
    // Toplevel Dialog shown while gradients are calculated
    vtkKWProgressDialog *GradientDialog;

    // Description:
    // Actor used for Volume Rendering
    vtkVolume *Volume;

    // Description:
    // Flag if tcl debug messages are enabled or not
    int TCLDebug;

    // Description:
    // Reference to the VolumeRenderingModuleGUI. No delete!
    vtkVolumeRenderingModuleGUI *Gui;

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
    static void VolumeRenderingCallback( vtkObject *__caller,unsigned long eid, void *__clientData, void *callData );

    // Description:
    // Called when volume rendering callbacks are invoked
    virtual void ProcessVolumeRenderingEvents(vtkObject *caller,unsigned long eid,void *callData);

    // Description:
    // called to initialize the rendering (only the first time).
    virtual void Rendering(void);

    // Description:
    // Calculate the matrix that will be used for the rendering (includes the consideration of possible transformnodes and the IJK to RAS transform)
    void CalculateMatrix(vtkMatrix4x4 *output);

private:
    // Description:
    // Caution: Not implemented
    vtkSlicerVRHelper(const vtkSlicerVRHelper&);//not implemented
    void operator=(const vtkSlicerVRHelper&);//not implemented
};
#endif
