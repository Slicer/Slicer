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
// Main application GUI for slicer3.  Points to the ApplicationLogic and
// reflects changes in that logic back onto the UI.  Also routes changes
// from the GUI into the Logic to effect the user's desires.


#ifndef __vtkSlicerApplicationGUI_h
#define __vtkSlicerApplicationGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerComponentGUI.h"

// include GUIs for Slicer Base modules
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerVolumesGUI.h"
#include "vtkSlicerModelsGUI.h"

// includes + fwd declarations for ApplicationGUI widgets
#include "vtkKWWindow.h"
#include "vtkKWMenuButton.h"
#include "vtkKWFrame.h"

class vtkObject;
class vtkKWUserInterfacePanel;
class vtkKWPushButton;

// Description:
// This class implements Slicer's main Application GUI.
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerApplicationGUI : public vtkSlicerComponentGUI
{
 public:
    static vtkSlicerApplicationGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerApplicationGUI, vtkSlicerComponentGUI );

    // Description:
    // Get/Set Macros for protected vtkSlicerApplicationGUI dimensions
    vtkGetMacro ( DefaultSlicerWindowWidth, int );
    vtkSetMacro ( DefaultSlicerWindowWidth, int );
    vtkGetMacro ( DefaultSlicerWindowHeight, int );
    vtkSetMacro ( DefaultSlicerWindowHeight, int );
    vtkGetMacro ( DefaultMainViewerWidth, int);
    vtkSetMacro ( DefaultMainViewerWidth, int);
    vtkGetMacro ( DefaultMainViewerHeight, int);
    vtkSetMacro ( DefaultMainViewerHeight, int);
    vtkGetMacro ( DefaultSliceGUIFrameHeight, int);
    vtkSetMacro ( DefaultSliceGUIFrameHeight, int);
    vtkGetMacro ( DefaultSliceGUIFrameWidth, int);
    vtkSetMacro ( DefaultSliceGUIFrameWidth, int);
    vtkGetMacro ( DefaultSliceWindowWidth, int);
    vtkSetMacro ( DefaultSliceWindowWidth, int);
    vtkGetMacro ( DefaultSliceWindowHeight, int);
    vtkSetMacro ( DefaultSliceWindowHeight, int);
    vtkGetMacro ( DefaultGUIPanelWidth, int);
    vtkSetMacro ( DefaultGUIPanelWidth, int);
    vtkGetMacro ( DefaultGUIPanelHeight, int);
    vtkSetMacro ( DefaultGUIPanelHeight, int);
    vtkGetMacro ( DefaultLogoFrameHeight, int );
    vtkSetMacro ( DefaultLogoFrameHeight, int );
    vtkGetMacro ( DefaultSlicerControlFrameHeight, int );
    vtkSetMacro ( DefaultSlicerControlFrameHeight, int );
    vtkGetMacro ( DefaultModuleControlFrameHeight, int );
    vtkSetMacro ( DefaultModuleControlFrameHeight, int );
    vtkGetMacro ( DefaultSliceControlFrameHeight, int );
    vtkSetMacro ( DefaultSliceControlFrameHeight, int );
    vtkGetMacro ( DefaultViewControlFrameHeight, int );
    vtkSetMacro ( DefaultViewControlFrameHeight, int );

    // Description:
    // These Get/Set methods for frames in the GUI panel.
    vtkGetObjectMacro ( ModuleControlFrame, vtkKWFrame );
    vtkSetObjectMacro ( ModuleControlFrame, vtkKWFrame );
    vtkGetObjectMacro ( SlicerControlFrame, vtkKWFrame );
    vtkSetObjectMacro ( SlicerControlFrame, vtkKWFrame );
    vtkGetObjectMacro ( SliceControlFrame, vtkKWFrame );
    vtkSetObjectMacro ( SliceControlFrame, vtkKWFrame );
    vtkGetObjectMacro ( ViewControlFrame, vtkKWFrame );
    vtkSetObjectMacro ( ViewControlFrame, vtkKWFrame );
    vtkGetObjectMacro ( ModulesButton, vtkKWMenuButton );
    vtkSetObjectMacro ( ModulesButton, vtkKWMenuButton );
    vtkGetObjectMacro ( HomeButton, vtkKWPushButton );
    vtkSetObjectMacro ( HomeButton, vtkKWPushButton );
    vtkGetObjectMacro ( VolumesButton, vtkKWPushButton );
    vtkSetObjectMacro ( VolumesButton, vtkKWPushButton );
    vtkGetObjectMacro ( ModelsButton, vtkKWPushButton );
    vtkSetObjectMacro ( ModelsButton, vtkKWPushButton );
    vtkGetObjectMacro ( DataButton, vtkKWPushButton );
    vtkSetObjectMacro ( DataButton, vtkKWPushButton );

    // Description:
    // These Get/Set methods for Slice Base Module GUIs.
    vtkGetObjectMacro ( SliceGUI, vtkSlicerSliceGUI);
    vtkSetObjectMacro ( SliceGUI, vtkSlicerSliceGUI);
    vtkGetObjectMacro ( VolumesGUI, vtkSlicerVolumesGUI);
    vtkSetObjectMacro ( VolumesGUI, vtkSlicerVolumesGUI);
    vtkGetObjectMacro ( ModelsGUI, vtkSlicerModelsGUI);
    vtkSetObjectMacro ( ModelsGUI, vtkSlicerModelsGUI);
    vtkGetObjectMacro ( MainSlicerWin, vtkKWWindow );
    vtkSetObjectMacro ( MainSlicerWin, vtkKWWindow );
    
    // Description:
    // This method builds Slicer's main GUI
    virtual void BuildGUI ( );
    virtual void AddGUIObservers ( );
    virtual void AddLogicObservers ( );
    virtual void AddMrmlObservers ( );
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event,
                                            void *callData );
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event,
                                           void *callData );
    virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                           void *callData );
    
    // Description:
    // These methods set up default dimensions for the Slicer Window
    virtual void InitDefaultGUIPanelDimensions ( );
    virtual void InitDefaultSlicePanelDimensions ( );
    virtual void InitDefaultMainViewerDimensions ( );
    virtual void InitDefaultSlicerWindowDimensions ( );
    
    // Desrciption:
    // These methods delete widgets belonging to components of the Slicer Window
    virtual void DeleteSliceGUI ( );
    virtual void DeleteGUIPanelWidgets ( );
    virtual void DeleteFrames ( );

    // Description:
    // These methods configure and pack the Slicer Window
    virtual void ConfigureMainSlicerWindow ( );
    virtual void ConfigureMainViewer ( );
    virtual void ConfigureSliceViewers ( );
    virtual void ConfigureGUIPanel ( );
    // Description:
    // These methods populate the various GUI Panel frames
    virtual void BuildLogoGUI ( );
    virtual void BuildSlicerControlGUI ( );
    virtual void BuildModuleControlGUI ( );
    virtual void BuildSliceControlGUI ( );
    virtual void BuildViewControlGUI ( );
    // Description:
    // Display Slicer's main window
    virtual void DisplayMainSlicerWindow ( );

 protected:
    vtkSlicerApplicationGUI ( );
    ~vtkSlicerApplicationGUI ( );

    // Description:
    // Widgets for the main Slicer UI panel    
    vtkKWFrame *LogoFrame;
    vtkKWFrame *SlicerControlFrame;
    vtkKWFrame *ModuleControlFrame;
    vtkKWFrame *SliceControlFrame;
    vtkKWFrame *ViewControlFrame;
    vtkKWPushButton *HomeButton;
    vtkKWPushButton *DataButton;
    vtkKWPushButton *VolumesButton;
    vtkKWPushButton *ModelsButton;
    vtkKWFrame *DefaultSlice0Frame;
    vtkKWFrame *DefaultSlice1Frame;
    vtkKWFrame *DefaultSlice2Frame;

    // Description:
    // Widgets for the modules GUI panels
    vtkKWUserInterfacePanel *ui_panel;
    vtkKWMenuButton *ModulesButton;
    
    // Description:
    // Dimensions for the Default Window & components
    int DefaultSlicerWindowHeight;
    int DefaultSlicerWindowWidth;
    int DefaultMainViewerHeight;
    int DefaultMainViewerWidth;
    int DefaultSliceGUIFrameHeight;
    int DefaultSliceGUIFrameWidth;
    int DefaultSliceWindowHeight;
    int DefaultSliceWindowWidth;
    int DefaultGUIPanelHeight;
    int DefaultGUIPanelWidth;

    // Description:
    // Dimensions for specific GUI panel components
    int DefaultLogoFrameHeight;
    int DefaultSlicerControlFrameHeight;
    int DefaultModuleControlFrameHeight;
    int DefaultSliceControlFrameHeight;    
    int DefaultViewControlFrameHeight;
    
    // Description:
    // Slice GUI containing SliceWidgetCollection
    vtkKWWindow *MainSlicerWin;
    vtkSlicerSliceGUI *SliceGUI;
    vtkSlicerVolumesGUI *VolumesGUI;
    vtkSlicerModelsGUI *ModelsGUI;

 private:
    vtkSlicerApplicationGUI ( const vtkSlicerApplicationGUI& ); // Not implemented.
    void operator = ( const vtkSlicerApplicationGUI& ); //Not implemented.
}; 

#endif
