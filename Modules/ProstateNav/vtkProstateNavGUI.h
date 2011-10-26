/*=auto=========================================================================

  Portions (c) Copyright 2007 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
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

#include "vtkCallbackCommand.h"
#include "vtkSlicerInteractorStyle.h"

#include <string>
#include <list>

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
class vtkSlicerNodeSelectorWidget;

class vtkProstateNavStep;
class vtkSlicerSecondaryViewerWindow;

class vtkMRMLProstateNavManagerNode;
 
// Description:    
// This class implements Slicer's Volumes GUI
//
class VTK_PROSTATENAV_EXPORT vtkProstateNavGUI : public vtkSlicerModuleGUI
{
 public:
  
  virtual void Register(vtkObject *o) { Superclass::Register(o); };
  virtual void UnRegister(vtkObject *o) { Superclass::UnRegister(o); };

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
  enum {
    BRING_MARKERS_TO_VIEW_KEEP_CURRENT_ORIENTATION, // show slices in their original (acquisition) directions
    BRING_MARKERS_TO_VIEW_ALIGN_TO_NEEDLE           // show needle aligned slices (parallel and perpendicular to the needle and robot main axis)
  };
  
  
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
  void SetModuleLogic ( vtkSlicerLogic *logic )
  { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
  void SetAndObserveModuleLogic ( vtkProstateNavLogic *logic )
  { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }
  // Description: 
  // Get wizard widget
  vtkGetObjectMacro(WizardWidget, vtkKWWizardWidget);

  vtkGetObjectMacro ( ProstateNavManager, vtkMRMLProstateNavManagerNode );

  // Description:    
  // This method builds the IGTDemo module GUI
  virtual void BuildGUI ( );

  // Description:    
  // This method builds the IGTDemo module GUI
  virtual void TearDownGUI ( );
  
  // Description:
  // Add/Remove observers on widgets in the GUI
  virtual void AddGUIObservers ( );
  virtual void RemoveGUIObservers ( );

  void AddLogicObservers ( );
  void RemoveLogicObservers ( );

  void AddMRMLObservers();
  void RemoveMRMLObservers();

  // Description: 
  // Get the categorization of the module.
  const char *GetCategory() const { return "IGT"; }
  
  // Description:
  // Class's mediator methods for processing events invoked by
  // either the Logic, MRML or GUI.    
  virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  void HandleMouseEvent(vtkSlicerInteractorStyle *style);
  
  // Description:
  // Describe behavior at module startup and exit.
  virtual void Enter (vtkMRMLNode *node);
  virtual void Enter ();
  virtual void Exit ( );
  
  void Init();

  static void DataCallback(vtkObject *caller, 
                           unsigned long eid, void *clientData, void *callData);
  
  
  // Description:
  // Bring a marker to view in all three slice views along its principal axes
  // N - the direction vector of the locator,
  // T - the transverse direction vector of the locator (optional)
  // P - the tip location of the locator (optional)
  // All the above values are in RAS space. 
  void BringMarkerToViewIn2DViews(double* P, double* N=NULL, double* T=NULL);

  // Description:
  // Bring current target to view in all three slice views
  void BringTargetToViewIn2DViews(int mode);

 protected:
  vtkProstateNavGUI ( );
  virtual ~vtkProstateNavGUI ( );
  
  void SetProstateNavManager(vtkMRMLProstateNavManagerNode* node);
  void SetRobot(vtkMRMLRobotNode* robot);
  void SetTargetPlanList(vtkMRMLFiducialListNode* targetPlanList);

  // Return i-th worfklow step page
  vtkProstateNavStep* GetStepPage(int i);

  //----------------------------------------------------------------
  // GUI widgets
  //----------------------------------------------------------------

  vtkSlicerSecondaryViewerWindow* SecondaryWindow;

  // Configuration Frame

  vtkSlicerNodeSelectorWidget* ProstateNavManagerSelectorWidget;
  vtkSlicerNodeSelectorWidget* RobotSelectorWidget;

  //----------------------------------------------------------------
  // Workphase Frame
  
  vtkKWFrame *StatusButtonFrame;
  vtkKWPushButtonSet *StatusButtonSet;

  vtkKWFrame *WorkphaseButtonFrame;
  vtkKWPushButtonSet *WorkphaseButtonSet;
  
  //----------------------------------------------------------------
  // Wizard Frame

  vtkSlicerModuleCollapsibleFrame *WizardFrame;
  vtkKWWizardWidget *WizardWidget;
  
  //----------------------------------------------------------------
  // Logic Values
  //----------------------------------------------------------------

  vtkProstateNavLogic *Logic;

  vtkCallbackCommand *DataCallbackCommand;

  //----------------------------------------------------------------
  // Target Fiducials
  //----------------------------------------------------------------

  void UpdateGUI();

 private:

  vtkProstateNavGUI ( const vtkProstateNavGUI& ); // Not implemented.
  void operator = ( const vtkProstateNavGUI& ); //Not implemented.
  
  void BuildGUIForConfigurationFrame();
  void BuildGUIForWorkphaseFrame();
  void BuildGUIForWizardFrame();
  void BuildGUIForHelpFrame();

  void UpdateStatusButtons();
  void UpdateWorkflowSteps();
  
  int  ChangeWorkphase(int phase, int fChangeWizard=0);
  const char* AddZFrameModel(const char* nodeName);

  // Description:
  // Display current target fiducial highlighted
  void UpdateCurrentTargetDisplay();
  void UpdateCurrentTargetDisplayInSecondaryWindow();

  int Entered;

  // store the currently displayed workflow steps
  // if the same steps requested to be displayed, then nothing will happen
  vtkStringArray* DisplayedWorkflowSteps;

  vtkMRMLProstateNavManagerNode* ProstateNavManager;
  vtkMRMLRobotNode* Robot;
  vtkMRMLFiducialListNode* TargetPlanList;
  
};



#endif
