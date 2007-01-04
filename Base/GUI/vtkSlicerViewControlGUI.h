// .NAME vtkSlicerViewControlGUI
// .SECTION Description
// Main Data GUI and mediator methods for
// ViewControl GUI Panel in slicer3.

#ifndef __vtkSlicerViewControlGUI_h
#define __vtkSlicerViewControlGUI_h

#include "vtkObserverManager.h"

#include "vtkMRMLViewNode.h"
#include "vtkMRMLCameraNode.h"

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerComponentGUI.h"


class vtkKWFrame;
class vtkKWPushButton;
class vtkKWTopLevel;
class vtkKWRenderWidget;

class vtkSlicerApplicationGUI;
class vtkKWScale;
class vtkKWScaleWithEntry;
class vtkKWPushButton;
class vtkKWCheckButton;
class vtkKWRadioButton;
class vtkKWLabel;
class vtkKWMenuButton;
class vtkKWEntry;
class vtkKWEntryWithLabel;
class vtkSlicerViewControlIcons;

class vtkCallbackCommand;

//BTX
#ifndef vtkSetAndObserveMRMLNodeMacro
#define vtkSetAndObserveMRMLNodeMacro(node,value)  { \
  vtkObject *oldNode = (node); \
  this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &(node) ), (value) ); \
  if ( oldNode != (node) ) \
    { \
    this->InvokeEvent (vtkCommand::ModifiedEvent); \
    } \
};
#endif

