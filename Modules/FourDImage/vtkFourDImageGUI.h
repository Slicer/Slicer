/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.na-mic.org/NAMICSandBox/trunk/IGTLoadableModules/FourDImage/vtkFourDImageGUI.h $
  Date:      $Date: 2009-02-03 12:05:00 -0500 (Tue, 03 Feb 2009) $
  Version:   $Revision: 3633 $

==========================================================================*/

#ifndef __vtkFourDImageGUI_h
#define __vtkFourDImageGUI_h

#ifdef WIN32
#include "vtkFourDImageWin32Header.h"
#endif

#include "vtkSlicerModuleGUI.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerInteractorStyle.h"
#include "vtkDoubleArray.h"

#include "vtkFourDImageLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"

#include "vtkKWMultiColumnListWithScrollbars.h"

#include "vtkFourDImageIcons.h"

#include <map>

class vtkKWPushButton;
class vtkKWMenuButton;
class vtkKWSpinBox;
class vtkKWProgressDialog;
class vtkKWRadioButtonSet;
class vtkKWRadioButton;
class vtkKWRange;
class vtkKWEntry;
class vtkKWScaleWithEntry;
class vtkKWLoadSaveButtonWithLabel;
class vtkKWCheckButtonWithLabel;
class vtkKWRadioButtonSetWithLabel;
class vtkKWMenuButtonWithLabel;

class VTK_FourDImage_EXPORT vtkFourDImageGUI : public vtkSlicerModuleGUI
{
 public:

  vtkTypeRevisionMacro ( vtkFourDImageGUI, vtkSlicerModuleGUI );

  //----------------------------------------------------------------
  // Set/Get Methods
  //----------------------------------------------------------------

  vtkGetObjectMacro ( Logic, vtkFourDImageLogic );
  virtual void SetModuleLogic ( vtkFourDImageLogic *logic )
  { 
    this->SetLogic ( vtkObjectPointer (&this->Logic), logic );
  }
  virtual void SetAndObserveModuleLogic ( vtkFourDImageLogic *logic )
  { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }
  virtual void SetModuleLogic( vtkSlicerLogic *logic )
  { this->SetModuleLogic (reinterpret_cast<vtkFourDImageLogic*> (logic)); }
  

 protected:
  //----------------------------------------------------------------
  // Constructor / Destructor (proctected/private) 
  //----------------------------------------------------------------

  vtkFourDImageGUI ( );
  virtual ~vtkFourDImageGUI ( );

 private:
  vtkFourDImageGUI ( const vtkFourDImageGUI& ); // Not implemented.
  void operator = ( const vtkFourDImageGUI& ); //Not implemented.

 public:
  //----------------------------------------------------------------
  // New method, Initialization etc.
  //----------------------------------------------------------------

  static vtkFourDImageGUI* New ();
  void Init();
  virtual void Enter ( );
  //BTX
  using vtkSlicerComponentGUI::Enter; 
  //ETX
  virtual void Exit ( );
  void PrintSelf (ostream& os, vtkIndent indent );

  // Get the categorization of the module.
  const char *GetCategory() const { return "4D"; }

  //----------------------------------------------------------------
  // Observer Management
  //----------------------------------------------------------------

  virtual void AddGUIObservers ( );
  virtual void RemoveGUIObservers ( );
  void AddLogicObservers ( );
  void RemoveLogicObservers ( );

  //----------------------------------------------------------------
  // Event Handlers
  //----------------------------------------------------------------

  virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  void ProcessTimerEvents();
  void HandleMouseEvent(vtkSlicerInteractorStyle *style);
  static void DataCallback(vtkObject *caller, 
                           unsigned long eid, void *clientData, void *callData);
  
  //----------------------------------------------------------------
  // Build Frames
  //----------------------------------------------------------------

  virtual void BuildGUI ( );
  //BTX
  using vtkSlicerComponentGUI::BuildGUI; 
  //ETX
  virtual void TearDownGUI ( );
  void BuildGUIForHelpFrame();
  void BuildGUIForLoadFrame(int show);
  void BuildGUIForActiveBundleSelectorFrame();
  void BuildGUIForFrameControlFrame(int show);
  void BuildGUIForFrameFrameEditor(int show);

  //----------------------------------------------------------------
  // Update routines
  //----------------------------------------------------------------

  void UpdateAll();
  void SelectActiveTimeSeriesBundle(vtkMRMLTimeSeriesBundleNode* bundleNode);


  //----------------------------------------------------------------
  // Viewer control
  //----------------------------------------------------------------

