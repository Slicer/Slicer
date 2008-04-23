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
#include "vtkSlicerViewControlGUI.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkSlicerModuleChooseGUI.h"
#include "vtkSlicerLogoDisplayGUI.h"

#include "vtkSlicerSliceLogic.h"

#include "vtkSlicerWindow.h"
#include "vtkKWFrame.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWIcon.h"
#include "vtkKWMenu.h"

#include "vtkImageData.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkSlicerViewerWidget.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerSliceGUICollection.h"
#include "vtkSlicerFoundationIcons.h"

#include "vtkCacheManager.h"
#include "vtkDataIOManager.h"

class vtkObject;
class vtkLogoWidget;
class vtkKWLabel;
class vtkKWDialog;
class vtkSlicerMRMLSaveDataWidget;
class vtkSlicerFiducialListWidget;

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
        if (this->ViewerWidget == NULL ||
            this->ViewerWidget->GetMainViewer() == NULL ||
            this->ViewerWidget->GetMainViewer()->GetRenderWindow() == NULL)
          {
          return NULL;
          }
        else
          {
          return this->ViewerWidget->GetMainViewer()->GetRenderWindow()->GetInteractor();
          }
    };

    // Description:
    // The main 3D Viewer Widget
    vtkGetObjectMacro (ViewerWidget, vtkSlicerViewerWidget);

    // Description:
    // The Fiducial List Widget
    vtkGetObjectMacro (FiducialListWidget, vtkSlicerFiducialListWidget);
    
    // Description:
    // A pointer to the SliceGUI's SliceGUICollection so
    // the three main Slice Viewers can be added and
    // removed from the collection as required.
    vtkGetObjectMacro (SliceGUICollection, vtkSlicerSliceGUICollection);
    vtkSetObjectMacro (SliceGUICollection, vtkSlicerSliceGUICollection);
    
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
    // Get the frames that populate the Slicer GUI
    vtkGetObjectMacro ( LogoFrame, vtkKWFrame);
    vtkGetObjectMacro ( DropShadowFrame, vtkKWFrame );
    vtkGetObjectMacro ( SlicesControlFrame, vtkSlicerModuleCollapsibleFrame );
    vtkGetObjectMacro ( ViewControlFrame, vtkSlicerModuleCollapsibleFrame );
    
    // Description:
    // A frame used in the MainViewFrame of SlicerMainWin
    vtkGetObjectMacro ( GridFrame1, vtkKWFrame );
    vtkGetObjectMacro ( GridFrame2, vtkKWFrame );
    
    // Description:
    // The following (ApplicationToolbar, ViewControlGUI, SlicesControlGUI,
    // ModuleChooseGUI) are collections of widgets that populate
    // the main applicaiton GUI. Each has a pointer to this instance
    // of vtkSlicerApplicationGUI and the ProcessGUIEvents method
    // in each calls methods from this class.
    // Get the application Toolbar.
    vtkGetObjectMacro ( ApplicationToolbar, vtkSlicerToolbarGUI );
    // Get the GUI containing widgets for controlling the 3D View
    vtkGetObjectMacro ( ViewControlGUI, vtkSlicerViewControlGUI );
    // Get the GUI containing widgets for controlling the Slice Views
    vtkGetObjectMacro ( SlicesControlGUI, vtkSlicerSlicesControlGUI );
    // Get the GUI containing the widgets to select modules.
