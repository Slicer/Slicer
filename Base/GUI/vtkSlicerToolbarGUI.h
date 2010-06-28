///  vtkSlicerToolbarGUI
/// 
/// Main Data GUI and mediator methods for
/// the application Toolbar in slicer3.

#ifndef __vtkSlicerToolbarGUI_h
#define __vtkSlicerToolbarGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"


#include <vtkMRMLInteractionNode.h>

#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerToolbarIcons.h"
#include "vtkSlicerModuleChooseGUI.h"
#include "vtkSlicerPopUpHelpWidget.h"

#include "vtkKWLoadSaveDialog.h"
#include "vtkKWFrame.h"
#include "vtkKWToolbar.h"
#include "vtkKWPushButton.h"
#include "vtkKWMenuButton.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWCheckButton.h"
#include "vtkKWTopLevel.h"
#include "vtkKWEntry.h"
#include "vtkKWDialog.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWLoadSaveButton.h"

class vtkSlicerApplicationGUI;

/// Description:
/// This class implements Slicer's Application Toolbar 
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerToolbarGUI : public vtkSlicerComponentGUI
{
 public:
    /// 
    /// Usual vtk class functions
    static vtkSlicerToolbarGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerToolbarGUI, vtkSlicerComponentGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );
    
    /// 
    /// Get the class containing all slicer GUI images for logos/icons
    vtkGetObjectMacro (SlicerToolbarIcons, vtkSlicerToolbarIcons);

    /// 
    /// Get the main slicer toolbars.
    vtkGetObjectMacro (ApplicationGUI, vtkSlicerApplicationGUI );
    virtual void SetApplicationGUI ( vtkSlicerApplicationGUI *appGUI );
    
    vtkGetObjectMacro (ModulesToolbar, vtkKWToolbar);
    vtkGetObjectMacro (LoadSaveToolbar, vtkKWToolbar );
    vtkGetObjectMacro ( UtilitiesToolbar, vtkKWToolbar );
    vtkGetObjectMacro (ViewToolbar, vtkKWToolbar);
    vtkGetObjectMacro (InteractionModeToolbar, vtkKWToolbar);
    vtkGetObjectMacro (UndoRedoToolbar, vtkKWToolbar);
    vtkGetObjectMacro (ModuleChooseGUI, vtkSlicerModuleChooseGUI );

    /// 
    /// Get the widgets that display the toolbar icons
    vtkGetObjectMacro (HomeIconButton, vtkKWPushButton); 
    vtkGetObjectMacro (DataIconButton, vtkKWPushButton);
    vtkGetObjectMacro (VolumeIconButton, vtkKWPushButton );
    vtkGetObjectMacro (ModelIconButton, vtkKWPushButton );
    vtkGetObjectMacro (EditorIconButton, vtkKWPushButton );
    vtkGetObjectMacro (EditorToolboxIconButton, vtkKWPushButton );
    vtkGetObjectMacro (TransformIconButton, vtkKWPushButton );
    vtkGetObjectMacro (ColorIconButton, vtkKWPushButton );
    vtkGetObjectMacro (FiducialsIconButton, vtkKWPushButton);
    vtkGetObjectMacro (MeasurementsIconButton, vtkKWPushButton);
    vtkGetObjectMacro (SaveSceneIconButton, vtkKWPushButton );
    vtkGetObjectMacro (LoadSceneIconButton, vtkKWMenuButton );
    vtkGetObjectMacro (ChooseLayoutIconMenuButton, vtkKWMenuButton );
    vtkGetObjectMacro (UndoIconButton, vtkKWPushButton );
    vtkGetObjectMacro (RedoIconButton, vtkKWPushButton );
    
    vtkGetObjectMacro (MousePickOptionsButton, vtkKWMenuButton );
    vtkGetObjectMacro (MousePlaceOptionsButton, vtkKWMenuButton );
    
    vtkGetObjectMacro (MousePickButton, vtkKWRadioButton);
    vtkGetObjectMacro (MousePlaceButton, vtkKWRadioButton);
    vtkGetObjectMacro (MouseTransformViewButton, vtkKWRadioButton);    

    vtkGetObjectMacro (CompareViewLightboxRowEntry, vtkKWEntry );
    vtkGetObjectMacro (CompareViewLightboxColumnEntry, vtkKWEntry );

    /// 
    /// This method builds the Data module's GUI
    virtual void BuildGUI ( void ) ;
    virtual void BuildGUI ( vtkKWFrame * f ) { this->Superclass::BuildGUI(f); }
    virtual void BuildGUI ( vtkKWFrame * f, double * bgColor ) { this->Superclass::BuildGUI(f,bgColor); }

    
    /// 
    /// Add/Remove observers on widgets in the GUI
    virtual void AddGUIObservers ( );
    virtual void AddMRMLObservers ( );
    virtual void RemoveGUIObservers ( );
    virtual void RemoveMRMLObservers ( );

    /// 
    /// Class's mediator methods for processing events invoked by
    /// either the Logic, MRML or GUI.
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  virtual void PopUpCompareViewCustomLayoutFrame();
  virtual void HideCompareViewCustomLayoutFrame();

    
    /// 
    /// Updates the pulldown menu of layouts.
    virtual void UpdateLayoutMenu();

    //BTX
    using vtkSlicerComponentGUI::Enter; 
    //ETX
    
    /// 
    /// Describe the behavior at module enter and exit.
    virtual void Enter ( );
    virtual void Exit ( );
    /// 
    /// Method called when a view reconfiguration is chosen
    /// from the toolbar; before Slice and 3D viewers are
    /// unpacked and repacked in a new configuration,
    /// render requests from automatic rock or spin are
    /// stopped. Resumes when finished packing.
    virtual int StopViewRockOrSpin ( );
    virtual void ResumeViewRockOrSpin ( int mode );
    virtual void SetLayoutMenubuttonValueToCurrentLayout ();
    virtual void SetLayoutMenubuttonValueToLayout (int layout);
    virtual void ReconfigureGUIFonts ( );


    /// 
    /// Getting and setting the mrml interaction node
    vtkGetStringMacro(InteractionNodeID);
    vtkGetObjectMacro(InteractionNode, vtkMRMLInteractionNode);
    virtual void SetInteractionNode ( vtkMRMLInteractionNode *node );

    // not used.
    void SetAndObserveInteractionNode (vtkMRMLInteractionNode *node)
    {
    vtkSetAndObserveMRMLNodeMacro(this->InteractionNode, node);
    }

    /// set the lock flag on all 3d widgets, so they will either respond to
    /// events or not.
    void ModifyAll3DWidgetsLock(int lockFlag);

 protected:
    vtkSlicerToolbarGUI ( );
    virtual ~vtkSlicerToolbarGUI ( );

    /// 
    /// Main Application Toolbars
    vtkKWToolbar *ModulesToolbar;
    vtkKWToolbar *LoadSaveToolbar;
    vtkKWToolbar *ViewToolbar;
    vtkKWToolbar *InteractionModeToolbar;
    vtkKWToolbar *UndoRedoToolbar;
    vtkKWToolbar *UtilitiesToolbar;
    vtkKWTopLevel *CompareViewBoxTopLevel;

    /// 
    /// Contains icons
    vtkSlicerToolbarIcons *SlicerToolbarIcons;
    
    /// 
    /// Widgets that display toolbar icons
    vtkKWPushButton *UndoIconButton;
    vtkKWPushButton *RedoIconButton;
    vtkKWPushButton *HomeIconButton;
    vtkKWPushButton *DataIconButton;
    vtkKWPushButton *VolumeIconButton;
    vtkKWPushButton *ModelIconButton;
    vtkKWPushButton *EditorIconButton;
  vtkKWPushButton *EditorToolboxIconButton;
    vtkKWPushButton *TransformIconButton;    
    vtkKWPushButton *ColorIconButton;
    vtkKWPushButton *FiducialsIconButton;
    vtkKWPushButton *MeasurementsIconButton;
    vtkKWPushButton *SaveSceneIconButton;
    vtkKWMenuButton *LoadSceneIconButton;
    vtkKWMenuButton *ChooseLayoutIconMenuButton;

    vtkKWEntry *CompareViewBoxRowEntry;
///    vtkKWEntry *CompareViewBoxColumnEntry;
    vtkKWPushButton *CompareViewBoxApplyButton;

    vtkKWEntry *CompareViewLightboxRowEntry;
    vtkKWEntry *CompareViewLightboxColumnEntry;

    vtkKWMenuButton *MousePickOptionsButton;
    vtkKWMenuButton *MousePlaceOptionsButton;
    
    vtkKWRadioButton *MousePickButton;
    vtkKWRadioButton *MousePlaceButton;
    vtkKWRadioButton *MouseTransformViewButton;

    vtkSlicerModuleChooseGUI *ModuleChooseGUI;
    vtkSlicerApplicationGUI *ApplicationGUI;

    /// 
    /// the id of the selection node, for tracking changes to the mouse
    /// interation mode
    char *InteractionNodeID;
    vtkMRMLInteractionNode *InteractionNode;
    int ProcessingMRMLEvent;
    
 private:
    vtkSlicerToolbarGUI ( const vtkSlicerToolbarGUI& ); /// Not implemented.
    void operator = ( const vtkSlicerToolbarGUI& ); //Not implemented.
};


#endif
