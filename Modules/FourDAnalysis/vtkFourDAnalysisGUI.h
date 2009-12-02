/*==========================================================================

  Portions (c) Copyright 2009 Brigham and Women's Hospital (BWH) All Rights Reserved.

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
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerXYPlotWidget.h"

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
class vtkKWEntryWithLabel;
class vtkIntensityCurves;
class vtkKWCheckButtonWithLabel;
class vtkSlicerNodeSelectorWidget;
class vtkKWMenu;
class vtkKWRadioButtonSetWithLabel;

#ifdef FourDAnalysis_USE_SCIPY
class vtkCurveAnalysisPythonInterface;
#endif //FourDAnalysis_USE_SCIPY


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
  //BTX
  void BuildGUIForHelpFrame();
  void BuildGUIForActiveBundleSelectorFrame();
  void BuildGUIForFrameControlFrame(int show);
  void BuildGUIForFunctionViewer(int show);
#ifdef FourDAnalysis_USE_SCIPY
  void BuildGUIForScriptSetting(int show);
  void BuildGUIForCurveFitting(int show);
  void BuildGUIForMapGenerator(int show);
#endif //FourDAnalysis_USE_SCIPY
  //ETX

  //----------------------------------------------------------------
  // Update routines
  //----------------------------------------------------------------

  void UpdateAll();
  void SelectActive4DBundle(vtkMRMLTimeSeriesBundleNode* bundleNode);

  //----------------------------------------------------------------
  // Viewer control
  //----------------------------------------------------------------

  // -----------------------------------------
  // Frame control
  void SetForeground(const char* bundleID, int index);
  void SetBackground(const char* bundleID, int index);
  void SetWindowLevelForCurrentFrame();

  // -----------------------------------------
  // Plot list
  void UpdatePlotList();
  void UpdatePlotListElement(int row, int col, char * str);
  void DeleteSelectedPlots();
  void SelectAllPlots();
  void DeselectAllPlots();

  // Description:
  // GeneratePlotNodes() calculates time-intensity curves in the regions specified by the label data.
  void GeneratePlotNodes();
  void ImportPlotNode(const char* path);
  void SaveMarkedPlotNode(const char* path);

  // -----------------------------------------
  // Initial parameter list
#ifdef FourDAnalysis_USE_SCIPY
  //BTX
  void UpdateInitialParameterList(vtkMRMLCurveAnalysisNode* curveNode);
  void GetInitialParametersAndInputCurves(vtkMRMLCurveAnalysisNode* curveNode, int start, int end);
  void OnInitialParameterListSelected();
  void ProcPlotSelectPopUpMenu(int row, int col, const char* nodeID);
  void UpdatePlotSelectPopUpMenu(const char* command);
  //ETX
#endif //FourDAnalysis_USE_SCIPY

  // -----------------------------------------
  // Output parameter list
#ifdef FourDAnalysis_USE_SCIPY
  //BTX
  void UpdateOutputParameterList(vtkMRMLCurveAnalysisNode* curveNode);
  void UpdateFittingTargetMenu();
  //ETX
#endif //FourDAnalysis_USE_SCIPY

 protected:
  
  //----------------------------------------------------------------
  // Timer
  //----------------------------------------------------------------
  
  int TimerFlag;
  int TimerInterval;  // ms

  //----------------------------------------------------------------
  // GUI widgets
  //----------------------------------------------------------------

  vtkKWProgressDialog*                ProgressDialog;

  // -----------------------------------------
  // Active 4D Bundle selector
  vtkSlicerNodeSelectorWidget*        Active4DBundleSelectorWidget;

  // -----------------------------------------
  // Frame Control
  vtkKWScaleWithEntry*                ForegroundVolumeSelectorScale;
  vtkKWScaleWithEntry*                BackgroundVolumeSelectorScale;
  vtkKWRange*                         WindowLevelRange;
  vtkKWRange*                         ThresholdRange;

  // -----------------------------------------
  // Intensity Plot
  vtkSlicerNodeSelectorWidget*        MaskNodeSelector;
  vtkKWPushButton*                    GenerateCurveButton;
  vtkKWRadioButtonSetWithLabel*       ValueTypeButtonSet;
  vtkSlicerXYPlotWidget*              IntensityPlot;
  vtkKWCheckButtonWithLabel*          ErrorBarCheckButton;
  vtkKWMultiColumnListWithScrollbars* PlotList;

  vtkKWPushButton*                    ImportPlotButton;
  vtkKWPushButton*                    SelectAllPlotButton;
  vtkKWPushButton*                    DeselectAllPlotButton;
  vtkKWPushButton*                    PlotDeleteButton;
  vtkKWPushButton*                    SavePlotButton;

  // -----------------------------------------
  // Model / Parameters
#ifdef FourDAnalysis_USE_SCIPY
  vtkKWMenuButton*                    FittingTargetMenu;
  vtkKWLoadSaveButtonWithLabel*       CurveScriptSelectButton;
  vtkKWEntryWithLabel*                CurveScriptMethodName;
  vtkKWSpinBox*                       CurveFittingStartIndexSpinBox;
  vtkKWSpinBox*                       CurveFittingEndIndexSpinBox;
  vtkKWMultiColumnListWithScrollbars* InitialParameterList;
  vtkKWMenu*                          PlotSelectPopUpMenu;
#endif //FourDAnalysis_USE_SCIPY

  // -----------------------------------------
  // Curve Fitting
#ifdef FourDAnalysis_USE_SCIPY
  vtkKWPushButton*                    RunFittingButton;
  vtkKWMultiColumnListWithScrollbars* ResultParameterList;
#endif // FourDAnalysis_USE_SCIPY

  // -----------------------------------------
  // Parameter Map
#ifdef FourDAnalysis_USE_SCIPY
  vtkKWRadioButtonSetWithLabel*       ParameterMapRegionButtonSet;
  vtkSlicerNodeSelectorWidget*        MapRegionMaskSelectorWidget;
  vtkKWEntryWithLabel*                MapRegionMaskLabelEntry;
  vtkKWSpinBox*                       MapIMinSpinBox;
  vtkKWSpinBox*                       MapIMaxSpinBox;
  vtkKWSpinBox*                       MapJMinSpinBox;
  vtkKWSpinBox*                       MapJMaxSpinBox;
  vtkKWSpinBox*                       MapKMinSpinBox;
  vtkKWSpinBox*                       MapKMaxSpinBox;
  vtkKWPushButton*                    GenerateMapButton;
  vtkKWEntryWithLabel*                MapOutputVolumePrefixEntry;
#endif // FourDAnalysis_USE_SCIPY


  //----------------------------------------------------------------
  // Logic Values
  //----------------------------------------------------------------

  vtkFourDAnalysisLogic*      Logic;
  vtkCallbackCommand*         DataCallbackCommand;
  int                         CloseScene;

  // -----------------------------------------
  // Window/level adjustment  -- will be integrated in 4D Image
  double  RangeLower;
  double  RangeUpper;

  double  Window;
  double  Level;
  double  ThresholdUpper;
  double  ThresholdLower;

  // -----------------------------------------
  // Curve / plot data
  vtkIntensityCurves*              IntensityCurves;
  vtkMRMLXYPlotManagerNode*        PlotManagerNode;

#ifdef FourDAnalysis_USE_SCIPY
  //BTX
  vtkCurveAnalysisPythonInterface* CurveAnalysisScript;
  //ETX
#endif //FourDAnalysis_USE_SCIPY


  // -----------------------------------------
  // List management
  //BTX
  // Row index for   vtkKWMultiColumnListWithScrollbars* PlotList;
  enum {
    COLUMN_SELECT    = 0,
    COLUMN_VISIBLE   = 1,
    COLUMN_COLOR     = 2,
    COLUMN_NODE_NAME = 3,
    COLUMN_MRML_ID   = 4
  };
  //ETX

  //BTX
  std::vector<int> InitialParameterListInputType;
  std::vector<std::string> InitialParameterListNodeNames;
  enum {
    INPUT_VALUE_INITIALPARAM = 0,
    INPUT_VALUE_CONSTANT,
    INPUT_PLOTNODE,
  };
  std::vector< std::string > FittingTargetMenuNodeList;
  //ETX

};



#endif
