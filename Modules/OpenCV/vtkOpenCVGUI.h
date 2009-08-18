/*==========================================================================
 
 Portions (c) Copyright 2009 Atsushi Yamada (Fujimoto Lab, Nagoya Institute of Technology (NIT) All Rights Reserved.
 
 Acknowledgement: K. Chinzei (AIST), Y. Hayashi(Nagoya Univ.), T. Takeuchi (SFC Corp.), J. Tokuda(BWH), N. Hata(BWH), and H. Fujimoto(NIT) 
 CMakeLists.txt, FindOpenCV.cmake, and FindOpenIGTLink.cmake are contributions of K. Chinzei(AIST) and T. Takeuchi (SFC Corp.).
 
 See README.txt
 or http://www.slicer.org/copyright/copyright.txt for details.
 
 Program:   OpenCV
 Module:    $HeadURL: $
 Date:      $Date:07/17/2009 $
 Version:   $Revision: $
 
 ==========================================================================*/

#ifndef __vtkOpenCVGUI_h
#define __vtkOpenCVGUI_h

#ifdef WIN32
#include "vtkOpenCVWin32Header.h"
#endif

#include "vtkSlicerModuleGUI.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerInteractorStyle.h"

#include "vtkOpenCVLogic.h"

class vtkKWPushButton;

class VTK_OpenCV_EXPORT vtkOpenCVGUI : public vtkSlicerModuleGUI
{
 public:

  vtkTypeRevisionMacro ( vtkOpenCVGUI, vtkSlicerModuleGUI );

  //----------------------------------------------------------------
  // Set/Get Methods
  //----------------------------------------------------------------

  vtkGetObjectMacro ( Logic, vtkOpenCVLogic );
  void SetModuleLogic ( vtkSlicerLogic *logic )
  { 
    this->SetLogic ( vtkObjectPointer (&this->Logic), logic );
  }

  // Description: 
  // Get the categorization of the module.
  const char *GetCategory() const { return "IGT"; }


 protected:
  //----------------------------------------------------------------
  // Constructor / Destructor (proctected/private) 
  //----------------------------------------------------------------

  vtkOpenCVGUI ( );
  virtual ~vtkOpenCVGUI ( );

 private:
  vtkOpenCVGUI ( const vtkOpenCVGUI& ); // Not implemented.
  void operator = ( const vtkOpenCVGUI& ); //Not implemented.

 public:
  //----------------------------------------------------------------
  // New method, Initialization etc.
  //----------------------------------------------------------------

  static vtkOpenCVGUI* New ();
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
  void BuildGUIForTestFrame1();
  void BuildGUIForTestFrame2();

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
    
  int viewerInitFlag ;

  //----------------------------------------------------------------
  // GUI widgets
  //----------------------------------------------------------------

  vtkKWPushButton* showCaptureData;
  vtkKWPushButton* closeCaptureData;
  vtkKWPushButton* TestButton21;
  vtkKWPushButton* TestButton22;

  //----------------------------------------------------------------
  // Logic Values
  //----------------------------------------------------------------

  vtkOpenCVLogic *Logic;
  vtkCallbackCommand *DataCallbackCommand;
  int                        CloseScene;

};



#endif
