/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkOpenIGTLinkGUI_h
#define __vtkOpenIGTLinkGUI_h

#ifdef WIN32
#include "vtkOpenIGTLinkWin32Header.h"
#endif

#include "vtkSlicerModuleGUI.h"
#include "vtkOpenIGTLinkLogic.h"
#include "vtkIGTLConnector.h"

#include "vtkIGTDataManager.h"
#include "vtkIGTPat2ImgRegistration.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerInteractorStyle.h"

#include <string>

class vtkKWPushButton;
class vtkKWPushButtonSet;
class vtkKWRadioButtonSet;
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

class vtkOpenIGTLinkStep;

class vtkTransform;

// Description:    
// This class implements Slicer's Volumes GUI
//
class VTK_OPENIGTLINK_EXPORT vtkOpenIGTLinkGUI : public vtkSlicerModuleGUI
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

  // Connector List update level options
  enum {
    UPDATE_SELECTED_ONLY   = 0,  // Update selected item only
    UPDATE_STATUS_ALL      = 1,  // Update status for all items
    UPDATE_PROPERTY_ALL    = 2,  // Update all properties for all items
    UPDATE_ALL             = 3,  // Update whole list (incl. changed number of items)
  };
  
  static const char* ConnectorTypeStr[vtkIGTLConnector::NUM_TYPE];
  static const char* ConnectorStatusStr[vtkIGTLConnector::NUM_STATE];
  //ETX

 public:
  // Description:    
  // Usual vtk class functions
  static vtkOpenIGTLinkGUI* New ();
  vtkTypeRevisionMacro ( vtkOpenIGTLinkGUI, vtkSlicerModuleGUI );
  void PrintSelf (ostream& os, vtkIndent indent );
  
  //SendDATANavitrack
  // Description:    
  // Get methods on class members (no Set methods required)
  vtkGetObjectMacro ( Logic, vtkOpenIGTLinkLogic );
  
  // Description:
  // API for setting VolumeNode, VolumeLogic and
  // for both setting and observing them.
  void SetModuleLogic ( vtkSlicerLogic *logic )
  { 
    this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }

  //void SetAndObserveModuleLogic ( vtkOpenIGTLinkLogic *logic )
  //{ this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }
  // Description: 

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

  void ProcessTimerEvents();

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
  vtkOpenIGTLinkGUI ( );
  virtual ~vtkOpenIGTLinkGUI ( );
  
  //----------------------------------------------------------------
  // Timer
  //----------------------------------------------------------------
  
  int TimerFlag;
  int TimerInterval;

  //----------------------------------------------------------------
  // GUI widgets
  //----------------------------------------------------------------
  
  //----------------------------------------------------------------
  // Connector Browser Frame

  vtkKWMultiColumnListWithScrollbars* ConnectorList;
  vtkKWPushButton* AddConnectorButton;
  vtkKWPushButton* DeleteConnectorButton;

  vtkKWEntry*          ConnectorNameEntry;
  vtkKWRadioButtonSet* ConnectorTypeButtonSet;
  vtkKWCheckButton*    ConnectorStatusCheckButton;
  vtkKWEntry*          ConnectorAddressEntry;
  vtkKWEntry*          ConnectorPortEntry;

  
  //----------------------------------------------------------------
  // Visualization Control Frame

  vtkKWCheckButton *FreezeImageCheckButton;
  vtkKWCheckButton *ObliqueCheckButton;
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

  bool              IsSliceOrientationAdded;

  
  // Module logic and mrml pointers

  //----------------------------------------------------------------
  // Logic Values
  //----------------------------------------------------------------

  vtkOpenIGTLinkLogic *Logic;

  vtkIGTDataManager *DataManager;
  vtkIGTPat2ImgRegistration *Pat2ImgReg;
  vtkCallbackCommand *DataCallbackCommand;

  // Access the slice windows
  vtkMRMLSliceNode *SliceNode0;
  vtkMRMLSliceNode *SliceNode1;
  vtkMRMLSliceNode *SliceNode2;
  /*
  vtkSlicerSliceLogic *Logic0;
  vtkSlicerSliceLogic *Logic1;
  vtkSlicerSliceLogic *Logic2;
  vtkSlicerSliceControllerWidget *Control0;
  vtkSlicerSliceControllerWidget *Control1;
  vtkSlicerSliceControllerWidget *Control2;
  */

  //BTX
  std::string LocatorModelID;
  std::string LocatorModelID_new;
  //ETX
  
  //int RealtimeImageOrient;

  //----------------------------------------------------------------
  // Locator Model
  //----------------------------------------------------------------

  //vtkMRMLModelNode           *LocatorModel;
  int                        CloseScene;


  //----------------------------------------------------------------
  // Target Fiducials
  //----------------------------------------------------------------

  char *FiducialListNodeID;
  vtkMRMLFiducialListNode *FiducialListNode;

  void UpdateAll();

 private:

  vtkOpenIGTLinkGUI ( const vtkOpenIGTLinkGUI& ); // Not implemented.
  void operator = ( const vtkOpenIGTLinkGUI& ); //Not implemented.
  
  void BuildGUIForWizardFrame();
  void BuildGUIForConnectorBrowserFrame ();
  void BuildGUIForHelpFrame();
  void BuildGUIForDeviceFrame();
  void BuildGUIForVisualizationControlFrame();
  
  int  ChangeWorkPhase(int phase, int fChangeWizard=0);
  void ChangeSlicePlaneDriver(int slice, const char* driver);


  //----------------------------------------------------------------
  // Connector List and Properties control
  //----------------------------------------------------------------

  void UpdateConnectorList(int updateLevel);
  void UpdateConnectorPropertyFrame(int i);

 
};



#endif
