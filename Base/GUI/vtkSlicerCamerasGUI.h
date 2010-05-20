///  vtkSlicerCamerasGUI 
/// 
/// Main Camera GUI and mediator methods for slicer3. 

#ifndef __vtkSlicerCamerasGUI_h
#define __vtkSlicerCamerasGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"

class vtkMRMLCameraNode;
class vtkSlicerNodeSelectorWidget;
class vtkKWCheckButton;
class vtkSlicerCamerasGUIInternals;

/// Description:
/// This class implements Slicer's Camera GUI
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerCamerasGUI : public vtkSlicerModuleGUI
{
 public:
    /// 
    /// Usual vtk class functions
    static vtkSlicerCamerasGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerCamerasGUI, vtkSlicerModuleGUI );
    void PrintSelf (ostream& os, vtkIndent indent );
    
    /// 
    /// This method builds the Camera module GUI
    virtual void BuildGUI ( void );
    virtual void BuildGUI ( vtkKWFrame * f ) { this->Superclass::BuildGUI(f); }
    virtual void BuildGUI ( vtkKWFrame * f, double * bgColor ) { this->Superclass::BuildGUI(f,bgColor); }

    /// 
    /// Add/Remove observers on widgets in the GUI
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );

    /// 
    /// Class's mediator methods for processing events invoked by
    /// either the Logic, MRML or GUI.
    virtual void ProcessGUIEvents(
      vtkObject *caller, unsigned long event, void *callData);
    virtual void ProcessMRMLEvents(
      vtkObject *caller, unsigned long event, void *callData);
    
    /// 
    /// Describe behavior at module startup and exit.
    virtual void Enter ( );
    //BTX
    using vtkSlicerComponentGUI::Enter; 
    //ETX
    virtual void Exit ( );

    /// 
    /// Update selectors
    virtual void UpdateViewSelector();
    virtual void UpdateCameraSelector();
    virtual void ScheduleUpdateCameraSelector();

    /// 
    /// Callbacks
    virtual void UpdateCameraSelectorCallback();

    // Description:
    // Get the categorization of the module.  The category is used for
    // grouping modules together into menus.
    const char *GetCategory() const {return "Developer Tools";}

    ///
    /// Update the labels from the currently selected camera node
    void UpdateCameraLabels();
    
 protected:
    vtkSlicerCamerasGUI ( );
    virtual ~vtkSlicerCamerasGUI ( );
    
    vtkSlicerNodeSelectorWidget *CameraSelectorWidget;
    vtkSlicerNodeSelectorWidget *ViewSelectorWidget;

    /// labels to show the current camera's information
    vtkKWLabel *PositionLabel;
    vtkKWLabel *FocalPointLabel;
    vtkKWLabel *ViewUpLabel;
    
  /// PIMPL Encapsulation for STL containers
  //BTX
  vtkSlicerCamerasGUIInternals *Internals;
  //ETX

 private:
    vtkSlicerCamerasGUI ( const vtkSlicerCamerasGUI& ); /// Not implemented.
    void operator = ( const vtkSlicerCamerasGUI& ); //Not implemented.
};

#endif
