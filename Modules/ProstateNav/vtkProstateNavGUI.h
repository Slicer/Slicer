/*=auto=========================================================================

  Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: $
  Date:      $Date: $
  Version:   $Revision: $

=========================================================================auto=*/

#ifndef __vtkProstateNavGUI_h
#define __vtkProstateNavGUI_h

#ifdef WIN32
#include "vtkProstateNavWin32Header.h"
#endif

#include "vtkSlicerModuleGUI.h"
#include "vtkProstateNavLogic.h"

#include "vtkIGTDataManager.h"
#include "vtkIGTPat2ImgRegistration.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerInteractorStyle.h"

#include <string>

#ifdef USE_IGSTK
#include "vtkIGTIGSTKStream.h"
#endif

class vtkKWPushButton;
class vtkKWPushButtonSet;
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
class vtkKWWizardWidget;

class vtkProstateNavStep;

class vtkTransform;

// Description:    
// This class implements Slicer's Volumes GUI
//
class VTK_PROSTATENAV_EXPORT vtkProstateNavGUI : public vtkSlicerModuleGUI
{
 public:
  //BTX
  enum {
    SLICE_PLANE_RED    = 0,
    SLICE_PLANE_YELLOW = 1,
    SLICE_PLANE_GREEN  = 2
  };
  enum {
    SLICE_RTIMAGE_NONE      = 0,
    SLICE_RTIMAGE_PERP      = 1,
    SLICE_RTIMAGE_INPLANE90 = 2,
    SLICE_RTIMAGE_INPLANE   = 3
  };
  
  static const double WorkPhaseColor[vtkProstateNavLogic::NumPhases][3];
  static const double WorkPhaseColorActive[vtkProstateNavLogic::NumPhases][3];
  static const double WorkPhaseColorDisabled[vtkProstateNavLogic::NumPhases][3];
  static const char* WorkPhaseStr[vtkProstateNavLogic::NumPhases];
  //ETX
  
 public:
  // Description:    
  // Usual vtk class functions
  static vtkProstateNavGUI* New (  );
  vtkTypeRevisionMacro ( vtkProstateNavGUI, vtkSlicerModuleGUI );
  void PrintSelf (ostream& os, vtkIndent indent );
  
  //SendDATANavitrack
  // Description:    
  // Get methods on class members (no Set methods required)
  vtkGetObjectMacro ( Logic, vtkProstateNavLogic );
  
  // Description:
  // API for setting VolumeNode, VolumeLogic and
  // for both setting and observing them.
  void SetModuleLogic ( vtkProstateNavLogic *logic )
  { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
  void SetAndObserveModuleLogic ( vtkProstateNavLogic *logic )
  { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }
  // Description: 
  // Get wizard widget
  vtkGetObjectMacro(WizardWidget, vtkKWWizardWidget);

  // Get Target Fiducials (used in the wizard steps)
  vtkGetStringMacro ( FiducialListNodeID );
  vtkSetStringMacro ( FiducialListNodeID );
  vtkGetObjectMacro ( FiducialListNode, vtkMRMLFiducialListNode );
  vtkSetObjectMacro ( FiducialListNode, vtkMRMLFiducialListNode );

  // Description:    
  // This method builds the IGTDemo module GUI
  virtual void BuildGUI ( );
  
  // Description:
  // Add/Remove observers on widgets in the GUI
  virtual void AddGUIObservers ( );
  virtual void RemoveGUIObservers ( );

  void AddLogicObservers ( );
  void RemoveLogicObservers ( );
  
  
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
  vtkProstateNavGUI ( );
  virtual ~vtkProstateNavGUI ( );
  
  //----------------------------------------------------------------
  // GUI widgets
  //----------------------------------------------------------------
  
  //----------------------------------------------------------------
  // Workphase Frame
  
  vtkKWPushButtonSet *WorkPhaseButtonSet;

  vtkKWEntry *ScannerStatusLabelDisp;
  vtkKWEntry *SoftwareStatusLabelDisp;
  vtkKWEntry *RobotStatusLabelDisp;
  
  //----------------------------------------------------------------
  // Wizard Frame
  
  vtkKWWizardWidget *WizardWidget;
  vtkProstateNavStep **WizardSteps;
  
  //----------------------------------------------------------------
  // Visualization Control Frame

  vtkKWCheckButton *FreezeImageCheckButton;
  vtkKWPushButton  *SetLocatorModeButton;
  vtkKWPushButton  *SetUserModeButton;

  vtkKWMenuButton  *RedSliceMenu;
  vtkKWMenuButton  *YellowSliceMenu;
  vtkKWMenuButton  *GreenSliceMenu;
  vtkKWCheckButton *ImagingControlCheckButton;
  vtkKWMenuButton  *ImagingMenu;
  
  vtkKWPushButton  *StartScanButton;
  vtkKWPushButton  *StopScanButton;

  vtkKWCheckButton *LocatorCheckButton;
  
  // Module logic and mrml pointers

  //----------------------------------------------------------------
  // Logic Values
  //----------------------------------------------------------------

  vtkProstateNavLogic *Logic;

  vtkIGTDataManager *DataManager;
  vtkIGTPat2ImgRegistration *Pat2ImgReg;
  vtkCallbackCommand *DataCallbackCommand;

  // Access the slice windows
  vtkSlicerSliceLogic *Logic0;
  vtkSlicerSliceLogic *Logic1;
  vtkSlicerSliceLogic *Logic2;
  vtkMRMLSliceNode *SliceNode0;
  vtkMRMLSliceNode *SliceNode1;
  vtkMRMLSliceNode *SliceNode2;
  vtkSlicerSliceControllerWidget *Control0;
  vtkSlicerSliceControllerWidget *Control1;
  vtkSlicerSliceControllerWidget *Control2;

  //BTX
  std::string LocatorModelID;
  std::string LocatorModelID_new;
  //ETX
  
  int NeedOrientationUpdate0;
  int NeedOrientationUpdate1;
  int NeedOrientationUpdate2;
  
  //int NeedRealtimeImageUpdate;
  int FreezeOrientationUpdate;

  int RealtimeImageOrient;

  //----------------------------------------------------------------
  // Target Fiducials
  //----------------------------------------------------------------

  char *FiducialListNodeID;
  vtkMRMLFiducialListNode *FiducialListNode;

  void UpdateAll();
  /*
  void UpdateLocator(vtkTransform *, vtkTransform *);
  void UpdateSliceDisplay(float nx, float ny, float nz, 
                          float tx, float ty, float tz, 
                          float px, float py, float pz);

  void UpdateLocator();
  void UpdateSliceDisplay();
  */

  void UpdateDeviceStatus();
  
 private:

  vtkProstateNavGUI ( const vtkProstateNavGUI& ); // Not implemented.
  void operator = ( const vtkProstateNavGUI& ); //Not implemented.
  
  void BuildGUIForWorkPhaseFrame();
  void BuildGUIForWizardFrame();
  void BuildGUIForHelpFrame();
  void BuildGUIForDeviceFrame();
  void BuildGUIForVisualizationControlFrame();
  
  int  ChangeWorkPhase(int phase, int fChangeWizard=0);
  void ChangeSlicePlaneDriver(int slice, const char* driver);
  
};



#endif