  void SetForeground(const char* bundleID, int index);
  void SetBackground(const char* bundleID, int index);
  void SetWindowLevelForCurrentFrame();
  //void UpdateSeriesSelectorMenus();

  //----------------------------------------------------------------
  // Editor
  //----------------------------------------------------------------
  
  void UpdateFrameList(const char* bundleID, int selectColumn=-1);
  void UpdateTimeStamp(const char* bundleID);
  int  ImportFramesFromScene(const char* bundleID, const char* format, int min, int max);

  //----------------------------------------------------------------
  // Other utility functions
  //----------------------------------------------------------------
  const char* GetActiveTimeSeriesBundleNodeID();
  vtkMRMLTimeSeriesBundleNode *GetActiveTimeSeriesBundleNode ();

 protected:
  
  //----------------------------------------------------------------
  // Timer
  //----------------------------------------------------------------
  
  int TimerFlag;
  int TimerInterval;  // ms

  //----------------------------------------------------------------
  // GUI widgets
  //----------------------------------------------------------------

  vtkKWProgressDialog *ProgressDialog;

  // -----------------------------------------
  // Icons
  vtkFourDImageIcons* Icons;

  // -----------------------------------------
  // Active 4D Bundle selector
  vtkSlicerNodeSelectorWidget*  ActiveTimeSeriesBundleSelectorWidget;

  // -----------------------------------------
  // Load / Save / Active frame
  vtkKWLoadSaveButtonWithLabel* SelectInputDirectoryButton;
  vtkKWPushButton*              LoadImageButton;
  vtkKWLoadSaveButtonWithLabel* SelectOutputDirectoryButton;
  vtkKWPushButton*              SaveImageButton;
  vtkKWRadioButtonSetWithLabel* LoadOptionButtonSet;
  vtkKWEntryWithLabel*          LoadTimePointsEntry;
  vtkKWEntryWithLabel*          LoadSlicesEntry;
  vtkKWEntryWithLabel*          LoadChannelsEntry;
  vtkKWEntryWithLabel*          LoadFileFilterEntry;
  vtkKWMenuButtonWithLabel*     LoadFileOrderMenu;

  // -----------------------------------------
  // Frame control
  vtkKWRange *WindowLevelRange;
  vtkKWRange *ThresholdRange;

  vtkKWScaleWithEntry* ForegroundVolumeSelectorScale;
  vtkKWScaleWithEntry* BackgroundVolumeSelectorScale;

  vtkKWPushButton*     AutoPlayFGButton;
  vtkKWPushButton*     AutoPlayBGButton;
  vtkKWEntry*          AutoPlayIntervalEntry;

  // -----------------------------------------
  // Frame editor
  vtkKWMultiColumnListWithScrollbars* FrameList;
  vtkKWPushButton*                    FrameMoveUpButton;
  vtkKWPushButton*                    FrameMoveDownButton;
  vtkKWPushButton*                    RemoveFrameButton;
  vtkSlicerNodeSelectorWidget*        AddFrameNodeSelector;
  vtkKWPushButton*                    AddFrameNodeButton;
  vtkKWRadioButtonSetWithLabel*       TimeStampMethodButtonSet;
  vtkKWEntry*                         TimeStepEntry;
  vtkKWEntry*                         ImportFrameFormatEntry;
  vtkKWEntry*                         ImportFrameRangeMinEntry;
  vtkKWEntry*                         ImportFrameRangeMaxEntry;
  vtkKWPushButton*                    ImportFrameNodeButton;


  //----------------------------------------------------------------
  // Logic Values
  //----------------------------------------------------------------

  //BTX
  vtkFourDImageLogic::RegistrationParametersType DefaultAffineRegistrationParam;
  vtkFourDImageLogic::RegistrationParametersType DefaultRegistrationParam;
  //ETX
  
  vtkFourDImageLogic *Logic;
  vtkCallbackCommand *DataCallbackCommand;
  int                        CloseScene;

  double  RangeLower;
  double  RangeUpper;

  double  Window;
  double  Level;
  double  ThresholdUpper;
  double  ThresholdLower;


  // Auto play functions
  int     AutoPlayFG;
  int     AutoPlayBG;
  int     AutoPlayInterval;        // interval = TimerInterval * AutoPlayInterval; 
  int     AutoPlayIntervalCounter;

  //BTX
  typedef std::vector<int> WindowLevelUpdateStatusType;
  typedef std::vector<std::string> NodeIDListType;
  //ETX
  
  WindowLevelUpdateStatusType WindowLevelUpdateStatus;

  NodeIDListType MaskNodeIDList;
  NodeIDListType BundleNodeIDList;

  int BundleNameCount; // used to name 4D bundle

};



#endif
