// .NAME vtkNeuroNavGUI 
// .SECTION Description
// Main Volumes GUI and mediator methods for slicer3. 


#ifndef __vtkNeuroNavGUI_h
#define __vtkNeuroNavGUI_h


#ifdef WIN32
#include "vtkNeuroNavWin32Header.h"
#endif

#include "vtkSlicerModuleGUI.h"
#include "vtkNeuroNavLogic.h"

#include "vtkIGTDataManager.h"
#include "vtkIGTPat2ImgRegistration.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerInteractorStyle.h"


#include <string>

#ifdef USE_OPENTRACKER
#include "vtkIGTOpenTrackerStream.h"
#endif
#ifdef USE_IGSTK
#include "vtkIGTIGSTKStream.h"
#endif


class vtkKWPushButton;
class vtkKWEntryWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWMenuButton;
class vtkKWCheckButton;
class vtkKWScaleWithEntry;
class vtkKWEntry;
class vtkKWFrame;
class vtkKWEntryWithLabel;
class vtkKWLoadSaveButtonWithLabel;
class vtkKWMultiColumnListWithScrollbars;



// Description:
// This class implements Slicer's Volumes GUI
//
class VTK_NEURONAV_EXPORT vtkNeuroNavGUI : public vtkSlicerModuleGUI
{
 public:
    // Description:
    // Usual vtk class functions
    static vtkNeuroNavGUI* New (  );
    vtkTypeRevisionMacro ( vtkNeuroNavGUI, vtkSlicerModuleGUI );
    void PrintSelf (ostream& os, vtkIndent indent );
    
    // Description:
    // Get methods on class members (no Set methods required)
    vtkGetObjectMacro ( Logic, vtkNeuroNavLogic );

    // Description:
    // API for setting VolumeNode, VolumeLogic and
    // for both setting and observing them.
    void SetModuleLogic ( vtkNeuroNavLogic *logic )
        { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
    void SetAndObserveModuleLogic ( vtkNeuroNavLogic *logic )
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

    void HandleMouseEvent(vtkSlicerInteractorStyle *style);

    
    // Description:
    // Describe behavior at module startup and exit.
    virtual void Enter ( );
    virtual void Exit ( );

    void Init();

  //BTX
  static void DataCallback(vtkObject *caller, 
                unsigned long eid, void *clientData, void *callData);

  //ETX

 protected:
    vtkNeuroNavGUI ( );
    virtual ~vtkNeuroNavGUI ( );

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

    vtkKWFrame *FileFrame;

    /*
    vtkKWScaleWithEntry* RedColorScale;
    vtkKWScaleWithEntry* GreenColorScale;
    vtkKWScaleWithEntry* BlueColorScale;
    */

    vtkKWCheckButton *ConnectCheckButton;
    vtkKWCheckButton *LocatorCheckButton;
    vtkKWCheckButton *HandleCheckButton;
    vtkKWCheckButton *GuideCheckButton;

    vtkKWCheckButton *LocatorModeCheckButton;
    vtkKWCheckButton *UserModeCheckButton;

    vtkKWMenuButton *RedSliceMenu;
    vtkKWMenuButton *YellowSliceMenu;
    vtkKWMenuButton *GreenSliceMenu;

#ifdef USE_OPENTRACKER
    vtkKWLoadSaveButtonWithLabel *LoadConfigButton;
    vtkKWEntry *ConfigFileEntry;
#endif
#ifdef USE_IGSTK
    vtkKWMenuButtonWithLabel *DeviceMenuButton;
#endif

    vtkKWEntryWithLabel *UpdateRateEntry;
    vtkKWEntryWithLabel * MultiFactorEntry;

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
    vtkNeuroNavLogic *Logic;

    // int StopTimer;
    vtkMatrix4x4 *LocatorMatrix;

    vtkMRMLModelNode *LocatorModelNode;
    vtkMRMLModelDisplayNode *LocatorModelDisplayNode;

    //BTX
    std::string LocatorModelID;
    //ETX
    vtkIGTDataManager *DataManager;
    vtkIGTPat2ImgRegistration *Pat2ImgReg;

    vtkCallbackCommand *DataCallbackCommand;

    void UpdateAll();
    void UpdateLocator();
    void UpdateSliceDisplay(float nx, float ny, float nz, 
                            float tx, float ty, float tz, 
                            float px, float py, float pz);

 private:
    vtkNeuroNavGUI ( const vtkNeuroNavGUI& ); // Not implemented.
    void operator = ( const vtkNeuroNavGUI& ); //Not implemented.

    void BuildGUIForHandPieceFrame ();
    void BuildGUIForTrackingFrame ();
    void BuildGUIForDeviceFrame ();
    void BuildGUIForRegistrationFrame ();

    void TrackerLoop();


#ifdef USE_OPENTRACKER
    vtkIGTOpenTrackerStream *OpenTrackerStream;
#endif

#ifdef USE_IGSTK    
    vtkIGTIGSTKStream *IGSTKStream;
#endif

};

#endif
