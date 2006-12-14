// .NAME vtkSlicerIGTDemoGUI 
// .SECTION Description
// Main Volumes GUI and mediator methods for slicer3. 


#ifndef __vtkSlicerIGTDemoGUI_h
#define __vtkSlicerIGTDemoGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"

#include "vtkSlicerIGTDemoLogic.h"

class vtkKWPushButton;
class vtkKWEntryWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWMenuButton;
class vtkKWCheckButton;
class vtkKWScaleWithEntry;
class vtkKWEntry;
class vtkKWEntryWithLabel;
class vtkKWLoadSaveButtonWithLabel;
class vtkKWMultiColumnListWithScrollbars;


// Description:
// This class implements Slicer's Volumes GUI
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerIGTDemoGUI : public vtkSlicerModuleGUI
{
 public:
    // Description:
    // Usual vtk class functions
    static vtkSlicerIGTDemoGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerIGTDemoGUI, vtkSlicerModuleGUI );
    void PrintSelf (ostream& os, vtkIndent indent );
    
    // Description:
    // Get methods on class members (no Set methods required)
    vtkGetObjectMacro ( Logic, vtkSlicerIGTDemoLogic );

    // Description:
    // API for setting VolumeNode, VolumeLogic and
    // for both setting and observing them.
    void SetModuleLogic ( vtkSlicerIGTDemoLogic *logic )
        { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
    void SetAndObserveModuleLogic ( vtkSlicerIGTDemoLogic *logic )
        { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }

    // Description:
    // This method builds the IGTDemo module GUI
    virtual void BuildGUI ( );

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

    void ProcessTimerEvents ();
    void MakeLocator ();
    void UpdateLocator ();
    void UpdateSliceDisplay(float px, float py, float pz);

    
    // Description:
    // Describe behavior at module startup and exit.
    virtual void Enter ( );
    virtual void Exit ( );

 protected:
    vtkSlicerIGTDemoGUI ( );
    virtual ~vtkSlicerIGTDemoGUI ( );

    vtkKWMenuButtonWithLabel *ServerMenu;

    vtkKWEntryWithLabel *NormalOffsetEntry;
    vtkKWEntryWithLabel *TransOffsetEntry;
    vtkKWEntryWithLabel *NXTOffsetEntry;

    vtkKWEntryWithLabel *NormalSizeEntry;
    vtkKWEntryWithLabel *TransSizeEntry;
    vtkKWEntryWithLabel *RadiusEntry;

    vtkKWEntry *NREntry;
    vtkKWEntry *NAEntry;
    vtkKWEntry *NSEntry;
    vtkKWEntry *TREntry;
    vtkKWEntry *TAEntry;
    vtkKWEntry *TSEntry;
    vtkKWEntry *PREntry;
    vtkKWEntry *PAEntry;
    vtkKWEntry *PSEntry;

    /*
    vtkKWScaleWithEntry* RedColorScale;
    vtkKWScaleWithEntry* GreenColorScale;
    vtkKWScaleWithEntry* BlueColorScale;
    */

    vtkKWCheckButton *ConnectCheckButton;
    vtkKWCheckButton *PauseCheckButton;

    vtkKWCheckButton *LocatorCheckButton;
    vtkKWCheckButton *HandleCheckButton;
    vtkKWCheckButton *GuideCheckButton;

    vtkKWCheckButton *LocatorModeCheckButton;
    vtkKWCheckButton *UserModeCheckButton;

    /*
    vtkKWMenuButton *RedSliceMenu;
    vtkKWMenuButton *YellowSliceMenu;
    vtkKWMenuButton *GreenSliceMenu;
    */

    vtkKWLoadSaveButtonWithLabel *LoadConfigButton;

    vtkKWEntry *ConfigFileEntry;
    vtkKWEntryWithLabel *UpdateRateEntry;

    vtkKWEntryWithLabel *PatCoordinatesEntry;
    vtkKWEntryWithLabel *SlicerCoordinatesEntry;
    vtkKWPushButton *GetPatCoordinatesPushButton;
    vtkKWPushButton *AddPointPairPushButton;

    vtkKWMultiColumnListWithScrollbars *PointPairMultiColumnList;

//    vtkKWPushButton *LoadPointPairPushButton;
//    vtkKWPushButton *SavePointPairPushButton;
    vtkKWPushButton *DeletePointPairPushButton;
    vtkKWPushButton *DeleteAllPointPairPushButton;
    vtkKWPushButton *RegisterPushButton;
    vtkKWPushButton *ResetPushButton;


    // Module logic and mrml pointers
    vtkSlicerIGTDemoLogic *Logic;

    int StopTimer;
    vtkMatrix4x4 *LocatorMatrix;

    vtkMRMLModelNode *LocatorModelNode;
    vtkMRMLModelDisplayNode *LocatorModelDisplayNode;


 private:
    vtkSlicerIGTDemoGUI ( const vtkSlicerIGTDemoGUI& ); // Not implemented.
    void operator = ( const vtkSlicerIGTDemoGUI& ); //Not implemented.

    void BuildGUIForHandPieceFrame ();
    void BuildGUIForTrackingFrame ();
    void BuildGUIForServerFrame ();
    void BuildGUIForRegistrationFrame ();

};

#endif
