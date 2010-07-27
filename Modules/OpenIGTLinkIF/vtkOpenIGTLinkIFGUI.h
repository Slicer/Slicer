/*==========================================================================

  Portions (c) Copyright 2008-2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL$
  Date:      $Date$
  Version:   $Revision$

==========================================================================*/

#ifndef __vtkOpenIGTLinkIFGUI_h
#define __vtkOpenIGTLinkIFGUI_h

#ifdef WIN32
#include "vtkOpenIGTLinkIFWin32Header.h"
#endif

#include "vtkSlicerModuleGUI.h"
#include "vtkOpenIGTLinkIFLogic.h"
#include "vtkMRMLIGTLConnectorNode.h"

#include "vtkIGTLRemoteDataListWindow.h"
#include "vtkIGTLTrackingDataControllerWindow.h"


#include "vtkIGTDataManager.h"
#include "vtkIGTPat2ImgRegistration.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerInteractorStyle.h"

#include <string>
#include <vector>

class vtkKWPushButton;
class vtkKWRadioButtonSet;
class vtkKWMenuButton;
class vtkKWCheckButton;
class vtkKWScaleWithEntry;
class vtkKWEntry;
class vtkKWFrame;
class vtkKWMultiColumnListWithScrollbars;
class vtkKWTreeWithScrollbars;
class vtkSlicerNodeSelectorWidget;
class vtkKWRadioButtonSetWithLabel;

class vtkTransform;

// Description:    
// This class implements Slicer's Volumes GUI
//
class VTK_OPENIGTLINKIF_EXPORT vtkOpenIGTLinkIFGUI : public vtkSlicerModuleGUI
{

  //----------------------------------------------------------------
  // Type defines
  //----------------------------------------------------------------
 public:
  //BTX
  enum {
    SLICE_PLANE_RED    = 0,
    SLICE_PLANE_YELLOW = 1,
    SLICE_PLANE_GREEN  = 2
  };

  // Connector List update level options
  enum {
    UPDATE_SELECTED_ONLY   = 0,  // Update selected item only
    UPDATE_STATUS_ALL      = 1,  // Update status for all items
    UPDATE_PROPERTY_ALL    = 2,  // Update all properties for all items
    UPDATE_ALL             = 3,  // Update whole list (incl. changed number of items)
  };

  enum {
    NODE_NONE      = 0,
    NODE_CONNECTOR = 1,
    NODE_IO        = 2,
    NODE_DEVICE    = 3
  };

  static const char* ConnectorTypeStr[vtkMRMLIGTLConnectorNode::NUM_TYPE];
  static const char* ConnectorStatusStr[vtkMRMLIGTLConnectorNode::NUM_STATE];

  typedef struct {
    std::string nodeName;
    std::string connectorID;
    std::string nodeID;
    int         io;
  } IOConfigNodeInfoType;

  typedef std::list<IOConfigNodeInfoType> IOConfigNodeInfoListType;
  //ETX

  //----------------------------------------------------------------
  // Access functions
  //----------------------------------------------------------------
 public:
  // Description:    
  // Usual vtk class functions
  static vtkOpenIGTLinkIFGUI* New ();
  vtkTypeRevisionMacro ( vtkOpenIGTLinkIFGUI, vtkSlicerModuleGUI );
  void PrintSelf (ostream& os, vtkIndent indent );
  
  // Description: 
  // Get the categorization of the module.
  const char *GetCategory() const { return "IGT"; }

  //SendDATANavitrack
  // Description:    
  // Get methods on class members (no Set methods required)
  vtkGetObjectMacro ( Logic, vtkOpenIGTLinkIFLogic );
  
  // Description:
  // API for setting VolumeNode, VolumeLogic and
  // for both setting and observing them.
  void SetModuleLogic ( vtkSlicerLogic *logic )
  { 
    this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }

  //void SetAndObserveModuleLogic ( vtkOpenIGTLinkIFLogic *logic )
  //{ this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }
  // Description: 

