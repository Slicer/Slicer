/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerApplicationGUI.h,v $
  Date:      $Date$
  Version:   $Revision$

=========================================================================auto=*/
///  vtkSlicerApplicationGUI 
/// 
/// Main application GUI and mediator methods for slicer3.  

#ifndef __vtkSlicerApplicationGUI_h
#define __vtkSlicerApplicationGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerToolbarGUI.h"
#include "vtkSlicerViewControlGUI.h"
#include "vtkSlicerModuleChooseGUI.h"
#include "vtkSlicerLogoDisplayGUI.h"
#include "vtkMRMLSliceLogic.h"

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
//#include "vtkSlicerSliceGUICollection.h"
#include "vtkSlicerFoundationIcons.h"
#include "vtkSlicerFoundationIcons.h"

#include "vtkCacheManager.h"
#include "vtkDataIOManager.h"
#include "vtkMRMLLayoutNode.h"

class vtkObject;
class vtkLogoWidget;
class vtkKWLabel;
class vtkKWDialog;
class vtkSlicerMRMLSaveDataWidget;
class vtkSlicerFiducialListWidget;
class vtkSlicerROIViewerWidget;
class vtkSlicerSlicesGUI;
class vtkSlicerSlicesControlGUI;
class vtkSlicerModulesWizardDialog;
class vtkSlicerApplicationGUIInternals;
class vtkMRMLViewNode;
class vtkSlicerApplication;

/// Description:
/// This class implements Slicer's main Application GUI.
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerApplicationGUI : public vtkSlicerComponentGUI
{
 public:
    /// 
    /// Usual vtk class functions
    static vtkSlicerApplicationGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerApplicationGUI, vtkSlicerComponentGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );

    /// 
    /// Get Viewer Widget associated to a given view node
    virtual int GetNumberOfViewerWidgets();
    virtual vtkSlicerViewerWidget* GetNthViewerWidget(int idx);
    virtual vtkSlicerViewerWidget* GetViewerWidgetForNode(vtkMRMLViewNode*);

    /// 
    /// Get the active 3D Viewer Widget
    virtual vtkSlicerViewerWidget* GetActiveViewerWidget();

    /// 
    /// Get active render window interactor (the interactor of the
    /// active 3D viewer)
    virtual vtkRenderWindowInteractor *GetActiveRenderWindowInteractor();

    /// 
    /// Get Fiducial List Widget associated to a given view node
    virtual int GetNumberOfFiducialListWidgets();
    virtual vtkSlicerFiducialListWidget* GetNthFiducialListWidget(int idx);
    virtual vtkSlicerFiducialListWidget* GetFiducialListWidgetForNode(vtkMRMLViewNode*);
    /// 
    /// Get the active Fiducial List Widget
    virtual vtkSlicerFiducialListWidget* GetActiveFiducialListWidget();
    
    /// 
    /// Get/Set the layout node
    vtkMRMLLayoutNode *GetGUILayoutNode ( );
    void SetAndObserveGUILayoutNode ( vtkMRMLLayoutNode *node );
    const char* GetCurrentLayoutStringName ( );

    vtkGetMacro (Built, bool);
    vtkSetMacro (Built, bool);
    vtkGetMacro (CurrentLayout, int);
    vtkSetMacro (CurrentLayout, int);
    
    /// 
    /// The ROI Viewer Widget
    vtkGetObjectMacro (ROIViewerWidget, vtkSlicerROIViewerWidget);

    /// 
    /// Pointers to the SlicesGUI used by the ApplicationGUI.
    vtkGetObjectMacro (SlicesGUI, vtkSlicerSlicesGUI);
    virtual void SetSlicesGUI(vtkSlicerSlicesGUI*);

    /// 
    /// Get the frames that populate the Slicer GUI
    vtkGetObjectMacro ( LogoFrame, vtkKWFrame);
    vtkGetObjectMacro ( DropShadowFrame, vtkKWFrame );
    vtkGetObjectMacro ( SlicesControlFrame, vtkSlicerModuleCollapsibleFrame );
    vtkGetObjectMacro ( ViewControlFrame, vtkSlicerModuleCollapsibleFrame );
    
    /// 
    /// A frame used in the MainViewFrame of SlicerMainWin
    vtkGetObjectMacro ( GridFrame1, vtkKWFrame );
    vtkGetObjectMacro ( GridFrame2, vtkKWFrame );
    
    /// 
    /// The following (ApplicationToolbar, ViewControlGUI, SlicesControlGUI,
    /// ModuleChooseGUI) are collections of widgets that populate
    /// the main applicaiton GUI. Each has a pointer to this instance
    /// of vtkSlicerApplicationGUI and the ProcessGUIEvents method
    /// in each calls methods from this class.
    /// Get the application Toolbar.
    vtkGetObjectMacro ( ApplicationToolbar, vtkSlicerToolbarGUI );
    /// Get the GUI containing widgets for controlling the 3D View
    vtkGetObjectMacro ( ViewControlGUI, vtkSlicerViewControlGUI );
    /// Get the GUI containing widgets for controlling the Slice Views
    vtkGetObjectMacro ( SlicesControlGUI, vtkSlicerSlicesControlGUI );
    /// Get the GUI containing the widgets to select modules.
