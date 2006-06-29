/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationGUI.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/
// .NAME vtkSlicerApplicationGUI 
// .SECTION Description
// Main application GUI and mediator methods for slicer3.  

#ifndef __vtkSlicerApplicationGUI_h
#define __vtkSlicerApplicationGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerToolbarIcons.h"
#include "vtkSlicerLogoIcons.h"
#include "vtkSlicerModuleNavigationIcons.h"
#include "vtkSlicerViewControlIcons.h"

#include "vtkSlicerWindow.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWLoadSaveDialog.h"

#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"

class vtkObject;
class vtkKWPushButton;
class vtkKWToolbar;
class vtkKWScale;
class vtkImplicitPlaneWidget;
class vtkKWCheckButton;
class vtkKWEntryWithLabel;
class vtkKWLabel;

// Description:
// This class implements Slicer's main Application GUI.
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerApplicationGUI : public vtkSlicerComponentGUI
{
 public:
    // Description:
    // Usual vtk class functions
    static vtkSlicerApplicationGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerApplicationGUI, vtkSlicerComponentGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );

    // Description:
    // These Get/Set methods for frames in the GUI panel.
    vtkGetObjectMacro ( MainViewer, vtkKWRenderWidget );

    vtkRenderWindowInteractor *GetRenderWindowInteractor() {
        return this->MainViewer->GetRenderWindow()->GetInteractor();
    };

    // Description:
    // Get the frames that populate the Slicer GUI
    vtkGetObjectMacro ( LogoFrame, vtkKWFrame);
    vtkGetObjectMacro ( ModuleChooseFrame, vtkKWFrame );
    vtkGetObjectMacro ( SliceControlFrame, vtkKWFrame );
    vtkGetObjectMacro ( ViewControlFrame, vtkKWFrame );
    vtkGetObjectMacro ( DefaultSlice0Frame, vtkKWFrame );
    vtkGetObjectMacro ( DefaultSlice1Frame, vtkKWFrame );
    vtkGetObjectMacro ( DefaultSlice2Frame, vtkKWFrame );

    // Description:
    // Get the widgets that display the toolbar icons
    vtkGetObjectMacro (HomeIconButton, vtkKWPushButton); 
    vtkGetObjectMacro (DataIconButton, vtkKWPushButton);
    vtkGetObjectMacro (VolumeIconButton, vtkKWPushButton );
    vtkGetObjectMacro (ModelIconButton, vtkKWPushButton );
    vtkGetObjectMacro (EditorIconButton, vtkKWPushButton );
    vtkGetObjectMacro (EditorToolboxIconButton, vtkKWPushButton );
    vtkGetObjectMacro (TransformIconButton, vtkKWPushButton );
    vtkGetObjectMacro (ColorIconButton, vtkKWPushButton );
    vtkGetObjectMacro (FiducialsIconButton, vtkKWPushButton);
    vtkGetObjectMacro (SaveSceneIconButton, vtkKWPushButton );
    vtkGetObjectMacro (LoadSceneIconButton, vtkKWPushButton );
    vtkGetObjectMacro (ConventionalViewIconButton, vtkKWPushButton );    
    vtkGetObjectMacro (OneUp3DViewIconButton, vtkKWPushButton );
    vtkGetObjectMacro (OneUpSliceViewIconButton, vtkKWPushButton );
    vtkGetObjectMacro (FourUpViewIconButton, vtkKWPushButton );
    vtkGetObjectMacro (TabbedViewIconButton, vtkKWPushButton );
    vtkGetObjectMacro (LightBoxViewIconButton, vtkKWPushButton );
    vtkGetObjectMacro (MousePickIconButton, vtkKWPushButton );
    vtkGetObjectMacro (MousePanIconButton, vtkKWPushButton );
    vtkGetObjectMacro (MouseRotateIconButton, vtkKWPushButton );
    vtkGetObjectMacro (MouseZoomIconButton, vtkKWPushButton );

    // Description:
    // Get the widgets in the LogoFrame
    vtkGetObjectMacro (SlicerLogoLabel, vtkKWLabel);

    // Description:
    // Get the widgets in the ModuleChooseFrame
    vtkGetObjectMacro ( ModulesMenuButton, vtkKWMenuButton );
    vtkGetObjectMacro ( ModulesLabel, vtkKWLabel );
    vtkGetObjectMacro ( ModulesPrev, vtkKWPushButton );
    vtkGetObjectMacro ( ModulesNext, vtkKWPushButton );    
    vtkGetObjectMacro (ModulesHistory, vtkKWPushButton );
    vtkGetObjectMacro (ModulesRefresh, vtkKWPushButton );

    // Description:
    // Get the widgets in the SliceControlFrame
    vtkGetObjectMacro (ToggleAnnotationButton, vtkKWPushButton );
    vtkGetObjectMacro (ToggleFgBgButton, vtkKWPushButton );
    vtkGetObjectMacro (SliceFadeScale, vtkKWScale );
    vtkGetObjectMacro (SliceOpacityScale, vtkKWScale );

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


    // Description:
    // Get the Widgets that display the Navigation Zoom images
    // in the ViewControlFrame.
    vtkGetObjectMacro (NavZoomInIconButton, vtkKWPushButton );
    vtkGetObjectMacro (NavZoomOutIconButton, vtkKWPushButton );
    vtkGetObjectMacro (NavZoomScale, vtkKWScale );

    // Description:
    // Get the class containing all slicer GUI images for logos/icons
    vtkGetObjectMacro (SlicerToolbarIcons, vtkSlicerToolbarIcons);
    vtkGetObjectMacro (SlicerLogoIcons, vtkSlicerLogoIcons );
    vtkGetObjectMacro (SlicerModuleNavigationIcons, vtkSlicerModuleNavigationIcons );
    vtkGetObjectMacro (SlicerViewControlIcons, vtkSlicerViewControlIcons );
    
    // Description:
    // Get the main slicer window.
    vtkGetObjectMacro ( MainSlicerWin, vtkSlicerWindow );
    // Description:

    // Get the main slicer toolbars.
    vtkGetObjectMacro (ModulesToolbar, vtkKWToolbar);
    vtkGetObjectMacro (LoadSaveToolbar, vtkKWToolbar );
    vtkGetObjectMacro (ViewToolbar, vtkKWToolbar);
    vtkGetObjectMacro (MouseModeToolbar, vtkKWToolbar);
    
    // Description:
    // a Plane widget in the main window
    vtkGetObjectMacro ( PlaneWidget, vtkImplicitPlaneWidget );
    
    // Description:
    // This method builds Slicer's main GUI
    virtual void BuildGUI ( );

    // Description:
    // Add/Remove observers on widgets in Slicer's main GUI
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );

    // Description:
    // Class's mediator methods for processing events invoked by
    // the Logic, MRML or GUI objects observed.
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

    void ProcessLoadSceneCommand();
    void ProcessSaveSceneAsCommand();
    
    // Description:
    // Methods describe behavior on startup and exit.
    virtual void Enter ( );
    virtual void Exit ( );
    
    // Description:
    // These methods configure and pack the Slicer Window
    virtual void ConfigureMainSlicerWindow ( );
    virtual void ConfigureMainViewerPanel ( );
    virtual void ConfigureSliceViewersPanel ( );
    virtual void PackSliceViewers ( );
    virtual void PackFirstSliceViewer ( );
    virtual void UnpackSliceViewers ( );
    virtual void ConfigureGUIPanel ( );
    
    // Description:
    // These methods populate the various GUI Panel frames
    virtual void BuildToolBar ( );
    virtual void BuildMainViewer ( );
    virtual void BuildLogoGUIPanel ( );
    virtual void BuildSlicerControlGUIPanel ( );
    virtual void BuildSliceControlGUIPanel ( );
    virtual void BuildViewControlGUIPanel ( );
    virtual void AssignViewControlIcons ( );
    virtual void MakeViewControlRolloverBehavior ( );

    // Desrciption:
    // These methods delete widgets belonging to components of the Slicer Window
    virtual void DeleteToolbarWidgets ( );
    virtual void DeleteGUIPanelWidgets ( );
    virtual void DeleteFrames ( );

    // Description:
    // Display Slicer's main window
    virtual void DisplayMainSlicerWindow ( );

    // Description:
    // Groups of callbacks that handle the state change of
    // rollover images in the ViewControlFrame. These
    // callbacks only update the visual behavior of the GUI,
    // but don't impact the application state at all.
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
    vtkSlicerApplicationGUI ( );
    ~vtkSlicerApplicationGUI ( );

    // Description:
    // Main Slicer window
    vtkSlicerWindow *MainSlicerWin;

    // Description:
    // Contains logos and icons
    vtkSlicerToolbarIcons *SlicerToolbarIcons;
    vtkSlicerLogoIcons *SlicerLogoIcons;
    vtkSlicerModuleNavigationIcons *SlicerModuleNavigationIcons;
    vtkSlicerViewControlIcons *SlicerViewControlIcons;
    
    // Description:
    // Widgets for the main Slicer UI panel    
    vtkKWFrame *LogoFrame;
    vtkKWFrame *ModuleChooseFrame;
    vtkKWFrame *SliceControlFrame;
    vtkKWFrame *ViewControlFrame;
    vtkKWFrame *DefaultSlice0Frame;
    vtkKWFrame *DefaultSlice1Frame;
    vtkKWFrame *DefaultSlice2Frame;
    vtkKWRenderWidget *MainViewer;
    vtkImplicitPlaneWidget *PlaneWidget;

    // Description:
    // Widgets for the Logo frame
    vtkKWLabel *SlicerLogoLabel;

    // Description:
    // Widgets for the modules GUI panels
    vtkKWMenuButton *ModulesMenuButton;
    vtkKWLabel *ModulesLabel;
    vtkKWPushButton *ModulesPrev;
    vtkKWPushButton *ModulesNext;
    vtkKWPushButton *ModulesHistory;
    vtkKWPushButton *ModulesRefresh;
    
    //Description:
    // Widgets for the SliceControlFrame in the GUI
    vtkKWPushButton *ToggleAnnotationButton;
    vtkKWPushButton *ToggleFgBgButton;
    vtkKWScale *SliceFadeScale;
    vtkKWScale *SliceOpacityScale;

    //Description:
    // Widgets for the ViewControlFrame in the GUI
    vtkKWCheckButton *SpinButton;
    vtkKWCheckButton *RockButton;
    vtkKWCheckButton *OrthoButton;
    vtkKWPushButton *CenterButton;
    vtkKWMenuButton *SelectButton;
    vtkKWEntryWithLabel *FOVEntry;
    
    vtkKWLoadSaveDialog *LoadSceneDialog;
    vtkKWLoadSaveDialog *SaveSceneDialog;

    // Description:
    // Main Slicer toolbars
    vtkKWToolbar *ModulesToolbar;
    vtkKWToolbar *LoadSaveToolbar;
    vtkKWToolbar *ViewToolbar;
    vtkKWToolbar *MouseModeToolbar;

    // Description:
    // Widgets that display toolbar icons
    vtkKWPushButton *HomeIconButton;
    vtkKWPushButton *DataIconButton;
    vtkKWPushButton *VolumeIconButton;
    vtkKWPushButton *ModelIconButton;
    vtkKWPushButton *EditorIconButton;
    vtkKWPushButton *EditorToolboxIconButton;
    vtkKWPushButton *TransformIconButton;    
    vtkKWPushButton *ColorIconButton;
    vtkKWPushButton *FiducialsIconButton;
    vtkKWPushButton *SaveSceneIconButton;
    vtkKWPushButton *LoadSceneIconButton;
    vtkKWPushButton *ConventionalViewIconButton;
    vtkKWPushButton *OneUp3DViewIconButton;
    vtkKWPushButton *OneUpSliceViewIconButton;
    vtkKWPushButton *FourUpViewIconButton;
    vtkKWPushButton *TabbedViewIconButton;
    vtkKWPushButton *LightBoxViewIconButton;
    vtkKWPushButton *MousePickIconButton;
    vtkKWPushButton *MousePanIconButton;
    vtkKWPushButton *MouseRotateIconButton;
    vtkKWPushButton *MouseZoomIconButton;

    
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

    // Description:
    // These widgets display icons that indicate
    // zoom-in and zoom-out functionality in the
    // ViewControlFrame's Navigation widget
    vtkKWPushButton *NavZoomInIconButton;
    vtkKWPushButton *NavZoomOutIconButton;
    vtkKWScale *NavZoomScale;
    
 private:
    vtkSlicerApplicationGUI ( const vtkSlicerApplicationGUI& ); // Not implemented.
    void operator = ( const vtkSlicerApplicationGUI& ); //Not implemented.
}; 

#endif
