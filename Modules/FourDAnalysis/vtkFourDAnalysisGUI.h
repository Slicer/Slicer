/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: http://svn.na-mic.org/NAMICSandBox/trunk/IGTLoadableModules/FourDAnalysis/vtkFourDAnalysisGUI.h $
  Date:      $Date: 2009-02-03 12:05:00 -0500 (Tue, 03 Feb 2009) $
  Version:   $Revision: 3633 $

==========================================================================*/

#ifndef __vtkFourDAnalysisGUI_h
#define __vtkFourDAnalysisGUI_h

#ifdef WIN32
#include "vtkFourDAnalysisWin32Header.h"
#endif

#include "vtkSlicerModuleGUI.h"
#include "vtkCallbackCommand.h"
#include "vtkSlicerInteractorStyle.h"
#include "vtkDoubleArray.h"

#include "vtkFourDAnalysisLogic.h"
#include "vtkKWPlotGraph.h"
#include "vtkSlicerNodeSelectorWidget.h"

#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWLoadSaveButtonWithLabel.h"

#include <map>

class vtkKWScaleWithEntry;
class vtkKWPushButton;
class vtkKWPiecewiseFunctionEditor;
class vtkKWMenuButton;
class vtkKWSpinBox;
class vtkKWCanvas;
class vtkKWProgressDialog;
class vtkKWRadioButtonSet;
class vtkKWRadioButton;
class vtkKWRange;
class vtkIntensityCurves;
class vtkKWCheckButtonWithLabel;
class vtkCurveAnalysisPythonInterface;


//class vtkFourDImageGUI;

class VTK_FourDAnalysis_EXPORT vtkFourDAnalysisGUI : public vtkSlicerModuleGUI
{
 public:

  vtkTypeRevisionMacro ( vtkFourDAnalysisGUI, vtkSlicerModuleGUI );

  //----------------------------------------------------------------
  // Set/Get Methods
  //----------------------------------------------------------------

  vtkGetObjectMacro ( Logic, vtkFourDAnalysisLogic );
  virtual void SetModuleLogic ( vtkFourDAnalysisLogic *logic )
  { 
    this->SetLogic ( vtkObjectPointer (&this->Logic), logic );
  }
  virtual void SetAndObserveModuleLogic ( vtkFourDAnalysisLogic *logic )
  { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }
  virtual void SetModuleLogic( vtkSlicerLogic *logic )
  { this->SetModuleLogic (reinterpret_cast<vtkFourDAnalysisLogic*> (logic)); }
  

 protected:
  //----------------------------------------------------------------
  // Constructor / Destructor (proctected/private) 
  //----------------------------------------------------------------

  vtkFourDAnalysisGUI ( );
  virtual ~vtkFourDAnalysisGUI ( );

 private:
  vtkFourDAnalysisGUI ( const vtkFourDAnalysisGUI& ); // Not implemented.
  void operator = ( const vtkFourDAnalysisGUI& ); //Not implemented.

 public:
  //----------------------------------------------------------------
  // New method, Initialization etc.
  //----------------------------------------------------------------

  static vtkFourDAnalysisGUI* New ();
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
  void BuildGUIForActiveBundleSelectorFrame();
  void BuildGUIForFrameControlFrame(int show);
  void BuildGUIForFunctionViewer(int show);
  void BuildGUIForScriptSetting(int show);
  void BuildGUIForCurveFitting(int show);
  void BuildGUIForMapGenerator(int show);

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
  void UpdateMaskSelectMenu();

  void UpdateInitialParameterList(vtkMRMLCurveAnalysisNode* curveNode);
  void GetInitialParametersAndInputCurves(vtkMRMLCurveAnalysisNode* curveNode, int start, int end);
  void UpdateOutputParameterList(vtkMRMLCurveAnalysisNode* curveNode);

  void UpdateIntensityPlot(vtkIntensityCurves* intensityCurves);
  void UpdateIntensityPlotWithFittedCurve(vtkIntensityCurves* intensityCurves, vtkDoubleArray* array);

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

  // -----------------------------------------
  // Intensity Curve

  vtkKWEntry*       AcqTimeEntry;
  vtkKWMenuButton*  SeriesToPlotMenu;
  vtkKWMenuButton*  MaskSelectMenu;
  vtkKWPlotGraph*   IntensityPlot;
  vtkKWCheckButtonWithLabel* ErrorBarCheckButton;

  vtkKWMenuButton* FittingLabelMenu;
  vtkKWLoadSaveButtonWithLabel* CurveScriptSelectButton;
  vtkKWSpinBox*    CurveFittingStartIndexSpinBox;
  vtkKWSpinBox*    CurveFittingEndIndexSpinBox;
  vtkKWPushButton* RunFittingButton;


  vtkKWMultiColumnListWithScrollbars* InitialParameterList;
  vtkKWPushButton* RunPlotButton;
  vtkKWLoadSaveButtonWithLabel* SaveFittedCurveButton;
  vtkKWLoadSaveButtonWithLabel* SavePlotButton;
  vtkKWMultiColumnListWithScrollbars* ResultParameterList;

  // -----------------------------------------
  // Parameter Map
  vtkSlicerNodeSelectorWidget* MapOutputSelector;
  vtkKWEntry*                  MapOutputVolumePrefixEntry;
  //vtkKWMenuButton* MapOutputVolumeMenu;
  vtkKWLoadSaveButtonWithLabel* ScriptSelectButton;
  vtkKWPushButton* RunScriptButton;

  vtkKWSpinBox*    MapIMinSpinBox;
  vtkKWSpinBox*    MapIMaxSpinBox;
  vtkKWSpinBox*    MapJMinSpinBox;
  vtkKWSpinBox*    MapJMaxSpinBox;
  vtkKWSpinBox*    MapKMinSpinBox;
  vtkKWSpinBox*    MapKMaxSpinBox;

  //----------------------------------------------------------------
  // Logic Values
  //----------------------------------------------------------------

  vtkFourDAnalysisLogic *Logic;
  vtkCallbackCommand *DataCallbackCommand;
  int                        CloseScene;

  //vtkFourDImageGUI* FourDImageGUI;

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

  vtkIntensityCurves* IntensityCurves;
  vtkDoubleArray*     FittedCurve;

  vtkCurveAnalysisPythonInterface* CurveAnalysisScript;

};



#endif