///    vtkGetObjectMacro ( ModuleChooseGUI, vtkSlicerModuleChooseGUI );
    /// Get the GUI containing the widgets to display logos
    vtkGetObjectMacro ( LogoDisplayGUI, vtkSlicerLogoDisplayGUI );
    
    /// 
    /// Get the main slicer window.
    vtkGetObjectMacro ( MainSlicerWindow, vtkSlicerWindow );

    /// 
    /// Basic icons for the slicer application.
    vtkGetObjectMacro ( SlicerFoundationIcons, vtkSlicerFoundationIcons );
    
    /// 
    /// This method builds Slicer's main GUI
    virtual void BuildGUI ( void );
    virtual void BuildGUI ( vtkKWFrame * f ) { this->Superclass::BuildGUI(f); }
    virtual void BuildGUI ( vtkKWFrame * f, double * bgColor ) { this->Superclass::BuildGUI(f,bgColor); }

    vtkSlicerSliceGUI* GetMainSliceGUI(const char *layoutName);
    void AddMainSliceGUI(const char *layoutName);

    /// 
    /// Add/Remove observers on widgets in Slicer's main GUI
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );

    /// 
    /// Class's mediator methods for processing events invoked by
    /// the Logic, MRML or GUI objects observed.
    virtual void ProcessLogicEvents(
      vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessGUIEvents (
      vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessMRMLEvents (
      vtkObject *caller, unsigned long event, void *callData );

    ///
    /// Download sample data: general method
    /// and specific methods that map to specific
    /// file menu commands. GOTO: make this
    /// more general.
    ///
    //void ProcessDownloadSampleLabelMap (const char *uri );
    //void ProcessDownloadSampleModel( const char *uri);
    void DownloadSampleVolume( const char *uri);
    void ProcessDownloadDTIBrain();
    void ProcessDownloadCTChest();
    void ProcessDownloadCTACardio();
    void ProcessDownloadMRIHead();
    void ProcessDownloadRegistration1();
    void ProcessDownloadRegistration2();

    void ProcessLoadSceneCommand();
    void ProcessImportSceneCommand();
    void ProcessPublishToXnatCommand();
    void ProcessAddDataCommand();
    void ProcessAddVolumeCommand();
    void ProcessAddTransformCommand();
    void ProcessSaveSceneAsCommand();
    void ProcessCloseSceneCommand();
    void ProcessAddRulerCommand();
    void ProcessMouseModeToggleCommand();
    void UpdateLayout();
    void CustomizeStatusBarLayout();

    
    /// 
    /// Methods describe behavior on startup and exit.
    virtual void Enter ( );
    virtual void Enter ( vtkMRMLNode* node ) { this->Superclass::Enter(node); }
    virtual void Exit ( );
    virtual void DeleteComponentGUIs();
    
    /// 
    /// Gives progress feedback in the main slicer window
    /// and is called when a storage node's StageReadData is called.
    /// Provides "signs of life" when large datasets are being loaded.
    virtual void UpdateLoadStatusText();

    /// 
    /// Gives progress feedback in the main slicer window
    /// and is called when a storage node's StageWriteData is called.
    /// Provides "signs of life" when large datasets are being saved.
    virtual void UpdateSaveStatusText();
    
    /// 
    /// These methods configure and pack the Slicer Window
    virtual void PackFirstSliceViewerFrame ( );

    /// 
    /// These methods configure the Main Viewer's layout
    virtual void PackConventionalView ( );
    virtual void PackOneUp3DView ( );
    virtual void PackOneUpSliceView ( const char *whichSlice);
    virtual void PackFourUpView ( );
    virtual void PackTabbed3DView ( );
    virtual void PackTabbedSliceView ( );
    virtual void PackLightboxView ( );
    virtual void PackCompareView();
    virtual void PackSideBySideLightboxView();
    virtual void PackDual3DView();
    virtual void UnpackConventionalView ( );
    virtual void UnpackOneUp3DView ( );
    virtual void UnpackOneUpSliceView ();
    virtual void UnpackFourUpView ( );
    virtual void UnpackTabbed3DView ( );
    virtual void UnpackTabbedSliceView ( );
    virtual void UnpackLightboxView ( );
    virtual void UnpackCompareView();
    virtual void UnpackSideBySideLightboxView();
    virtual void UnpackDual3DView();

    /// 
    /// Methods to manage Slice viewers
    virtual void ConfigureMainSliceViewers ( );
    virtual void AddMainSliceViewerObservers ( );
    virtual void RemoveMainSliceViewerObservers ( );
    virtual void SetAndObserveMainSliceLogic ( vtkMRMLSliceLogic *l1,
                                               vtkMRMLSliceLogic *l2,
                                               vtkMRMLSliceLogic *l3 );
    
    /// 
    /// These methods populate the various GUI Panel frames
    virtual void BuildGUIFrames ( );
    virtual void BuildMainViewer ( int arrangementType);

    virtual void PackMainViewer (  int arrangementType, const char *whichSlice );
    virtual void UnpackMainViewer ( );

    virtual void UpdateMain3DViewers ( );
    virtual void CreateMainSliceViewers ( );

    virtual void TearDownViewers ( );
    virtual void DestroyMain3DViewer ( );
    virtual void DestroyMainSliceViewers ( );

    virtual void RepackMainViewer ( int arrangementType, const char *whichSlice );

    virtual void PopulateModuleChooseList ( );
    virtual void SetCurrentModuleToHome();
    //BTX
    virtual void PythonCommand(char *cmd) {this->PythonCommand(static_cast<const char *>(cmd));}
    //ETX
    virtual void PythonCommand(const char *cmd);
    virtual void PythonConsole();
    /// Result of the most recent PythonCommand
    vtkGetStringMacro(PythonResult);
    vtkSetStringMacro(PythonResult);

    virtual void InitializeViewControlGUI ();
    virtual void InitializeSlicesControlGUI ();

    virtual void Save3DViewConfig ( );
    virtual void Restore3DViewConfig ( );

    /// 
    /// Methods invoked by making selections from Help menu
    /// on the menu bar; give access to Slicer tutorials,
    /// and web pages for reporting bugs, usability problems,
    /// making feature requests, and a script to upload
    /// screenshots and a caption to slicer's visual blog.
    virtual void OpenTutorialsLink ();
    virtual void OpenDocumentationLink ();
    virtual void OpenPublicationsLink ();
    virtual void OpenBugLink ();
    virtual void OpenUsabilityLink ();
    virtual void OpenFeatureLink ();
    virtual void PostToVisualBlog ();

    /// 
    /// Method to display the Loadable Modules wizard
    virtual void ShowModulesWizard();

    /// 
    /// Display Slicer's main window
    virtual void DisplayMainSlicerWindow ( );

    /// 
    /// Raise module's panel.
    /// - use either the module name or a node
    /// - SelectModuleForNode has internal logic to map node types to modules
    void SelectModule ( const char *moduleName, vtkMRMLNode *node );
    void SelectModule ( const char *moduleName );
    void SelectModuleForNode ( vtkMRMLNode *node );

    /// 
    /// Helper routine to set images for icons
    void SetIconImage (vtkKWIcon *icon, vtkImageData *image);

    virtual void UpdateRemoteIOConfigurationForRegistry();

    //---Description:
    //--- Called by main application to propagate initial registry
    //--- and subsequent Application Settings changes for remoteio
    //--- through the ApplicationGUI to CacheManager.
    virtual void ConfigureRemoteIOSettings();

    /// 
    /// Callback set on the configure event of main window
    virtual void ConfigureCallback(char *);
    virtual void MapCallback(char *);
    virtual void UnMapCallback(char *);

    /// 
    /// Show/hide a QtModule. The position/size of the Qt widget 
    /// is mapped against the corresponding KwWidget
    //void SetQtModuleVisible(const char* moduleTitle, bool visible); 
    //void SetCurrentQtModuleVisible(bool visible); 
    void SetCurrentQtModule(const char* moduleTitle);

    /// 
    /// Callbacks set on the configure events of the split frames
    void MainSplitFrameConfigureCallback(int width, int height);
    void SecondarySplitFrameConfigureCallback(int width, int height);

    /// 
    /// Use a helper process to display a progress message and progress bar.
    /// Need to use this for feedback when a normal KWWidgets progress bar
    /// would cause event loops (since it calls the tk 'update' command
    /// to force display)
    void SetExternalProgress(char *message, float progress);

    /// 
    /// Get SlicerApplication
    vtkSlicerApplication* GetSlicerApplication();

    ///
    /// This method iterates through all widgets
    /// that control vtk widgets and update the
    /// process events flag on them.
    /// It does not change mrml node flags.
    /// The method is called with the appropriate lockFlag
    /// by the vtkSlicerToolbarGUI, when the
    /// vtkMRMLInteractionNode invokes an
    /// InteractionModeChangedEvent.
    /// Developers note: this method should be
    /// extended to include any new widgets that
    /// are pickable, so that they are integrated with
    /// the Application's pick/place/transform mouse-modes.
    void ModifyAllWidgetLock(int lockFlag);

protected:
    vtkSlicerApplicationGUI ( );
    virtual ~vtkSlicerApplicationGUI ( );
    /// 
    /// Main Slicer window
    vtkSlicerWindow *MainSlicerWindow;

        /// 
    /// Frames for the main Slicer UI panel    
    vtkKWFrame *TopFrame;
    vtkKWFrame *LogoFrame;
    vtkKWFrame *DropShadowFrame;
    vtkSlicerModuleCollapsibleFrame *SlicesControlFrame;
    vtkSlicerModuleCollapsibleFrame *ViewControlFrame;

    /// 
    /// Frame for Lightbox viewing (not yet implemented)
    vtkKWFrame *GridFrame1;
    vtkKWFrame *GridFrame2;

    /// 
    /// Widgets for the File menu
    vtkKWLoadSaveDialog *LoadSceneDialog;

    /// 
    /// Main Slicer toolbar and components
    vtkSlicerToolbarGUI *ApplicationToolbar;
    vtkSlicerViewControlGUI *ViewControlGUI;
    vtkSlicerSlicesControlGUI *SlicesControlGUI;
    vtkSlicerLogoDisplayGUI *LogoDisplayGUI;
    
    double MainRendererBBox[6];


    /// 
    /// ROI Viewer Widget
    vtkSlicerROIViewerWidget *ROIViewerWidget;

    /// 
    /// Contains the state of the ApplicationGUI's layout
    vtkMRMLLayoutNode *GUILayoutNode;

    /// 
    /// use STL::Map to hold all SliceViewers where key is the layoutName
    vtkSlicerSlicesGUI *SlicesGUI;

    /// 
    /// Collection of Icons all GUIs can have access to.
    /// TODO: move basic icons from misc icon collections into here.
    vtkSlicerFoundationIcons *SlicerFoundationIcons;

    /// 
    /// Used to tag all pages added to the tabbed notebook
    /// arrangement of the main viewer.
    int ViewerPageTag;

    vtkSlicerMRMLSaveDataWidget *SaveDataWidget;

    /// 
    /// Wizard-based dialog for selecting and downoading Loadable Modules
    vtkSlicerModulesWizardDialog *ModulesWizardDialog;

    int ProcessingMRMLEvent;
    bool SceneClosing;
    bool Built;
    int CurrentLayout;
    /// 
    /// Used for user feedback during loading of datasets
    int DataCount;

    int UpdatingMain3DViewers;

    /// 
    /// If the active viewer widget has changed, update the dependencies
    void UpdateActiveViewerWidgetDependencies(vtkSlicerViewerWidget*);

    /// PIMPL Encapsulation for STL containers
    //BTX
    vtkSlicerApplicationGUIInternals *Internals;
    //ETX

  /// 
  /// Called when a view node has been added/removed to/from the scene
  virtual void OnViewNodeAdded(vtkMRMLViewNode *node);
  virtual void OnViewNodeRemoved(vtkMRMLViewNode *node);
  virtual void OnViewNodeNeeded();

  virtual int GetNumberOfVisibleViewNodes();
     
  /// 
  /// Synchronize the position of the Qt module panel with the KWWidget
  /// module panel
  //BTX
  void ReposModulePanel();
  //ETX

  /// Result of the most recent python command as a string
  char *PythonResult;

 private:

    vtkSlicerApplicationGUI ( const vtkSlicerApplicationGUI& ); /// Not implemented.
    void operator = ( const vtkSlicerApplicationGUI& ); //Not implemented.
}; 

#endif