//    vtkGetObjectMacro ( ModuleChooseGUI, vtkSlicerModuleChooseGUI );
    // Get the GUI containing the widgets to display logos
    vtkGetObjectMacro ( LogoDisplayGUI, vtkSlicerLogoDisplayGUI );
    
    // Description:
    // Get the main slicer window.
    vtkGetObjectMacro ( MainSlicerWindow, vtkSlicerWindow );
    // Description:
    // Basic icons for the slicer application.
    vtkGetObjectMacro ( SlicerFoundationIcons, vtkSlicerFoundationIcons );
    
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
    void ProcessPublishToXnatCommand();
    void ProcessAddDataCommand();
    void ProcessAddVolumeCommand();
    void ProcessSaveSceneAsCommand();
    void ProcessCloseSceneCommand();
    
    // Description:
    // Methods describe behavior on startup and exit.
    virtual void Enter ( );
    virtual void Exit ( );
    virtual void DeleteComponentGUIs();
    
    // Description:
    // These methods configure and pack the Slicer Window
    virtual void PackFirstSliceViewerFrame ( );
    virtual void SetApplicationFontSize ( );
    virtual void SetApplicationFontFamily ( );

    // Description:
    // These methods configure the Main Viewer's layout
    virtual void PackConventionalView ( );
    virtual void PackOneUp3DView ( );
    virtual void PackOneUpSliceView ( const char *whichSlice);
    virtual void PackFourUpView ( );
    virtual void PackTabbed3DView ( );
    virtual void PackTabbedSliceView ( );
    virtual void PackLightboxView ( );

    // Description:
    // Methods to manage Slice viewers
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
    virtual void BuildGUIFrames ( );
    virtual void BuildMainViewer ( int arrangementType);

    virtual void PackMainViewer (  int arrangementType, const char *whichSlice );

    virtual void CreateMain3DViewer ( int arrangementType );
    virtual void CreateMainSliceViewers ( int arrangementType );

    virtual void DestroyMain3DViewer ( );
    virtual void DestroyMainSliceViewers ( );

    virtual void RepackMainViewer ( int arrangementType, const char *whichSlice );
    virtual void UnpackMain3DViewer (  );
    virtual void UnpackMainSliceViewers ( );

    virtual void PopulateModuleChooseList ( );
    virtual void SetCurrentModuleToHome();
    virtual void PythonConsole();

    virtual void InitializeViewControlGUI ();
    virtual void InitializeSlicesControlGUI ();

    virtual void Save3DViewConfig ( );
    virtual void Restore3DViewConfig ( );

    virtual void UpdateFontSizeMenu();
    virtual void UpdateFontFamilyMenu();

    // Description:
    // Methods invoked by making selections from Help menu
    // on the menu bar; give access to Slicer tutorials,
    // and web pages for reporting bugs, usability problems,
    // making feature requests, and a script to upload
    // screenshots and a caption to slicer's visual blog.
    virtual void OpenTutorialsLink ();
    virtual void OpenBugLink ();
    virtual void OpenUsabilityLink ();
    virtual void OpenFeatureLink ();
    virtual void PostToVisualBlog ();

    // Description:
    // Display Slicer's main window
    virtual void DisplayMainSlicerWindow ( );

    // Description:
    // Raise module's panel.
    void SelectModule ( const char *moduleName );

    // Description:
    // Helper routine to set images for icons
    //
    void SetIconImage (vtkKWIcon *icon, vtkImageData *image)
      {
      int *dims = image->GetDimensions();
      int nComps = image->GetNumberOfScalarComponents();
      icon->SetImage ( static_cast <const unsigned char *> (image->GetScalarPointer()),
                       dims[0], dims[1], nComps,
                       dims[0] * dims[1] * nComps, vtkKWIcon::ImageOptionFlipVertical);
      };

    virtual void UpdateRemoteIOConfigurationForRegistry();

    //---Description:
    //--- Called by main application to propagate initial registry
    //--- and subsequent Application Settings changes for remoteio
    //--- through the ApplicationGUI to CacheManager.
    virtual void ConfigureRemoteIOSettings();

protected:
    vtkSlicerApplicationGUI ( );
    virtual ~vtkSlicerApplicationGUI ( );

    // Description:
    // Main Slicer window
    vtkSlicerWindow *MainSlicerWindow;

    // Description:
    // Frames for the main Slicer UI panel    
    vtkKWFrame *TopFrame;
    vtkKWFrame *LogoFrame;
    vtkKWFrame *DropShadowFrame;
    vtkSlicerModuleCollapsibleFrame *SlicesControlFrame;
    vtkSlicerModuleCollapsibleFrame *ViewControlFrame;

    // Description:
    // Frame for Lightbox viewing (not yet implemented)
    vtkKWFrame *GridFrame1;
    vtkKWFrame *GridFrame2;

    // Description:
    // Widgets for the File menu
    vtkKWLoadSaveDialog *LoadSceneDialog;

    // Description:
    // Main Slicer toolbar and components
    vtkSlicerToolbarGUI *ApplicationToolbar;
    vtkSlicerViewControlGUI *ViewControlGUI;
    vtkSlicerSlicesControlGUI *SlicesControlGUI;
//    vtkSlicerModuleChooseGUI *ModuleChooseGUI;
    vtkSlicerLogoDisplayGUI *LogoDisplayGUI;
    
    // Description:
    // Main Slicer 3D Viewer
    vtkSlicerViewerWidget *ViewerWidget;
    double MainRendererBBox[6];

    // Description:
    // Fiducial List Widget
    vtkSlicerFiducialListWidget *FiducialListWidget;
    
    // Description:
    // Main 3 Slice Viewers
    vtkSlicerSliceGUI *MainSliceGUI0;
    vtkSlicerSliceGUI *MainSliceGUI1;
    vtkSlicerSliceGUI *MainSliceGUI2;
    // Description:
    // Logic for three main slice viewers. We keep
    // pointers to these so that they may be reassigned
    // if three viewers are deleted and recreated, as
    // they are during view reconfiguration for instance.
    vtkSlicerSliceLogic *MainSliceLogic0;
    vtkSlicerSliceLogic *MainSliceLogic1;    
    vtkSlicerSliceLogic *MainSliceLogic2;
    // Description:
    // Collection of SliceViewers
    vtkSlicerSliceGUICollection *SliceGUICollection;

    // Description:
    // Collection of Icons all GUIs can have access to.
    // TODO: move basic icons from misc icon collections into here.
    vtkSlicerFoundationIcons *SlicerFoundationIcons;

    // Description:
    // Used to tag all pages added to the tabbed notebook
    // arrangement of the main viewer.
    int ViewerPageTag;

    vtkSlicerMRMLSaveDataWidget *SaveDataWidget;
    
 private:
    vtkSlicerApplicationGUI ( const vtkSlicerApplicationGUI& ); // Not implemented.
    void operator = ( const vtkSlicerApplicationGUI& ); //Not implemented.
}; 

#endif
