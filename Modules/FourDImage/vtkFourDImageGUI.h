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

#include <map>

class vtkKWScaleWithEntry;
class vtkKWPushButton;
class vtkKWLoadSaveButtonWithLabel;
class vtkKWMenuButton;
class vtkKWSpinBox;
class vtkKWProgressDialog;
class vtkKWRadioButtonSet;
class vtkKWRadioButton;
class vtkKWRange;
class vtkKWCheckButtonWithLabel;

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
  void BuildGUIForHelpFrame();
  void BuildGUIForLoadFrame(int show);
  void BuildGUIForActiveBundleSelectorFrame();
  void BuildGUIForFrameControlFrame(int show);

  //----------------------------------------------------------------
  // Update routines
  //----------------------------------------------------------------

  void UpdateAll();
  void SelectActive4DBundle(vtkMRML4DBundleNode* bundleNode);


  //----------------------------------------------------------------
  // Viewer control
  //----------------------------------------------------------------

  void SetForeground(const char* bundleID, int index);
  void SetBackground(const char* bundleID, int index);
  void SetWindowLevelForCurrentFrame();
  void UpdateSeriesSelectorMenus();

  //----------------------------------------------------------------
  // Other utility functions
  //----------------------------------------------------------------
  const char* GetActive4DBundleNodeID();


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
  // Load / Save / Active frame
  vtkKWLoadSaveButtonWithLabel* SelectInputDirectoryButton;
  vtkKWPushButton*              LoadImageButton;
  vtkKWMenuButton*              SaveSeriesMenu;
  vtkKWLoadSaveButtonWithLabel* SelectOutputDirectoryButton;
  vtkKWPushButton*              SaveImageButton;
  vtkKWMenuButton*              ActiveSeriesMenu;

  // -----------------------------------------
  // Active 4D Bundle selector
  vtkSlicerNodeSelectorWidget*  Active4DBundleSelectorWidget;

  // -----------------------------------------
  // Frame control
  vtkKWRange *WindowLevelRange;
  vtkKWRange *ThresholdRange;

  vtkKWScaleWithEntry* ForegroundVolumeSelectorScale;
  vtkKWScaleWithEntry* BackgroundVolumeSelectorScale;

  vtkKWPushButton*     AutoPlayFGButton;
  vtkKWPushButton*     AutoPlayBGButton;
  vtkKWEntry*          AutoPlayIntervalEntry;

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