  // Get Target Fiducials (used in the wizard steps)
  vtkGetStringMacro ( FiducialListNodeID );
  vtkSetStringMacro ( FiducialListNodeID );
  vtkGetObjectMacro ( FiducialListNode, vtkMRMLFiducialListNode );
  vtkSetObjectMacro ( FiducialListNode, vtkMRMLFiducialListNode );

  //----------------------------------------------------------------
  // Event handlers
  //----------------------------------------------------------------
 public:
  // Description:    
  // This method builds the IGTDemo module GUI
  virtual void BuildGUI ( );

 private:
  void BuildGUIForWizardFrame();
  void BuildGUIForHelpFrame();
  void BuildGUIForConnectorBrowserFrame();
  void BuildGUIForIOConfig();
  void BuildGUIForDeviceFrame();
  void BuildGUIForVisualizationControlFrame();
  
  //----------------------------------------------------------------
  // Event handlers
  //----------------------------------------------------------------
 public:
  // Description:
  // Add/Remove observers and even handlers
  virtual void AddGUIObservers ( );
  virtual void RemoveGUIObservers ( );
  void         AddLogicObservers ( );
  void         RemoveLogicObservers ( );

  virtual void OpenRemoteDataListWindow(const char* conID);
  virtual void OpenTrackingDataControllerWindow(const char* conID);
  //virtual void AddNodeCallback(const char* conID, int io, const char* name, const char* type);
  virtual void AddNodeCallback(const char* conID, int io, const char* nodeID, const char* devType);
  virtual void ExportDataToIGTLCallback(const char* conID, const char* nodeID);
  virtual void DeleteNodeCallback(const char* conID, int io, const char* nodeID);

  // Description:
  // Class's mediator methods for processing events invoked by
  // either the Logic, MRML or GUI.    

  virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  //virtual int  OnMrmlNodeListChanged(int row, int col, const char* item);
  void         ProcessTimerEvents();
  void         HandleMouseEvent(vtkSlicerInteractorStyle *style);
  //BTX
  static void  DataCallback(vtkObject *caller, 
                           unsigned long eid, void *clientData, void *callData);
  //ETX
  
 public:
  // Description:
  // Describe behavior at module startup and exit.
  virtual void Enter ( );
  virtual void Exit ( );
  void Init();

  void UpdateAll();
  
  //----------------------------------------------------------------
  // Constructor / Destructor
  //----------------------------------------------------------------
 protected:
  vtkOpenIGTLinkIFGUI ( );
  virtual ~vtkOpenIGTLinkIFGUI ( );

  //----------------------------------------------------------------
  // Operators
  //----------------------------------------------------------------
 private:
  vtkOpenIGTLinkIFGUI ( const vtkOpenIGTLinkIFGUI& ); // Not implemented.
  void operator = ( const vtkOpenIGTLinkIFGUI& ); //Not implemented.
  
  //----------------------------------------------------------------
  // Dynamic GUIs
  //----------------------------------------------------------------
 private:
  void IOConfigTreeContextMenu(const char *callData);
  //BTX
  //int  IsIOConfigTreeLeafSelected(const char* callData, std::string& conID, int* devID, int* io);
  int  IsIOConfigTreeLeafSelected(const char* callData, std::string& conID, int* io, std::string& nodeID);
  //ETX
  void AddIOConfigContextMenuItem(int type, const char* conID, int io, const char* nodeID);

  // Description:
  //  Enable/disable menus in Driver panel based on Locator source data.
  void UpdateDriverPanel();

  void ChangeSlicePlaneDriver(int slice, const char* driver);
  void UpdateIOConfigTree();
  void UpdateConnectorList(int updateLevel);
  void UpdateConnectorPropertyFrame(int i);
  void UpdateConnectorNodeList();

  vtkMRMLIGTLConnectorNode* GetConnector(const char* nodeID);

 private:
  //----------------------------------------------------------------
  // Timer
  //----------------------------------------------------------------
  int TimerFlag;
  int TimerInterval;
  vtkTimerLog* TimerLog;

  //----------------------------------------------------------------
  // GUI widgets
  //----------------------------------------------------------------
  
