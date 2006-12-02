// .NAME vtkSlicerViewControlGUI
// .SECTION Description
// Main Data GUI and mediator methods for
// ViewControl GUI Panel in slicer3.

#ifndef __vtkSlicerViewControlGUI_h
#define __vtkSlicerViewControlGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerComponentGUI.h"

class vtkKWFrame;
class vtkKWPushButton;
class vtkSlicerApplicationGUI;
class vtkKWScale;
class vtkKWPushButton;
class vtkKWCheckButton;
class vtkKWRadioButton;
class vtkKWLabel;
class vtkKWMenuButton;
class vtkKWEntryWithLabel;
class vtkSlicerViewControlIcons;

// Description:
// This class implements Slicer's 3DView Control Panel on Slicer's main GUI Panel.
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
  vtkGetObjectMacro (OrthoButton, vtkKWPushButton);
  vtkGetObjectMacro (CenterButton, vtkKWPushButton);
  vtkGetObjectMacro (SelectButton, vtkKWMenuButton);
  vtkGetObjectMacro (StereoButton, vtkKWMenuButton);

  vtkGetObjectMacro (FOVEntry, vtkKWEntryWithLabel);
  vtkGetObjectMacro (ZoomEntry, vtkKWEntryWithLabel);
  vtkGetObjectMacro (LookFromButton, vtkKWRadioButton);
  vtkGetObjectMacro (RotateAroundButton, vtkKWRadioButton );

  // Description:
  // Flags to indicate active view spinning and rocking

  vtkGetMacro ( Spin, int );
  vtkSetMacro ( Spin, int );
  vtkGetMacro ( SpinDirection, int );
  vtkSetMacro ( SpinDirection, int );    
  vtkGetMacro ( SpinDegrees, double );
  vtkSetMacro ( SpinDegrees, double );
  vtkGetMacro ( SpinMs, int );
  vtkSetMacro ( SpinMs, int );
    
  vtkGetMacro ( Rock, int );
  vtkSetMacro ( Rock, int );
  vtkGetMacro ( RockLength, int );
  vtkSetMacro ( RockLength, int );
  vtkGetMacro ( RockCount, int );
  vtkSetMacro ( RockCount, int );

  // Description:
  // flags to indicate stereo modes
  vtkGetMacro ( Stereo, int );
  vtkSetMacro ( Stereo, int );
  vtkGetMacro ( StereoType, int );
  vtkSetMacro ( StereoType, int );

  // Description:
  // Flag to indicate camera control mode ( look from direction, or rotate around axis )
  vtkGetMacro ( ViewAxisMode, int );
  vtkSetMacro ( ViewAxisMode, int );
    
  // Description:
  // Flag to indicate render mode (orthographic or perspective )
  vtkGetMacro ( RenderMode, int );
  vtkSetMacro ( RenderMode, int );
  
  // Description:
  // Get the Widgets that display the RotateAround image
  // in the ViewControlFrame.
  vtkGetObjectMacro (ViewAxisAIconButton, vtkKWLabel );
  vtkGetObjectMacro (ViewAxisPIconButton, vtkKWLabel );
  vtkGetObjectMacro (ViewAxisRIconButton, vtkKWLabel );
  vtkGetObjectMacro (ViewAxisLIconButton, vtkKWLabel );
  vtkGetObjectMacro (ViewAxisSIconButton, vtkKWLabel );
  vtkGetObjectMacro (ViewAxisIIconButton, vtkKWLabel );
  vtkGetObjectMacro (ViewAxisCenterIconButton, vtkKWLabel );
  vtkGetObjectMacro (ViewAxisTopCornerIconButton, vtkKWLabel );
  vtkGetObjectMacro (ViewAxisBottomCornerIconButton, vtkKWLabel);
    
  vtkGetObjectMacro ( SlicerViewControlIcons, vtkSlicerViewControlIcons );

  // Description:
  // Get the Widgets that display the Navigation Zoom images
  // in the ViewControlFrame.
  vtkGetObjectMacro (NavZoomInIconButton, vtkKWPushButton );
  vtkGetObjectMacro (NavZoomOutIconButton, vtkKWPushButton );
  vtkGetObjectMacro (NavZoomScale, vtkKWScale );
  vtkGetObjectMacro (NavZoomLabel, vtkKWLabel );

  // Description:
  // Get the main slicer toolbars.
  vtkGetObjectMacro (ApplicationGUI, vtkSlicerApplicationGUI );
  virtual void SetApplicationGUI ( vtkSlicerApplicationGUI *appGUI );
    
  // Description:
  // This method builds the Data module's GUI
  virtual void BuildGUI ( vtkKWFrame *appF );
    
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
  // Starts and stops automatic view spinning
  virtual void StopViewSpin ( );
  virtual void MainViewSpin ( );

  // Description:
  // Starts and stops automatic view rocking
  virtual void StopViewRock ( );
  virtual void MainViewRock ( );

  // Description:
  // Rotates the main view in direction: Up(0), Down(1), Left(2), Right(3)
  // by degrees if specified, or by default number of degrees.
  virtual void MainViewRotate ( int direction  );
  virtual void MainViewRotate ( int direction, double degrees );
    
  // Description:
  // Groups of callbacks that handle the state change of
  // rollover images in the ViewControlFrame. These
  // callbacks only update the visual behavior of the GUI,
  // but don't impact the application state at all.
  void EnterViewAxisACallback ( );
  void LeaveViewAxisACallback ( );
  void EnterViewAxisPCallback ( );
  void LeaveViewAxisPCallback ( );
  void EnterViewAxisRCallback ( );
  void LeaveViewAxisRCallback ( );    
  void EnterViewAxisLCallback ( );
  void LeaveViewAxisLCallback ( );
  void EnterViewAxisSCallback ( );
  void LeaveViewAxisSCallback ( );
  void EnterViewAxisICallback ( );
  void LeaveViewAxisICallback ( );

  void MakeViewControlRolloverBehavior ( );
  void BuildViewSelectMenu ( );
  void BuildStereoSelectMenu ( );
  
  //BTX
  // Modes for controlling camera by clicking axes
  enum
    {
      RotateAround = 0,
      LookFrom
    };
    
  // Rotate camera directions
  enum
    {
      Up = 0,
      Down,
      Left,
      Right
    };
    
  // Stereo modes
  enum
    {
      NoStereo = 0,
      FullColor,
      RedBlue
    };

  // render modes
  enum
    {
      Perspective = 0,
      Orthographic
    };
  //ETX

 protected:
  vtkSlicerViewControlGUI ( );
  virtual ~vtkSlicerViewControlGUI ( );
    
  vtkSlicerApplicationGUI *ApplicationGUI;
  vtkSlicerViewControlIcons *SlicerViewControlIcons;
  vtkKWCheckButton *SpinButton;
  vtkKWCheckButton *RockButton;
  vtkKWPushButton *OrthoButton;
  vtkKWRadioButton *LookFromButton;
  vtkKWRadioButton *RotateAroundButton;
  vtkKWPushButton *CenterButton;
  vtkKWMenuButton *StereoButton;
  vtkKWMenuButton *SelectButton;
  vtkKWEntryWithLabel *FOVEntry;
  vtkKWEntryWithLabel *ZoomEntry;
    
  // navzoom scale, navzoomin/outiconbutton tmpNavZoom, all the icon buttons.    
  // Description:
  // These widgets display icons that indicate
  // zoom-in and zoom-out functionality in the
  // ViewControlFrame's Navigation widget
  vtkKWPushButton *NavZoomInIconButton;
  vtkKWPushButton *NavZoomOutIconButton;
  vtkKWScale *NavZoomScale;
  vtkKWLabel *NavZoomLabel;

  // Description:
  // These widgets tile a composite image
  // for labeling the 3 axes (R,L,S,I,A,P)
  // The composite image displays highlighting
  // behavior during mouseover
  vtkKWLabel *ViewAxisAIconButton;
  vtkKWLabel *ViewAxisPIconButton;
  vtkKWLabel *ViewAxisRIconButton;
  vtkKWLabel *ViewAxisLIconButton;
  vtkKWLabel *ViewAxisSIconButton;
  vtkKWLabel *ViewAxisIIconButton;
  vtkKWLabel *ViewAxisCenterIconButton;    
  vtkKWLabel *ViewAxisTopCornerIconButton;
  vtkKWLabel *ViewAxisBottomCornerIconButton;

  // Description:
  // parameters of automatic spin
  int Spin;
  int SpinDirection;
  double SpinDegrees;
  int SpinMs;

  // Description:
  // parameters of automatic rock
  int Rock;
  int RockLength;
  int RockCount;

  // Description:
  // parameters for stereo viewing
  int Stereo;
  int StereoType;
    
  // Description:
  // mode that switches camera increments
  // between 'rotate around' and 'look from'
  // when user clicks on the little axis control
  int ViewAxisMode;
  int RenderMode;

 private:
  vtkSlicerViewControlGUI ( const vtkSlicerViewControlGUI& ); // Not implemented.
  void operator = ( const vtkSlicerViewControlGUI& ); //Not implemented.
};


#endif
