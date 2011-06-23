/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkCollectFiducialsGUI_h
#define __vtkCollectFiducialsGUI_h

#ifdef WIN32
#include "vtkCollectFiducialsWin32Header.h"
#endif

#include "vtkSlicerModuleGUI.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerInteractorStyle.h"

#include "vtkCollectFiducialsLogic.h"

class vtkKWPushButton;
class vtkKWEntryWithLabel;
class vtkKWMultiColumnListWithScrollbars;
class vtkSlicerNodeSelectorWidget;

class VTK_CollectFiducials_EXPORT vtkCollectFiducialsGUI : public vtkSlicerModuleGUI
{
 public:

  vtkTypeRevisionMacro ( vtkCollectFiducialsGUI, vtkSlicerModuleGUI );

  //----------------------------------------------------------------
  // Set/Get Methods
  //----------------------------------------------------------------

  vtkGetObjectMacro ( Logic, vtkCollectFiducialsLogic );
  void SetModuleLogic ( vtkSlicerLogic *logic )
  { 
    this->SetLogic ( vtkObjectPointer (&this->Logic), logic );
  }

  // Get the categorization of the module.
  const char *GetCategory() const { return "IGT"; }

 protected:
  //----------------------------------------------------------------
  // Constructor / Destructor (proctected/private) 
  //----------------------------------------------------------------

  vtkCollectFiducialsGUI ( );
  virtual ~vtkCollectFiducialsGUI ( );

 private:
  vtkCollectFiducialsGUI ( const vtkCollectFiducialsGUI& ); // Not implemented.
  void operator = ( const vtkCollectFiducialsGUI& ); //Not implemented.

 public:
  //----------------------------------------------------------------
  // New method, Initialization etc.
  //----------------------------------------------------------------

  static vtkCollectFiducialsGUI* New ();
  void Init();
  virtual void Enter ( );
  virtual void Exit ( );
  void PrintSelf (ostream& os, vtkIndent indent );

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
  void BuildGUIForFiducialListNode();
  void BuildGUIForTrackerInfo();

  //----------------------------------------------------------------
  // Update routines
  //----------------------------------------------------------------

  void UpdateAll();
  void ResetFiducialList();
  void GetNewFiducialMeasure();
  void SaveFiducialList();

 protected:
  
  //----------------------------------------------------------------
  // Timer
  //----------------------------------------------------------------
  
  int TimerFlag;
  int TimerInterval;

  //----------------------------------------------------------------
  // GUI widgets
  //----------------------------------------------------------------

  vtkSlicerNodeSelectorWidget* FiducialListSelector;
  vtkKWMultiColumnListWithScrollbars *FiducialListMultiColumnList;
  vtkKWPushButton *ResetFiducialsButton;
  vtkKWEntryWithLabel *NumFiducialsEntry;
  vtkKWPushButton *GetNewMeasureButton;
  vtkKWPushButton *SaveFiducialsButton;

  vtkSlicerNodeSelectorWidget* ProbeTransformSelector;


  //----------------------------------------------------------------
  // Logic Values
  //----------------------------------------------------------------

  vtkCollectFiducialsLogic *Logic;
  vtkCallbackCommand *DataCallbackCommand;
  int                        CloseScene;

};



#endif
