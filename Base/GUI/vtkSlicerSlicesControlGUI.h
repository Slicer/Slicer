// .NAME vtkSlicerSlicesControlGUI
// .SECTION Description
// Main Data GUI and mediator methods for
// SlicesControl GUI Panel in slicer3.

#ifndef __vtkSlicerSlicesControlGUI_h
#define __vtkSlicerSlicesControlGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerComponentGUI.h"

class vtkSlicerApplicationGUI;
class vtkSlicerSlicesControlIcons;
class vtkKWScale;
class vtkKWPushButton;
class vtkKWMenuButton;
class vtkKWScaleWithEntry;
class vtkKWTopLevel;
class vtkKWFrame;

// Description:
// This class implements Slicer's SlicesControl Panel on Main GUI panel
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSlicesControlGUI : public vtkSlicerComponentGUI
{
 public:
    // Description:
    // Usual vtk class functions
    static vtkSlicerSlicesControlGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerSlicesControlGUI, vtkSlicerComponentGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );

    // Description:
    // Get the widgets in the SlicesControlFrame
    vtkGetObjectMacro ( SliceFadeScale, vtkKWScale );
    vtkGetObjectMacro ( ShowFgButton, vtkKWPushButton );
    vtkGetObjectMacro ( ShowBgButton, vtkKWPushButton );
    vtkGetObjectMacro ( ToggleFgBgButton, vtkKWPushButton );
    vtkGetObjectMacro ( LabelOpacityButton, vtkKWPushButton );
    vtkGetObjectMacro ( LabelOpacityScale, vtkKWScaleWithEntry );
    vtkGetObjectMacro ( LabelOpacityTopLevel, vtkKWTopLevel );
//    vtkGetObjectMacro ( GridButton, vtkKWMenuButton );
    vtkGetObjectMacro ( AnnotationButton, vtkKWMenuButton );
    vtkGetObjectMacro ( SpatialUnitsButton, vtkKWMenuButton );
    vtkGetObjectMacro ( CrossHairButton, vtkKWMenuButton );
    vtkGetObjectMacro (FeaturesVisibleButton, vtkKWMenuButton );
    vtkGetObjectMacro (FitToWindowButton, vtkKWPushButton );
    vtkGetObjectMacro ( SlicesControlIcons, vtkSlicerSlicesControlIcons );
    
    // Description:
    // Get the main slicer toolbars.
    vtkGetObjectMacro (ApplicationGUI, vtkSlicerApplicationGUI );
    virtual void SetApplicationGUI ( vtkSlicerApplicationGUI *appGUI );
    
    // Description:
    // This method builds the Data module's GUI
    virtual void BuildGUI ( vtkKWFrame *appF ) ;
    virtual void TearDownGUI ( );
    
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

    virtual void HideLabelOpacityScaleAndEntry ( );
    virtual void PopUpLabelOpacityScaleAndEntry ( );
    virtual void BuildAnnotationMenu ( );
    virtual void BuildCrossHairMenu ( );
    virtual void BuildSpacesMenu ( );
//    virtual void BuildGridMenu ( );
    virtual void BuildVisibilityMenu ( );
    
    virtual void FitSlicesToBackground ( );
    virtual void ModifyAnnotationMode ( );
    virtual void ModifySpatialUnitsMode ( );
    virtual void ModifyCrossHairMode ( );
    virtual void ModifyVisibility ();
    
 protected:
    vtkSlicerSlicesControlGUI ( );
    virtual ~vtkSlicerSlicesControlGUI ( );

    vtkSlicerApplicationGUI *ApplicationGUI;
    vtkSlicerSlicesControlIcons *SlicesControlIcons;
    //Description:
    // Widgets for the SlicesControlFrame in the GUI
    vtkKWScale *SliceFadeScale;
    vtkKWPushButton *ShowFgButton;
    vtkKWPushButton *ShowBgButton;
    vtkKWPushButton *ToggleFgBgButton;
    vtkKWPushButton *LabelOpacityButton;
    vtkKWScaleWithEntry *LabelOpacityScale;
    vtkKWTopLevel *LabelOpacityTopLevel;
//    vtkKWMenuButton *GridButton;
    vtkKWMenuButton *AnnotationButton;
    vtkKWMenuButton *SpatialUnitsButton;
    vtkKWMenuButton *CrossHairButton;
    vtkKWMenuButton *FeaturesVisibleButton;
    vtkKWPushButton *FitToWindowButton;
    
 private:
    vtkSlicerSlicesControlGUI ( const vtkSlicerSlicesControlGUI& ); // Not implemented.
    void operator = ( const vtkSlicerSlicesControlGUI& ); //Not implemented.
};


#endif
