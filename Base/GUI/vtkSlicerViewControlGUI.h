// .NAME vtkSlicerViewControlGUI
// .SECTION Description
// Main Data GUI and mediator methods for
// ViewControl GUI Panel in slicer3.

#ifndef __vtkSlicerViewControlGUI_h
#define __vtkSlicerViewControlGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerComponentGUI.h"

#include "vtkKWFrame.h"
#include "vtkKWPushButton.h"

class vtkSlicerApplicationGUI;
class vtkKWScale;
class vtkKWPushButton;
class vtkKWCheckButton;
class vtkKWLabel;
class vtkKWMenuButton;
class vtkKWEntryWithLabel;
class vtkSlicerViewControlIcons;

// Description:
// This class implements Slicer's Application Toolbar 
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerViewControlGUI : public vtkSlicerComponentGUI
{
 public:
    // Description:
    // Usual vtk class functions
    static vtkSlicerViewControlGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerViewControlGUI, vtkSlicerComponentGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );

    // Description:
    // Get the widgets in the ViewControlFrame
    vtkGetObjectMacro (SpinButton, vtkKWCheckButton);
    vtkGetObjectMacro (RockButton, vtkKWCheckButton);
    vtkGetObjectMacro (OrthoButton, vtkKWCheckButton);
    vtkGetObjectMacro (CenterButton, vtkKWPushButton);
    vtkGetObjectMacro (SelectButton, vtkKWMenuButton);
    vtkGetObjectMacro (FOVEntry, vtkKWEntryWithLabel);
    
    // Description:
    // Get the Widgets that display the RotateAround image
    // in the ViewControlFrame.
    vtkGetObjectMacro (RotateAroundAIconButton, vtkKWLabel );
    vtkGetObjectMacro (RotateAroundPIconButton, vtkKWLabel );
    vtkGetObjectMacro (RotateAroundRIconButton, vtkKWLabel );
    vtkGetObjectMacro (RotateAroundLIconButton, vtkKWLabel );
    vtkGetObjectMacro (RotateAroundSIconButton, vtkKWLabel );
    vtkGetObjectMacro (RotateAroundIIconButton, vtkKWLabel );
    vtkGetObjectMacro (RotateAroundMiddleIconButton, vtkKWLabel );
    vtkGetObjectMacro (RotateAroundTopCornerIconButton, vtkKWLabel );
    vtkGetObjectMacro (RotateAroundBottomCornerIconButton, vtkKWLabel);

    // Description:
    // Get the Widgets that display the LookFrom image
    // in the ViewControlFrame.
    vtkGetObjectMacro (LookFromAIconButton, vtkKWLabel );
    vtkGetObjectMacro (LookFromPIconButton, vtkKWLabel );
    vtkGetObjectMacro (LookFromRIconButton, vtkKWLabel );
    vtkGetObjectMacro (LookFromLIconButton, vtkKWLabel );
    vtkGetObjectMacro (LookFromSIconButton, vtkKWLabel );
    vtkGetObjectMacro (LookFromIIconButton, vtkKWLabel );
    vtkGetObjectMacro (LookFromMiddleIconButton, vtkKWLabel );
    vtkGetObjectMacro (LookFromTopCornerIconButton, vtkKWLabel );
    vtkGetObjectMacro (LookFromBottomCornerIconButton, vtkKWLabel);

    vtkGetObjectMacro ( SlicerViewControlIcons, vtkSlicerViewControlIcons );

    // Description:
    // Get the Widgets that display the Navigation Zoom images
    // in the ViewControlFrame.
    vtkGetObjectMacro (NavZoomInIconButton, vtkKWPushButton );
    vtkGetObjectMacro (NavZoomOutIconButton, vtkKWPushButton );
    vtkGetObjectMacro (NavZoomScale, vtkKWScale );

    // Description:
    // Get the main slicer toolbars.
    vtkGetObjectMacro (ApplicationGUI, vtkSlicerApplicationGUI );
    virtual void SetApplicationGUI ( vtkSlicerApplicationGUI *appGUI );
    
    // Description:
    // This method builds the Data module's GUI
    virtual void BuildGUI ( vtkKWFrame *appF ) ;
    virtual void AssignIcons ( );
    
    // Description:
    // Add/Remove observers on widgets in the GUI
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );

    // Description:
    // Class's mediator methods for processing events invoked by
    // either the Logic, MRML or GUI.
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
    
    // Description:
    // Describe the behavior at module enter and exit.
    virtual void Enter ( );
    virtual void Exit ( );

       // Description:
    // Groups of callbacks that handle the state change of
    // rollover images in the ViewControlFrame. These
    // callbacks only update the visual behavior of the GUI,
    // but don't impact the application state at all.
    void MakeViewControlRolloverBehavior ( );
    void EnterLookFromACallback ( );
    void LeaveLookFromACallback ( );
    void EnterLookFromPCallback ( );
    void LeaveLookFromPCallback ( );
    void EnterLookFromRCallback ( );
    void LeaveLookFromRCallback ( );    
    void EnterLookFromLCallback ( );
    void LeaveLookFromLCallback ( );
    void EnterLookFromSCallback ( );
    void LeaveLookFromSCallback ( );
    void EnterLookFromICallback ( );
    void LeaveLookFromICallback ( );

    void EnterRotateAroundACallback ( );
    void LeaveRotateAroundACallback ( );
    void EnterRotateAroundPCallback ( );
    void LeaveRotateAroundPCallback ( );
    void EnterRotateAroundRCallback ( );
    void LeaveRotateAroundRCallback ( );    
    void EnterRotateAroundLCallback ( );
    void LeaveRotateAroundLCallback ( );
    void EnterRotateAroundSCallback ( );
    void LeaveRotateAroundSCallback ( );
    void EnterRotateAroundICallback ( );
    void LeaveRotateAroundICallback ( );
    
 protected:
    vtkSlicerViewControlGUI ( );
    virtual ~vtkSlicerViewControlGUI ( );

    vtkSlicerApplicationGUI *ApplicationGUI;
    vtkSlicerViewControlIcons *SlicerViewControlIcons;
    vtkKWCheckButton *SpinButton;
    vtkKWCheckButton *RockButton;
    vtkKWCheckButton *OrthoButton;
    vtkKWPushButton *CenterButton;
    vtkKWMenuButton *SelectButton;
    vtkKWEntryWithLabel *FOVEntry;
    
    // navzoom scale, navzoomin/outiconbutton tmpNavZoom, all the icon buttons.    
    // Description:
    // These widgets display icons that indicate
    // zoom-in and zoom-out functionality in the
    // ViewControlFrame's Navigation widget
    vtkKWPushButton *NavZoomInIconButton;
    vtkKWPushButton *NavZoomOutIconButton;
    vtkKWScale *NavZoomScale;

       // Description:
    // These widgets tile a composite image
    // for automatically rotating the view
    // around a selected axis. The composite image
    // displays state during mouseover.
    vtkKWLabel *RotateAroundAIconButton;
    vtkKWLabel *RotateAroundPIconButton;
    vtkKWLabel *RotateAroundRIconButton;
    vtkKWLabel *RotateAroundLIconButton;
    vtkKWLabel *RotateAroundSIconButton;
    vtkKWLabel *RotateAroundIIconButton;
    vtkKWLabel *RotateAroundMiddleIconButton;    
    vtkKWLabel *RotateAroundTopCornerIconButton;
    vtkKWLabel *RotateAroundBottomCornerIconButton;
    
    // Description:
    // These widgets tile a composite image
    // for automatically positioning the camera down
    // a selected axis and pointing at origin. The composite 
    // image displays state during mouseover.
    vtkKWLabel *LookFromAIconButton;
    vtkKWLabel *LookFromPIconButton;
    vtkKWLabel *LookFromRIconButton;
    vtkKWLabel *LookFromLIconButton;
    vtkKWLabel *LookFromSIconButton;
    vtkKWLabel *LookFromIIconButton;
    vtkKWLabel *LookFromMiddleIconButton;    
    vtkKWLabel *LookFromTopCornerIconButton;
    vtkKWLabel *LookFromBottomCornerIconButton;
    
 private:
    vtkSlicerViewControlGUI ( const vtkSlicerViewControlGUI& ); // Not implemented.
    void operator = ( const vtkSlicerViewControlGUI& ); //Not implemented.
};


#endif
