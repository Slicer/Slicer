/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#ifndef __vtkIGTToolSelectorGUI_h
#define __vtkIGTToolSelectorGUI_h

#ifdef WIN32
#include "vtkIGTToolSelectorWin32Header.h"
#endif

#include "vtkSlicerModuleGUI.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerInteractorStyle.h"

#include "vtkIGTToolSelectorLogic.h"


class vtkKWPushButton;
class vtkKWCheckButton;
class vtkKWListBoxWithScrollbarsWithLabel;
class vtkKWScaleWithEntry;
class vtkSlicerNodeSelectorWidget;

class VTK_IGTToolSelector_EXPORT vtkIGTToolSelectorGUI : public vtkSlicerModuleGUI
{
 public:

  vtkTypeRevisionMacro ( vtkIGTToolSelectorGUI, vtkSlicerModuleGUI );

  //----------------------------------------------------------------
  // Set/Get Methods
  //----------------------------------------------------------------

  vtkGetObjectMacro ( Logic, vtkIGTToolSelectorLogic );
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

  vtkIGTToolSelectorGUI ( );
  virtual ~vtkIGTToolSelectorGUI ( );

 private:
  vtkIGTToolSelectorGUI ( const vtkIGTToolSelectorGUI& ); // Not implemented.
  void operator = ( const vtkIGTToolSelectorGUI& ); //Not implemented.

 public:
  //----------------------------------------------------------------
  // New method, Initialization etc.
  //----------------------------------------------------------------

  static vtkIGTToolSelectorGUI* New ();
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
  void BuildGUIForToolPropertiesFrame();

  //----------------------------------------------------------------
  // Update routines
  //----------------------------------------------------------------

  void UpdateAll();


 protected:
  
  //----------------------------------------------------------------
  // Timer
  //----------------------------------------------------------------
  
  int TimerFlag;
  int TimerInterval;

  //----------------------------------------------------------------
  // GUI widgets
  //----------------------------------------------------------------

  //BTX
  // transform selector.
  vtkSmartPointer<vtkSlicerNodeSelectorWidget> ProbeTransformSelector;

  // list of tool types.
  vtkSmartPointer<vtkKWListBoxWithScrollbarsWithLabel> ToolTypeListBox;

  enum
  {
    OPENIGT_DEFAULT,
    NEEDLE,
    POINTER
  };

  // enable tool device.
  vtkSmartPointer<vtkKWCheckButton> EnableToolCheckButton;
  vtkSmartPointer<vtkKWCheckButton> ShowAxesCheckButton;
  vtkSmartPointer<vtkKWCheckButton> ShowProjectionCheckButton;
  vtkSmartPointer<vtkKWScaleWithEntry> ProjectionLengthScale;
  vtkSmartPointer<vtkKWScaleWithEntry> ProjectionDiameterScale;
  vtkSmartPointer<vtkKWScaleWithEntry> ToolTipDiameterScale;
  vtkSmartPointer<vtkKWCheckButton> ShowToolTipCheckButton;
 //ETX

  //----------------------------------------------------------------
  // Logic Values
  //----------------------------------------------------------------
  vtkIGTToolSelectorLogic *Logic;
  vtkCallbackCommand *DataCallbackCommand;
  int CloseScene;

};



#endif
