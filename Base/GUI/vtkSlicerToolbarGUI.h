// .NAME vtkSlicerToolbarGUI
// .SECTION Description
// Main Data GUI and mediator methods for
// the application Toolbar in slicer3.

#ifndef __vtkSlicerToolbarGUI_h
#define __vtkSlicerToolbarGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"

#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerToolbarIcons.h"
#include "vtkSlicerModuleChooseGUI.h"

#include "vtkKWFrame.h"
#include "vtkKWToolbar.h"
#include "vtkKWPushButton.h"

#include "vtkKWMenuButton.h"
#include "vtkKWRadioButtonSet.h"


class vtkSlicerApplicationGUI;

// Description:
// This class implements Slicer's Application Toolbar 
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerToolbarGUI : public vtkSlicerComponentGUI
{
 public:
    // Description:
    // Usual vtk class functions
    static vtkSlicerToolbarGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerToolbarGUI, vtkSlicerComponentGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );
    
    // Description:
    // Get the class containing all slicer GUI images for logos/icons
    vtkGetObjectMacro (SlicerToolbarIcons, vtkSlicerToolbarIcons);

    // Description:
    // Get the main slicer toolbars.
    vtkGetObjectMacro (ApplicationGUI, vtkSlicerApplicationGUI );
    virtual void SetApplicationGUI ( vtkSlicerApplicationGUI *appGUI );
    
    vtkGetObjectMacro (ModulesToolbar, vtkKWToolbar);
    vtkGetObjectMacro (LoadSaveToolbar, vtkKWToolbar );
    vtkGetObjectMacro (ViewToolbar, vtkKWToolbar);
    vtkGetObjectMacro (InteractionModeToolbar, vtkKWToolbar);
    vtkGetObjectMacro (UndoRedoToolbar, vtkKWToolbar);
    vtkGetObjectMacro (ModuleChooseGUI, vtkSlicerModuleChooseGUI );

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
//    vtkGetObjectMacro (MeasurementsIconButton, vtkKWPushButton);
    vtkGetObjectMacro (SaveSceneIconButton, vtkKWPushButton );
    vtkGetObjectMacro (LoadSceneIconButton, vtkKWMenuButton );
    vtkGetObjectMacro (ChooseLayoutIconMenuButton, vtkKWMenuButton );
    vtkGetObjectMacro (UndoIconButton, vtkKWPushButton );
    vtkGetObjectMacro (RedoIconButton, vtkKWPushButton );
    
    vtkGetObjectMacro (MousePickButton, vtkKWRadioButton);
    vtkGetObjectMacro (MousePlaceButton, vtkKWRadioButton);
    vtkGetObjectMacro (MouseTransformViewButton, vtkKWRadioButton);    



    // Description:
    // This method builds the Data module's GUI
    virtual void BuildGUI ( ) ;

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
    // Method called when a view reconfiguration is chosen
    // from the toolbar; before Slice and 3D viewers are
    // unpacked and repacked in a new configuration,
    // render requests from automatic rock or spin are
    // stopped. Resumes when finished packing.
    virtual int StopViewRockOrSpin ( );
    virtual void ResumeViewRockOrSpin ( int mode );
    virtual void SetLayoutMenubuttonValueToCurrentLayout ();
    const char* GetCurrentLayoutStringName ( );
    virtual void ReconfigureGUIFonts ( );
  
    // Description:
    // Getting and setting the mrml selection node id
    vtkGetStringMacro(InteractionNodeID);
    vtkSetStringMacro(InteractionNodeID);
    // Description:
    // Get/Set the selection node
    vtkGetObjectMacro(InteractionNode, vtkMRMLInteractionNode);
    void SetAndObserveInteractionNode (vtkMRMLInteractionNode *node)
    {
    vtkSetAndObserveMRMLNodeMacro(this->InteractionNode, node);
    }
    
 protected:
    vtkSlicerToolbarGUI ( );
    virtual ~vtkSlicerToolbarGUI ( );

    // Description:
    // Main Application Toolbars
    vtkKWToolbar *ModulesToolbar;
    vtkKWToolbar *LoadSaveToolbar;
    vtkKWToolbar *ViewToolbar;
    vtkKWToolbar *InteractionModeToolbar;
    vtkKWToolbar *UndoRedoToolbar;

    // Description:
    // Contains icons
    vtkSlicerToolbarIcons *SlicerToolbarIcons;
    
    // Description:
    // Widgets that display toolbar icons
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
//    vtkKWPushButton *MeasurementsIconButton;
    vtkKWPushButton *SaveSceneIconButton;
    vtkKWMenuButton *LoadSceneIconButton;
    vtkKWMenuButton *ChooseLayoutIconMenuButton;

    vtkKWRadioButton *MousePickButton;
    vtkKWRadioButton *MousePlaceButton;
    vtkKWRadioButton *MouseTransformViewButton;

    vtkSlicerModuleChooseGUI *ModuleChooseGUI;
    vtkSlicerApplicationGUI *ApplicationGUI;

    // Description:
    // the id of the selection node, for tracking changes to the mouse
    // interation mode
    char *InteractionNodeID;
    vtkMRMLInteractionNode *InteractionNode;
    
 private:
    vtkSlicerToolbarGUI ( const vtkSlicerToolbarGUI& ); // Not implemented.
    void operator = ( const vtkSlicerToolbarGUI& ); //Not implemented.
};


#endif
