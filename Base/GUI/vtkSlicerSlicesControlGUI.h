///  vtkSlicerSlicesControlGUI
/// 
/// Main Data GUI and mediator methods for
/// SlicesControl GUI Panel in slicer3.

#ifndef __vtkSlicerSlicesControlGUI_h
#define __vtkSlicerSlicesControlGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkMRMLSliceNode.h"
#include "vtkMRMLCrosshairNode.h"
#include "vtkSlicerInteractorStyle.h"

class vtkSlicerApplicationGUI;
class vtkSlicerSlicesControlIcons;
class vtkKWScale;
class vtkKWPushButton;
class vtkKWMenuButton;
class vtkKWScaleWithEntry;
class vtkKWTopLevel;
class vtkKWFrame;
class vtkKWEntry;
class vtkKWEntryWithLabel;
class vtkSlicerVisibilityIcons;
class vtkSlicerFoundationIcons;


/// Description:
/// This class implements Slicer's SlicesControl Panel on Main GUI panel
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSlicesControlGUI : public vtkSlicerComponentGUI
{
 public:
    /// 
    /// Usual vtk class functions
    static vtkSlicerSlicesControlGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerSlicesControlGUI, vtkSlicerComponentGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );

    /// 
    /// Get the widgets in the SlicesControlFrame
    vtkGetObjectMacro ( SliceFadeScale, vtkKWScale );
    vtkGetObjectMacro ( ShowFgButton, vtkKWPushButton );
    vtkGetObjectMacro ( ShowBgButton, vtkKWPushButton );
    vtkGetObjectMacro ( ToggleFgBgButton, vtkKWPushButton );
    vtkGetObjectMacro ( LabelOpacityButton, vtkKWPushButton );
    vtkGetObjectMacro ( LabelOpacityScale, vtkKWScaleWithEntry );
    vtkGetObjectMacro ( LabelOpacityToggleButton, vtkKWPushButton);
    vtkGetObjectMacro ( LabelOpacityTopLevel, vtkKWTopLevel );
    vtkGetObjectMacro ( FieldOfViewTopLevel, vtkKWTopLevel );
///    vtkGetObjectMacro ( GridButton, vtkKWMenuButton );
    vtkGetObjectMacro ( CompositingButton, vtkKWMenuButton );
    vtkGetObjectMacro ( AnnotationButton, vtkKWMenuButton );
    vtkGetObjectMacro ( SpatialUnitsButton, vtkKWMenuButton );
    vtkGetObjectMacro ( CrossHairButton, vtkKWMenuButton );
    vtkGetObjectMacro (FeaturesVisibleButton, vtkKWMenuButton );
    vtkGetObjectMacro (FitToWindowButton, vtkKWPushButton );
    vtkGetObjectMacro ( FieldOfViewButton, vtkKWPushButton );
    vtkGetObjectMacro ( SlicesControlIcons, vtkSlicerSlicesControlIcons );
    vtkGetObjectMacro (RedFOVEntry, vtkKWEntryWithLabel);
    vtkGetObjectMacro (GreenFOVEntry, vtkKWEntryWithLabel);    
    vtkGetObjectMacro (YellowFOVEntry, vtkKWEntryWithLabel);

    vtkGetObjectMacro ( RedSliceNode, vtkMRMLSliceNode );
    vtkGetObjectMacro ( YellowSliceNode, vtkMRMLSliceNode );  
    vtkGetObjectMacro ( GreenSliceNode, vtkMRMLSliceNode );
    vtkSetObjectMacro ( RedSliceNode, vtkMRMLSliceNode );
    vtkSetObjectMacro ( YellowSliceNode, vtkMRMLSliceNode );  
    vtkSetObjectMacro ( GreenSliceNode, vtkMRMLSliceNode );
    vtkSetObjectMacro ( RedSliceEvents, vtkSlicerInteractorStyle );
    vtkSetObjectMacro ( YellowSliceEvents, vtkSlicerInteractorStyle );
    vtkSetObjectMacro ( GreenSliceEvents, vtkSlicerInteractorStyle );
    vtkGetObjectMacro ( RedSliceEvents, vtkSlicerInteractorStyle );
    vtkGetObjectMacro ( YellowSliceEvents, vtkSlicerInteractorStyle );
    vtkGetObjectMacro ( GreenSliceEvents, vtkSlicerInteractorStyle );

    vtkGetObjectMacro( CrosshairNode, vtkMRMLCrosshairNode );
    vtkSetObjectMacro( CrosshairNode, vtkMRMLCrosshairNode );

    vtkGetMacro (EntryUpdatePending, int );
    vtkSetMacro (EntryUpdatePending, int);
    vtkGetMacro ( SliceInteracting, int );
    vtkGetMacro (ProcessingMRMLEvent, int);

    vtkGetObjectMacro ( VisibilityIcons, vtkSlicerVisibilityIcons );
    vtkGetObjectMacro ( FoundationIcons, vtkSlicerFoundationIcons );
    
    /// 
    /// Get the main slicer toolbars.
    vtkGetObjectMacro (ApplicationGUI, vtkSlicerApplicationGUI );
    virtual void SetApplicationGUI ( vtkSlicerApplicationGUI *appGUI );
    
    /// 
    /// This method builds the Data module's GUI
    virtual void BuildGUI ( vtkKWFrame *appF ) ;
    virtual void TearDownGUI ( );
    //BTX
    using vtkSlicerComponentGUI::BuildGUI;
    //ETX
    
    /// 
    /// Add/Remove observers on widgets in the GUI
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );

    /// 
    /// Class's mediator methods for processing events invoked by
    /// either the Logic, MRML or GUI.
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
    
    /// 
    /// Describe the behavior at module enter and exit.
    virtual void Enter ( );
    //BTX
    using vtkSlicerComponentGUI::Enter;
    //ETX
    virtual void Exit ( );

    virtual void HideLabelOpacityScaleAndEntry ( );
    virtual void PopUpLabelOpacityScaleAndEntry ( );
    virtual void PopUpFieldOfViewEntries ( );
    virtual void HideFieldOfViewEntries ( );
    virtual void FitFOVToBackground( double fov, int viewer );

    virtual void BuildCompositingMenu();
    virtual void BuildAnnotationMenu ( );
    virtual void BuildCrossHairMenu ( );
    virtual void BuildSpacesMenu ( );