#ifndef vtkSetAndObserveMRMLNodeEventsMacro
#define vtkSetAndObserveMRMLNodeEventsMacro(node,value,events)  { \
  vtkObject *oldNode = (node); \
  this->MRMLObserverManager->SetAndObserveObjectEvents ( vtkObjectPointer( &(node)), (value), (events)); \
  if ( oldNode != (node) ) \
    { \
    this->InvokeEvent (vtkCommand::ModifiedEvent); \
    } \
};
#endif
//ETX

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
  // Set when a render is pending.
  vtkGetMacro ( RenderPending, int );
  vtkSetMacro ( RenderPending, int );

  // Description:
  // used for animated rock
  vtkGetMacro ( RockCount, int );
  vtkSetMacro ( RockCount, int );

  // Description:
  // Get the widgets in the ViewControlFrame
  vtkGetObjectMacro (SpinButton, vtkKWCheckButton);
  vtkGetObjectMacro (RockButton, vtkKWCheckButton);
  vtkGetObjectMacro (OrthoButton, vtkKWPushButton);
  vtkGetObjectMacro (CenterButton, vtkKWPushButton);
  vtkGetObjectMacro (SelectViewButton, vtkKWMenuButton);
  vtkGetObjectMacro (SelectCameraButton, vtkKWMenuButton);
  vtkGetObjectMacro (StereoButton, vtkKWMenuButton);
  vtkGetObjectMacro (VisibilityButton, vtkKWMenuButton );
  vtkGetObjectMacro (FOVEntry, vtkKWEntryWithLabel);
  vtkGetObjectMacro (ZoomEntry, vtkKWEntryWithLabel);
  vtkGetObjectMacro (LookFromButton, vtkKWRadioButton);
  vtkGetObjectMacro (RotateAroundButton, vtkKWRadioButton );
  vtkGetObjectMacro (NavZoomFrame, vtkKWFrame );
  
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
  vtkGetObjectMacro (NavWidget, vtkKWRenderWidget );
  vtkGetObjectMacro (ZoomWidget, vtkKWRenderWidget );
    
  vtkGetObjectMacro ( SlicerViewControlIcons, vtkSlicerViewControlIcons );

  // Description:
  // Get the Widgets that display the Navigation Zoom images
  // in the ViewControlFrame.
  vtkGetObjectMacro (NavZoomInIconButton, vtkKWPushButton );
  vtkGetObjectMacro (NavZoomOutIconButton, vtkKWPushButton );
  vtkGetObjectMacro (NavZoomScale, vtkKWScale );
  vtkGetObjectMacro (NavZoomLabel, vtkKWLabel );

    // Description:
    // API for setting SliceNode, SliceLogic and
    // for both setting and observing them.
    void SetMRMLViewNode ( vtkMRMLViewNode *node )
        { vtkSetMRMLNodeMacro ( this->ViewNode, node ); }
    void SetAndObserveMRMLViewNode ( vtkMRMLViewNode *node )
        { vtkSetAndObserveMRMLNodeMacro (this->ViewNode, node ); }
    vtkGetObjectMacro ( ViewNode, vtkMRMLViewNode );

    void SetMRMLCameraNode ( vtkMRMLCameraNode *node )
        { vtkSetMRMLNodeMacro ( this->CameraNode, node ); }
    void SetAndObserveMRMLCameraNode ( vtkMRMLCameraNode *node )
        { vtkSetAndObserveMRMLNodeMacro (this->CameraNode, node ); }
    vtkGetObjectMacro ( CameraNode, vtkMRMLCameraNode );

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
  // Make sure the view control GUI is observing
  // the current (active) MRMLCameraNode.
  // Removes existing observers,
  // sets and observes active camera node
  virtual void UpdateCamerasFromMRML ( );

  virtual void RemoveViewObservers () ;
  virtual void AddViewObservers ( );

  // Description:
  // Removes existing observers,
  // using RemoveViewObservers()
  // Updates all view nodes from MRML
  // and puts observers on them using AddViewObservers().
  virtual void UpdateView ( vtkMRMLViewNode *node );
  // Description:
  // Makes sure the view control GUI is
  // observing the current (active) MRMLViewNode.
  // Calls UpdateView() with the current active node.
  virtual void UpdateViewsFromMRML ( );
  // Description:
  // Called whenever a node is created or
  // deleted from the scene. It calls
  // UpdateViewsFromMRML and UpdateCamarasFromMRML.
  virtual void UpdateFromMRML ( );

  virtual void UpdateNavZoomCameraAndActors ( );

  // Description:
  // Renders the Nav/Zoom widget fresh
  // when scene has been modified, or
  // when view is changed.
  virtual void RequestRender ( );
  virtual void Render ( );
  
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
  virtual void MainViewSpin ( );
  virtual void SpinView (int direction, double degrees );

  // Description:
  // Resets focal point to origin
  virtual void MainViewResetFocalPoint ( );
  // Description:
  // Sets cameras focal point.
  virtual void MainViewSetFocalPoint ( double x, double y, double z);
    
  // Description:
  // Starts and stops automatic view rocking
  virtual void MainViewRock ( );
  virtual void RockView ( );

  // Description:
  // Moves camera down selected axis and looks at
  // focal point from there
  virtual void MainViewLookFrom ( const char *dir );
  // Description:
  // Rotates camera about selected axis by an increment.
  virtual void MainViewRotateAround ( int axis );

  // Description:
  // Sets either Parallel or Perspective Projection
  virtual void MainViewSetProjection ( );

  // Description:
  // Sets stereo options or turns off stereo, if
  // stereo is enabled.
  virtual void MainViewSetStereo ( );

  // Description:
  // Sets background color in the Main View
  virtual void MainViewBackgroundColor ( double *color );

  // Description:
  // Toggles visibility of actors in the Main View
  virtual void MainViewVisibility( );
  
  // Description:
  // Groups of callbacks that handle the state change of
  // rollover images in the ViewControlFrame. These
  // callbacks only update the visual behavior of the GUI,
  // but don't impact the application state at all.
  void ViewControlACallback ( );
  void ViewControlPCallback ( );
  void ViewControlSCallback ( );
  void ViewControlICallback ( );
  void ViewControlLCallback ( );
  void ViewControlRCallback ( );
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
  void BuildCameraSelectMenu ( );
  void BuildViewSelectMenu ( );
  void BuildStereoSelectMenu ( );
  void BuildVisibilityMenu ( );

  virtual void PackNavWidget ( );
  virtual void PackZoomWidget ( ) ;

 protected:
  vtkSlicerViewControlGUI ( );
  virtual ~vtkSlicerViewControlGUI ( );
    
  int RenderPending;
  bool SceneClosing;
  int ProcessingMRMLEvent;
  
  vtkSlicerApplicationGUI *ApplicationGUI;
  vtkSlicerViewControlIcons *SlicerViewControlIcons;
  vtkKWCheckButton *SpinButton;
  vtkKWCheckButton *RockButton;
  vtkKWPushButton *OrthoButton;
  vtkKWRadioButton *LookFromButton;
  vtkKWRadioButton *RotateAroundButton;
  vtkKWPushButton *CenterButton;
  vtkKWMenuButton *StereoButton;
  vtkKWMenuButton *SelectViewButton;
  vtkKWMenuButton *SelectCameraButton;
  vtkKWMenuButton *VisibilityButton;
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
  vtkKWRenderWidget *NavWidget;
  vtkKWRenderWidget *ZoomWidget;
  vtkKWFrame *NavZoomFrame;

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

  vtkMRMLViewNode *ViewNode;
  vtkMRMLCameraNode *CameraNode;
  
  int RockCount;

 private:
  vtkSlicerViewControlGUI ( const vtkSlicerViewControlGUI& ); // Not implemented.
  void operator = ( const vtkSlicerViewControlGUI& ); //Not implemented.
};


#endif
