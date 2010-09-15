///  vtkSlicerViewControlGUI
/// 
/// Main Data GUI and mediator methods for
/// ViewControl GUI Panel in slicer3.

#ifndef __vtkSlicerViewControlGUI_h
#define __vtkSlicerViewControlGUI_h

#include "vtkObserverManager.h"
#include "vtkImageData.h"
#include "vtkTransform.h"
#include "vtkOutlineSource.h"
#include "vtkPolyDataMapper.h"
#include "vtkImageMapper.h"
#include "vtkActor2D.h"
#include "vtkActor.h"
#include "vtkFollower.h"

#include "vtkSlicerImageCloseUp2D.h"
#include "vtkSlicerImageCrossHair2D.h"
#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerInteractorStyle.h"
#include "vtkSlicerViewerInteractorStyle.h"

#include "vtkMRMLViewNode.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLSelectionNode.h"
#include "vtkMRMLCameraNode.h"

class vtkKWFrame;
class vtkKWPushButton;
class vtkKWTopLevel;
class vtkKWRenderWidget;

class vtkSlicerApplicationGUI;
class vtkSlicerSliceGUI;
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
class vtkKWSimpleEntryDialog;
class vtkKWTopLevel;
class vtkKWLoadSaveButton;

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

/// Description:
/// This class implements Slicer's 3DView Control Panel on Slicer's main GUI Panel.
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerViewControlGUI : public vtkSlicerComponentGUI
{
 public:
  /// 
  /// Usual vtk class functions
  static vtkSlicerViewControlGUI* New ( );
  vtkTypeRevisionMacro ( vtkSlicerViewControlGUI, vtkSlicerComponentGUI );
  void PrintSelf ( ostream& os, vtkIndent indent );

  /// 
  /// Get/Set when a render is pending.
  vtkGetMacro ( NavigationRenderPending, int );
  vtkSetMacro ( NavigationRenderPending, int );
  /// 
  /// Get/Set when a zoom is pending.
  vtkGetMacro ( ZoomRenderPending, int );
  vtkSetMacro ( ZoomRenderPending, int );

  vtkGetMacro (EntryUpdatePending, int );
  vtkSetMacro (EntryUpdatePending, int);
  
  /// 
  /// parameters used for animated rock
  vtkGetMacro ( RockCount, int );
  vtkSetMacro ( RockCount, int );
  vtkGetMacro ( SliceMagnification, double );  
  vtkGetMacro ( SliceInteracting, int );

  vtkGetMacro (ProcessingMRMLEvent, int);
  vtkGetMacro (NavigationZoomWidgetWid, int);
  vtkGetMacro (NavigationZoomWidgetHit, int);
  
  /// 
  /// Icons that modify the widgets in ViewControlGUI
  vtkGetObjectMacro ( SlicerViewControlIcons, vtkSlicerViewControlIcons );

  /// 
  /// Get the widgets in the ViewControlGUI
  vtkGetObjectMacro (SpinButton, vtkKWCheckButton);
  vtkGetObjectMacro (RockButton, vtkKWCheckButton);
  vtkGetObjectMacro (OrthoButton, vtkKWPushButton);
  vtkGetObjectMacro (CenterButton, vtkKWPushButton);
  vtkGetObjectMacro (ScreenGrabButton, vtkKWPushButton);
  vtkGetObjectMacro (StereoButton, vtkKWMenuButton);
  vtkGetObjectMacro (VisibilityButton, vtkKWMenuButton );
  vtkGetObjectMacro (PitchButton, vtkKWPushButton);
  vtkGetObjectMacro (YawButton, vtkKWPushButton);  
  vtkGetObjectMacro (RollButton, vtkKWPushButton);
  vtkGetObjectMacro (ZoomInButton, vtkKWPushButton);
  vtkGetObjectMacro (ZoomOutButton, vtkKWPushButton);
  vtkGetObjectMacro ( SelectSceneSnapshotMenuButton, vtkKWMenuButton );
  vtkGetObjectMacro ( SceneSnapshotButton, vtkKWPushButton );


  /// 
  /// Get the Widgets that display the RotateAround rollover images
  /// and the LookFrom rollover images in the ViewControlGUI
  vtkGetObjectMacro (ViewAxisAIconButton, vtkKWLabel );
  vtkGetObjectMacro (ViewAxisPIconButton, vtkKWLabel );
  vtkGetObjectMacro (ViewAxisRIconButton, vtkKWLabel );
  vtkGetObjectMacro (ViewAxisLIconButton, vtkKWLabel );
  vtkGetObjectMacro (ViewAxisSIconButton, vtkKWLabel );
  vtkGetObjectMacro (ViewAxisIIconButton, vtkKWLabel );
  vtkGetObjectMacro (ViewAxisCenterIconButton, vtkKWLabel );
  vtkGetObjectMacro (ViewAxisTopCornerIconButton, vtkKWLabel );
  vtkGetObjectMacro (ViewAxisBottomCornerIconButton, vtkKWLabel);

  vtkGetObjectMacro (NavigationWidget, vtkKWRenderWidget );
  vtkGetObjectMacro (ZoomWidget, vtkKWRenderWidget );
  vtkGetObjectMacro (NavigationZoomFrame, vtkKWFrame );
  
  /// 
  /// Box that represents the 3DViewer's window in the
  /// Navigation rendered view
  vtkGetObjectMacro (FOVBox, vtkOutlineSource );
  vtkSetObjectMacro (FOVBox, vtkOutlineSource );
  vtkGetObjectMacro (FOVBoxMapper, vtkPolyDataMapper);
  vtkGetObjectMacro (FOVBoxActor, vtkFollower);

  /// 
  /// Get the Widgets that display the Zoom images
  /// and cursor in the ViewControlGUI
  vtkGetObjectMacro (SliceMagnifier, vtkSlicerImageCloseUp2D);
  vtkGetObjectMacro (SliceMagnifierCursor, vtkSlicerImageCrossHair2D);
  vtkGetObjectMacro (SliceMagnifierMapper, vtkImageMapper);
  vtkGetObjectMacro (SliceMagnifierActor, vtkActor2D);

  /// 
  /// Get the main slicer application
  vtkGetObjectMacro (ApplicationGUI, vtkSlicerApplicationGUI );
  virtual void SetApplicationGUI ( vtkSlicerApplicationGUI *appGUI );

  /// 
  /// API for getting & setting SliceGUI and MainViewer's interactor style
  vtkSetObjectMacro ( RedSliceEvents, vtkSlicerInteractorStyle );
  vtkSetObjectMacro ( YellowSliceEvents, vtkSlicerInteractorStyle );
  vtkSetObjectMacro ( GreenSliceEvents, vtkSlicerInteractorStyle );
  vtkGetObjectMacro ( RedSliceEvents, vtkSlicerInteractorStyle );
  vtkGetObjectMacro ( YellowSliceEvents, vtkSlicerInteractorStyle );
  vtkGetObjectMacro ( GreenSliceEvents, vtkSlicerInteractorStyle );
  vtkSetObjectMacro ( MainViewerEvents, vtkSlicerViewerInteractorStyle );

  vtkGetObjectMacro ( MainViewerEvents, vtkSlicerViewerInteractorStyle);

  /// 
  /// Methods for Get/Set the MRMLViewNode (probably
  /// will be modified or phased out), or refactored into
  /// an accompanying Logic class.
  virtual void SetViewNode(vtkMRMLViewNode*);
  vtkGetObjectMacro ( ViewNode, vtkMRMLViewNode );
  vtkGetObjectMacro ( RedSliceNode, vtkMRMLSliceNode );
  vtkGetObjectMacro ( YellowSliceNode, vtkMRMLSliceNode );  
  vtkGetObjectMacro ( GreenSliceNode, vtkMRMLSliceNode );
  vtkMRMLViewNode *GetActiveView();
  vtkMRMLCameraNode *GetActiveCamera();


  /// 
  /// Get the button which enables/disables the navigation widget
  /// Caution: This Button is not packed at the moment
  vtkGetObjectMacro (EnableDisableNavButton, vtkKWCheckButton);

  /// 
  /// These are the base filename and snapshot numbers
  /// used to name screenshots.
  vtkGetStringMacro ( ScreenGrabName );
  vtkSetStringMacro ( ScreenGrabName );
  vtkGetStringMacro ( ScreenGrabDirectory );
  vtkSetStringMacro ( ScreenGrabDirectory );
  vtkGetMacro ( ScreenGrabNumber, int );
  vtkSetMacro ( ScreenGrabNumber, int );
  vtkGetMacro ( ScreenGrabMagnification, int );
  vtkSetMacro ( ScreenGrabMagnification, int );

  /// 
  /// Widgets that  populate the ScreenGrab Window.
  vtkGetObjectMacro ( ScreenGrabOptionsWindow, vtkKWTopLevel );
  vtkGetObjectMacro ( ScreenGrabNameEntry, vtkKWEntry );
  vtkGetObjectMacro ( ScreenGrabNumberEntry, vtkKWEntry );
  vtkGetObjectMacro ( ScreenGrabOverwriteButton, vtkKWCheckButton );
  vtkGetObjectMacro ( ScreenGrabCaptureButton, vtkKWPushButton );
  vtkGetObjectMacro ( ScreenGrabCloseButton, vtkKWPushButton );
  vtkGetObjectMacro ( ScreenGrabDialogButton, vtkKWLoadSaveButton );
  vtkGetObjectMacro ( ScreenGrabMagnificationEntry, vtkKWEntry );
  vtkGetObjectMacro ( ScreenGrabFormatMenuButton, vtkKWMenuButton );

  virtual const char *GetScreenGrabFormat ( );
  virtual void SetScreenGrabFormat ( const char *format );

  /// 
  /// Methods to raise, populate and dismantle the ScreenGrab options window.
  virtual void WithdrawScreenGrabOptionsWindow ( );
  virtual void RaiseScreenGrabOptionsWindow ( );
  virtual void DestroyScreenGrabOptionsWindow ( );
    
  //BTX
  using vtkSlicerComponentGUI::BuildGUI;
  //ETX
    
  /// 
  /// This method builds the Data module's GUI
  virtual void BuildGUI ( vtkKWFrame *appF );
  virtual void TearDownGUI  ( );
  
  /// 
  /// Add/Remove observers on widgets in the GUI
  virtual void AddGUIObservers ( );
  virtual void RemoveGUIObservers ( );

  /// 
  /// Add and remove observers on the
  /// MainViewer so that we can manage
  /// functionality of the Navigation widget
  void UpdateMainViewerInteractorStyles ( );
  void AddMainViewerEventObservers();
  void RemoveMainViewerEventObservers();
  
  /// 
  /// Add and remove observers on the
  /// slice GUIs so that we can manage
  /// functionality of the Zoom widget
  virtual void UpdateSliceGUIInteractorStyles();
  virtual void AddSliceEventObservers();
  virtual void RemoveSliceEventObservers();
  
  /// 
  /// Methods to update GUI from MRML
  virtual void UpdateViewFromMRML();
  virtual void UpdateSlicesFromMRML();
  virtual void UpdateFromMRML ( );
  virtual void UpdateAppearanceFromMRML ( );
  virtual void UpdateBehaviorFromMRML ( );
  virtual void UpdateSceneSnapshotsFromMRML ( );

  virtual void RestoreSceneSnapshot( const char *name);
  virtual void DeleteSceneSnapshot( const char *name);  
  
  /// 
  /// Renders the Navigation/Zoom widget fresh
  /// when scene has been modified, or
  /// when view is changed.
  virtual void RequestNavigationRender ( );
  virtual void NavigationRender ( );
  virtual void RequestZoomRender ( );
  virtual void ZoomRender ( );
  
  /// 
  /// Class's mediator methods for processing events invoked by
  /// either the Logic, MRML or GUI.
  virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  //BTX
  using vtkSlicerComponentGUI::Enter;
  //ETX
  
  /// 
  /// Describe the behavior at module enter and exit.
  virtual void Enter ( );
  virtual void Exit ( );

  /// 
  /// manages the fiducial visibility across ViewControlGUI,
  /// SliceControlGUI and FiducialsGUI
  virtual void SetMRMLFiducialPointVisibility ( int state);
  virtual void SetMRMLFiducialLabelVisibility ( int state);
  virtual const char *CreateSceneSnapshotNode ( const char *name );
  virtual int InvokeNameDialog( const char *msg, const char *name);

  /// 
  /// Starts and stops automatic view spinning
  virtual void MainViewSpin ( );
  virtual void SpinView (int direction, double degrees );
  /// 
  /// Zooms the main view with user-specified
  /// relative (percent) zoom factor.
  virtual void MainViewZoom ( double factor );
  /// 
  /// Resets focal point to origin
  virtual void MainViewResetFocalPoint ( );
  /// 
  /// Sets cameras focal point.
  virtual void MainViewSetFocalPoint ( double x, double y, double z);
  /// 
  /// Starts and stops automatic view rocking
  virtual void MainViewRock ( );
  virtual void RockView ( );
  /// 
  /// Moves camera down selected axis in MainViewer and looks at
  /// focal point from there
  virtual void MainViewLookFrom ( const char *dir );
  /// 
  /// Rotates camera about selected axis by an increment in MainViewer
  virtual void MainViewRotateAround ( int axis );
  virtual void MainViewRotateAround ( const char *axis );
  virtual void ArbitraryRotate(double *p, double theta, double *p1, double *p2, double *q);

  /// 
  /// Basic zoom in/out, pitch roll and yaw controls for the main viewer.
  virtual void MainViewZoomIn();
  virtual void MainViewZoomOut();
  virtual void MainViewPitch();
  virtual void MainViewRoll();
  virtual void MainViewYaw();

  /// 
  /// Sets either Parallel or Perspective Projection in MainViewer
  virtual void MainViewSetProjection ( );
  /// 
  /// Sets stereo options or turns off stereo, if
  /// stereo is enabled in MainViewer
  virtual void MainViewSetStereo ( );
  /// 
  /// Sets background color in the MainViewer
  virtual void MainViewBackgroundColor ( double *color );
  /// 
  /// Toggles visibility of actors in the MainViewer
  virtual void MainViewVisibility( );

  /// 
  /// Creates the magnified slice view in ZoomWidget
  /// when mouse moves over a slice window.
  virtual void SliceViewMagnify( int event, vtkSlicerInteractorStyle *istyle);

  /// 
  /// Keeps the actors added to the Navigation Widget's
  /// renderer the same as those in the 3DView's renderer.
  virtual void UpdateNavigationWidgetViewActors ( );

  /// 
  /// Updates the Navigation widget's camera to track
  /// the 3DView camera
  virtual void ConfigureNavigationWidgetRender ( );

  /// 
  /// Configures the Navigation widget's camera on
  /// startup.
  virtual void InitializeNavigationWidgetCamera ( );

  ///  Similar to vtkRenderer's ResetCamera
  /// method, but tries to use a smaller ViewAngle to
  /// compute a nearer camera distance that still
  /// accommodates all visible actors in scene but fills
  /// more of the little render window with stuff. Not
  /// working any differently than ResetCamera yet,
  /// for reasons unknown... 
  virtual void ResetNavigationCamera ( );

  /// 
  /// Create the rectangle on the NavigationWidget that
  /// indicates the 3DView's FOV.
  virtual void CreateFieldOfViewBoxActor ( );
  
  /// 
  /// Method updates the GUI's widget's font size
  /// when a user selects a different font size.
  /// Normally the theme would take care of this,
  /// but since the GUI isn't derived from a vtkKWWidget,
  /// KWWidgets theme mechanism doesn't recognize the
  /// class context.
  virtual void ReconfigureGUIFonts ( );
  
  /// 
  /// Groups of callbacks that handle the state change of
  /// rollover images in the ViewControlFrame. These
  /// callbacks only update the visual behavior of the GUI,
  /// but don't impact the application state at all.
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

  /// 
  /// Assigns <enter> and <leave> bindings for rollover images.
  void MakeViewControlRolloverBehavior ( );

  /// 
  /// Builds pulldown menus for GUI menubuttons
  void BuildStereoSelectMenu ( );
  void BuildVisibilityMenu ( );

  /// 
  /// Methods for unpacking and packing the
  /// Navigation and Zoom widgets into same parcel.
  virtual void PackNavigationWidget ( );
  virtual void PackZoomWidget ( ) ;

  /// 
  /// Convert an xy device coordinate into an XYZ coordinate in a slice
  /// node (where xy is now relative to a viewport (lightbox) and z is a slice).
  virtual void DeviceCoordinatesToXYZ(vtkSlicerSliceGUI *sgui,
                                      int x, int y, int xyz[3] );
  
 protected:
  vtkSlicerViewControlGUI ( );
  virtual ~vtkSlicerViewControlGUI ( );
  /// 
  /// Check if an interaction occurs in the main viewer. If that is the case, abort navigation rendering
  void CheckAbort(void);
    
  int NavigationRenderPending;
  int ZoomRenderPending;
  int  EntryUpdatePending;
  int ProcessingMRMLEvent;
  bool SceneClosing;
  
  vtkSlicerApplicationGUI *ApplicationGUI;
  vtkSlicerViewControlIcons *SlicerViewControlIcons;

  //Caution: this Button is not packed at the moment
  //Enable/Disable if the Navigation widget is active or not
  vtkKWCheckButton *EnableDisableNavButton;
  vtkKWCheckButton *SpinButton;
  vtkKWCheckButton *RockButton;
  vtkKWPushButton *OrthoButton;
  vtkKWPushButton *CenterButton;
  vtkKWMenuButton *StereoButton;
  vtkKWPushButton *ScreenGrabButton;
  vtkKWMenuButton *VisibilityButton;
  vtkKWPushButton *PitchButton;
  vtkKWPushButton *YawButton;
  vtkKWPushButton *RollButton;
  vtkKWPushButton *ZoomInButton;
  vtkKWPushButton *ZoomOutButton;

  //--- Screen snapshot configure window
  vtkKWTopLevel *ScreenGrabOptionsWindow;
  vtkKWEntry *ScreenGrabNameEntry;
  vtkKWEntry *ScreenGrabNumberEntry;
  vtkKWEntry *ScreenGrabMagnificationEntry;
  vtkKWMenuButton *ScreenGrabFormatMenuButton;
  vtkKWCheckButton *ScreenGrabOverwriteButton;
  vtkKWPushButton *ScreenGrabCaptureButton;
  vtkKWPushButton *ScreenGrabCloseButton;
  vtkKWLoadSaveButton *ScreenGrabDialogButton;

  vtkKWMenuButton *SelectSceneSnapshotMenuButton;
  vtkKWPushButton *SceneSnapshotButton;


  /// navzoom scale, navzoomin/outiconbutton tmpNavigationZoom, all the icon buttons.    
  /// 
  /// These widgets display icons that indicate
  /// zoom-in and zoom-out functionality in the
  /// ViewControlFrame's Navigation widget
  vtkKWRenderWidget *NavigationWidget;
  vtkKWRenderWidget *ZoomWidget;
  vtkKWFrame *NavigationZoomFrame;

  /// 
  /// These widgets tile a composite image
  /// for labeling the 3 axes (R,L,S,I,A,P)
  /// The composite image displays highlighting
  /// behavior during mouseover
  vtkKWLabel *ViewAxisAIconButton;
  vtkKWLabel *ViewAxisPIconButton;
  vtkKWLabel *ViewAxisRIconButton;
  vtkKWLabel *ViewAxisLIconButton;
  vtkKWLabel *ViewAxisSIconButton;
  vtkKWLabel *ViewAxisIIconButton;
  vtkKWLabel *ViewAxisCenterIconButton;    
  vtkKWLabel *ViewAxisTopCornerIconButton;
  vtkKWLabel *ViewAxisBottomCornerIconButton;

  /// 
  /// pipeline objects for accomplishing the slice zoom
  /// in the Zoom widget.
  vtkSlicerImageCloseUp2D *SliceMagnifier;
  vtkSlicerImageCrossHair2D *SliceMagnifierCursor;
  vtkImageMapper *SliceMagnifierMapper;
  vtkActor2D *SliceMagnifierActor;
  
  /// 
  /// objects for building the FOV box in the
  /// Navigation widget
  vtkOutlineSource *FOVBox;
  vtkPolyDataMapper *FOVBoxMapper;
  vtkFollower *FOVBoxActor;

  /// 
  /// MRML and GUI objects that this class will need
  /// to set and observe.
  vtkMRMLViewNode *ViewNode;
  vtkSlicerInteractorStyle *RedSliceEvents;
  vtkSlicerInteractorStyle *YellowSliceEvents;
  vtkSlicerInteractorStyle *GreenSliceEvents;
  vtkMRMLSliceNode *RedSliceNode;
  vtkMRMLSliceNode *YellowSliceNode;
  vtkMRMLSliceNode *GreenSliceNode;
  vtkSlicerViewerInteractorStyle *MainViewerEvents;
  vtkKWSimpleEntryDialog *NameDialog;

  int SelectedSceneSnapshot;
  int RockCount;
  int NavigationZoomWidgetWid;
  int NavigationZoomWidgetHit;
  double SliceMagnification;
  int SliceInteracting;
  const char *MySnapshotName;

  char *ScreenGrabName;
  char *ScreenGrabDirectory;
  int ScreenGrabNumber;
  int ScreenGrabMagnification;
  int ScreenGrabOverwrite;

  //BTX
  std::string ScreenGrabFormat;
  //ETX


 private:
  vtkSlicerViewControlGUI ( const vtkSlicerViewControlGUI& ); /// Not implemented.
  void operator = ( const vtkSlicerViewControlGUI& ); //Not implemented.
};


#endif