///    virtual void BuildGridMenu ( );
    virtual void BuildVisibilityMenu ( );
    
    virtual void FitSlicesToBackground ( );
    virtual void ModifyCompositingMode ( );
    virtual void ModifyAnnotationMode ( );
    virtual void ModifySpatialUnitsMode ( );
    virtual void ModifyCrossHairMode ( );
    virtual void ModifyVisibility ();
    
    virtual void UpdateSliceGUIInteractorStyles ( );
    virtual void UpdateSlicesFromMRML();
    virtual void UpdateFromMRML( );
    virtual void RequestFOVEntriesUpdate ( );
    virtual void FOVEntriesUpdate ( );
    virtual void RemoveSliceEventObservers();
    virtual void AddSliceEventObservers();
    
 protected:
    vtkSlicerSlicesControlGUI ( );
    virtual ~vtkSlicerSlicesControlGUI ( );

    vtkSlicerApplicationGUI *ApplicationGUI;
    vtkSlicerSlicesControlIcons *SlicesControlIcons;
    vtkSlicerFoundationIcons *FoundationIcons;

    /// 
    /// visibility icons, for the label opacity toggle
    vtkSlicerVisibilityIcons *VisibilityIcons;
    //Description:
    /// Widgets for the SlicesControlFrame in the GUI
    vtkKWScale *SliceFadeScale;
    vtkKWPushButton *ShowFgButton;
    vtkKWPushButton *ShowBgButton;
    vtkKWPushButton *ToggleFgBgButton;
    vtkKWPushButton *LabelOpacityButton;
    vtkKWScaleWithEntry *LabelOpacityScale;
    vtkKWPushButton *LabelOpacityToggleButton;
    vtkKWTopLevel *LabelOpacityTopLevel;
    vtkKWTopLevel *FieldOfViewTopLevel;
///    vtkKWMenuButton *GridButton;
    vtkKWMenuButton *CompositingButton;
    vtkKWMenuButton *AnnotationButton;
    vtkKWMenuButton *SpatialUnitsButton;
    vtkKWMenuButton *CrossHairButton;
    vtkKWMenuButton *FeaturesVisibleButton;
    vtkKWPushButton *FitToWindowButton;
    vtkKWPushButton *FieldOfViewButton;
    vtkKWEntryWithLabel *RedFOVEntry;
    vtkKWEntryWithLabel *YellowFOVEntry;
    vtkKWEntryWithLabel *GreenFOVEntry;
    vtkMRMLSliceNode *RedSliceNode;
    vtkMRMLSliceNode *YellowSliceNode;
    vtkMRMLSliceNode *GreenSliceNode;
    vtkSlicerInteractorStyle *RedSliceEvents;
    vtkSlicerInteractorStyle *YellowSliceEvents;
    vtkSlicerInteractorStyle *GreenSliceEvents;
    vtkMRMLCrosshairNode *CrosshairNode;
    int SliceInteracting;
    int ProcessingMRMLEvent;
    int EntryUpdatePending;
    float LastLabelOpacity;

 private:
    vtkSlicerSlicesControlGUI ( const vtkSlicerSlicesControlGUI& ); /// Not implemented.
    void operator = ( const vtkSlicerSlicesControlGUI& ); //Not implemented.
};

#endif