  //----------------------------------------------------------------
  // Connector Browser Frame

  vtkKWMultiColumnListWithScrollbars* ConnectorList;
  vtkKWPushButton*     AddConnectorButton;
  vtkKWPushButton*     DeleteConnectorButton;
  vtkKWEntry*          ConnectorNameEntry;
  vtkKWRadioButtonSet* ConnectorTypeButtonSet;
  vtkKWCheckButton*    ConnectorStatusCheckButton;
  vtkKWEntry*          ConnectorAddressEntry;
  vtkKWEntry*          ConnectorPortEntry;
  vtkKWRadioButtonSetWithLabel* CheckCRCButtonSet;

  //----------------------------------------------------------------
  // Data I/O Configuration frame

  vtkKWCheckButton*        EnableAdvancedSettingButton;
  vtkKWTreeWithScrollbars* IOConfigTree;
  vtkKWMenu*               IOConfigContextMenu;
  vtkKWMultiColumnListWithScrollbars* MrmlNodeList;

  //----------------------------------------------------------------
  // Visualization Control Frame

  vtkKWCheckButton*    FreezeImageCheckButton;
  vtkKWCheckButton*    ObliqueCheckButton;
  vtkKWPushButton*     SetLocatorModeButton;
  vtkKWPushButton*     SetUserModeButton;

  vtkKWMenuButton*     RedSliceMenu;
  vtkKWMenuButton*     YellowSliceMenu;
  vtkKWMenuButton*     GreenSliceMenu;
  vtkKWCheckButton*    ImagingControlCheckButton;
  vtkKWMenuButton*     ImagingMenu;

  vtkSlicerNodeSelectorWidget* ImageSourceSelectorWidget;
  vtkSlicerNodeSelectorWidget* LocatorSourceSelectorWidget;

  vtkKWCheckButton* LocatorCheckButton;
  bool              IsSliceOrientationAdded;
  // Module logic and mrml pointers

  //----------------------------------------------------------------
  // Remote Data List Window
  vtkIGTLRemoteDataListWindow* RemoteDataWindow;
  vtkIGTLTrackingDataControllerWindow* TrackingDataControllerWindow;

  //----------------------------------------------------------------
  // Logic Values
  //----------------------------------------------------------------

  vtkOpenIGTLinkIFLogic* Logic;

  vtkIGTDataManager* DataManager;
  vtkCallbackCommand* DataCallbackCommand;

  // Access the slice windows
  vtkMRMLSliceNode *SliceNode0;
  vtkMRMLSliceNode *SliceNode1;
  vtkMRMLSliceNode *SliceNode2;

  // GUI Update flags
  int UpdateConnectorListFlag;
  int UpdateConnectorPropertyFrameFlag;
  int UpdateIOConfigTreeFlag;

  //----------------------------------------------------------------
  // Connector and MRML Node list management
  //----------------------------------------------------------------

  //ConnectorIDListType ConnectorIDList;
  //BTX
  // List of connector node
  std::vector<std::string> ConnectorNodeList;
  //ETX

  //BTX
  vtkOpenIGTLinkIFLogic::IGTLMrmlNodeListType CurrentNodeListAvailable;
  vtkOpenIGTLinkIFLogic::IGTLMrmlNodeListType CurrentNodeListSelected;
  vtkOpenIGTLinkIFLogic::IGTLMrmlNodeListType LocatorSourceList;
  vtkOpenIGTLinkIFLogic::IGTLMrmlNodeListType RealTimeImageSourceList;

  IOConfigNodeInfoListType IOConfigTreeConnectorList;
  IOConfigNodeInfoListType IOConfigTreeIOList;
  IOConfigNodeInfoListType IOConfigTreeNodeList;

  //ETX

  //----------------------------------------------------------------
  // Locator Model
  //----------------------------------------------------------------

  int                        CloseScene;

  //----------------------------------------------------------------
  // Target Fiducials
  //----------------------------------------------------------------

  char *FiducialListNodeID;
  vtkMRMLFiducialListNode *FiducialListNode;

};



#endif
