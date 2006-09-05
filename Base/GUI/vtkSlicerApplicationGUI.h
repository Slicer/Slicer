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
#include "vtkSlicerToolbarGUI.h"
#include "vtkSlicerLogoIcons.h"
#include "vtkSlicerModuleNavigationIcons.h"
#include "vtkSlicerViewControlIcons.h"
#include "vtkSlicerSliceLogic.h"

#include "vtkSlicerWindow.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWLoadSaveDialog.h"

#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSlicerViewerWidget.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerSliceGUICollection.h"

class vtkObject;
class vtkKWPushButton;
class vtkKWScale;
class vtkImplicitPlaneWidget;
class vtkLogoWidget;
class vtkKWCheckButton;
class vtkKWEntryWithLabel;
class vtkKWLabel;
class vtkKWDialog;
class vtkSlicerMRMLSaveDataWidget;

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

    vtkRenderWindowInteractor *GetRenderWindowInteractor() {
        return this->ViewerWidget->GetMainViewer()->GetRenderWindow()->GetInteractor();
    };

    // Description:
    // The main 3D Viewer Widget
    vtkGetObjectMacro (ViewerWidget, vtkSlicerViewerWidget);

    // Description:
    // The main three Slice Viewers and temporary storage
    // of their logic nodes used during view reconfiguration
    vtkGetObjectMacro (MainSliceGUI0, vtkSlicerSliceGUI);
    vtkGetObjectMacro (MainSliceGUI1, vtkSlicerSliceGUI);
    vtkGetObjectMacro (MainSliceGUI2, vtkSlicerSliceGUI);
    vtkGetObjectMacro (MainSliceLogic0, vtkSlicerSliceLogic);
    vtkSetObjectMacro (MainSliceLogic0, vtkSlicerSliceLogic);
    vtkGetObjectMacro (MainSliceLogic1, vtkSlicerSliceLogic);
    vtkSetObjectMacro (MainSliceLogic1, vtkSlicerSliceLogic);
    vtkGetObjectMacro (MainSliceLogic2, vtkSlicerSliceLogic);
    vtkSetObjectMacro (MainSliceLogic2, vtkSlicerSliceLogic);

    // Description:
    // A frame used in the MainViewFrame of SlicerMainWin
    // when lightbox view is being displayed.
    vtkGetObjectMacro ( LightboxFrame, vtkKWFrame );
    
    // Description:
    // A pointer to the SliceGUI's SliceGUICollection so
    // the three main Slice Viewers can be added and
    // removed from the collection as required.
    vtkGetObjectMacro (SliceGUICollection, vtkSlicerSliceGUICollection);
    vtkSetObjectMacro (SliceGUICollection, vtkSlicerSliceGUICollection);
    
    // Description:
    // Get the frames that populate the Slicer GUI
    vtkGetObjectMacro ( LogoFrame, vtkKWFrame);
    vtkGetObjectMacro ( ModuleChooseFrame, vtkKWFrame );
    vtkGetObjectMacro ( SliceControlFrame, vtkKWFrame );
    vtkGetObjectMacro ( ViewControlFrame, vtkKWFrame );
    //    vtkGetObjectMacro ( DefaultSlice0Frame, vtkKWFrame );
    //    vtkGetObjectMacro ( DefaultSlice1Frame, vtkKWFrame );
    //    vtkGetObjectMacro ( DefaultSlice2Frame, vtkKWFrame );

    // Description:
    // Get the application Toolbar.
    vtkGetObjectMacro ( ApplicationToolbar, vtkSlicerToolbarGUI );

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
    vtkGetObjectMacro (SlicerLogoIcons, vtkSlicerLogoIcons );
    vtkGetObjectMacro (SlicerModuleNavigationIcons, vtkSlicerModuleNavigationIcons );
    vtkGetObjectMacro (SlicerViewControlIcons, vtkSlicerViewControlIcons );
    
    // Description:
    // Get the main slicer window.
    vtkGetObjectMacro ( MainSlicerWin, vtkSlicerWindow );
    // Description:

    // Description:
    // a Plane widget in the main window
    vtkGetObjectMacro ( PlaneWidget, vtkImplicitPlaneWidget );

    // Description:
    // a logo widget in the main window
    vtkGetObjectMacro ( LogoWidget, vtkLogoWidget );
    
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
    void ProcessImportSceneCommand();
    void ProcessSaveSceneAsCommand();
    
    // Description:
    // Methods describe behavior on startup and exit.
    virtual void Enter ( );
    virtual void Exit ( );
    
    // Description:
    // These methods configure and pack the Slicer Window
    virtual void PackFirstSliceViewerFrame ( );

    virtual void DisplayConventionalView ( );
    virtual void DisplayOneUp3DView ( );
    virtual void DisplayOneUpSliceView ( );
    virtual void DisplayFourUpView ( );
    virtual void DisplayTabbed3DViewSliceViewers ( );
    virtual void DisplayTabbedSliceView ( );
    virtual void DisplayLightboxView ( );

    virtual void AddSliceGUIToCollection ( vtkSlicerSliceGUI *s );
    virtual void RemoveSliceGUIFromCollection ( vtkSlicerSliceGUI *s);
    virtual void ConfigureMainSliceViewers ( );
    virtual void AddMainSliceViewersToCollection ( );
    virtual void RemoveMainSliceViewersFromCollection ( );
    virtual void AddMainSliceViewerObservers ( );
    virtual void RemoveMainSliceViewerObservers ( );
    virtual void SetAndObserveMainSliceLogic ( vtkSlicerSliceLogic *l1,
                                               vtkSlicerSliceLogic *l2,
                                               vtkSlicerSliceLogic *l3 );
    
    // Description:
    // These methods populate the various GUI Panel frames
    virtual void BuildGUIPanel ( );
    virtual void BuildMainViewer ( int arrangementType);
    virtual void DestroyMain3DViewer ( );
    virtual void DestroyMainSliceViewers ( );
    virtual void CreateMain3DViewer ( int arrangementType );
    virtual void CreateMainSliceViewers ( int arrangementType );
    virtual void BuildLogoGUIPanel ( );
    virtual void BuildModuleChooseGUIPanel ( );
    virtual void PopulateModuleChooseList ( );
    virtual void BuildSliceControlGUIPanel ( );
    virtual void BuildViewControlGUIPanel ( );
    virtual void AssignViewControlIcons ( );
    virtual void MakeViewControlRolloverBehavior ( );
    virtual void Save3DViewConfig ( );
    virtual void Restore3DViewConfig ( );

    // Desrciption:
    // These methods delete widgets belonging to components of the Slicer Window
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
    virtual ~vtkSlicerApplicationGUI ( );

    // Description:
    // Main Slicer window
    vtkSlicerWindow *MainSlicerWin;

    vtkSlicerLogoIcons *SlicerLogoIcons;
    vtkSlicerModuleNavigationIcons *SlicerModuleNavigationIcons;
    vtkSlicerViewControlIcons *SlicerViewControlIcons;
    
    // Description:
    // Widgets for the main Slicer UI panel    
    vtkKWFrame *LogoFrame;
    vtkKWFrame *ModuleChooseFrame;
    vtkKWFrame *SliceControlFrame;
    vtkKWFrame *ViewControlFrame;
    //    vtkKWFrame *DefaultSlice0Frame;
    //    vtkKWFrame *DefaultSlice1Frame;
    //    vtkKWFrame *DefaultSlice2Frame;
    vtkImplicitPlaneWidget *PlaneWidget;
    vtkLogoWidget *LogoWidget;

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
    // Main Slicer toolbar
    vtkSlicerToolbarGUI *ApplicationToolbar;
    
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

    vtkSlicerViewerWidget *ViewerWidget;
    vtkSlicerSliceGUI *MainSliceGUI0;
    vtkSlicerSliceGUI *MainSliceGUI1;
    vtkSlicerSliceGUI *MainSliceGUI2;
    vtkKWFrame *LightboxFrame;
    vtkSlicerSliceGUICollection *SliceGUICollection;
    vtkSlicerSliceLogic *MainSliceLogic0;
    vtkSlicerSliceLogic *MainSliceLogic1;    
    vtkSlicerSliceLogic *MainSliceLogic2;

    // Description:
    // Used to tag all pages added to the tabbed notebook
    // arrangement of the main viewer.
    int ViewerPageTag;
    // Description:

    // contains the visible prop bounds for the main 3Dviewer's renderer
    double MainRendererBBox[6];

    vtkSlicerMRMLSaveDataWidget *SaveDataWidget;
    vtkKWDialog                 *SaveDataDialog;
    
 private:
    vtkSlicerApplicationGUI ( const vtkSlicerApplicationGUI& ); // Not implemented.
    void operator = ( const vtkSlicerApplicationGUI& ); //Not implemented.
}; 

#endif
