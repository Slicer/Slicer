// .NAME vtkPatientToImageRegistrationGUI 
// .SECTION Description
// Main Volumes GUI and mediator methods for slicer3. 


#ifndef __vtkPatientToImageRegistrationGUI_h
#define __vtkPatientToImageRegistrationGUI_h


#ifdef WIN32
#include "vtkPatientToImageRegistrationWin32Header.h"
#endif

#include "vtkSlicerModuleGUI.h"
#include "vtkPatientToImageRegistrationLogic.h"

#include "vtkIGTDataManager.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerInteractorStyle.h"


#include <string>

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
class vtkSlicerNodeSelectorWidget;
class vtkKWLabel;


// Description:    
// This class implements Slicer's Volumes GUI.
//
class VTK_PatientToImageRegistration_EXPORT vtkPatientToImageRegistrationGUI : public vtkSlicerModuleGUI
{
public:

  // Description:    
  // Usual vtk class functions
  static vtkPatientToImageRegistrationGUI* New (  );
  vtkTypeRevisionMacro ( vtkPatientToImageRegistrationGUI, vtkSlicerModuleGUI );
  void PrintSelf (ostream& os, vtkIndent indent );

  // Description: 
  // Get the categorization of the module.
  const char *GetCategory() const { return "IGT"; }


  // Description:    
  // Get methods on class members (no Set methods required)
  vtkGetObjectMacro ( Logic, vtkPatientToImageRegistrationLogic );

  // Description:
  // API for setting VolumeNode, VolumeLogic and
  // for both setting and observing them.
  void SetModuleLogic ( vtkSlicerLogic *logic )
    { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
  //void SetAndObserveModuleLogic ( vtkPatientToImageRegistrationLogic *logic )
  //    { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }

  // Description:    
  // This method builds the IGTDemo module GUI
  virtual void BuildGUI ( );

  // Description:
  // Add/Remove observers on widgets in the GUI
  virtual void AddGUIObservers();
  virtual void RemoveGUIObservers();


  // Description:
  // Class's mediator methods for processing events invoked by
  // either the Logic, MRML or GUI.    
  virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  void ProcessTimerEvents();

  void HandleMouseEvent(vtkSlicerInteractorStyle *style);


  // Description:
  // Describe behavior at module startup and exit.
  virtual void Enter ( );
  virtual void Exit ( );

  //BTX
  static void DataCallback(vtkObject *caller, 
                           unsigned long eid, void *clientData, void *callData);

  //ETX
  void TransformChangedCallback(double);
  void TransformChangingCallback(double);

protected:
  vtkPatientToImageRegistrationGUI ( );
  virtual ~vtkPatientToImageRegistrationGUI ( );

  vtkKWEntryWithLabel *TransformNodeNameEntry;
  vtkKWEntryWithLabel *FiducialListNodeNameEntry;

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

  vtkSlicerNodeSelectorWidget *TrackerSelector;
  vtkKWEntryWithLabel *PivotCalibrationErrorReport;
  vtkKWLabel *PivotCalibrationLabel;
  vtkKWPushButton *StartPivotCalibrationPushButton;
  vtkKWPushButton *FinishPivotCalibrationPushButton;
 
  // Module logic and mrml pointers
  vtkPatientToImageRegistrationLogic *Logic;

  vtkKWScaleWithEntry *TranslationScale;

private:
  vtkPatientToImageRegistrationGUI ( const vtkPatientToImageRegistrationGUI& ); // Not implemented.
  void operator = ( const vtkPatientToImageRegistrationGUI& ); //Not implemented.

  void BuildGUIForICPFrame();
  void BuildGUIForLandmarksFrame();
  void BuildGUIForHelpFrame();
  void BuildGUIForTrackerFrame();
  void BuildGUIForCalibrationFrame();

  void TrackerLoop();

  bool CloseScene;
  int TimerFlag;
  int TimerInterval;

};

#endif
